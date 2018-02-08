#ifndef _LOGGER_H
#define _LOGGER_H

#include <string>
#include <set>
#include <map>
#include <list>
#include <utility>

#include "error.h"

#include <boost/thread/mutex.hpp>

namespace SC{


class Logger {
public:
	virtual ~Logger() {}

	virtual Logger & log(std::string const &, std::string const &, std::string const &) = 0;
	virtual Logger & add(std::string const &) = 0;
	virtual Logger & error(std::string const &) = 0;
	virtual Logger & groups(std::string const &) = 0;
	virtual Logger & exceptions(std::string const &) = 0;
	virtual void Repeat2Cout(bool) = 0;
	virtual void Repeat2Cout(std::string const &) = 0;
	virtual bool Repeat2Cout() const = 0;
	virtual std::string fileName() const = 0;
	virtual void clear() = 0;
};

class EmptyLogger : public Logger {
public:
	Logger & log(std::string const &, std::string const &, std::string const &) { return *this; }
	Logger & add(std::string const &) { return *this; }
	// write to log file without external calls of function (getTime etc)
	Logger & error(std::string const &) { return *this; }

	Logger & groups(std::string const &) { return *this; }
	Logger & exceptions(std::string const &) { return *this; }

	void Repeat2Cout(bool) {}
	void Repeat2Cout(std::string const &) {}
	bool Repeat2Cout() const { return false; }

	std::string fileName() const { return ""; }

	void clear() {}
};

class WorkLogger : public Logger {
	bool _full;
	bool _repeat;
	std::string _fileName;
	mutable boost::mutex _mutex;
	std::set<std::string> _groups;
	std::set<std::string> _exceptions;

protected:
	std::string logText( std::string const & msg, std::string const & place, std::string const & group ) const;
	std::string logText( std::string const & msg ) const;

public:
	WorkLogger(std::string const & fName);
	~WorkLogger() {}

	Logger & log(std::string const & msg, std::string const & place, std::string const & group);
	Logger & add(std::string const & msg);
	// write to log file without external calls of function (getTime etc)
	Logger & error(std::string const & msg);

	Logger & groups(std::string const & list);
	Logger & exceptions(std::string const & list);

	void Repeat2Cout(bool repeat);
	void Repeat2Cout(std::string const & repeat);
	bool Repeat2Cout() const;

	std::string fileName() const;

	void clear();
};

struct Resource{
	std::string _info;
	void *_res;
	std::string _threadId;
	int _count;
	Resource();
	void print() const;
};

class ResLogger {
	mutable boost::mutex _mutex;
	std::map<std::string, Resource> _locked;
public:
	void lock(void *res, std::string const & info);
	void release(void *res);

	void printLocked2std() const;
	std::string state() const;
};

// logger to pass log to support
class PassLogger: public WorkLogger {
	mutable boost::mutex _logMutex;
	std::map< std::string, std::list<std::string> > _log;
	int _maxGroupAmnt;

public:
	// maxGroupAmnt - max amount messages in group to send to server
	PassLogger( std::string const & fName, int maxGroupAmnt );

	std::string data( std::string const & group ) const;

	Logger & log(std::string const & msg, std::string const & place, std::string const & group);
};

}; //SC


#endif		// _LOGGER_H
