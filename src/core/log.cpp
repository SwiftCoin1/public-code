#include <sstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <fstream>

#include "log.h"
#include "logcalls.h"
#include "consts.h"

#include "datetime.h"
#include "parser.h"
#include "strutils.h"
#include "application.h"
#include "fileutils.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/utsname.h>
#endif

using namespace SC;
using std::cout;
using std::list;
using std::endl;
using std::string;
using std::map;
using std::endl;

string getTime() throw(){
	return DateTime::Current().data("yyyy/mm/dd hh:ii:ss-ooo");
}
static string getThreadId(){
	std::stringstream sstr;
	sstr << boost::this_thread::get_id();
	return sstr.str();
}
void log2File(string const & fileName, string const & text){
	std::ofstream out(fileName.c_str(), std::ios_base::app);
	out << text << endl;
	out.flush();
	out.close();
}
WorkLogger::WorkLogger(string const & fName) : _full(false), _repeat(false), _fileName(fName){
}

std::string WorkLogger::logText( std::string const & msg ) const {
	return "[ " + getTime() + " ]{ " + getThreadId() + " } " + msg;
}
std::string WorkLogger::logText( std::string const & msg, std::string const & place, std::string const & group ) const {
	return  "<" + place + ">(" + group + ") " + msg;
}
Logger & WorkLogger::add(string const & msg) {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	string text = logText( msg );
	log2File(_fileName, text );
	if(_repeat){
		cout << text << endl;
	}
	return *this;
}
Logger & WorkLogger::log(std::string const & msg, std::string const & place, std::string const & group) {
//cout << "WorkLogger::log 1 group=" << group << endl;	
	if( (_full || _groups.count(group)) && !_exceptions.count(group) ){
		add( logText( msg, place, group) );
	}
	return *this;
}
Logger & WorkLogger::error(std::string const & msg) {
	string text = "{ " + getThreadId() + " } " + msg;
	log2File( _fileName, text );
	if(_repeat){
		cout << text << endl;
	}
	return *this;
}

void WorkLogger::clear(){
	_full = false;
}
Logger & WorkLogger::groups(std::string const & list) {
	Parser parser;
	_full = false;
	Parser::StringVector v = parser.parseString(list, ';');
	for(int i = 0; i < (int)v.size(); ++i){
		if(strIcmp(v[i], CSTR_LOG_ALL )){
			_full = true;
		}
		_groups.insert(v[i]);
	}
	return *this;
}
Logger & WorkLogger::exceptions(std::string const & list) {
	Parser parser;
	Parser::StringVector v = parser.parseString(list, ';');
	for(int i = 0; i < (int)v.size(); ++i){
		_exceptions.insert(v[i]);
	}
	return *this;
}
void WorkLogger::Repeat2Cout(bool repeat) {
	_repeat = repeat;
}
void WorkLogger::Repeat2Cout(string const & repeat) {
	Repeat2Cout(strIcmp(repeat, "yes"));
}
bool WorkLogger::Repeat2Cout() const {
	return _repeat;
}
string WorkLogger::fileName() const {
	return _fileName;
}

//=============================================================== ResLogger

Resource::Resource() : _count(0) {}
void Resource::print() const {
	cout << "*res=" << toString(_res) << ", threadId=" << _threadId << ", count=" << _count << ", info=" << _info << endl;
}

void ResLogger::lock(void *res, std::string const & info) {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	string id = toString(res);
	Resource sres = _locked[id];
	if(sres._count > 0){
		cout << "ResLogger::lock id=" << id << " - second lock\n";
	}
	++sres._count;
	sres._info = info;
	sres._res = res;
	sres._threadId = getThreadId();
	_locked[id] = sres;
}
void ResLogger::release(void *res){
	boost::mutex::scoped_lock scoped_lock(_mutex);
	string id = toString(res);
	map<string, Resource>::const_iterator it = _locked.find(id);
	if(it != _locked.end()){
		Resource sres = it->second;
		--sres._count;
		if(sres._count == 0){
			_locked.erase(id);
		}else{
			_locked[id] = sres;
		}
	}
}
void ResLogger::printLocked2std() const {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	for(map<string, Resource>::const_iterator i = _locked.begin(); i != _locked.end(); ++i){
		i->second.print();
	}
}
string ResLogger::state() const {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	return "ResLogger:" + toString( (int)_locked.size() ) + " records";
}

