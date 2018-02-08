#ifndef _PROCESS_LOCK_H
#define _PROCESS_LOCK_H

#include <string>

namespace SC{

class ProcessLock {
	bool _locked;
	std::string _lockFName;
	std::string lockFName( std::string const & appConfPath, std::string const & appDataPath ) const;
	
public:
	ProcessLock( std::string const & appConfPath = "", std::string const & appDataPath = "");
	~ProcessLock();

	bool tryLock();

};

}; //SC
#endif //_PROCESS_LOCK_H
