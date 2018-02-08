#include "error.h"

using namespace SC;
using std::string;

Error::Error() : _place(""), _text(""), _code(NoError) {}
Error::Error(string const & text, string const & place) : _place(place), _text(text), _code(NoError) {}

Error::~Error() throw() {}

string Error::place() const {
	return _place;
}
string Error::text() const {
	return _text;
}
Error & Error::place( std::string const & v ) {
	_place = v;
	return *this;
}
Error & Error::text( std::string const & v ) {
	_text = v;
	return *this;
}
Error::operator bool() const {
	return !_text.size();
}
Error Error::operator &&(Error const & r){
	if(!*this){
		return *this;
	}
	return r;
}
Error::Code Error::code() const {
	return _code;
}
void Error::code(Error::Code const & c) {
	_code = c;
}

//
