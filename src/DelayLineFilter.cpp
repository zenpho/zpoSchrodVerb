/*
 *  DelayLineFilter.cpp
 *
 *  Created by tjmitche on 01/12/2010.
 *  Last edited by zenpho on 21/02/2012.
 *
 */

#include "DelayLineFilter.h"

DelayLineFilter::DelayLineFilter()
{
	for (int counter = 0; counter < ASSUMEDSAMPLERATE; counter++) 
		delayLine[counter] = 0.f;
  
	readDelaySamples = 1.f;
	writeIndex = 0;
	feedbackGain = 0.f;
}
DelayLineFilter::~DelayLineFilter()
{
	
}

void DelayLineFilter::setDelayFactor(float delayFactor)
{
	if (delayFactor < 0) delayFactor = 0;
	if (delayFactor > 1) delayFactor = 1;
	
	setDelayInSamples( delayFactor * ASSUMEDSAMPLERATE );
}

void DelayLineFilter::setDelayInSamples(float delayInSamples)
{
	if(delayInSamples >= 0.f && delayInSamples <= ASSUMEDSAMPLERATE-1)
		readDelaySamples = delayInSamples;
}

void DelayLineFilter::setFeedbackGain(float val)
{
	if (val >= 0.f && val <= 1.0)
	{
		feedbackGain = val;
	}
}

float DelayLineFilter::delayLineRead()
{
	float readIndex = writeIndex - readDelaySamples;
	if (readIndex < 0)
		readIndex += ASSUMEDSAMPLERATE;
	
	int pos1 = static_cast<int>(readIndex);
	int pos2 = pos1 + 1;
	if(pos2 == ASSUMEDSAMPLERATE)
		pos2 = 0;
	
	float fraction = readIndex - static_cast<int>(pos1);
	float amprange = delayLine[pos2] - delayLine[pos1];
	float output = delayLine[pos1] + (fraction * amprange);
	
	return output;
}

void DelayLineFilter::delayLineWrite(float input)
{
	if(++writeIndex == ASSUMEDSAMPLERATE)
		writeIndex = 0;
	delayLine[writeIndex] = input;
}
