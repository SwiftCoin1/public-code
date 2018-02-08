#ifndef _LOGCALLS_H
#define _LOGCALLS_H

#include <string>
#include <stack>
#include <list>
#include <map>
#include <utility>

#include <boost/thread/mutex.hpp>

void pushCallStackInt(std::string const & funcName);
void popCallStackInt(std::string const & funcName);
void addMarkCallStack(std::string const & mark);
void addLineCallStack(int line);


struct CallsPusher {
	std::string _fName;
	CallsPusher( const std::string & fName ) :_fName( fName ) {
		pushCallStackInt( fName );
	}
	~CallsPusher() {
		popCallStackInt( _fName );
	}
};

namespace SC{

struct CallStack {
	typedef std::pair<std::string, std::string> tStackData;
	typedef std::map<std::string, std::stack<tStackData> > tCallStacks;

	tCallStacks _stacks;
	boost::mutex _mutex;

//	CallStack(std::string const & fname);

	void push(std::string const & threadId, std::string const & function);
	void addMark(std::string const & label);
	bool pop(std::string const & threadId, std::string const & function);
};

struct CalLoger{
	virtual ~CalLoger() = 0;
	virtual void push(std::string const & function) = 0;
	virtual void addMark(std::string const & label) = 0;
	virtual void pop(std::string const & function) = 0;

	virtual void dump(std::ostream & out) = 0;
	virtual std::string state() = 0;
};

class MemCallStack : public CalLoger, public CallStack {
public:
	void push(std::string const & function);
	void addMark(std::string const & label);
	void pop(std::string const & function);

	void dump(std::ostream & out);
	std::string state();
};

class LoggedStack : public CallStack {
	std::string _fileName;
public:
	LoggedStack(std::string const & fname);

	void clearFile();
	void backupFile(std::string const & fname);

	void push(std::string const & );
	void pop(std::string const &);
};

class CallReport {
	std::string _fileName;
	CallStack _stack;
public:
	CallReport();

	bool open(std::string const & fname);

	std::list<std::string> threads();
	std::list<std::string> calls(std::string const & threadId);

	void outLastCals(std::ostream & out);
};


}; //SC
#endif		// _LOGCALLS_H
