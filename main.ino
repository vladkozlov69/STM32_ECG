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



int samplingFreq = 250;
int horizontalScale = 2;

Biquad * bs0 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 0.707, 0);
Biquad * bs1 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 0.54119610, 0);
Biquad * bs2 = new Biquad(bq_type_notch, 50.0 / samplingFreq, 1.3065630, 0);
Biquad * hpFilter = new Biquad(bq_type_highpass, 0.3 / samplingFreq, 0.707, 0);

BeatDetector * beatDetector = new BeatDetector(samplingFreq);

RTC_DS3231 rtc;

DataRecorder * dataRecorder = new DataRecorder(samplingFreq, &rtc);



int correction_count = 0;
unsigned long correction_start;
unsigned long correction_end;
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

// display
#define SPI_DC  PA11
#define SPI_RST PA12
#define SPI_CS  PB5

// sd
#define SD_CS PA4



Ucglib_ILI9341_18x240x320_HWSPI ucg(SPI_DC, SPI_CS, SPI_RST);

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

unsigned long lastConnect;
unsigned int disconnectTimeout = 10;

#define ECG_LO1 PB10
#define ECG_LO2 PB11

bool hasSD = false;

void readADC(void)
{
	rawValue = analogRead(PA1);
	notchFilteredValue = bs1->process(bs2->process(rawValue));
	filteredValue = hpFilter->process(notchFilteredValue);

	correction_count++;

	hasData = true;
}


//================ MENU DEFINITION ==================
#define ENC_A PB12
#define ENC_B PB14
#define ENC_BTN PB13

ClickEncoder encoder(ENC_A, ENC_B, ENC_BTN, 2);
ClickEncoderStream encStream(encoder,1);

HardwareTimer timerEnc(4);

void timerEncIsr()
{
  encoder.service();
}

int autoRecord=1;
int hZoom = 2;
int m_Year;
int m_Month;
int m_Day;
int m_Hour;
int m_Minute;

result doSave()
{
	return proceed;
}

result doUpdateRtc()
{
	rtc.adjust(DateTime(m_Year, m_Month, m_Day, m_Hour, m_Minute, 0));

	return proceed;
}

TOGGLE(autoRecord,setAutoRecord,"Auto record: ",doNothing,noEvent,noStyle
  ,VALUE("On",1,doNothing,noEvent)
  ,VALUE("Off",0,doNothing,noEvent)
);

TOGGLE(hZoom,setHZoom,"hZoom: ",doNothing,noEvent,noStyle
  ,VALUE("1",1,doNothing,noEvent)
  ,VALUE("2",2,doNothing,noEvent)
);

altMENU(menu,time,"Time",doUpdateRtc,exitEvent,noStyle,(systemStyles)(_asPad|Menu::_menuData|Menu::_canNav|_parentDraw)
		,FIELD(m_Day,"","-",1,31,1,0,doNothing,noEvent,noStyle)
		,FIELD(m_Month,"","-",1,12,1,0,doNothing,noEvent,noStyle)
		,FIELD(m_Year,""," ",2010,2025,1,0,doNothing,noEvent,noStyle)
		,FIELD(m_Hour,"",":",0,23,1,0,doNothing,noEvent,noStyle)
		,FIELD(m_Minute,"","",0,59,1,0,doNothing,noEvent,wrapStyle)
);

MENU(mainMenu, "SETTINGS", Menu::doNothing, Menu::noEvent, Menu::wrapStyle
  ,SUBMENU(setAutoRecord)
  ,SUBMENU(setHZoom)
  ,FIELD(disconnectTimeout,"Timeout","",5,60,1,1,doNothing,noEvent,noStyle)
  ,SUBMENU(time)
  ,OP(" SAVE SETTINGS",doSave,enterEvent)
  ,EXIT(" EXIT")
);

MENU_INPUTS(inp,&encStream);

#define UC_Width 320
#define UC_Height 240


//font size plus margins
#define fontX 18
#define fontY 18

#define MAX_DEPTH 2

//define colors
#define BLACK {0,0,0}
#define BLUE {0,0,255}
#define GRAY {128,128,128}
#define WHITE {255,255,255}
#define YELLOW {255,255,0}
#define RED {255,0,0}
#define GREEN {0,255,0}

const colorDef<rgb> colors[] MEMMODE={
  {{BLACK,BLACK},{BLACK,BLUE,BLUE}},//bgColor
  {{GRAY,GRAY},{WHITE,WHITE,WHITE}},//fgColor
  {{WHITE,BLACK},{YELLOW,YELLOW,RED}},//valColor
  {{WHITE,BLACK},{WHITE,YELLOW,YELLOW}},//unitColor
  {{WHITE,GRAY},{BLACK,BLUE,WHITE}},//cursorColor
  {{WHITE,YELLOW},{GREEN,WHITE,WHITE}},//titleColor
};

