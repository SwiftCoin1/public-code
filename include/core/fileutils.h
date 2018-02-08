#pragma once

#include <string>

namespace SC {

bool fileExist( const std::string & fName );
bool fileRemove( const std::string & fName );
void createDirectory( const std::string & fName );
void fileRename( const std::string & src, const std::string & dst );

}


