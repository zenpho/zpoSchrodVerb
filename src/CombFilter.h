/*
 *  CombFilter.h
 *
 *  Last edited 21/02/2012.
 *
 */

#ifndef H_COMBFILTER
#define H_COMBFILTER

#include "DelayLineFilter.h"

class CombFilter : public DelayLineFilter
{
public:
		
		CombFilter();
		~CombFilter();
		
		float filter(float input);
	
private:
		// none
};

#endif
