/*
 * ms5837.c
 *
 *  Created on: September 4, 2916
 *      Author: BURC - Corentin
 *
 *  Code for MS5837

  How to use

main()
{

	initMS5837();
	startMS5837();

	while(true)
	{
		// TASK1

		// TASK2

		...

		routinePressureSensor();

		// TASK4


		if(!getPressureSensorPendingAction())
			ENTER_LPM3;	// Sleep mode until next interrupt

	}
}
 */

#include <msp430.h>
#include "ms5837.h"
#include "crc.h"


// Used to transmit and receive data
long pData;
unsigned char pDataIndex;
char pSensorFlag = PFLAG_IDLE;
char d1cmd=0, d2cmd=0;				// CMD for D1,D2 when ordering a new conversion. Determine the ADC resolution.
char startConversionFlag = 0;		// 1: start conversion of MS5837;
char pFlagDataAvailable = 0;		// 1: MS5837 data available;

// Calibration values (C1 to C6)
unsigned short sensorC[6];

// ADC values
unsigned long D1;
unsigned long D2;

// Calculated pressures and temperatures
// Notice they follow datasheet's format
long P;		// 39998 for 3999.8 mbar
long TEMP;	// 1981 for 19.81 �C

// Records the time of last communication with sensor. Generates a timeout is comm is on hold
unsigned short pSensorTime;

// Records the D1/D2 precision to determine the conversion time
char latestDcmd = 0;


// Hardware providing time and timeout functions
// TODO: replace with your own set of functions
#include "clocks.h"

// TODO
// Define here how to access the time in seconds (unsigned short)
// In this example, the time is given by clocks.h and clocks.h, where
// a timer interrupts 20 times per second and increments a counter
unsigned short getTimeSeconds()
{
	// TODO
	// User to define how the hardware counts time
	return 0;
}

// TODO
// This function needs to call for a timeout in the hardware, corresponding
// to the conversion time of the sensor.
void callTimeout(char d1d2cmd)
{
	switch(d1d2cmd & 0x0F)
	{
	case 0:
		// Call for a 0.6 ms timeout
		startTimeout(T0MS60, conversionTimeoutEndedCallback);
		break;
	case 2:
		// Call for a 1.17 ms timeout
		startTimeout(T1MS17, conversionTimeoutEndedCallback);
		break;
	case 4:
		// Call for a 2.28 ms timeout
		startTimeout(T2MS28, conversionTimeoutEndedCallback);
		break;
	case 6:
		// Call for a 4.54 ms timeout
		startTimeout(T4MS54, conversionTimeoutEndedCallback);
		break;
	case 8:
		// Call for a 9.04 ms timeout
		startTimeout(T9MS04, conversionTimeoutEndedCallback);
		break;
	case 0x0A:
		// Call for a 18.08 ms timeout
		startTimeout(T18MS08, conversionTimeoutEndedCallback);
		break;
	}
}



// Interruptions
#pragma vector=MS5837_INT_VECT
__interrupt void USCI_VECTOR_LP_fct(void)
{
	// If Write enabled
	if((MS5837_IFG & UCTXIFG) && (MS5837_IE & UCTXIE))
	{
		if(pDataIndex > 0)
		{
			MS5837_TXBUF = pData;		// Transmit
			pData = pData >> 8;
			pDataIndex--;
		}
		else
		{
			MS5837_IFG &= ~UCTXIFG;		// Clear flag
			MS5837_IE &= ~UCTXIE;		// Disable Interrupt
			MS5837_UCBCTL1 |= UCTXSTP;	// Send a STOP right after the command

			// FROM HERE, END OF COMM
			// The next operation can be executed ONLY when the STOP was sent
			// (i2c line is not BUSY anymore), except for the last step,
			if(pSensorFlag != 0)
				pSensorFlag++;
		}
	}

	// If Read enabled
	if((MS5837_IFG & UCRXIFG) && (MS5837_IE & UCRXIE))
	{
		if(pDataIndex > 0)
		{
			pData = (pData << 8) | MS5837_RXBUF;
			pDataIndex --;

			// CAREFUL HERE: IF ONLY 1 BYTE TO BE RECEIVED = BUG
			if(pDataIndex == 1)
				MS5837_UCBCTL1 |= UCTXSTP;	// Send a STOP
		}

		if(pDataIndex == 0)
		{
			// Disable Read Interrupt
			MS5837_IFG &= ~UCRXIFG;	// Clear flag
			MS5837_IE &= ~UCRXIE;	// Disable Interrupt

			// FROM HERE, END OF COMM
			// The next operation can be executed ONLY when the STOP was sent
			// (i2c line is not BUSY anymore), except for the last step,
			if(pSensorFlag != 0)
				pSensorFlag++;

			// Exit LPM if need to calculate. Calculation is long, it is
			// executed in the routine, not in the interrupt
			if(pSensorFlag == PFLAG_READADC_D2_E)
				LPM3_EXIT;
		}
	}

	LPM3_EXIT;
}

