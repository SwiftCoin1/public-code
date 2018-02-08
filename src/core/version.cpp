#include <iostream>
#include <cstdio>

#include "strutils.h"
#include "version.h"

using namespace SC;
using std::string;
using std::cout;

Version::Version() : _major(0), _minor(0) {}
Version::Version(const string & str) : _major(0), _minor(0) {
	load(str);
}
bool Version::operator<(const Version & r) const {
	return (_major < r._major) || ((_major == r._major) && (_minor < r._minor));
}
bool Version::operator>(const Version & r) const {
	return r < *this;
}
bool Version::operator==(const Version & r) const {
	return (_major == r._major) && (_minor == r._minor);
}
bool Version::operator!=(const Version & r) const{
	return !(*this == r);
}
string Version::toString() const {
	return ::toString(_major) + "." + ::toString(_minor);
}
Version & Version::AddMinor(int a){
	Minor( Minor() + a);
	return *this;
}
Version & Version::AddMajor(int a){
	Major( Major() + a);
	return *this;
}
int Version::Minor() const{
	return _minor;
}
void Version::Minor(int a){
	_minor = a;
}
int Version::Major() const{
	return _major;
}
void Version::Major(int a){
	_major = a;
}
string Version::data() const{
	return toString();
}
void Version::load(string const & in){
	sscanf(in.c_str(), "%d.%d", &_major, &_minor);
}


