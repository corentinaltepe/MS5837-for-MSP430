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
	// First verify the CRC-4 code works well
	if(crcTestMain())
		printf("CRC Test PASSED\n\n");
	else
		printf("CRC Test FAILED\n\n");

	// Then run the unit tests on the pressure sensor
	if(MS5837TestMain())
		printf("MS5837 Test PASSED\n\n");
	else
		printf("MS5837 Test FAILED\n\n");

}



