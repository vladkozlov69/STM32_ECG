/*
 * DataRecorder.cpp
 *
 *  Created on: Oct 5, 2018
 *      Author: vkozlov
 */

#include "DataRecorder.h"

DataRecorder::DataRecorder()
{

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
	if (sdReady)
	{
		if (!sdOpenedWriting)
		{
			recordingFile = SD.open(getNewFileName(), FILE_WRITE);
		}

		recordingFile.print(data);
	}
}

char * DataRecorder::getNewFileName()
{
	sprintf(buf, "%d.txt", (int)millis());

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

