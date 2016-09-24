/*
 * clocks.h
 *
 *  Created on: 30 Haz 2014
 *      Author: BURC - Corentin
 */

#ifndef CLOCKS_H_
#define CLOCKS_H_

#define T0MS60		1
#define T1MS17		2
#define T2MS28		3
#define	T4MS54		4
#define T9MS04		5
#define T18MS08		6

void initClockSystem(unsigned long freq);

double getCurrentTime();
unsigned short getCurrentSeconds();

void startTimer();
void interruptTimer();
void endTimer();

void startTimeout(char timeout, void(*callback)(void));
void timeoutExpired();

void turnOnTimerB0();
void turnOffTimerB0();

#endif /* CLOCKS_H_ */
