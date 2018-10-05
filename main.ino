#include "Arduino.h"
#include "Biquad.h"
#include <SPI.h>
#include "Ucglib.h"
#include "BeatDetector.h"

int samplingFreq = 250;
int horizontalScale = 2;

Biquad * bs0 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 0.707, 0);
Biquad * bs1 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 0.54119610, 0);
Biquad * bs2 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 1.3065630, 0);
Biquad * hpFilter = new Biquad(bq_type_highpass, 0.3 / samplingFreq, 0.707, 0);

BeatDetector * beatDetector = new BeatDetector(samplingFreq);

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

Ucglib_ILI9341_18x240x320_HWSPI ucg(PB6, PB8, PB7);

HardwareTimer timer(2);

int height = 240;
int width = 320;

int scaleX = horizontalScale;
int posX = 1;
int posY = 0;
int prevY = 0;

volatile bool hasData = false;
volatile int rawValue = 0;
volatile double notchFilteredValue;
volatile double filteredValue;

void readADC(void)
{
	rawValue = analogRead(PA1);
	notchFilteredValue = bs1->process(bs2->process(rawValue));
	filteredValue = hpFilter->process(notchFilteredValue);

	correction_count++;

	hasData = true;
}

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

	correction_start = tick250 = tick500 = millis();

	timer.pause();
	timer.setPeriod(1000 * 1000 / samplingFreq);
	timer.setCompare(TIMER_CH1, 1);
	timer.attachCompare1Interrupt(readADC);
	timer.refresh();
	timer.resume();
}


void loop()
{
	if (hasData)
	{
		hasData = false;

		posY = height/2 - round(((filteredValue)/4095)*(height-STATUS_LINE_HEIGHT));

		tickCur = millis();

		ucg.setClipRange(0, STATUS_LINE_HEIGHT, width, height-STATUS_LINE_HEIGHT);
		ucg.setColor(0,0,0);
		ucg.drawVLine(posX, 0, height);
		ucg.setColor(0,255,0);
		ucg.drawLine(posX-1, prevY, posX, posY);

		if (tickCur - tick500 > 500)
		{
			tick500 = tick250 = tickCur;
			tickCur = 0;

			ucg.setColor(63,63,63);
			ucg.drawVLine(posX, 0, height);
		}

		if (tickCur - tick250 > 250)
		{
			tick250 = tickCur;
			tickCur = 0;

			ucg.setColor(127,127,127);
			ucg.drawVLine(posX, 0, height);
		}

		scaleX--;

		if (scaleX == 0)
		{
			prevY = posY;
			scaleX = horizontalScale;
			posX++;
		}

		if (posX > width)
		{
		  posX = 0;

		  if (correction_count > samplingFreq)
		  {
			correction_end = millis();
			double quant_Fq = correction_count*1000.0/(correction_end - correction_start);
			correction_start = correction_end;
			correction_count = 0;

			sprintf(buf, "%d s/s %d bpm Y:%d V:%d B:%.2f ",round(quant_Fq), beatDetector->getBps(), posY, rawValue, beatDetector->getBaseline());
			ucg.setClipRange(0, 0, width, STATUS_LINE_HEIGHT);
			ucg.setColor(255, 255, 255);
			ucg.setPrintPos(0,20);
			ucg.print(buf);
		  }
		}

    	beatDetector->push(rawValue);
	}
}


