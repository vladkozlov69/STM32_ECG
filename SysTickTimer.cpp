/*
 * SysTickTimer.cpp
 *
 *  Created on: Oct 4, 2018
 *      Author: vkozlov
 */

#include "SysTickTimer.h"
#include <libmaple/systick.h>
#include "Arduino.h"


SysTickTimer::SysTickTimer()
{
	// TODO Auto-generated constructor stub

}

void SysTickTimer::calibrate(const int milliseconds)
{
	uint32 begin = systick_get_count();

	delay(milliseconds);

	uint32 end = systick_get_count();

	uint32 ticksPerMcs = (end - begin)/(milliseconds * 1000);
}

void SysTickTimer::delay(const int microseconds)
{
	for (int i = 0; i < microseconds * 72; i++);
}

