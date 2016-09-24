/*
 * crc.c
 *
 *  Created on: 4 Kas 2014
 *      Author: BURC - Corentin
 */

#include "crc.h"

//********************************************************
//! @brief calculate the CRC code for details look into CRC CODE NOTES
//!
//! @return crc code
//********************************************************
unsigned char crc4(unsigned short n_prom[])  // n_prom defined as 8x unsigned int (n_prom[8])
{
	int cnt; // simple counter
	unsigned int n_rem=0; 				// crc remainder
	unsigned char n_bit;
	n_prom[0]=((n_prom[0]) & 0x0FFF); 	// CRC byte is replaced by 0
	n_prom[7]=0;  						// Subsidiary value, set to 0
	for (cnt = 0; cnt < 16; cnt++) 		// operation is performed on bytes
	{ // choose LSB or MSB
		if (cnt%2==1) n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
		else n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000))  n_rem = (n_rem << 1) ^ 0x3000;
			else  n_rem = (n_rem << 1);
		}
	}
	n_rem= ((n_rem >> 12) & 0x000F); 	// final 4-bit remainder is CRC code
	return (n_rem ^ 0x00);
}

