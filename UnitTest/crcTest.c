/*
 * crcTest.c
 *
 *  Created on: 19 Feb 2016
 *      Author: Corentin
 */
#include <msp430.h>
#include <stdio.h>

#include "crcTest.h"

#include "crc.h"

char crcTestMain()
{
	// Check CRC-4 function
	unsigned short nprom[] = {0x3001,0x81A1,0x85CC,0x4DB1,0x5504,0x68CD,0x6887, 0x0000};
	if(crc4(nprom) != 0x03)
		return 0;
	return 1;
}

