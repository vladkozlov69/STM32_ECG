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

	return sdReady;
}

bool DataRecorder::hasSD()
{
	return sdReady;
}

void DataRecorder::push(double value)
{
	m_Data[m_SamplePointer] = value;
	m_SamplePointer = (m_SamplePointer + 1) % m_SamplingFreq;

	if (m_SamplePointer == 0)
	{
		// flushing full row of samples
		if (sdReady)
		{
			if (!sdOpenedWriting)
			{
				recordingFile = SD.open(getNewFileName(), FILE_WRITE);
				sdOpenedWriting = true;
			}

			DateTime now = m_Rtc->now();
			sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
					now.year(),
					now.month(),
					now.day(),
					now.hour(),
					now.minute(),
					now.second());

			recordingFile.print(buf);

			for (int i = 0; i < m_SamplingFreq; i++)
			{
				recordingFile.print(",");
				recordingFile.print(m_Data[i], 3);
			}

			recordingFile.println();
		}
	}
}

char * DataRecorder::getNewFileName()
{
	DateTime now = m_Rtc->now();
	sprintf(buf, "%04d%02d%02d.csv", now.year(), now.month(), now.day());
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

