/*
 * ms5837.h
 *
 *  Created on: September 4, 2916
 *      Author: BURC - Corentin
 */

#ifndef MS5837_H_
#define MS5837_H_

// PIN MAPPING - Define the port which the pressure sensor is connected to
#define MS5837_UCBCTL0			UCB0CTL0
#define MS5837_UCBCTL1			UCB0CTL1
#define MS5837_UCBBR1			UCB0BR1
#define MS5837_UCBBR0			UCB0BR0
#define MS5837_POUT				P3OUT
#define MS5837_PDIR				P3DIR
#define MS5837_PSEL				P3SEL
#define MS5837_SDABIT			BIT1
#define MS5837_SCLBIT			BIT2
#define MS5837_INT_VECT			USCI_B0_VECTOR
#define MS5837_UCBCSA			UCB0I2CSA
#define MS5837_STAT				UCB0STAT
#define MS5837_IE				UCB0IE
#define MS5837_IFG				UCB0IFG
#define MS5837_TXBUF			UCB0TXBUF
#define MS5837_RXBUF			UCB0RXBUF

// Configuration
#define CALC_TEMPERATURE		1				// If cleared, the temperature and second order compensation are skipped
												// Saves on calculation time but loses accuracy on pressure measures.
#define SECONDORDERCOMP			1				// If set, use second order temperature compensation. Gives better accuracy,
												// especially at 'extreme' temperatures but nearly doubles calculation time.
												// From the test in lab (temperature: 21.12°C, pressure 1010.8 mbar),
												// Second Order takes 16.1 ms to compute
												// First  Order takes  8.4 ms to compute
#define PTIMEOUT				3				// Time in seconds before the sensor communication is reset


// I2C Line configuration
#define PSENS_I2C_ADDRESS		0x76			// I2C address of the MS5837 (see sensor's datasheet)

// Sensor Commands
#define	CMD_RESET				0x1E
#define CMD_D1_256				0x40			// Max conversion time: 0.60 ms ; 1.57 mbar RMS resolution
#define CMD_D1_512				0x42			// Max conversion time: 1.17 ms
#define CMD_D1_1024				0x44			// Max conversion time: 2.28 ms
#define CMD_D1_2048				0x46			// Max conversion time: 4.54 ms ; 0.38 mbar RMS resolution
#define CMD_D1_4096				0x48			// Max conversion time: 9.04 ms
#define CMD_D1_8192				0x4A			// Max conversion time: 18.08 ms
#define CMD_D2_256				0x50			// Max conversion time: 0.60 ms ; 0.0086 °C RMS resolution
#define CMD_D2_512				0x52			// Max conversion time: 1.17 ms
#define CMD_D2_1024				0x54			// Max conversion time: 2.28 ms
#define CMD_D2_2048				0x56			// Max conversion time: 4.54 ms ; 0.0033 °C RMS resolution
#define CMD_D2_4096				0x58			// Max conversion time: 9.04 ms
#define CMD_D2_8192				0x5A			// Max conversion time: 18.08 ms
#define	CMD_ADC_READ			0x00
#define CMD_PROM_READ			0xA0

// Flags and Statuses
#define PFLAG_IDLE				0		// Nothing to do, sensor is idle

#define PFLAG_CONVERTD1			1		// Order given to convert D1, send cmd to sensor
#define PFLAG_CONVERTD1_E		2		// The order was executed, waiting for STOP to be sent
#define PFLAG_STARTTIMERD1		3		// Start the timer once the STOP was sent, and not before! (in routine, by polling)
#define PFLAG_GETD1				4		// Send cmd to sensor to read D1
#define PFLAG_GETD1_E			5
#define PFLAG_READADC_D1		6		// Start the reading process
#define PFLAG_READADC_D1_E		7

#define PFLAG_CONVERTD2			11		// Order given to convert D2, send cmd to sensor
#define PFLAG_CONVERTD2_E		12		// The order was executed, waiting for STOP to be sent
#define PFLAG_STARTTIMERD2		13		// Start the timer once the STOP was sent, and not before! (in routine, by polling)
#define PFLAG_GETD2				14		// Send cmd to sensor to read D2
#define PFLAG_GETD2_E			15
#define PFLAG_READADC_D2		16		// Start the reading process
#define PFLAG_READADC_D2_E		17

// FUNCTION RETURN VALUES
#define RET_SUCCESS				0
#define RET_FAIL				1
#define RET_FAIL_CALIB			4
#define RET_TIMEOUT				5
#define RET_WRONG_PASSWORD		8
#define RET_WRONG_CRC			9
#define RET_CONVERSIONFAILED	10

void initMS5837(unsigned short prescaler);
char startMS5837();
char stopMS5837Sensors(unsigned char timeoutSeconds);
char unlockI2Cline(unsigned short maxLoops);
void routinePressureSensor();
void startNextConversion();
void pSensorFailureProcedure();

void reset();
char readCalibrationFactors();
char readCalibrationLP(long * output, unsigned char nbSamples);	// Returns {pressure(bar), temperature(°C)}
char convertCalibrationFactors(unsigned short * prom);

char calculateTempPress(char calculateTemperature, char secondOrder);

char isMS5837Available();
char startMS5837Acquisition(char cmdD1, char cmdD2);

void sendCMD(char cmd);
void readData(char len);

void conversionTimeoutEndedCallback();

char isNewDataAvailable();
long getLatestPressureMeasure();
long getLatestTemperatureMeasure();
long getCalibrationLP();
long getCalibrationTemperature();
char getPressureSensorPendingAction();

unsigned short getTimeSeconds();
void callTimeout(char d1d2cmd);

#endif /* MS5837_H_ */
