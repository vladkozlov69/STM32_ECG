/*
 * DataRecorder.h
 *
 *  Created on: Oct 5, 2018
 *      Author: vkozlov
 */

#ifndef DATARECORDER_H_
#define DATARECORDER_H_

#include "SD.h"

class DataRecorder
{
	char buf[25];
public:
	DataRecorder();
	bool begin(int csPin);
	bool hasSD();
	void push(const char * data);
	void close();
private:
	bool sdReady = false;
	bool sdOpenedWriting = false;
	File recordingFile;
	char * getNewFileName();
};

#endif /* DATARECORDER_H_ */
