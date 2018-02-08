#ifndef _STDSTRINGUTILS_
#define _STDSTRINGUTILS_

#include <string>
#include <list>
#include <istream>

// Converts string to upper
std::string ToUpper(const std::string &str);
// Converts string to lower
std::string ToLower(const std::string &str);
// Converts string to camel
std::string ToCamel(const std::string &str);

// Converts first sumbol of string to upper form
std::string UpFirst(std::string str);

// Compares strings ignoring case
bool strIcmp(const std::string &l, const std::string &r);
// true if in string "where" is substring "what"
bool isInIsens(std::string where, std::string what);
// Removes spaces from the left end
std::string LTrim(const std::string &str);
// Removes spaces from the right end
std::string RTrim(const std::string &str);

std::string Right(const std::string &str, int len);

// Removes substrings (rtimed) from the left end of str
std::string LTrimStr(const std::string &str, std::string const & trimed);
// Removes substrings from the right end
std::string RTrimStr(const std::string &str, std::string const & trimed);

// Removes spaces from both ends
std::string AllTrim(const std::string &str);
// Get substring from the delimeted string
// where - source string which contains fields separated by delimeter
// delim - delimeter
// num - field number
std::string GetField(const std::string &where, const std::string &delim, int num);

std::string toString(int val, size_t len = 0);
int toInt(std::string const & val);

std::string toString(double val, size_t len = 0);
double toDouble(std::string const & val);

std::string toString(float val, size_t len);
float toFloat(std::string const & val);

std::string toString(bool val);
bool toBool(std::string const & val);

std::string toString(void *p);

std::string toStringL(long long val, size_t len = 0);
long long toLLong(std::string const & val);

// checks - string is float number
bool checkFloat(std::string const & val);

std::string fillRight(std::string const & val, int len, char fill = ' ');

std::string sTransform(std::string const & input, std::string iformat, std::string oformat, std::string const & keys);

std::string toString(std::list<std::string> const &l, std::string const & delim);

std::string strReplace(std::string const &s, std::string const & f, std::string const & r);
std::string strReplaceAll( std::string const &s, std::string const & f, std::string const & r, int extraSize = 1 );

std::string readFile(std::string const &fname);
std::list< std::string > readLinesFromFile( std::string const &fname );

// reads lines and removes comments - # starting 
std::list< std::string > readConfLines( std::string const &fname );

//only if T is Serializable 
template <typename T>
std::string ser2str( const T & v ) {
	return v.data();
}
template <typename It>
std::string toString(It begin, It end, std::string const & delim ) {
	std::string ret;
	for( It i = begin; i != end; ++i ) {
		ret += *i + delim;
	}
	if( ret.size() > delim.size() ){
		ret = ret.substr(0, ret.size() - delim.size());
	}
	return ret;
};
template <typename It, typename F>
std::string toString(It begin, It end, F f, std::string const & delim ) {
	std::string ret;
	for( It i = begin; i != end; ++i ) {
		ret += f(*i) + delim;
	}
	if( ret.size() > delim.size() ){
		ret = ret.substr(0, ret.size() - delim.size());
	}
	return ret;
};

void dumpHex(std::string const & str);

// translate string to its 16 code representation
std::string toHexString(std::string const & str);
// translate int to its hex representation
std::string toHexString(int a);

std::list< std::string > split( std::string const & val, std::string const & delim );

std::string getBlock( std::string const & val, int blockNum, int blockSize);
std::string getBlock( std::istream & in, int fileSize, int blockNum, int blockSize);

std::list< std::string > splitWithEmpty( std::string const & val, std::string const &delim );

#endif // _STDSTRINGUTILS_
