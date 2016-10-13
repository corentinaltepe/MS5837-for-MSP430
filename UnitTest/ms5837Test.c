/*
 * pressureSensorTest.c
 *
 *  Created on: 19 Feb 2016
 *      Author: Corentin
 */

#include <msp430.h>
#include <stdio.h>

#include "../clocks.h"
#include "../ms5837.h"
#include "m5837Test.h"

extern char pSensorFlag[];
/*extern char flagIPstatus;
extern char flagLPstatus;
extern double tempLP, pressLP, pressIP;*/

// Note: for this test to run properly,
// clocks must be running.
char MS5837TestMain()
{
	// Start the clocks
	initClockSystem(4000000);	// 4 MHz
	__delay_cycles(1000000);
	startTimer();

	// Initialize I2C
	initMS5837(0x1000);
	//P2DIR |= BIT6;
	//P2SEL |= BIT6;
	//P4DIR |= BIT7;
	//P4SEL |= BIT7;

	__enable_interrupt();

	// Enable the regulator to power the pressure sensor
/*	P1OUT &= ~BIT4;
	P1DIR |= BIT4;
	__delay_cycles(2000000);
	P1OUT |= BIT4;
	__delay_cycles(1000000);
*/

	if(MS5837TestInitAndCalibration() == 0)
		return 0;

	if(MS5837TestAcquisition() == 0)
		return 0;

	if(MS5837TestSdtDev() == 0)
			return 0;

	/*if(MS5837PerfTest() == 0)
		return 0;

	if(MS5837I2CTest() == 0)
		return 0;*/

	return 1;
}

char MS5837TestInitAndCalibration()
{
	// Test the Start routine and measure time
	if(startMS5837() > 0)
		return 0;	// Error (timeout or other)

	// Read 10 times the temperature and pressure and then return
	// the average of the measures
	// Long call, only use this function occasionally
	long averages[2];
	if(readCalibrationPressureAndTemperature(averages, 10) != RET_SUCCESS)
		return 0;

	// Measured values
	printf("Pressure: %ld\n", averages[0]);
	printf("Temperature: %ld\n\n", averages[1]);

	return 1;
}

char MS5837TestAcquisition()
{
	double time = 0.0;
	unsigned short i = 10;	// Number of loops

	do
	{
		// Wait for sensor to be ready - Timeout of 5 seconds
		while(!isMS5837Available() || getPSensorFlag()!=PFLAG_IDLE)
		{
			if(getCurrentTime() - time > 5.0)
				return 0;
			routinePressureSensor();
		}

		time = getCurrentTime();

		// Order sensor acquisition with maximum resolution (18 ms)
		startMS5837Acquisition(CMD_D1_8192, CMD_D2_8192);

		while(!isNewDataAvailable())
		{
			routinePressureSensor();	// Need to run the routine to acquire values

			// Timeout of 5 seconds
			if(getCurrentTime() - time > 5.0)
				return 0;
		}

		// Read the values
		long pressLP = getLatestPressureMeasure();
		long tempLP = getLatestTemperatureMeasure();

		// Measured values
		printf("Pressure: %ld\n", pressLP);
		printf("Temperature: %ld\n", tempLP);

		if(pressLP > 11000 || pressLP < 9500 || tempLP < 1000 || tempLP > 3000)
			return 0;	// Wrong values

		i--;

	}while(i > 0);

	return 1;
}
char MS5837TestSdtDev()
{
		unsigned short i = 1000;	// Number of loops
		printf("Start of Data\n");
		do
		{
			// Wait for sensor to be ready - Timeout of 5 seconds
			while(!isMS5837Available() || getPSensorFlag()!=PFLAG_IDLE)
			{
				routinePressureSensor();
			}
			getLatestPressureMeasure();

			// Order sensor acquisition with maximum resolution (18 ms)
			startMS5837Acquisition(CMD_D1_8192, CMD_D2_8192);

			while(!isNewDataAvailable())
			{
				routinePressureSensor();	// Need to run the routine to acquire values
			}

			// Read the values
			long pressLP = getLatestPressureMeasure();
			long tempLP = getLatestTemperatureMeasure();

			// Measured values
			printf("%ld; %ld\n", pressLP, tempLP);

			if(pressLP > 11000 || pressLP < 9500 || tempLP < 1000 || tempLP > 3000)
				return 0;	// Wrong values

			i--;

		}while(i > 0);

		printf("End of Data\n");
		return 1;
}


char MS5837PerfTest()
{
	// 1: over 1000 acquisitions of IP, show calculation time
	// difference between compensated and uncompensated pressure.

	// Assume Sensor is started and runs properly
	/*startMS5837Acquisition(SENSOR_IP);
	while(!isNewDataAvailable(SENSOR_IP)) routinePressureSensor();

	// D1, and D2 are acquired for SENSOR_IP
	double startTime = getCurrentTime();
	unsigned short i;
	for(i = 0; i < 100; i++)
	{
		// Use second order, first
		calculateTempPress(SENSOR_IP, 1, 1);
	}
	double secondOrderTime = getCurrentTime() - startTime;

	// Display results for Second Order
	long temperature = getLatestTemperatureMeasure(SENSOR_IP);
	long pressure = getLatestPressureMeasure(SENSOR_IP);
	printf("Second Order\n");
	printf("Time: %f s\n", secondOrderTime);
	printf("Temperature: %ld C\n", temperature);
	printf("Pressure: %ld C\n\n", pressure);

	// Do the same without second order
	startTime = getCurrentTime();
	for(i = 0; i < 100; i++)
	{
		// Use second order, first
		calculateTempPress(SENSOR_IP, 1, 0);
	}
	secondOrderTime = getCurrentTime() - startTime;

	// Display results for Second Order
	temperature = getLatestTemperatureMeasure(SENSOR_IP);
	pressure = getLatestPressureMeasure(SENSOR_IP);
	printf("First Order\n");
	printf("Time: %f s\n", secondOrderTime);
	printf("Temperature: %ld C\n", temperature);
	printf("Pressure: %ld C\n\n", pressure);


	// 2: check how long it takes to acquire 100 x SENSOR_IP (overall)
	// Compare to SENSOR_LP
	startTime = getCurrentTime();
	for(i = 0; i < 100; i++)
	{
		startMS5837Acquisition(SENSOR_IP);
		while(!isNewDataAvailable(SENSOR_IP)) routinePressureSensor();
		getLatestPressureMeasure(SENSOR_IP);	// Clears flag
	}
	secondOrderTime = getCurrentTime() - startTime;
	printf("SENSOR_IP acquisition: %f s\n", secondOrderTime);

	startTime = getCurrentTime();
	for(i = 0; i < 100; i++)
	{
		startMS5837Acquisition(SENSOR_LP);
		while(!isNewDataAvailable(SENSOR_LP)) routinePressureSensor();
		getLatestPressureMeasure(SENSOR_LP);	// Clears flag
	}
	secondOrderTime = getCurrentTime() - startTime;
	printf("SENSOR_LP acquisition: %f s\n", secondOrderTime);
*/
	return 1;
}

char MS5837I2CTest()
{
	unsigned short i2cprescaler = 20;

	if(stopMS5837Sensors(2) != RET_SUCCESS)
		return 0;

	while(i2cprescaler > 8)
	{
		initMS5837(i2cprescaler);
		if(startMS5837() != 0)
			return 0;	// Error (timeout or other)

		i2cprescaler--;
	}

	return 1;
}