// Only configure the I2C port on the MSP430
// No communication with the sensor is done here
void initMS5837(unsigned short prescaler)
{
	// Reset I2C
	MS5837_UCBCTL1 |= UCSWRST;

	// Disable PORT pins
	MS5837_PSEL |= MS5837_SDABIT | MS5837_SCLBIT;

	// I2C, Single-Master, Master, Sync
	MS5837_UCBCTL0 = UCSYNC | UCMODE_3 | UCMST;

	// SMCLK source
	MS5837_UCBCTL1 |= UCSSEL__SMCLK;

	// Prescaler
	MS5837_UCBBR1 = prescaler >> 8;
	MS5837_UCBBR0 = prescaler;

	// I2C not enabled yet
}

// The function is not optimized as it is called only at power-up
char startMS5837()
{
	char ret;

	// Initialize the flags to idle states
	pSensorFlag = PFLAG_IDLE;
	pSensorTime = getTimeSeconds();

	// Use Timeout
	unsigned short startTime = getTimeSeconds();

	// Enable I2C
	MS5837_UCBCTL1 &= ~UCSWRST;

	MS5837_UCBCSA = PSENS_I2C_ADDRESS;			// Slave (sensor) address

	// Here, unlock the I2C line if locked (slave holding line low)
	ret = unlockI2Cline(200);
	if(ret != RET_SUCCESS)
		return ret;

	// Wait for I2C lines to be available
	while(!isMS5837Available())	if(getTimeSeconds() - startTime > 2) return RET_TIMEOUT;
	reset();	// Send Reset cmd

	__delay_cycles(800000);	// 100 ms

	// Get the calibration values
	while(!isMS5837Available()) if(getTimeSeconds() - startTime > 2) return RET_TIMEOUT;
	if(readCalibrationFactors() == RET_TIMEOUT)	return RET_TIMEOUT;

	return RET_SUCCESS;
}

// Finishes the on-going operations with the sensors before closing-down I2C.
// Long call
char stopMS5837Sensors(unsigned char timeoutSeconds)
{
	unsigned short t = getTimeSeconds();
	while(pSensorFlag!=PFLAG_IDLE || !isMS5837Available())
	{
		if(getTimeSeconds() - t > timeoutSeconds)
			return RET_TIMEOUT;

		routinePressureSensor();
	}

	// Disable I2C
	MS5837_UCBCTL1 |= 0x01;

	return RET_SUCCESS;
}

