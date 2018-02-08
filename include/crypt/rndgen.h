#ifndef _RNDGEN_H
#define _RNDGEN_H

#include "sstring.h"

namespace SC{

class RndGen {
public:

	static sstring rand(int byteCnt);
	static sstring code64(int byteCnt);
	static sstring code16(int byteCnt);

};

}; //SC
#endif
