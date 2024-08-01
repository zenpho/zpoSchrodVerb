/*
 *  DelayLineFilter.h
 *
 *  Created by tjmitche on 01/12/2010.
 *  Last edited by zenpho on 21/02/2012.
 *
 */

#ifndef H_DELAYLINEFILTER
#define H_DELAYLINEFILTER

#define ASSUMEDSAMPLERATE 44100

class DelayLineFilter 
{
public:
	//==============================================================================

	// construct destruct
	DelayLineFilter();
	virtual ~DelayLineFilter();

	//==============================================================================
	/**
	 Set the delay time in samples of the delayline - can be fractional
	 */
	void setDelayInSamples(float delayInSamples);
	
	
	/**
	set delay as a (float) factor of sample rate 
	sensible range: 0 to 1
	*/
	void setDelayFactor(float delayFactor);
	
	/**
	 Sets the feedback gain
	 */
	void setFeedbackGain(float val);
	
	/**
	 Function to perform the filter processing should call delayLineRead and delayLineWrite at some point
	 in the process
	 */
	virtual float filter(float input)=0;
	
protected:
	/**
	 Reads from the delayline using linear interpolation - should be called once per filter call and prior 
	 to delayLineWrite or the delay will be one sample inacurate
	 */
	float delayLineRead();
	
	/**
	 Writes to the delayline - should be called once per filter call and after to delayLineRead or 
	 the delay will be one sample inacurate
	 */
	void delayLineWrite(float input);
	float feedbackGain;

private:
	float delayLine[ASSUMEDSAMPLERATE];
	float readDelaySamples;
	int writeIndex;
	

	
};

#endif //H_DELAYLINEFILTER
