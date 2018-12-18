/*
 * DataRecorder.cpp
 *
 *  Created on: Oct 5, 2018
 *      Author: vkozlov
 */

#include "DataRecorder.h"

DataRecorder::DataRecorder(int samplingFreq, RTC_DS3231 * rtc)
{
	this->m_Rtc = rtc;
	this->m_SamplingFreq = samplingFreq;
	this->m_Data = new double[samplingFreq];
}

bool DataRecorder::begin(int csPin)
{
	sdReady = SD.begin(csPin);

	if (sdReady)
	{
		SD.mkdir("/ECG");
	}

	return sdReady;
}

bool DataRecorder::hasSD()
{
	return sdReady;
}

bool DataRecorder::isActive()
{
	return sdOpenedWriting;
}

void DataRecorder::push(double value)
{
	if (sdReady)
	{
		if (!sdOpenedWriting)
		{
			prepareCurrentFolder();
			getNewFileName();
			recordingFile = SD.open(buf, O_CREAT | O_WRITE);
			sdOpenedWriting = true;
			m_SampleCount = 0;
		}

//		sprintf(buf, "%ld,%lu,", round(value*1000), millis());
		sprintf(buf, "%ld,", round(value*1000));
		recordingFile.print(buf);

		m_SampleCount = (m_SampleCount + 1) % m_SamplingFreq;

		if (m_SampleCount == 0)
		{
			DateTime now = m_Rtc->now();
			sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d,%lu",
					now.year(),
					now.month(),
					now.day(),
					now.hour(),
					now.minute(),
					now.second(),
					micros());

			recordingFile.println(buf);
			recordingFile.flush();
		}
	}
}

void DataRecorder::prepareCurrentFolder()
{
	DateTime now = m_Rtc->now();
	sprintf(currentFolder, "/ECG/%04d%02d%02d", now.year(), now.month(), now.day());
	SD.mkdir(currentFolder);
}

char * DataRecorder::getNewFileName()
{
	DateTime now = m_Rtc->now();
	sprintf(buf, "%s/%02d%02d%02d.csv", currentFolder, now.hour(), now.minute(), now.second());
	return buf;
}

void DataRecorder::close()
{
	if (sdReady && sdOpenedWriting)
	{
		recordingFile.close();
		sdOpenedWriting = false;
	}
}

