#include <iostream>
#include <sstream>
#include <cstdio>

#include "money.h"
#include "exception.h"
#include "strutils.h"

using namespace SC;
using std::string;
using std::cout;
using std::endl;

#define CSTR_DEFAULT_CURRENCY			"SC"

long long pow10( int p) {
	long long ret = 1;
	for(int i = 0; i < p; ++i ) {
		ret *= 10;
	}
	return ret;
}

const SC::Money SC::oneCoin( 0 );
const SC::Money SC::zeroCoin( 0 );

Money::Money() : _value(0), _currency(CSTR_DEFAULT_CURRENCY) {}
Money::Money(long long val) : _value(val), _currency(CSTR_DEFAULT_CURRENCY) {}

string Money::Currency() const{
	return _currency;
}
void Money::Currency(string const & currency){
	_currency = currency;
}
bool Money::TheSameCurrency(Money const & r) const{
	return _currency == r._currency;
}
string Money::Value() const{
	std::stringstream out;
	out << _value;
	return out.str();
}
Money Money::operator+(Money const & r) const {
	if(!TheSameCurrency(r)){
		throw Exception("Money::operator+ Error - Currencies are different", "Money::operator+");
	}
	Money ret;
	ret._value = _value + r._value;
	return ret;
}
Money Money::operator-(Money const & r) const {
	if(!TheSameCurrency(r)){
		throw Exception("Money::operator- Error - Currencies are different", "Money::operator-");
	}
	Money ret;
	ret._value = _value - r._value;
	return ret;
}
Money Money::operator+(int r) const {
	Money ret;
	ret._value = _value + r;
	return ret;
}
Money Money::operator-(int r) const {
	Money ret;
	ret._value = _value - r;
	return ret;
}
Money Money::operator*(double r) const {
	Money ret;
	ret._value = r * _value;
	return ret;
}
bool Money::operator==(Money const & r) const {
	return _value == r._value && _currency == r._currency;
}
bool Money::operator!=(Money const & r) const {
	return !operator==(r);
}
bool Money::operator>(Money const & r) const {
	if(!TheSameCurrency(r)){
		throw Exception("Money::operator> Error - Currencies are different", "Money::operator>");
	}
	return _value > r._value;;
}
bool Money::operator<(Money const & r) const {
	if(!TheSameCurrency(r)){
		throw Exception("Money::operator< Currencies are different", "Money::operator<");
	}
	return _value < r._value;;
}
bool Money::operator>=(Money const & r) const {
	return (*this > r) || (*this == r);
}
bool Money::operator<=(Money const & r) const {
	return (*this < r) || (*this == r);
}
Money Money::Divide(int parts, Money & rest) const {
	Money ret;
	ret._value = _value / parts;
	rest._value = ret._value + _value - (_value / parts) * parts;
	return ret;
}
Money Money::Divide(int parts) const {
	Money ret;
	ret._value = _value / parts;
	return ret;
}
string Money::data( const std::string & delim ) const{
	return Value() + delim + Currency();
}
void Money::load( string const & in, const std::string & delim ){
	char buf[10];
	int v;
	string tmpl = "%d" + delim + "%s";
	if(sscanf(in.c_str(), tmpl.c_str(), &v, buf) == 2){
		_value = v;
		_currency = buf;
	}
}
Money::operator bool() const {
	return _value > 0;
}
void Money::clear() {
	_value = 0;
}
string Money::toUser() const {
	string ret = toStringL(_value);
	int size = CINT_FRACTIONAL_PART - ret.size() + 1;
	if(size > 0){
		ret = string(size, '0') + ret;
	}
	ret = ret.substr(0, ret.size() - CINT_FRACTIONAL_PART) + "." + ret.substr(ret.size() - CINT_FRACTIONAL_PART, CINT_FRACTIONAL_PART);
	return ret;
}
void Money::fromUser(string const & m) {
	_value = 0;
	size_t pos = m.find('.');
	if(pos == string::npos){
		long long s = toLLong(m);
		_value = s * pow10( CINT_FRACTIONAL_PART );
	}else{
		string f = m.substr(pos + 1, CINT_FRACTIONAL_PART);
		int sfp = CINT_FRACTIONAL_PART - f.size();
		f = f + string(sfp, '0');
		int fi = toInt(f);
		string i = m.substr(0, pos);
		if(i.size()){
			_value = toLLong(i);
			_value *= pow10( CINT_FRACTIONAL_PART );
		}
		_value += fi;
	}
}


//