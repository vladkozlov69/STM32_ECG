#include "Arduino.h"
#include "Biquad.h"
#include <SPI.h>
#include "Ucglib.h"
#include "BeatDetector.h"
#include "SysTickTimer.h"

int samplingFreq = 250;

Biquad *bsFilter = new Biquad();
Biquad *hpFilter = new Biquad();
Biquad *llpFilter = new Biquad();
Biquad *lpFilter = new Biquad();

Biquad * bs = new Biquad(bq_type_notch, 50.0 / samplingFreq, 0.707, 0);

Biquad * bs1 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 0.54119610, 0);
Biquad * bs2 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 1.3065630, 0);

BeatDetector * beatDetector = new BeatDetector(samplingFreq);
//SysTickTimer * sysTickTimer = new SysTickTimer();

int correction_count = 0;
int correction_start;
int correction_end;
int tick250;
int tick500;
int tickCur;

char buf[100];

#define STATUS_LINE_HEIGHT 30



/*
Using the first SPI port (SPI)
SS  <-->PA4
SCK <-->PA5
MISO  <-->PA6
MOSI  <-->PA7

Using the second SPI port (SPI 2)

SPIClass SPI_2(2);

SS  <-->PB12
SCK <-->PB13
MISO  <-->PB14
MOSI  <-->PB15

*/

Ucglib_ILI9341_18x240x320_HWSPI ucg(/*cd=*/ PB6, /*cs=*/ PB8, /*reset=*/ PB7);

int height = 240;
int width = 320;

int posX = 1;
int posY = 0;
int prevY = 0;


void setup()
{
	ucg.begin(UCG_FONT_MODE_SOLID);
	ucg.setRotate90();
	ucg.clearScreen();

	height = ucg.getHeight();
	width = ucg.getWidth();

	ucg.setColor(1, 0, 0, 127);
	ucg.setColor(0, 0, 127);
	ucg.drawBox(0, 0, width, STATUS_LINE_HEIGHT);
	ucg.setFont(ucg_font_ncenR12_hr);

  //sysTickTimer->calibrate(50);

	bsFilter->setBiquad(bq_type_notch, 50.0 / samplingFreq, 0.707, 0);
	hpFilter->setBiquad(bq_type_highpass, 0.3 / samplingFreq, 0.707, 0);
	llpFilter->setBiquad(bq_type_lowpass, 0.3 / samplingFreq, 0.707, 0);
	lpFilter->setBiquad(bq_type_lowpass, 15.0 / samplingFreq, 0.707, 0);

	correction_start = tick250 = tick500 = millis();
}



void loop()
{
	//unsigned long cycleStart = micros();

    int value = analogRead(PA1);
    //filteredValue = bsFilter->process(value);
    //double filteredValue = hpFilter->process(bs1->process(bs2->process(value)));
    //double filteredValue = hpFilter->process(bsFilter->process(value));
    double notchFilteredValue = bs1->process(bs2->process(value));
    double filteredValue = hpFilter->process(notchFilteredValue);
    //posY = height - (int)round(((filteredValue + 2048)/4095)*(height-STATUS_LINE_HEIGHT));

    posY = height/2 - round(((filteredValue)/4095)*(height-STATUS_LINE_HEIGHT));

    tickCur = millis();

    ucg.setClipRange(0, STATUS_LINE_HEIGHT, width, height-STATUS_LINE_HEIGHT);
    ucg.setColor(0,0,0);
    ucg.drawVLine(posX, 0, height);
    ucg.setColor(0,255,0);
    ucg.drawLine(posX-1, prevY, posX, posY);

//    if (tickCur - tick500 > 500)
//    {
//        tick500 = tickCur;
//        tick250 = tickCur;
//        tickCur = 0;
//
//        ucg.setColor(63,63,63);
//        ucg.drawVLine(posX, 0, height);
//    }
//
    if (tickCur - tick250 > 250)
    {
        tick250 = tickCur;
        tickCur = 0;

        ucg.setColor(127,127,127);
        ucg.drawVLine(posX, 0, height);
    }

    prevY = posY;
    posX++;
    if (posX > width)
    {
      posX = 0;

      if (correction_count > 200)
          {
          	correction_end = millis();

          	double quant_Fq = correction_count*1000.0/(correction_end - correction_start);

          	if (abs((samplingFreq - quant_Fq)/samplingFreq) < 0.25)
      		{
      			bsFilter->setFc(50.0/quant_Fq);
      			bs1->setFc(50.0/quant_Fq);
      			bs2->setFc(50.0/quant_Fq);
      			hpFilter->setFc(0.3/quant_Fq);
      		}

          	correction_start = correction_end;
          	correction_count = 0;

          	sprintf(buf, "%d s/s %d bpm %d %d %.2f",(int)round(quant_Fq), beatDetector->getBps(), posX, posY, beatDetector->getBaseline());
          	ucg.setClipRange(0, 0, width, STATUS_LINE_HEIGHT);
          	ucg.setColor(255, 255, 255);
          	ucg.setPrintPos(0,20);
          	ucg.print(buf);
          }
    }

    beatDetector->push(notchFilteredValue);

    correction_count++;


    delay(2);
}


