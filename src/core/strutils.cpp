#include <cstring>
#include <functional>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <map>
#include <iostream>

#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>

#include "strutils.h"
#include "consts.h"

using std::string;
using std::list;
using std::istream;

// returns string in the upper case
string ToUpper(const string &str){
	string ret = str;
	std::transform(ret.begin(), ret.end(), ret.begin(), (int(*)(int)) toupper);
	return ret;
}
// Converts string to lower
string ToLower(const std::string &str){
	string ret = str;
	std::transform(ret.begin(), ret.end(), ret.begin(), (int(*)(int)) tolower);
	return ret;
}
// Converts string to camel
string ToCamel(const std::string &str){
	string ret = ToLower(str);
	if(ret.size()){
		ret[0] = toupper(ret[0]);
	}
	return ret;
}
// Converts first sumbol of string to upper form
std::string UpFirst(string str) {
	str[0] = toupper(str[0]);
	return str;
}
// the case insensitive strings compare function
bool strIcmp(const string &l, const string &r){
	// compare 2 strings in upper case
	return ToUpper(l) == ToUpper(r);
}
// searches pos of the first right none space symbol
inline int RPos(const char *buf, int len){
	register int i;
	for(i = len - 1; (i >= 0) && isspace(buf[i]); i--);
	return i;
}
// searches pos of the first left none space symbol
inline int LPos(const char *buf, int len){
	register int i;
	for(i = 0; (i < len) && isspace(buf[i]); i++);
	return i;
}
// deletes left space symbols in the string
string LTrim(const string &str){
	register int len = str.size();
	const char * buf = str.c_str();
	string ret(buf, LPos(buf, len), len);
	return ret;
}
// deletes right space symbols in the string
string RTrim(const string &str){
	register int len = str.size();
	const char * buf = str.c_str();
	string ret(buf, 0, RPos(buf, len) + 1);
	return ret;
}
// deletes left and right space symbols in the string
string AllTrim(const string &str){
	return LTrim(RTrim(str));
}
string LTrimStr(const string &str, string const & trimed){
	string ret(str, 0, trimed.size());
	if(ret == trimed){
		string ret(str, trimed.size(), str.size());
		return LTrimStr(ret, trimed);
	}
	return str;
}
string RTrimStr(const string &str, string const & trimed){
	std::size_t pos = str.rfind(trimed);
	if(pos != string::npos && pos + trimed.size() == str.size()){
		string ret(str, 0, str.size() - trimed.size());
		return RTrimStr(ret, trimed);
	}
	return str;
}
string Right(const string &str, int len) {
	return string(str, str.size() - len, len);
}
// return the string field in the string record where delimiter is "delim"
string GetField(const string &where, const string &delim, int num){
	if( delim.empty() )
		return "";
	string ret = "";
	int len = where.size();
	int posPast = 0;
	// iterate through fields in record
	for(int i = 0;;i++){
		// find position of the delimiter starts posPast symbol
		int pos = where.find(delim, posPast);
		// check end of string
		if(pos != (int)std::string::npos){
			// if it is a searched field - return it
			if(i == num){
				ret = where.substr(posPast, pos - posPast);
				break;
			}
			// increment position of search start with
			posPast = pos + delim.size();
		}else{
			if(i == num){
				ret = where.substr(posPast, len - posPast);
			}
			break;
		}
	}
	return ret;
}
template <typename T>
string toStringT(T val, size_t len){
	std::stringstream out;
	if(len){
		out << std::setw(len) << std::setfill('0') << val;
	}else{
		out << val;
	}
	return out.str();
}
template <typename T>
T toT(string val){
	T ret = 0;
	std::stringstream in(val);
	in >> ret;
	return ret;
}
string toString(int val, size_t len){
	return toStringT<int>(val ,len);
}
string toString(double val, size_t len) {
	return toStringT<double>(val ,len);
}
string toString(float val, size_t len) {
	return toStringT<float>(val ,len);
}
string toString(void *p) {
	return toStringT<void *>(p ,0);
}
string toStringL(long long val, size_t len) {
	return toStringT<long long>(val ,len);
}
int toInt(string const & val){
	return toT<int>(val);
}
double toDouble(string const & val) {
	return toT<double>(val);
}
float toFloat(string const & val) {
	return toT<float>(val);
}
long long toLLong(std::string const & val) {
	return toT<long long>(val);
}
bool checkFloat(string const & val){
	return abs(toFloat(val)) < abs(toFloat("1" + val));
}

