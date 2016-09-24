/*
 * unitTest.c
 *
 *  Created on: 13 Feb 2016
 *      Author: Corentin
 */
#include <msp430.h>
#include <stdio.h>

#include "unitTest.h"
#include "crcTest.h"
#include "m5837Test.h"


void unitTestMain()
{
	if(crcTestMain())
		printf("CRC Test PASSED\n\n");
	else
		printf("CRC Test FAILED\n\n");

	__enable_interrupt();
/*
	if(clocksUnitTestMain())
		printf("Clocks Unit Test PASSED\n\n");
	else
		printf("Clocks Unit Test FAILED\n\n");

	if(crcTestMain())
		printf("CRC Test PASSED\n\n");
	else
		printf("CRC Test FAILED\n\n");

	if(alarmsManagerTestMain())
		printf("AlarmsManager Test PASSED\n\n");
	else
		printf("AlarmsManager Test FAILED\n\n");*/

	/*if(pressureSensorTestMain())
		printf("Pressure Sensor Test PASSED\n");
	else
		printf("Pressure Sensor Test FAILED\n");*/

	/*if(breathingMonitorTestMain())
		printf("BreathingMonitor Test PASSED\n\n");
	else
		printf("BreathingMonitor Test FAILED\n\n");*/



	/*if(buzzerTestMain())
		printf("Buzzer Unit Test PASSED\n\n");
	else
		printf("Buzzer Unit Test FAILED\n\n");*/


	/*if(batteryMonitorTestMain())
		printf("BatteryMonitor Unit Test PASSED\n\n");
	else
		printf("BatteryMonitor Unit Test FAILED\n\n");*/

	if(modeManagerTestMain())
		printf("ModeManager Unit Test PASSED\n\n");
	else
		printf("ModeManager Unit Test FAILED\n\n");

	/*if(piezoButtonTestMain())
		printf("PiezoButtonTest Test PASSED\n\n");
	else
		printf("PiezoButtonTest Test FAILED\n\n");*/

	/*if(faultModeTestMain())
		printf("FaultMode Test PASSED\n\n");
	else
		printf("FaultMode Test FAILED\n\n");*/

	/*if(rtcTestMain())
		printf("RTC Test PASSED\n\n");
	else
		printf("RTC Test FAILED\n\n");*/

	/*if(ms5837_TestMain())
		printf("ms5837_TestMain PASSED\n\n");
	else
		printf("ms5837_TestMain FAILED\n\n");*/





	/*if(flashMainTest())
		printf("Memory Test PASSED\n");
	else
		printf("Memory Test FAILED\n");


	if(usb_hid_test_main())
		printf("USB Test PASSED\n");
	else
		printf("USB Test FAILED\n");*/

	//startFirmwareUpdate();
}



