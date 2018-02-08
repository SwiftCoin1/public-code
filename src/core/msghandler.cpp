#include "msghandler.h"

using namespace SC;

using std::string;

MsgHandler::~MsgHandler() {}

bool MsgHandler::process( Message const & ) {
	return false;
}
std::string MsgHandler::processWithResult( Message const & ) {
	return "";
}


//