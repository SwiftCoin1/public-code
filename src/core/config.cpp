#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

#include "config.h"
#include "strutils.h"
#include "exception.h"
#include "consts.h"
#include "sptr.h"

#include <boost/thread/recursive_mutex.hpp>

using namespace SC;
using std::string;
using std::map;
using std::endl;
using std::cout;

static boost::recursive_mutex _rwMutex;
typedef boost::recursive_mutex::scoped_lock tScoped_lock;

static boost::recursive_mutex _createMutex;
static SPtr::shared<Config> g_Config;

Config::~Config() {}

Config * Config::instance(const std::string &confPath) {
	tScoped_lock scopedLock(_createMutex);
	if( !g_Config ){
		Config *conf = new Config();
		string filename = confPath;
		if( !conf->load( filename ) ) {
			filename = confPath + CSTR_CONFIG_FILENAME;
			if( !conf->load( filename ) ) {
				 cout << "Config::instance Error load config !" << endl;
			}
		}
		g_Config = conf;
	}
	return g_Config.get();
}

// Loads options from the specified file
bool Config::load(const string &file){
	tScoped_lock scoped_lock(_rwMutex);
	std::ifstream input(file.c_str());
	bool ret = input;
	if(ret){
		for(string line; std::getline(input, line); ) {
			addstr(line);
		}
		input.close();
		_fname = file;
	}
	return ret;
}
// Clears internal structures
void Config::clear(){
	tScoped_lock scoped_lock(_rwMutex);
	_values.clear();
}
// Returns option's value for specified key
string Config::value(const string &key) const{
	tScoped_lock scoped_lock(_rwMutex);
	if(!exist(key)){
		throw Exception("Config::value Error - Unknown key=" + key, "Config::value");
	}
	return _values.find(key)->second;
//	return _values.find(key) == _values.end() ? string() : _values.find(key)->second;
}
string Config::value(string const & key, string const & defValue) const {
	return exist( key ) ? value( key ) : defValue;
}
// Checks is the specified option exist
bool Config::exist(const string &key) const{
	tScoped_lock scoped_lock(_rwMutex);
	return _values.find(key) != _values.end();
}
// Parses string and add option to internal dictionary
void Config::addstr(string str){
	str = AllTrim(str);
	if((str.substr(0, 1) == "#") || (str.size() == 0))
		return;
	string key = GetField(str, "=", 0);
	key = RTrim(key);
	string val = str.substr(key.size() + 1, str.size());
//		GetField(str, "=", 1);
	val = GetField(val, "#", 0);
	val = AllTrim(val);
	_values[key] = val;
}
void Config::saveValue(string const &key, string const &value){
//cout << "Config::saveValue 1 key=" << key << ", value=" << value << endl;
	tScoped_lock scoped_lock(_rwMutex);
	string addLine = "";
	if( !exist(key) && AllTrim(value).size() ){
//cout << "Config::saveValue 2 key=" << key << ", value=" << value << endl;
		addLine = key + "=" + AllTrim(value);
		_values[key] = AllTrim(value);
	}
	std::ifstream input(_fname.c_str());
	if(input) {
		std::stringstream buf;
		if(addLine.size()){
			buf << addLine << endl;
		}
		for(string line; std::getline(input, line); ) {
			line = AllTrim(line);
			if((line.substr(0, 1) == "#") || (line.size() == 0)){
				buf << line << endl;
				continue;
			}
			string ckey = GetField(line, "=", 0);
			ckey = RTrim(ckey);
			if( ckey == key ) {
				string val = GetField( line, "=", 1 );
				val = AllTrim( val );
				string comment = GetField( val, "#", 1 );
				comment = AllTrim( comment );
				_values[key] = value;
				if( value.size() ) {
					line = key + "=" + value;
					if(comment.size()){
						line += "# " + comment;
					}
				} else {
					line = "";
				}
			}
			if( line.size() ) {
				buf << line << endl;
			}
		}
		input.close();
		std::ofstream out(_fname.c_str(), std::ios_base::out | std::ios_base::trunc);
		out.write( buf.str().c_str(), buf.str().size() );
		out.close();
	}
}
string Config::operator [] (string const & key) const{
	return value(key);
}
void Config::insert( Config const & r) {
	tScoped_lock scoped_lock(_rwMutex);
	for(map<string, string>::const_iterator i = r._values.begin(); i != r._values.end(); ++i) {
		saveValue(i->first, i->second);
	}
}
bool Config::boolValue( std::string const & key, bool defValue ) const {
	return exist( key ) ? strIcmp( value( key ), CSTR_YES ) : defValue;
}

//======================================================== MemConfig

void MemConfig::keepValue(std::string const &key, std::string const &value) {
	tScoped_lock scoped_lock(_rwMutex);
	_values[ key ] = value;
}
void MemConfig::saveKept() {
	tScoped_lock scoped_lock(_rwMutex);
	for(map<string, string>::const_iterator i = _values.begin(); i != _values.end(); ++i) {
		saveValue(i->first, i->second);
	}
}
// Loads data from the string key=value;key1=value1;....
bool MemConfig::load( std::string const & str ) {
	std::list< std::string > v = splitWithEmpty( str, ";" );
	for( std::list< std::string >::const_iterator i = v.begin(); i != v.end(); ++i ) {
		addstr( *i );
	}
	return _values.size();
}
string MemConfig::data() const {
	string ret;
	for( map<string, string>::const_iterator i = _values.begin(); i != _values.end(); ++i ) {
		ret += i->first + "=" + i->second + ";";
	}
	return RTrimStr(ret, ";");
}


//
