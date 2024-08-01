/*
 *  AllpassFilter.h
 *
 *  Last edited 21/02/2012.
 *
 */

#ifndef H_ALLPASSFILTER
#define H_ALLPASSFILTER

#include "DelayLineFilter.h"

class AllpassFilter : public DelayLineFilter
{
public:
		
		AllpassFilter();
		~AllpassFilter();
		
		float filter(float input);
	
private:
		// none
};

#endif
