#include "fileutils.h"

#include <boost/filesystem.hpp>

bool SC::fileExist( const std::string & fName ) {
#ifdef _SERVER_
	return boost::filesystem::exists( fName );
#else
	boost::system::error_code ec;
	bool f = boost::filesystem::exists( fName, ec );
	return f && !ec; 
#endif
}

bool SC::fileRemove( const std::string & fName ) {
#ifdef _SERVER_
	boost::filesystem::remove( fName );
	return true;
#else
	boost::system::error_code ec;
	boost::filesystem::remove( fName, ec );
	return ec == 0;
#endif
}

void SC::createDirectory( const std::string & fName ) {
#ifdef _SERVER_
	boost::filesystem::create_directory( fName.c_str() );
#else
	boost::system::error_code ec;
	boost::filesystem::create_directory( fName.c_str(), ec );
#endif
}

void SC::fileRename( const std::string & src, const std::string & dst ) {
#ifdef _SERVER_
	boost::filesystem::rename( src, dst );
#else
	boost::system::error_code ec;
	boost::filesystem::rename( src, dst, ec );
#endif
}

