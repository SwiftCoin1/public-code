#include "exception.h"
#include "log.h"

using namespace SC;
using std::string;

Exception::Exception(const string & what, const string & place) : std::logic_error(what), _place(place) {
//	Application::instance()->errLogger()->log(what, place, "Exception");
}
Exception::~Exception() throw() {}

void Exception::place(const string & p) {
	_place = p;
}
string Exception::place() const {
	return _place;
}
string Exception::text() const {
	return place() + "-" + what();
}

