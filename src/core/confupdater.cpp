#include <iostream>

#include <boost/foreach.hpp>

#include "confupdater.h"
#include "application.h"
#include "strutils.h"
#include "parser.h"

using namespace SC;
using std::string;
using std::list;
using std::cout;
using std::endl;

void ConfUpdater::update() {
	FileSystemHelper * app = FileSystemHelper::instance();

	string fName = CSTR_UPDATER_FNAME;
//cout << "ConfUpdater::update fName=" << fName << endl;
	Parser p;
	list< string > lines = readLinesFromFile( fName );

	Config * gconf = (Config * ) app->config();
	Config * lconf = (Config * ) app->localConfig();
	Config * cconf = (Config * ) app->commonConfig();

	BOOST_FOREACH( string l, lines ) {

		Config * conf = NULL;
		Parser::StringVector v = p.parseString( l, ';' );
		if( 4 == v.size() ) {
			string confName = v[ 0 ];
			string updateNum = v[ 1 ];
			string pName = v[ 2 ];
			string newVal = v[ 3 ];
			if( CSTR_LOCAL_CONFIG_NAME == confName ) {
				conf = lconf;
			} else if( CSTR_COMMON_CONFIG_NAME == confName ) {
				conf = cconf;
			} else if( CSTR_GLOBAL_CONFIG_NAME == confName ) {
				conf = gconf;
			}
			if( conf ) {
				string curUpdNum = conf->value( CSTR_CONF_UPDATE_NUM_PARAM, "0" );
				if( toInt(curUpdNum) < toInt(updateNum) ) {
					conf->saveValue( pName, newVal );
					conf->saveValue( CSTR_CONF_UPDATE_NUM_PARAM, updateNum );
				}
			}
		}
	}
}

//