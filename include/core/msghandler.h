#ifndef _MESSAGEHANDLER_H
#define _MESSAGEHANDLER_H

#include <string>
#include "msg.h"

namespace SC{

class MsgHandler {
public:
	virtual ~MsgHandler();
	virtual bool process(Message const & msg);
	virtual std::string processWithResult(Message const & msg);

};

}; //SC
#endif //_MESSAGEHANDLER_H
