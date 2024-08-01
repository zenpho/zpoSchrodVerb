/*
 *  AllpassFilter.cpp
 *
 *  Last edited 21/02/2012.
 *
 */

#include "AllpassFilter.h"

AllpassFilter::AllpassFilter()
{
	setDelayInSamples(200);
	setFeedbackGain(0.1);
}

AllpassFilter::~AllpassFilter()
{
	
}

//==============================================================================

float AllpassFilter::filter(float input)
{	
    /** 
	   from http://freeverb3.sourceforge.net/reverb.shtml
	   http://freeverb3.sourceforge.net/AP_RESP.jpg (bottom structure, negative feedback path)
	
                        <gain>
                       |
	              ----(*)--------
	             |               |
	   IN --(+)--.--[delay]--.--(+)-- OUT
	         |               |
			  --------(*)----
			           |
			 -1 * <gain>
	*/
	
	float delayLineOutput = delayLineRead();
	
	float backPath     = delayLineOutput * (feedbackGain * -1);
	float forwardPath  = (input + backPath)  * feedbackGain;
	
	float delayLineInput = backPath + input;
	
	float output = delayLineOutput + forwardPath;
	
	delayLineWrite( delayLineInput );

	return output;
	
	/**
	  * I think this is broken

	
							         <feedbackGain>
	                                 |
		          __________________(x)_____
		         |                          |
                 |                          |
		input---(+)---.-[delay <samples>]---'--(+)---output
				      |                         |
				      |__(x)____________________|
						  |
						  <feedbackGain> * -1

	float delayLineOutput = delayLineRead();

	float delayLineInput = (delayLineOutput * feedbackGain) + input;
	
	float output = (delayLineInput * feedbackGain * -1) + (delayLineOutput * feedbackGain);
	
	delayLineWrite( delayLineInput );
	*/
}