void routinePressureSensor()
{
	// NOTE: there is no interrupt source when the STOP bit was sent (I2C line is not BUSY anymore).
	// As a result, it must be polled in this routine. This routine must be called regularly enough,
	// to ensure a proper speed of execution.
	// Each sensor follows the process:
	// 1. Send a CMD to convert D1
	// 2. When the CMD was sent (STOP sent), start a timer
	// 3. When timer is over, send a CMD to READ D1
	// 4. When the CMD was sent (STOP sent), start READING D1
	// 5. When READING D1 is over (STOP sent), repeat process for D2

	// D2 was received. The I2C communication may not be finished (STOP not properly sent yet).
	// But the (long) calculation may start anyway.
	// This function is placed on top, such that, immediately after it is executed, the function
	// startNextConversion(i) may be executed if the I2C line is available.

	do
	{
		if(pSensorFlag == PFLAG_READADC_D2_E)
		{
			char result;
			D2 = pData;	// Copy the data to D2
			result = calculateTempPress(CALC_TEMPERATURE, SECONDORDERCOMP);

			// Fire the failure procedure if data not properly received
			// TODO: add tolerance, 2 times in a row for example, before firing failsafe
			if(result == RET_CONVERSIONFAILED)
				pSensorFailureProcedure();

			// Reset to idle start. Pressure and (optional) temperature acquired
			pSensorFlag = PFLAG_IDLE;
		}

		// All operations, except calculateTempPress, must be executed
		// when the I2C line is available (previous STOP was sent). Otherwise
		// it will corrupt the current message being sent (the last byte is being
		// sent, followed by the STOP).
		if(isMS5837Available())
		{
			// Clear the timeout, since the I2C is not BUSY
			pSensorTime = getTimeSeconds();

			switch(pSensorFlag)
			{
				// Start the timer once the conversion CMD was finished sending
				case PFLAG_CONVERTD2_E:
					pSensorFlag++;
					callTimeout(d2cmd);
					break;
				case PFLAG_CONVERTD1_E:
					pSensorFlag++;
					callTimeout(d1cmd);
					break;

				// The CMD_ADC_READ was sent after the timeout
				// Now start READING.
				case PFLAG_GETD2_E:
				case PFLAG_GETD1_E:
					pSensorFlag++;
					readData(3);		// 3 bytes (24-bit) to be read
					break;

				// D1 was completely read. Now start same process with D2
				case PFLAG_READADC_D1_E:
					D1 = pData;	// Copy the data to D1
					pSensorFlag = PFLAG_CONVERTD2;
					sendCMD(d2cmd);
					break;

				// If the sensor is in idle state, check if it is requested to acquire new
				// data. If so, start the conversion.
				case PFLAG_IDLE:
					startNextConversion();
					break;
			}
		}
		else
		{
			// I2C is busy, check that the timeout hasn't expired yet
			if(getTimeSeconds() - pSensorTime > PTIMEOUT)
				pSensorFailureProcedure();
		}
	} while(getPressureSensorPendingAction());	// If new action pending (again), repeat the loop


}

// If a sensor need to start a new conversion, initiate it in the routine,
// because the I2C line must be polled and wait until it is not BUSY anymore.
void startNextConversion()
{
	if(startConversionFlag & BIT0)
	{
		startConversionFlag &= ~BIT0;	// Clear flag

		// Start conversion
		pSensorFlag = PFLAG_CONVERTD1;
		sendCMD(d1cmd);
	}
}

// Sometimes, after a reset, the I2C slave may be holding the line low
// because it is expecting or sending data, from a previous communication.
// Toggle SCL line untill it unlocks
char unlockI2Cline(unsigned short maxLoops)
{
	if(isMS5837Available())
		return RET_SUCCESS;

	// Loop for a max of maxLoops
	do
	{
		// First, de-activate SPI
		MS5837_UCBCTL1 |= 0x01;

		// Return the pins to GPIO and toggle SCL signal once
		MS5837_PSEL &= ~(MS5837_SCLBIT | MS5837_SDABIT);	// Re-select GPIO
		MS5837_POUT &= ~MS5837_SDABIT;					// SDA low
		MS5837_POUT |= MS5837_SCLBIT;					// SCL High
		MS5837_PDIR |= MS5837_SCLBIT | MS5837_SDABIT;	// Both outputs
		MS5837_POUT &= ~MS5837_SCLBIT;					// SCL low (toggled)
		MS5837_POUT |= MS5837_SCLBIT;					// SCL High
		MS5837_POUT &= ~MS5837_SCLBIT;					// SCL low (toggled)
		MS5837_POUT |= MS5837_SCLBIT;					// SCL High

		// Return to I2C
		MS5837_PSEL |= (MS5837_SCLBIT | MS5837_SDABIT);
		MS5837_UCBCTL1 &= ~0x01;

		maxLoops--;
	}while(isMS5837Available()==0 && maxLoops > 0);

	// if the line is still busy, return a timeout
	if(isMS5837Available()==0)
		return RET_TIMEOUT;

	return RET_SUCCESS;
}

void pSensorFailureProcedure()
{
	// TODO: determine what to do in case of a major failure
}


// Initiates the sending process. Returns before all data is sent
void sendCMD(char cmd)
{
	// Log the time the cmd was sent
	pSensorTime = getTimeSeconds();

	pData = cmd;				// 8-bit CMD to be transmitted
	pDataIndex = 1;				// Only 1 byte to be transmitted

	// Enable Write Interrupt
	MS5837_IFG &= ~UCTXIFG;		// Clear flag
	MS5837_IE |= UCTXIE;		// Enable Interrupt

	// Start Sending
	MS5837_UCBCTL1 |= UCTR;		// Transmitter
	MS5837_UCBCTL1 |= UCTXSTT;	// Start bit

}