string fillRight(string const & val, int len, char fill){
	std::stringstream out;
	out << std::setw(len) << std::setfill(fill) << val.substr(0, std::min((int)val.size(), len));
	return out.str();
}
string sTransform(string const & input, string iformat, string oformat, string const & keys){
	if(input.size() != iformat.size()){
//		throw SC::Exception("::sTransform Error iformat.size() != input.size()", "::sTransform");
		return string();
	}

	iformat = ToUpper(iformat);
	oformat = ToUpper(oformat);
	std::map<char, string> p;
	for(size_t i = 0; i != iformat.size(); ++i){
		if(keys.find(iformat[i]) != string::npos){
			p[iformat[i]].append(1, input[i]);
		}else if(iformat[i] != input[i]){
			return string();
		}
	}
	string ret;
	for(size_t i = 0; i != oformat.size(); ++i){
		if(keys.find(oformat[i]) != string::npos){
			if(p[oformat[i]].size()){
				ret.append(p[oformat[i]], 0, 1);
				p[oformat[i]].erase(0, 1);
			}else{
				ret.append("0");
			}
		}else{
			ret += oformat[i];
		}
	}
	return ret;
}
string toString(bool val){
	return val ? "true" : "false";
}
bool toBool(string const & val){
	if(strIcmp(val, "true"))
		return true;
	return false;
}
string toString(list<string> const &l, string const & delim) {
	string ret;
	for(list<string>::const_iterator i = l.begin(); i != l.end(); ++i){
		ret += *i + delim;
	}
	if(ret.size() >= delim.size()){
		ret.resize(ret.size() - delim.size());
	}
	return ret;
}
bool isInIsens(std::string where, std::string what) {
	where = ToUpper(where);
	what = ToUpper(what);
	return where.find(what, 0) != string::npos;
}
string strReplace(string const &s, string const & f, string const & r) {
	string ret = s;
	size_t pos = ret.find(f);
	if(pos != string::npos){
		ret = s.substr(0, pos) + r + s.substr(pos + f.size(), s.size());
	}
	return ret;
}
// only for r.size() < f.size()
// !!
string strReplaceAll( string const &s, string const & f, string const & r, int extraSize ) {
	string ret( s.size() + extraSize, 0 );
	size_t opos = 0, fpos = 0;
	char * buf = (char *)ret.data();
	do {
		fpos = s.find( f, fpos );
		if( string::npos != fpos ) {
			s.copy( buf, fpos - opos, opos );
			buf += fpos - opos;
			r.copy( buf, r.size(), 0 );
			buf += r.size();
			fpos += f.size();
			opos = fpos;
		} else {
			s.copy( buf, s.size() - opos, opos );
		}
	} while ( string::npos != fpos );
	return ret.c_str();
}
std::string readFile(std::string const &fname) {
	string ret;
	std::ifstream in( fname.c_str(), std::ios_base::binary );
	if( in ) {
		while( in.good() ) {
			char c = in.get();
			if( in.good() )
				ret.push_back( c );
		}
	}
	return ret;
}
list< string > readLinesFromFile( string const &fname ) {
	list< string > ret;
	std::ifstream input( fname.c_str() );
	if( (bool)input ){
		for(string line; std::getline(input, line); ) {
			ret.push_back( line );
		}
		input.close();
	}
	return ret;
}
list< string > readConfLines( string const &fname ) {
	list<string> t = readLinesFromFile( fname ), data;
	//erase comments
	BOOST_FOREACH( string s, t ) {
		s = AllTrim( s );
		if( s.empty() || s[0] == CCHAR_COMMENT_PREFIX ) {
			continue;
		}
		data.push_back( s );
	}
	return data;
}

void dumpHex(string const & str) {
	for(size_t i = 0; i != str.size(); ++i) {
		std::cout << (int)str[i] << " ";
	}
	std::cout << std::endl;
}
string toHexString(std::string const & str) {
	string ret;
	char buf[sizeof(int) * 2 + 1];
	for(int i = 0; i < (int)str.size(); ++i){
		int a = (unsigned char)str[ i] ;
		int const len = sprintf( buf, "%x", a );
		if( len == 1 ) {
			ret.append( 1, '0' );
		}
		ret.append( buf, len );
	}
	return ret;
}
string toHexString(int a) {
	string ret;
	char buf[sizeof(int) * 2 + 1];
	int const len = sprintf( buf, "%x", a );
	if( len == 1 ) {
		ret.append( 1, '0' );
	}
	ret.append( buf, len );
	return ret;
}
list< string > split( string const & val, string const & delim ) {
	list< string > ret;
	std::size_t posl = 0;
	std::size_t pos = val.find( delim, 0 );
	
	while( pos != string::npos ) {
		ret.push_back( string( val, posl, pos - posl ) );
		posl = pos + delim.size();
		pos = val.find( delim, posl );
	}

	if( posl != val.size() ) {
		ret.push_back( string( val, posl ) );
	}
	return ret;
}
string getBlock( string const & val, int blockNum, int blockSize) {
	if( blockNum * blockSize > (int)val.size() ) {
		return "";
	}
	int pos = blockNum * blockSize;
	int len = blockSize;
	if( pos + len > (int)val.size() ) {
		len = val.size() - pos;
	}
	return string( val, pos, len);
}
string getBlock( istream & in, int fileSize, int blockNum, int blockSize) {
	if( blockNum * blockSize > fileSize ) {
		return "";
	}
	int pos = blockNum * blockSize;
	int len = blockSize;
	if( pos + len > fileSize ) {
		len = fileSize - pos;
	}

	in.seekg( pos );
	string res( len, 0 );
	in.read( (char*)res.data(), len );
	return res;
}

list< string > splitWithEmpty( string const & val, string const & delim ) {
	list< string > ret;
	std::size_t posl = 0;
	std::size_t pos = val.find( delim, 0 );
	while( pos != string::npos ) {
		string str( val, posl, pos - posl );
		ret.push_back( str );

		posl = pos + delim.size();
		pos = val.find( delim, posl );
	}
	if( posl != string::npos ) {
		string str( val, posl );
		if( str.size() ) {
			ret.push_back( str );
		}
	}
	return ret;
}


//
