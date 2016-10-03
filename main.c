#include <msp430.h> 

#include "unitTest.h"
/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Start the main UnitTest function
    unitTestMain();
	

    while(1);
}