void readData(char len)
{
	// Log the time the cmd was sent
	pSensorTime = getTimeSeconds();

	pData = 0;					// Data cleared
	pDataIndex = len;			// Length of Data to be received

	// Enable Read Interrupt
	MS5837_IFG &= ~UCRXIFG;		// Clear flag
	MS5837_IE |= UCRXIE;		// Enable Interrupt

	// Start Sending
	MS5837_UCBCTL1 &= ~UCTR;	// Receiver Mode
	MS5837_UCBCTL1 |= UCTXSTT;	// Start bit

}

void reset()
{
	// Log the time the cmd was sent
	pSensorTime = getTimeSeconds();

	sendCMD(CMD_RESET);
}

// Not optimized because called only once
char readCalibrationFactors()
{
	// Use Timeout
	unsigned short startTime = getTimeSeconds();

	unsigned short prom[8];

	// Read the values
	unsigned char i;
	while(!isMS5837Available()) if(getTimeSeconds() - startTime > 5) return RET_TIMEOUT;
	for(i = 0; i < 7; i++)
	{
		sendCMD(0xA0 + (2 * i));
		while(!isMS5837Available()) if(getTimeSeconds() - startTime > 5) return RET_TIMEOUT;
		readData(2);	// Read two bytes of Ai
		while(!isMS5837Available()) if(getTimeSeconds() - startTime > 5) return RET_TIMEOUT;

		prom[i] = pData;
	}

	return convertCalibrationFactors(prom);
}

char convertCalibrationFactors(unsigned short * buff)
{
	unsigned char i;
	unsigned char receivedCrc;

	// Check the CRC before reading the values
	receivedCrc = ((buff[0] >> 8) >> 4) & 0x0F;		// Read received CRC
	if(crc4(buff) != receivedCrc)					// Compare with calculated CRC-4
		return RET_WRONG_CRC;

	// If CRC is correct, read the C values
	for(i = 0; i < 6; i++)
	{
		sensorC[i] = buff[i+1];
	}

	return RET_SUCCESS;
}

// Outputs an array: {calibration pressure, calibration temperature}
// in the format LONG as measured by the pressure sensor, for a faster
// execution (no format conversion or casting to double, float, unsigned, etc.)
// There is a risk of overflow of calibLP if the pressure is high and too many samples
char readCalibrationPressureAndTemperature(long * output, unsigned char nbSamples)
{
	unsigned char i;
	long calibLP = 0, temperature = 0;
	unsigned short startTime = getTimeSeconds();

	// Samples for the calibration - then averages
	while(pSensorFlag != PFLAG_IDLE)
	{
		routinePressureSensor();
		if(getTimeSeconds() - startTime > 2) return RET_TIMEOUT;
	}
	for(i = 0; i < nbSamples; i++)
	{
		startMS5837Acquisition(CMD_D1_2048, CMD_D2_2048);

		while(!isNewDataAvailable())
		{
			routinePressureSensor();
			if(getTimeSeconds() - startTime > 2) return RET_TIMEOUT;
		}

		calibLP += getLatestPressureMeasure();
		temperature += getLatestTemperatureMeasure();
	}

	output[0] = calibLP / nbSamples;
	output[1] = temperature / nbSamples;

	return RET_SUCCESS;
}

