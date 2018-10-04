/*
 * BeatDetector.cpp
 *
 *  Created on: Oct 4, 2018
 *      Author: vkozlov
 */

#include "BeatDetector.h"

BeatDetector::BeatDetector(const int samplingFreq)
{
	baselineFilter = new Biquad(bq_type_lowpass, 0.3 / samplingFreq, 0.707, 0);
}


void BeatDetector::push(const double value)
{
	baselineValue = baselineFilter->process(value);

	double threshold = baselineValue * 1.7;

	// BPM calculation check
	if (value > threshold && belowThreshold == true)
	{
		calculateBPM();
		belowThreshold = false;
	}
	else if(value < threshold)
	{
		belowThreshold = true;
	}
}

double BeatDetector::getBaseline()
{
	return baselineValue;
}

void BeatDetector::calculateBPM ()
{
  int beat_new = millis();    // get the current millisecond
  int diff = beat_new - beat_old;    // find the time between the last two beats
  double currentBPM = 60000 / diff;    // convert to beats per minute
  beats[beatIndex] = currentBPM;  // store to array to convert the average
  double total = 0.0;
  for (int i = 0; i < BEAT_BUFFER_SIZE; i++)
  {
    total += beats[i];
  }
  BPM = int(total / BEAT_BUFFER_SIZE);
  beat_old = beat_new;
  beatIndex = (beatIndex + 1) % BEAT_BUFFER_SIZE;  // cycle through the array instead of using FIFO queue
}

void BeatDetector::updateSamplingFrequency(const int frequency)
{
	baselineFilter->setFc(0.3 / frequency);
}

int BeatDetector::getBps()
{
	return BPM;
}

