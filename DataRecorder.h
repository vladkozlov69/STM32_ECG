/*
 * DataRecorder.h
 *
 *  Created on: Oct 5, 2018
 *      Author: vkozlov
 */

#ifndef DATARECORDER_H_
#define DATARECORDER_H_

#include "SD.h"
#include "RTClib.h"

class DataRecorder
{
	double * m_Data;
	int m_SamplingFreq;
	int m_SampleCount = 0;
	RTC_DS3231 * m_Rtc;
	char currentFolder[14];
	char buf[30];
public:
	DataRecorder(int samplingFreq, RTC_DS3231 * rtc);
	bool begin(int csPin);
	bool hasSD();
	void push(double value);
	void close();
	bool isActive();
private:
	bool sdReady = false;
	bool sdOpenedWriting = false;
	File recordingFile;
	char * getNewFileName();
	void prepareCurrentFolder();
};

#endif /* DATARECORDER_H_ */