// ====================================================== CallStack

//CallStack::CallStack(std::string const & fname) : _fileName(fname) {}

void CallStack::push(std::string const & threadId, std::string const & function) {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	std::stack<tStackData> &stack = _stacks[threadId];
	tStackData data = std::make_pair(function, string(""));
	stack.push(data);
}
void CallStack::addMark(std::string const & label) {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	std::stack<tStackData> &stack = _stacks[getThreadId()];
	stack.top().second += label + "::";
}
bool CallStack::pop(std::string const & threadId, std::string const & function) {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	std::stack<tStackData> &stack = _stacks[threadId];
	string lastLabel = stack.top().first;
	if(lastLabel != function) {
		cout << "Error in processing call stack, stack top="  << lastLabel << ", stack marks=" << stack.top().second << ", poped vallue=" << function << endl;
		return false;
	}
	if ( !stack.empty() ) {
		stack.pop();
		return true;
	}
	return false;
}

CalLoger::~CalLoger() {}

// ====================================================== MemCallStack

void MemCallStack::push(std::string const & function) {
	CallStack::push(getThreadId(), function);
}
void MemCallStack::addMark(std::string const & label) {
	CallStack::addMark(label);
}
void MemCallStack::pop(std::string const & function) {
	CallStack::pop(getThreadId(), function);
}

string GetOperatingSystemVersion() {
	string ver;
#ifdef WIN32
	OSVERSIONINFO VersionInfo;

	VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);

	if(GetVersionEx(&VersionInfo)) {
		// Test for the specific product family.
		if ( VersionInfo.dwMajorVersion == 6 && VersionInfo.dwMinorVersion == 0 )
			ver = "Microsoft Windows Vista or Server 2008 ";

		if ( VersionInfo.dwMajorVersion == 6 && VersionInfo.dwMinorVersion == 1 )
			ver = "Microsoft Windows 7 or Server 2008 R2 ";

		if ( VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 2 )
			ver = "Microsoft Windows Server 2003 Family ";

		if ( VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 1 )
			ver = "Microsoft Windows XP ";

		if ( VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 0 )
			ver = "Microsoft Windows 2000 ";

		if (VersionInfo.dwMajorVersion == 4 && VersionInfo.dwMinorVersion == 10) {
			ver = "Microsoft Windows 98 ";
			ver += VersionInfo.szCSDVersion[1]=='A' ? "SE " : "";
		} else if (VersionInfo.dwMajorVersion == 4 && VersionInfo.dwMinorVersion == 90) {
			ver = "Microsoft Windows Millennium Edition ";
		} else if ( VersionInfo.dwMajorVersion <= 4 )
			ver = "Microsoft Windows NT ";

		char versionum[40];
		sprintf(versionum,"%d.%d.%d",VersionInfo.dwMajorVersion,VersionInfo.dwMinorVersion,VersionInfo.dwBuildNumber);

		ver += versionum;
	} else {
		ver = "WINDOWS OS UNKNOWN";
	}
#else // UNIX
	struct utsname name;
	if(uname(&name) != -1) {
		ver = name.sysname;
		ver += name.version;
		ver += "." ;
		ver += name.release;
	}
#endif
	return ver;
}

