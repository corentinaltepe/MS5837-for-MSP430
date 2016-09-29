/*
 * clocks.c fff
 *
 *  Created on: 30 Haz 2014
 *      Author: BURC - Corentin
 */

#include <msp430.h>

#include "clocks.h"
#include "pmm.h"
#include "ucs.h"

extern unsigned char flagTimers = 0;

// Used for storing TBOR value at SecondCounter increments (counting time at ms precision)
extern unsigned short tbo = 0;
extern unsigned short secondCounter = 0;
unsigned char timerInterruptCounter = 0;
void(*timeoutCallback)(void);

// Timer B0
// CCTL0:
// CCTL1:
// CCTL2:
// CCTL3:
// CCTL4: interrupt at 20Hz to trigger Pressure Sensor Conversion and keep track of time
// CCTL5: 10 ms timer on SENSOR_LP
// CCTL6: 10 ms timer on SENSOR_IP

// TimerB0 Interruptions - Used multipurpose
// Do not leave LPMn
#pragma vector=TIMER0_B1_VECTOR
__interrupt void timer0_b1_vector(void)
{
	if(TB0CCTL4 & BIT0)
	{
		interruptTimer();

		/* Exit the Low Power Mode */
		LPM3_EXIT;
	}
	if((TB0CCTL5 & CCIE) && (TB0CCTL5 & BIT0))
	{
		timeoutExpired();

		/* Exit the Low Power Mode */
		LPM3_EXIT;
	}
}

void initClockSystem(unsigned long freq)
{
	// MSP430 USB requires the maximum Vcore setting; do not modify
	PMM_setVCore(PMM_CORE_LEVEL_2);

	// Select XT1 crystal
	P5SEL |= BIT4 | BIT5;

	/* Configures the system clocks:
	 * MCLK = SMCLK = DCO/FLL = 8000000 (expected to be expressed in Hz)
	 * ACLK = FLLref = REFO=32kHz
	 *
	 * XT2 is not configured here.  Instead, the USB API automatically starts XT2
	 * when beginning USB communication, and optionally disables it during USB
	 * suspend.  It's left running after the USB host is disconnected, at which
	 * point you're free to disable it.  You need to configure the XT2 frequency
	 * in the Descriptor Tool (currently set to 4MHz in this example).
	 * See the Programmer's Guide for more information.
	 */
    UCS_clockSignalInit(
            UCS_FLLREF,
            UCS_REFOCLK_SELECT,
            UCS_CLOCK_DIVIDER_1);

    UCS_clockSignalInit(
            UCS_ACLK,
            UCS_REFOCLK_SELECT,
            UCS_CLOCK_DIVIDER_1);

    UCS_initFLLSettle(
    		freq / 1000,
			freq / 32768);
}

double getCurrentTime()
{
	// An secondCounter interrupt can occur during this function call
	// Therefore, put the secondCounter and TBOR value aside
	unsigned secondCounterTmp = secondCounter;
	unsigned short tboTmp = tbo;
	unsigned short TB0Rtmp = TB0R;

	unsigned short tboDiff = TB0Rtmp - tboTmp;
	double mstime = (double)tboDiff / 16384.0;

	return ((double)secondCounterTmp + mstime);
}

unsigned short getCurrentSeconds()
{
	return secondCounter;
}


void startTimer()
{
	// The timer is configured to interrupt 20 times per second
	secondCounter = 0;

	// Turn on TimerB0 if not already in use
	if(!flagTimers) turnOnTimerB0();
	flagTimers |= BIT4;

	TB0R	= 0;						// Reset the value of the counter
	TB0CCTL4 = 0x0010;					// Interrupt enabled on CCTL4
	TB0CCR4 = TB0R + 819;				// Next interrupt in 1/20 sec
}

void interruptTimer()
{
	// Clear the flag
	TB0CCTL4 &= ~BIT0;

	// Program next interruption in 1/20 sec
	timerInterruptCounter++;

	if(timerInterruptCounter == 20)
		tbo = TB0R;

	if(timerInterruptCounter%5)
		TB0CCR4 += 819;
	else
		TB0CCR4 += 820;

	// Reset counter every 20 interrupts (1 sec)
	if(timerInterruptCounter == 20)
	{
		// New second!
		// A new pressure sensor value could be acquired
		// calling here startSensorAcquisition()

		timerInterruptCounter = 0;

		secondCounter++;
	}
}

void endTimer()
{
	TB0CCTL4 &= ~0x0010;	// Interrupt disabled

	// Clear the flag
	TB0CCTL4 &= ~BIT0;

	// Turn Off the timer B0 if not used
	flagTimers &= ~BIT4;
	if(!flagTimers)	turnOffTimerB0();
}

// Timeout: 1 for 0.6ms, 2 for 1.17ms, 3 for 2.28ms, 4 for 4.54ms, 5 for 9.04ms, 6 for 18.08ms
// callback is the function to be called once the timeout has expired
void startTimeout(char timeout, void(*callback)(void))
{
	// Turn on TimerB0 if not already in use
	if(!flagTimers) turnOnTimerB0();
	flagTimers |= BIT5;

	// Calculate the value to add to TB0R for the given time in ms
	unsigned short addedCount = 10;
	switch(timeout)
	{
	case T0MS60: // 0.6 ms
		addedCount = 10;
		break;
	case T1MS17: // 1.17 ms
		addedCount = 20;
		break;
	case T2MS28: // 2.28 ms
		addedCount = 38;
		break;
	case T4MS54: // 4.54 ms
		addedCount = 76;
		break;
	case T9MS04: // 9.04 ms
		addedCount = 150;
		break;
	case T18MS08: // 18.08 ms
		addedCount = 299;
		break;
	}

	TB0CCTL5 = CCIE;					// Interrupt enabled on CCTL
	TB0CCR5 = TB0R + addedCount;		// Next interrupt in X ms

	// Store in memory the function callback
	// Function to be called after the timeout expires
	timeoutCallback = callback;
}

// Function called once the timeout has expired
void timeoutExpired()
{
	TB0CCTL5 &= ~0x0010;	// Interrupt disabled

	// Clear the flag
	TB0CCTL5 &= ~BIT0;

	// Call the callback function (in MS5837)
	timeoutCallback();

	// Turn Off the timer B0 if not used
	flagTimers &= ~BIT5;
	if(!flagTimers)	turnOffTimerB0();
}



void turnOnTimerB0()
{
	// TimerA2 is used multipurpose
	TB0CTL = 0x0160; 	// ACLK source, /2, Up-Mode, TBIE cleared
}

void turnOffTimerB0()
{
	if(!flagTimers)
		TB0CTL &= ~0x0030;
}

