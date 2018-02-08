#ifndef _SECRET_H
#define _SECRET_H

#include "sstring.h"

namespace SC{

class Secret {
	sstring _secret;
public:
	sstring text() const {
		return _secret;
	}
	void text(const sstring & val){
		_secret = val;
	}

};

}; //SC
#endif
