/*
 *  CombFilter.cpp
 *
 *  Last edited 21/02/2012.
 *
 */

#include "CombFilter.h"

CombFilter::CombFilter()
{
	setDelayInSamples(1000);
	setFeedbackGain(0.1);
}

CombFilter::~CombFilter()
{
	
}

//==============================================================================

float CombFilter::filter(float input)
{	
	/*
		input---(+)---[delay <samples>]--,---output
				 |                       |
				 |_____(x)_______________|
						|
						<feedbackGain>
	*/

	float delayLineOutput = delayLineRead();
	
	float output = delayLineOutput;
	
	float delayLineInput = (output * feedbackGain) + input;
	
	delayLineWrite( delayLineInput );
	
	return output;
}
