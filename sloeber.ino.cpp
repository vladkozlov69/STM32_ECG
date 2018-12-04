#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2018-12-04 14:55:15

#include "Arduino.h"
#include "Arduino.h"
#include "Biquad.h"
#include <SPI.h>
#include "Ucglib.h"
#include "BeatDetector.h"
#include "DataRecorder.h"
#include "RTCLib.h"
#include "ClickEncoder.h"
#include <menu.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/serialIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/UCGLibOut.h>

void readADC(void) ;
void timerEncIsr() ;
result doSave() ;
result doUpdateRtc() ;
void setup() ;
void displayStatus(const char * status, bool clear) ;
void loop() ;


#include "main.ino"

#endif