#define offsetX 0
#define offsetY 0

MENU_OUTPUTS(out,MAX_DEPTH
  ,UCG_OUT(ucg,colors,fontX,fontY,offsetX,offsetY,{0,0,UC_Width/fontX,UC_Height/fontY})
  ,NONE
);

NAVROOT(nav,mainMenu,MAX_DEPTH,inp,out);
//===================================================


void setClipGraph()
{
	ucg.setClipRange(0, STATUS_LINE_HEIGHT, width, height-STATUS_LINE_HEIGHT);
}

void setClipStatus()
{
	ucg.setClipRange(0, 0, width, STATUS_LINE_HEIGHT);
}

void setup()
{
	Serial.begin(115200);

	pinMode(ECG_LO1, INPUT);
	pinMode(ECG_LO2, INPUT);

	//pinMode(ENC_BTN, INPUT_PULLUP);

	pinMode(PC13, OUTPUT);

	rtc.begin();

	if (rtc.lostPower())
	{
		rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
	}

	dataRecorder->begin(SD_CS);

	ucg.begin(UCG_FONT_MODE_SOLID);
	ucg.setRotate90();
	ucg.clearScreen();

	height = ucg.getHeight();
	width = ucg.getWidth();

	ucg.setColor(1, 0, 0, 127);
	ucg.setColor(0, 0, 127);
	ucg.drawBox(0, 0, width, STATUS_LINE_HEIGHT);
	ucg.setFontMode(UCG_FONT_MODE_SOLID);
	ucg.setFont(ucg_font_ncenR12_hr);

	setClipGraph();

	correction_start = tick250 = tick500 = millis();

	lastConnect = millis();

	timer.pause();
	timer.setPeriod(1000000 / samplingFreq);
	timer.setCompare(TIMER_CH1, 1);
	timer.attachCompare1Interrupt(readADC);
	timer.refresh();
	timer.resume();

	timerEnc.pause();
	timerEnc.setPeriod(1000);
	timerEnc.setCompare(TIMER_CH1, 1);
	timerEnc.attachCompare1Interrupt(timerEncIsr);
	timerEnc.refresh();
	timerEnc.resume();

	nav.idleOn();
}

void displayStatus(const char * status)
{
	setClipStatus();

	ucg.setColor(255, 255, 255);
	ucg.setPrintPos(0,20);
	ucg.print(status);
}


void loop()
{
	DateTime now = rtc.now();

	if (nav.sleepTask)
	{
		m_Day = now.day();
		m_Month = now.month();
		m_Year = now.year();
		m_Hour = now.hour();
		m_Minute = now.minute();
	}

	nav.poll();

	if (!nav.sleepTask)
	{
		return;
	}

	if((digitalRead(ECG_LO1) == 1) || (digitalRead(ECG_LO1) == 1))
	{
		if ((millis() - lastConnect) > 1000 * disconnectTimeout)
		{
			dataRecorder->close();
		}

	    digitalWrite(PC13, HIGH);

		sprintf(buf, "NO SIGNAL %s %02d-%02d-%04d %02d:%02d:%02d",
				(dataRecorder->isActive() ? "[W]" : ""),
				now.day(),
				now.month(),
				now.year(),
				now.hour(),
				now.minute(),
				now.second());

		displayStatus(buf);

		ucg.setClipRange(0, 0, width, height);

		return;
	}

	lastConnect = millis();

	if (hasData)
	{
	    digitalWrite(PC13, LOW);

		hasData = false;

		posY = height/2 - round(((filteredValue)/4095)*(height-STATUS_LINE_HEIGHT));

		tickCur = millis();

		setClipGraph();

		ucg.setColor(63,63,63);
		ucg.drawPixel(posX, STATUS_LINE_HEIGHT+(height-STATUS_LINE_HEIGHT)/4);
		ucg.setColor(127,127,127);
		ucg.drawPixel(posX, STATUS_LINE_HEIGHT+(height-STATUS_LINE_HEIGHT)/2);
		ucg.setColor(63,63,63);
		ucg.drawPixel(posX, STATUS_LINE_HEIGHT+(height-STATUS_LINE_HEIGHT)*3/4);

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
			ucg.setColor(0,0,0);
			ucg.drawVLine(posX, 0, height);
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

			sprintf(buf, "%d s/s %d bpm %s ",
					(int)round(quant_Fq),
					beatDetector->getBps(),
					(dataRecorder->hasSD() ? "SD" : "--"));

			displayStatus(buf);
		  }
		}

    	beatDetector->push(notchFilteredValue);
    	dataRecorder->push(filteredValue);
	}
}



