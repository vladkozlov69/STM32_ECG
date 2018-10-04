/*
 * SysTickTimer.h
 *
 *  Created on: Oct 4, 2018
 *      Author: vkozlov
 */

#ifndef SYSTICKTIMER_H_
#define SYSTICKTIMER_H_


class SysTickTimer
{
	unsigned long ticksPerMcs = 1;
public:
	SysTickTimer();
	void calibrate(const int milliseconds);
	void delay(const int microseconds);
};

#endif /* SYSTICKTIMER_H_ */
