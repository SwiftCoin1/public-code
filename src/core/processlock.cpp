#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

#include "processlock.h"
#include "config.h"
#include "sptr.h"
#include "consts.h"
#include "strutils.h"
#include "fileutils.h"

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>

using std::string;
using std::cout;
using std::endl;
using namespace SC;

using boost::interprocess::file_lock;
using boost::interprocess::interprocess_exception;

static file_lock * f_lock = NULL;

ProcessLock::ProcessLock( std::string const & appConfPath, std::string const & appDataPath ) : _locked(false) {
	_lockFName = lockFName( appConfPath, appDataPath );
//cout << "ProcessLock::ProcessLock _lockFName=" << _lockFName << endl;
	try{
		if( !fileExist( _lockFName ) ){
			std::ofstream out(_lockFName.c_str(), std::ios_base::trunc);
			out << "1";
			out.close();
		}
		f_lock = new file_lock(_lockFName.c_str());
	}catch(...){
	}
}
ProcessLock::~ProcessLock() {
	try {
		if(_locked && f_lock){
			f_lock->unlock();
			f_lock = NULL;
			remove(_lockFName.c_str());
		}
	} catch(interprocess_exception &ex) {}
//	
}
bool ProcessLock::tryLock() {
	if(!_locked && f_lock){
		try {
			_locked = f_lock->try_lock();
		} catch (interprocess_exception &ex) {
			_locked = false;
		}
	}else{
		return false;
	}
	return _locked;
}
string ProcessLock::lockFName( std::string const & appConfPath, std::string const & appDataPath ) const {
	Config config;
	string conFName = CSTR_CONFIG_FILENAME;
	if( appConfPath.size() ) {
		conFName = appConfPath + CSTR_PATH_DELIM + conFName;
	}

	config.load( conFName );

	string fname = appDataPath.size() ? appDataPath : config.value( CSTR_COMMON_USER_DATA_NAME_PARAM );

	fname = AllTrim(fname);
	fname = RTrimStr(fname, CSTR_PATH_DELIM);
	fname += CSTR_PATH_DELIM;
	
	if( config.exist(CSTR_LOCK_FNAME_NAME_PARAM) ) {
		fname += config.value(CSTR_LOCK_FNAME_NAME_PARAM);
	}else{
		fname += "plock.lck";
	}
	return fname;
}


//