// secondOrder is 1 if second order temperature compensation is to be used
// (longer computation time)
char calculateTempPress(char calculateTemperature, char secondOrder)
{
	// If the conversion failed, the value of D1 and/or D2 is 0
	// If the value is over 16777215 (0x00FFFFFF), then error
	if(D1==0 || D2==0 || (D1 & 0xFF000000) || (D2 & 0xFF000000))
		return RET_CONVERSIONFAILED;

	// Calculate temperature
	signed long dT = D2 - ((signed long)sensorC[4] * 256);
	if(dT < -16776960 || dT > 0x00FFFFFF) return RET_CONVERSIONFAILED;

	// Calculate temperature compensated pressure
	signed long long OFF = ((signed long long)sensorC[1] * 65536) +
							(((signed long long)sensorC[3] * (signed long long) dT)/128);
	if(OFF < -17179344900 || OFF > 25769410560)	return RET_CONVERSIONFAILED;

	signed long long SENS = ((signed long long)sensorC[0] * 32768) +
							(((signed long long)sensorC[2] * (signed long long) dT)/256);
	if(SENS < -8589672450 || SENS > 12884705280) return RET_CONVERSIONFAILED;

	signed long long p = (((signed long long)D1 * SENS / 2097152) - OFF) / 8192;
	if(p < 0 || p > 300000) return RET_CONVERSIONFAILED;

	P = p;

	// If temperature is not required, skip the following calulations
	if(calculateTemperature)
	{
		TEMP = 2000 + (dT * (signed long)sensorC[5] / 8388608);
		if(TEMP < -4000 || TEMP > 8500) return RET_CONVERSIONFAILED;

		// Second order temperature compensation
		if(secondOrder)
		{
			long long Ti, OFFi, SENSi;
			if(TEMP/100 < 20)
			{
				// Low temperature case
				long long tpi = (TEMP - 2000);
				tpi *= tpi;

				Ti = 3 * dT * dT / 8589934592;
				OFFi = 3 * tpi / 2;
				SENSi = 5 * tpi / 8;

				if(TEMP/100 < -15)
				{
					// Very low temperature case
					long long tpi2 = (TEMP + 1500);
					tpi2 *= tpi2;

					OFFi += 7 * tpi2;
					SENSi += 4 * tpi2;
				}
			}
			else
			{
				// High temperature case
				Ti = 3 * dT * dT / 137438953472;
				OFFi = (TEMP - 2000);
				OFFi *= OFFi / 16;
				SENSi = 0;
			}

			// Compensation
			OFF = OFF - OFFi;
			SENS = SENS - SENSi;

			TEMP = (TEMP - Ti);
			P = (((D1 * SENS) / 2097152) - OFF) / 8192;
		}
	}

	// Raise flags informing new pressure value available
	pFlagDataAvailable |= BIT0;

	return RET_SUCCESS;
}



char isMS5837Available()
{
	return !(MS5837_STAT & UCBBUSY || MS5837_IE & UCTXIE || MS5837_IE & UCRXIE || MS5837_UCBCTL1 & UCTXSTP);
}

/*
 * This function is a sort of waiting list. It only triggers a flag.
 * When the current conversion (if any) is finished, it will check
 * the status of startConversionFlags. If a new conversion was requested,
 * it will restart the conversion. If not, it does nothing until the flag
 * is set again.
 *
 * IMPORTANT: if a conversion is currently ongoing (pSensorFlag != IDLE) AND
 * the cmdD1 or cmdD2 is different from the currently ongoing conversion, then
 * the function returns error and does not set the flag for a conversion.
 */
char startMS5837Acquisition(char cmdD1, char cmdD2)
{
	if(pSensorFlag != PFLAG_IDLE && (cmdD1!=d1cmd || cmdD2!=d2cmd))
		return RET_FAIL;

	// Record the cmds in order to determine (later)
	// the required conversion time
	d1cmd = cmdD1;
	d2cmd = cmdD2;

	// Set a flag, requesting a conversion
	startConversionFlag |= BIT0;

	return RET_SUCCESS;
}

// TODO
// Function to be called from the hardware timeout
// once the timeout for ADC conversion has expired.
void conversionTimeoutEndedCallback()
{
	// After the waiting period, acquire the ADC value
	pSensorFlag++;

	// Since the timer is start AFTER the I2C conversion CMD is sent
	// (I2C is not BUSY), the next CMD can be sent here withtout using
	// the routine.
	sendCMD(CMD_ADC_READ);
}

// Return true when new data was acquired from the sensor.
// Returns false after either getLatestPressureMeasure or getLatestTemperatureMeasure
// or both functions were called to read the data.
char isNewDataAvailable()
{
	return(pFlagDataAvailable & BIT0);
}
long getLatestPressureMeasure()
{
	// Clear the flags
	pFlagDataAvailable &= ~BIT0;

	// Units: bar
	return P;
}
long getLatestTemperatureMeasure()
{
	// Clear the flags
	pFlagDataAvailable &= ~BIT0;

	// Units: �C
	return TEMP;
}

char getPressureSensorPendingAction()
{
	if(startConversionFlag) return 1;

	if(pSensorFlag == PFLAG_CONVERTD2_E) return 1;
	if(pSensorFlag == PFLAG_CONVERTD1_E) return 1;
	if(pSensorFlag == PFLAG_GETD2_E) return 1;
	if(pSensorFlag == PFLAG_GETD1_E) return 1;
	if(pSensorFlag == PFLAG_READADC_D1_E) return 1;

	return 0;
}