void MemCallStack::dump(std::ostream & out) {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	if(_stacks.size()){
		out << "Crash date and time:" << getTime() << endl;
		// insert data about OS
		out << GetOperatingSystemVersion() << endl;
	}

	for(CallStack::tCallStacks::const_iterator i = _stacks.begin(); i != _stacks.end(); ++i){
		std::stack<CallStack::tStackData> stack = i->second;
		if( stack.size() ){
			out << "\tThread Id=" << i->first << endl;
			out << "\tStack of calls" << endl;
			while ( !stack.empty() ) {
				CallStack::tStackData &data = stack.top();
				out << "function=" << data.first << ", labels=" << data.second << endl;
				stack.pop();
			}
		}
	}
}
string MemCallStack::state() {
	boost::mutex::scoped_lock scoped_lock(_mutex);
	string ret = "CallStack:";
	ret += "Map:" + toString( (int)_stacks.size() ) + " records;";
	typedef std::pair<string, std::stack<tStackData> > iType;
	BOOST_FOREACH( iType dd, _stacks ) {
		ret += dd.first + "-" + toString( (int)dd.second.size()) + " records,";
	}
	ret += ";";
	return ret;
}

// ====================================================== LoggedStack

LoggedStack::LoggedStack(std::string const & fname) : _fileName(fname) {}

void LoggedStack::push(std::string const & function) {
	string const strTId = getThreadId();
	CallStack::push(strTId, function);

	boost::mutex::scoped_lock scoped_lock(_mutex);
	string text = "A|" + strTId + "|" + function;
	log2File(_fileName, text);
}
void LoggedStack::pop(std::string const & function) {
	string const strTId = getThreadId();
	if( CallStack::pop(strTId, function) ) {
		string text = "D|" + strTId + "|" + function;
		log2File(_fileName, text);
	}
}

// ====================================================== CallStack

CallReport::CallReport() {}

bool CallReport::open(std::string const & fname) {
	// check exist fname
	if( !fileExist( fname ) ){
		return false;
	}
	_fileName = fname;
	// fill stack
	std::ifstream in(_fileName.c_str());
	if(! in) {
		return false;
	}
	string line;
	while( std::getline(in, line) ) {
		string oper = GetField(line, "|", 0);
		string tId = GetField(line, "|", 1);
		string label = GetField(line, "|", 2);
		if ( oper == "A" ){
			_stack.push(tId, label);
		}else{
			_stack.pop(tId, label);
		}
	}
	return true;
}
list<string> CallReport::threads() {
	list<string> ret;
	for(CallStack::tCallStacks::const_iterator i = _stack._stacks.begin(); i != _stack._stacks.end(); ++i){
		ret.push_back(i->first);
	}
	return ret;
}
list<string> CallReport::calls(std::string const & threadId) {
	list<string> ret;
	std::stack<CallStack::tStackData> &stack = _stack._stacks[threadId];

	if ( !stack.empty() ) {
		stack.pop();
		//w //!!
		return ret;
	}
	return ret;
}

void pushCallStackInt(std::string const & funcName) {
	SC::FileSystemHelper::instance()->calLogger()->push(funcName);
}
void popCallStackInt(std::string const & funcName) {
	SC::FileSystemHelper::instance()->calLogger()->pop(funcName);
}
void addMarkCallStack(std::string const & mark) {
	SC::FileSystemHelper::instance()->calLogger()->addMark(mark);
}
void addLineCallStack(int line) {
	addMarkCallStack(toString(line));
}

// ====================================================== PassLogger

PassLogger::PassLogger( std::string const & fName, int maxGroupAmnt ) : WorkLogger( fName ), _maxGroupAmnt( maxGroupAmnt ) {}

string PassLogger::data( std::string const & group ) const {
	boost::mutex::scoped_lock scoped_lock( _logMutex );

	std::map< std::string, std::list<std::string> >::const_iterator f = _log.find( group ); 
	if( f == _log.end() ) {
		return "";
	}

	list<string> const & l = f->second;
	if( l.empty() ) {
		return "";
	}

	return toString( l.begin(), l.end(), "\n");
}

Logger & PassLogger::log(std::string const & msg, std::string const & place, std::string const & group) {
	WorkLogger::log( msg, place, group );
	boost::mutex::scoped_lock scoped_lock( _logMutex );

	list<string> & l = _log[ group ];
	if( (int)l.size() > _maxGroupAmnt ) {
		l.pop_front();
	}
	l.push_back( logText( logText( msg, place, group ) ) );
	return *this;
}


//