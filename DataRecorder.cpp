/*
 * DataRecorder.cpp
 *
 *  Created on: Oct 5, 2018
 *      Author: vkozlov
 */

#include "DataRecorder.h"

DataRecorder::DataRecorder()
{
	// TODO Auto-generated constructor stub
}

bool DataRecorder::begin(int csPin)
{
	sdReady = SD.begin(csPin);

	return sdReady;
}

bool DataRecorder::hasSD()
{
	return sdReady;
}

void DataRecorder::push(const char * data)
{

}

void DataRecorder::close()
{

}

