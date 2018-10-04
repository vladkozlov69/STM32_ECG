/*
 * BeatDetector.h
 *
 *  Created on: Oct 4, 2018
 *      Author: vkozlov
 */

#ifndef BEATDETECTOR_H_
#define BEATDETECTOR_H_

#include "Arduino.h"
#include "Biquad.h"

#define BEAT_BUFFER_SIZE 5

class BeatDetector
{
	Biquad * baselineFilter;
	double baselineValue = 0;
	boolean belowThreshold = true;
	int BPM = 0;
	int beat_old = 0;
	double beats[BEAT_BUFFER_SIZE];  // Used to calculate average BPM
	int beatIndex = 0;

public:
	BeatDetector(const int samplingFreq);
	void updateSamplingFrequency(int frequency);
	void push(double value);
	int getBps();
	double getBaseline();
private:
	void calculateBPM();
	void internalPush();
};

#endif /* BEATDETECTOR_H_ */
