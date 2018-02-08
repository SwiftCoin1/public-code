#include <iostream>
#include <sstream>
#include <fstream>

#include "application.h"
#include "strutils.h"
#include "code64.h"
#include "rndgen.h"
#include "log.h"
#include "logcalls.h"
#include "consts.h"
#include "transport/proxyid.h"
#include "fileutils.h"
#include "confupdater.h"
#include "log.h"
#include "logcalls.h"

#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

using namespace SC;
using std::list;
using std::string;
using std::cout;
using std::endl;

typedef boost::recursive_mutex::scoped_lock tScoped_lock;

static boost::recursive_mutex _createMutex;
static boost::recursive_mutex _timeGMTMutex;
static FileSystemHelper * g_inst = NULL;
static bool g_released = false;

class EmptyFileSystemHelper : public FileSystemHelper {
	Config _econfig;
	EmptyLogger _elogger;
	MemCallStack _eclogger;
public:
	~EmptyFileSystemHelper() {}

	bool active() const { return false; }

	std::string createNewDataFolder( std::string const & ) { return ""; }
	std::string adDataFolder( std::string const &, std::string const & ) { return ""; }
	tWalletDescr activeWallet() const { return tWalletDescr(); }
	std::list< tWalletDescr > localWallets() const { return std::list< tWalletDescr >(); }
	bool changeCurrentWallet( int ) { return false; }
	void popBackWallet() {}
	bool changeCurrentWallet( int, Config & ) { return false; }
	std::list< tWalletDescr > localWallets( Config const & ) const { return std::list< tWalletDescr >(); } 
	std::string fullFileName(std::string const & ) const { return ""; }
	std::string fileNameInCommon(std::string const &) const { return ""; }
	std::string adDataFolderInternal( std::string const &, std::string const & ) { return ""; }
	std::string createNewDataFolder( std::string const &, Config & ) { return ""; }
	tWalletDescr activeWallet( Config const & ) const { return tWalletDescr(); }
	void freeLoggers() {}
	void loadUserConfigs() {}
	void setupLogs() {}
	void popBackWallet( Config & ) {}
	std::string confPath() const { return ""; }
	std::string commonConfPath() const { return ""; }
	bool checkWalletPath( std::string ) { return false; }
	bool walletDataFolderMade() const { return false; }
	void walletDataFolderMade( bool) {}
	void init( std::string const & ) {}

	Config *config() { return (Config *)&_econfig; }
	Config *localConfig() { return (Config *)&_econfig; }
	Config *commonConfig() { return (Config *)&_econfig; }
	Config const *commonConfig() const { return &_econfig; }
	Config *dbKeyConfig() { return (Config *)&_econfig; }
	Logger * logger() const { return (Logger *)&_elogger; }
	Logger * userLogger() const { return (Logger *)&_elogger; }
	Logger * errLogger() const { return (Logger *)&_elogger; }
	CalLoger * calLogger() const { return (CalLoger *)&_eclogger; }

	DateTime currentGMT() { return DateTime(); }
	void currentGMT(DateTime const & ) {}
	std::string socketMode() const { return ""; }
};

static EmptyFileSystemHelper g_emptyInst;

class WorkFileSystemHelper : public FileSystemHelper {
public:
	~WorkFileSystemHelper();

	bool active() const { return true; }

	std::string createNewDataFolder( std::string const & walletName );
	std::string adDataFolder( std::string const & walletDataPath, std::string const & walletName );
	tWalletDescr activeWallet() const;
	std::list< tWalletDescr > localWallets() const;
	bool changeCurrentWallet( int n );
	void popBackWallet();

	bool changeCurrentWallet( int n, Config & commonConfig );

	std::list< tWalletDescr > localWallets( Config const & commonConfig ) const;

	// + path to local folder
	std::string fullFileName(std::string const & shortFName) const;

	// + path to common folder
	std::string fileNameInCommon(std::string const & shortFName) const;
	std::string adDataFolderInternal( std::string const & walletDataPath, std::string const & walletName );

	// create new folder in CSTR_COMMON_USER_DATA_NAME_PARAM
	// and set it as active
	std::string createNewDataFolder( std::string const & walletName, Config & commonConfig );

	// first = name
	// second = [ath
	tWalletDescr activeWallet( Config const & commonConfig ) const;

	void freeLoggers();
	void loadUserConfigs();
	void setupLogs();

	void popBackWallet( Config & commonConfig );

	// path to the swiftcoin.conf
	std::string confPath() const;
	std::string commonConfPath() const;
	bool checkWalletPath( std::string walletDataPath );

	bool walletDataFolderMade() const;
	void walletDataFolderMade( bool newVal);

	void init( std::string const & confPath );

	// only for swstarter
//	static FileSystemHelper * instance(std::string const & confPath = "" );

	Config *config();
	Config *localConfig();
	Config *commonConfig();
	Config const *commonConfig() const;
	Config *dbKeyConfig();

	Logger * logger() const;
	Logger * userLogger() const;
	Logger * errLogger() const;
	CalLoger * calLogger() const;

	DateTime currentGMT();
	void currentGMT(DateTime const & dtGMT);

	std::string socketMode() const;

	WorkFileSystemHelper();

	void loadCommonConfigs( std::string const & confPath );

	void setConfPath( std::string const & confPath );
	std::string confFName() const;

	// config is swiftcoin.conf
	void setCommonConfPath( Config const & config );
	std::string commonConfFName( Config const & config ) const;

	// commonConfig - common.conf
	void setLocalPath( Config const & commonConfig );
	std::string localConfPath() const;
	// config is swiftcoin.conf
	std::string localConfFName( Config const & config ) const;

	int pushBackWallet( std::string const & walletDataPath, std::string const & walletName, Config & commonConfig );

	void setProxyMode();

	Config _config;
	Config _localConfig;
	Config _dbKeyConfig;
	Config _commonUserConfig;

	Logger *_logger;
	Logger *_userLogger;
	Logger *_errLogger;
	CalLoger *_calLogger;

	int pushBackWalletNoCheck( std::string const & walletDataPath, std::string const & walletName, Config & commonConfig );

	std::string _confPath;										// path to swiftcoin.conf
	std::string _commonConfPath;								// path to common.conf
	std::string _localConfPath;								// // path to folder with lsw.conf, consists DB folder
	bool _walletDataFolderMade;

	// difference between GMT and local time
	int _diffGMT;
	bool _isGMTSeted;

	std::string _socketMode;
};

FileSystemHelper * FileSystemHelper::instance(std::string const & confPath ) {
	tScoped_lock scopedLock(_createMutex);
	if( g_released ) {
		return &g_emptyInst;
	}
	if( !g_inst ){
		g_inst = new WorkFileSystemHelper();
		//init application
		g_inst->init( confPath );
	}
	return g_inst;
}
string FileSystemHelper::createNewUniqFolder( std::string const & existFolderPath ) {
	string ret = existFolderPath;
	while( fileExist( ret ) ) {
		ret += SC::RndGen::code16( 1 );
	}
	return ret;
}

WorkFileSystemHelper::WorkFileSystemHelper() : _logger( NULL ), _userLogger(NULL), _errLogger( NULL ), _calLogger( NULL ),
	_confPath( "" ), _commonConfPath( "" ), _localConfPath( "" ), _walletDataFolderMade( false ), _isGMTSeted(false), _socketMode("NoProxy") {}

WorkFileSystemHelper::~WorkFileSystemHelper() {
	tScoped_lock scopedLock(_createMutex);
	g_released = true;
	freeLoggers();
	if( _calLogger ) {

		delete _calLogger;
		_calLogger = NULL;
	}
	SPtr::shared<int>::shutDown();
	g_inst = NULL;
}
void WorkFileSystemHelper::freeLoggers() {
	if(_logger) {
		delete _logger;
		_logger = NULL;
	}
	if(_userLogger) {
		delete _userLogger;
		_userLogger = NULL;
	}
	if(_errLogger) {
		delete _errLogger;
		_errLogger = NULL;
	}
}
static string fixPath( string path ) {
	if( path.size() ) {
		path = RTrimStr( path, "\"" );
		path = LTrimStr( path, "\"" );
		path = RTrimStr( path, "\\" );
		path = RTrimStr( path, CSTR_PATH_DELIM );
		path += CSTR_PATH_DELIM;
	}
	return path;
}

void WorkFileSystemHelper::init( std::string const & confPath ) {
	loadCommonConfigs( confPath );
	loadUserConfigs();
	//update configurations using data from ./update.dat

#ifndef NO_UPDATER
	ConfUpdater::update();
#endif
	loadCommonConfigs( confPath );
	loadUserConfigs();
	setupLogs();
	setProxyMode();
	_calLogger = new MemCallStack();
	_diffGMT = 0;
}
void WorkFileSystemHelper::loadCommonConfigs( std::string const & confPath ) {
//cout << "Application::loadCommonConfigs 1 confPath=" << confPath << endl;
	setConfPath( confPath );
	if( !_config.load( confFName() )) {
		cout << "WorkFileSystemHelper::loadCommonConfigs Error load main config in file:" << confFName() << endl;
	}
	setCommonConfPath( _config );
	if( !_commonUserConfig.load( commonConfFName( _config ) ) ) {
		cout << "WorkFileSystemHelper::loadCommonConfigs Error load common config !" << endl;
	}
}
void WorkFileSystemHelper::loadUserConfigs() {
//cout << "Application::loadUserConfigs 1" << endl;
	setLocalPath( _commonUserConfig );
	_localConfig.clear();
	_localConfig.load( localConfFName( _config ) );
}

void WorkFileSystemHelper::setupLogs() {
	string logFName = fullFileName(_config.value(CSTR_CONFIG_APP_LOG_NAME_PARAM));
//cout << "Application::setupLogs() logFName=" << logFName << "\n";

	string groups = _config.value(CSTR_CONFIG_LOG_GROUPS_NAME_PARAM, "NoGroup");
	string userLogFName = fullFileName(_config.value(CSTR_CONFIG_USER_LOG_NAME_PARAM));
	string errorLogFName = fullFileName(_config.value(CSTR_CONFIG_ERROR_LOG_NAME_PARAM));

	if( _config.boolValue(CSTR_CONF_CAN_SEND_LOG_NAME_PARAM, false ) ) {
		//max amount of log records
		int maxLogs = toInt( _config.value(CSTR_CONF_MAX_SENDING_LOGS_NAME_PARAM, "256" ) );
		_logger = new PassLogger( logFName, maxLogs );
		_userLogger = new PassLogger( userLogFName, maxLogs );
		_errLogger = new PassLogger( errorLogFName, maxLogs );
	} else {
		_logger = new WorkLogger( logFName );
		_userLogger = new WorkLogger( userLogFName );
		_errLogger = new WorkLogger( errorLogFName );
	}

	_logger->groups(groups);
	if( _config.exist(CSTR_CONFIG_LOG_EXCEPTIONS_NAME_PARAM) ) {
		_logger->exceptions( _config.value(CSTR_CONFIG_LOG_EXCEPTIONS_NAME_PARAM) );
	}

	if(_config.exist(CSTR_CONFIG_LOG_2COUT_NAME_PARAM)){
		_logger->Repeat2Cout(_config.value(CSTR_CONFIG_LOG_2COUT_NAME_PARAM));
	}

	_userLogger->groups( CSTR_LOG_ALL );
	_errLogger->groups( CSTR_LOG_ALL );
}

void WorkFileSystemHelper::setConfPath( std::string const & confPath ) {
	string lconfPath = confPath;
	string dataPath;
#ifdef LINUX
	if( confPath.empty() ) {
		string confFile = CSTR_CONFIG_FILENAME;
		// if there is config file near the bin - lets use it
		if( !fileExist( confFile ) ) {
			// use shared config from /user/share/sw
			lconfPath = CSTR_SW_SHARED_FOLDER;
		} else {
			lconfPath = "./";
		}
	}
#endif
	_confPath = fixPath( lconfPath );
}
string WorkFileSystemHelper::confPath() const {
	return _confPath;
}
string WorkFileSystemHelper::confFName() const {
	return confPath() + CSTR_CONFIG_FILENAME;
}

void WorkFileSystemHelper::setCommonConfPath( Config const & config ) {
//cout << "WorkFileSystemHelper::setCommonConfPath 1" << endl;
	string path = config.value( CSTR_COMMON_USER_DATA_NAME_PARAM, "" );

	_commonConfPath = path;
	if( path.empty() ) {
		_commonConfPath = ".";
		string confFile = config.value( CSTR_COMMON_CONFIG_NAME_PARAM, CSTR_COMMON_CONFIG_DEFAULT );
		if( !fileExist( confFile ) ) {
#ifdef LINUX
			path = fixPath( getenv( "HOME" ) );
			if( path.empty() ) {
				throw Exception("WorkFileSystemHelper::setCommonConfPath - Error There is no home folder in user's environment", "WorkFileSystemHelper::commonConfFName");
			}
			_commonConfPath = fixPath( path + CSTR_SW_COMMON_DATA_FOLDER );
			if( !fileExist( _commonConfPath ) ) {
				// create common folder
				string cmd = CSTR_CD_CMD + path;
				if( system( cmd.c_str() ) ) {
					cout << "WorkFileSystemHelper::setCommonConfPath, File=" << __FILE__<< ", Line=" << __LINE__ << ". Error execution command=" << cmd << endl;
					return;
				}

				createDirectory( _commonConfPath );
				// check .sw folder it will copy conf1 if there is .sw
				string oldDataPath = path + CSTR_SW_OLD_DATA_FOLDER;
				string fromConfFile = confFile;
				bool oldConf = false;
				if( fileExist( oldDataPath ) ) {
//cout << "WorkFileSystemHelper::setCommonConfPath IS old SW" << endl;
					fromConfFile = CSTR_COMMON_OLD_CONFIG_DEFAULT;
					oldConf = true;
				} else {
					fromConfFile = CSTR_COMMON_CONFIG_DEFAULT;
				}
				cmd = CSTR_COPY_FILE_CMD CSTR_SW_SHARED_FOLDER CSTR_PATH_DELIM + fromConfFile;
				cmd += " " + _commonConfPath + confFile;

				if( system( cmd.c_str() ) ) {
					cout << "WorkFileSystemHelper::setCommonConfPath, File=" << __FILE__<< ", Line=" << __LINE__ << ". Error execution command=" << cmd << endl;
					return;
				}

				Config config;
				config.load( _commonConfPath + confFile );

				if( oldConf ) {
					pushBackWalletNoCheck( oldDataPath, "Main", config );
					changeCurrentWallet(1, config);
				} else {
					createNewDataFolder( "Main", config );
					_walletDataFolderMade = true;
				}
			}
#endif
		}
	}
	_commonConfPath = fixPath( _commonConfPath );
}
bool WorkFileSystemHelper::walletDataFolderMade() const {
	return _walletDataFolderMade;
}
void WorkFileSystemHelper::walletDataFolderMade( bool newVal) {
	_walletDataFolderMade = newVal;
}
string WorkFileSystemHelper::commonConfPath() const {
	return _commonConfPath;
}
string WorkFileSystemHelper::commonConfFName( Config const & config ) const {
	string confFile = config.value( CSTR_COMMON_CONFIG_NAME_PARAM, CSTR_COMMON_CONFIG_DEFAULT );
	return commonConfPath() + confFile;
}
void WorkFileSystemHelper::setLocalPath( Config const & commonConfig ) {
//cout << "WorkFileSystemHelper::setLocalPath 1" << endl;
	_localConfPath = commonConfig.value( CSTR_ACTIVE_WALLET_PATH_PARAM, "" );
	if( _localConfPath.empty() ) {
#ifdef LINUX
		_localConfPath = createNewDataFolder( "Main", (Config &) commonConfig );
#endif
	}
	if( _localConfPath.empty() ) {
		throw Exception("WorkFileSystemHelper::commonConfFName - Error There is no home folder in user's environment", "WorkFileSystemHelper::commonConfFName");
	}
	_localConfPath = fixPath( _localConfPath );
}
string WorkFileSystemHelper::localConfPath() const {
	return _localConfPath;
}
string WorkFileSystemHelper::localConfFName( Config const & config ) const {
	string confFile = config.value( CSTR_LOCAL_CONFIG_FILE_NAME_PARAM, CSTR_LOCAL_CONFIG_DEFAULT );
	return localConfPath() + confFile;
}
string WorkFileSystemHelper::fullFileName( string const & shortFName ) const {
	return _localConfPath + shortFName;
}
string WorkFileSystemHelper::fileNameInCommon( string const & shortFName ) const {
	return _commonConfPath + shortFName;
}
bool WorkFileSystemHelper::changeCurrentWallet( int n, Config & commonConfig ) {
	bool ret = false;
	// validate n
	int wAmnt = toInt( commonConfig.value(CSTR_WALLETS_AMOUNT_NAME_PARAM, "0") );
	if( n < 1 || n > wAmnt ) {
		return ret;
	}
	string wNumber = toString( n );
	string wPath = commonConfig.value( CSTR_WALLET_PATH_NAME_PARAM + wNumber, "" );
	string originWName = commonConfig.value( CSTR_WALLET_NAME_NAME_PARAM + wNumber, "" );
	string wName = Code64::text2Bin( originWName );
	if( wPath.empty() || originWName.empty() ) {
		return ret;
	}
	commonConfig.saveValue( CSTR_ACTIVE_WALLET_PATH_PARAM, wPath );
	commonConfig.saveValue( CSTR_ACTIVE_WALLET_NAME_PARAM, originWName );
	_localConfPath = fixPath( wPath );
	return true;
}
int WorkFileSystemHelper::pushBackWallet( string const & walletDataPath, string const & walletName, Config & commonConfig ) {
//cout << "WorkFileSystemHelper::pushBackWallet 1 walletDataPath=" << walletDataPath << endl;
	if( ! checkWalletPath( walletDataPath ) ) {
		return 0;
	}
	return pushBackWalletNoCheck( walletDataPath, walletName, commonConfig );
}
int WorkFileSystemHelper::pushBackWalletNoCheck( string const & walletDataPath, string const & walletName, Config & commonConfig ) {
	int wAmnt = toInt( commonConfig.value(CSTR_WALLETS_AMOUNT_NAME_PARAM, "0") );
	std::list< tWalletDescr > wDescrs = localWallets( commonConfig );
	BOOST_FOREACH( tWalletDescr wDescr, wDescrs ) {
		if( walletName == wDescr.first ) {
			return 0;
		}
	}
	string wNumber = toString( ++wAmnt );
	commonConfig.saveValue( CSTR_WALLET_NAME_NAME_PARAM + wNumber, Code64::bin2Text( walletName ) );
	commonConfig.saveValue( CSTR_WALLET_PATH_NAME_PARAM + wNumber, walletDataPath );
	commonConfig.saveValue( CSTR_WALLETS_AMOUNT_NAME_PARAM, wNumber );
	return wAmnt;
}
void WorkFileSystemHelper::popBackWallet( Config & commonConfig ) {
	int wAmnt = toInt( commonConfig.value(CSTR_WALLETS_AMOUNT_NAME_PARAM ) ) - 1;
	string wNumber = toString( wAmnt );
	commonConfig.saveValue( CSTR_WALLETS_AMOUNT_NAME_PARAM, wNumber );
	// set active wallet
	// default is first wallet
	if( wAmnt > 0 ) {
		changeCurrentWallet( 1, commonConfig );
	}
}
list< tWalletDescr > WorkFileSystemHelper::localWallets( Config const & commonConfig ) const {
	std::list< tWalletDescr > ret;
	int wAmnt = toInt( commonConfig.value(CSTR_WALLETS_AMOUNT_NAME_PARAM, "0") );
	for( int i = 1; i <= wAmnt; ++i) {
		string path = commonConfig.value( CSTR_WALLET_PATH_NAME_PARAM + toString(i), "" );
		string name = Code64::text2Bin( commonConfig.value( CSTR_WALLET_NAME_NAME_PARAM + toString(i), "" ) );
		if( path.size() && name.size() ) {
			ret.push_back( tWalletDescr( name, path ) );
		}
	}
	return ret;
}
// first = name
// second = path
tWalletDescr WorkFileSystemHelper::activeWallet( Config const & commonConfig ) const {
	tWalletDescr ret;
	ret.first  = Code64::text2Bin( commonConfig.value( CSTR_ACTIVE_WALLET_NAME_PARAM, "" ) );
	ret.second = commonConfig.value( CSTR_ACTIVE_WALLET_PATH_PARAM, "" );
	return ret;
}
bool WorkFileSystemHelper::checkWalletPath( std::string walletDataPath ) {
	walletDataPath = RTrimStr( walletDataPath, CSTR_PATH_DELIM );
//	boost::system::error_code ec;
	bool ret = fileExist( walletDataPath );
	string lFName = walletDataPath + CSTR_PATH_DELIM + config()->value( CSTR_LOCAL_CONFIG_FILE_NAME_PARAM );
	ret = ret && fileExist( lFName );
	lFName = walletDataPath + CSTR_PATH_DELIM + config()->value( CSTR_ENCRYPT_DB_KEY_NAME_PARAM );
	ret = ret && fileExist( lFName );
	return ret;
}
std::string WorkFileSystemHelper::createNewDataFolder( std::string const & walletName, Config & commonConfig ) {
//cout << "WorkFileSystemHelper::createNewDataFolder 1 walletName=" << walletName << endl;

	string quoter;
#ifdef _WIN32_
	quoter = "\"";
#endif
	string walletPath = createNewUniqFolder( commonConfPath() );
//cout << "commonConfPath()=" << commonConfPath() << endl;

	string templPath = commonConfPath() + CSTR_INSTALLATION_FOLDER;

	if( !fileExist(templPath) ) {
		templPath = ".";
#ifdef LINUX
		templPath = CSTR_SW_SHARED_FOLDER CSTR_PATH_DELIM CSTR_INSTALLATION_FOLDER;
#endif
	}

//cout << "templPath=" << templPath << endl;

//	string templPath = boost::filesystem::current_path().string();
//	if( templPath.empty() ) {
//		templPath = ".";
//#ifdef LINUX
//		templPath = CSTR_SW_SHARED_FOLDER;
//#endif
//	}
//cout << "WorkFileSystemHelper::createNewDataFolder 2 walletPath=" << walletPath << ", templPath=" << templPath << endl;
	// check the existance of application folder
	if( !fileExist( walletPath ) ) {
		string cmd = CSTR_CREATE_FOLDER_CMD " " + quoter + walletPath + quoter;
//cout << "WorkFileSystemHelper::createNewDataFolder 3 cmd=" << cmd << endl;
		if( system( cmd.c_str() ) ) {
//cout << "WorkFileSystemHelper::createNewDataFolder 4" << endl;
			std::stringstream sout;
			sout << "createWalletFolder, File=" << __FILE__<< ", Line=" << __LINE__ - 2 << ". Error execution command=" << cmd << endl;
			logger()->log( sout.str(), "createWalletFolder", CSTR_LOG_CRITICAL);
			cout << sout.str();
		}
		cmd = CSTR_COPY_FILE_CMD " " + quoter + templPath;
		cmd += CSTR_PATH_DELIM "data" CSTR_PATH_DELIM "*" + quoter + " " + quoter + walletPath + quoter;
//cout << "WorkFileSystemHelper::createNewDataFolder 5 cmd=" << cmd << endl;
		if( system( cmd.c_str() ) ) {
			std::stringstream sout;
			sout << "createWalletFolder, File=" << __FILE__<< ", Line=" << __LINE__ - 2 << ". Error execution command=" << cmd << endl;
			logger()->log( sout.str(), "createWalletFolder", CSTR_LOG_CRITICAL);
			cout << sout.str();
		}
		int n = pushBackWallet( walletPath, walletName, commonConfig);
//cout << "WorkFileSystemHelper::createNewDataFolder 6 n=" << n << endl;
		if( n > 0 && changeCurrentWallet( n, commonConfig) ){
			return walletPath;
		}
	}
//cout << "WorkFileSystemHelper::createNewDataFolder 7" << endl;
	return "";
}
string WorkFileSystemHelper::adDataFolderInternal( string const & walletDataPath, string const & walletName ) {
	if( !fileExist( walletDataPath ) ) {
		return "";
	}
	if( !checkWalletPath( walletDataPath ) ) {
		return "";
	}
//	SC::Config & commonConfig = *commonConfig();
	int nWallet = pushBackWallet( walletDataPath, walletName, *commonConfig() );
	if( !nWallet ) {
		return "";
	}
	if( ! changeCurrentWallet( nWallet, *commonConfig() ) ) {
		popBackWallet( *commonConfig() );
		return "";
	}
	return walletDataPath;
}

Config * WorkFileSystemHelper::config() {
	return &_config;
}
Config * WorkFileSystemHelper::localConfig() {
	return &_localConfig;
}
Config * WorkFileSystemHelper::commonConfig() {
	return &_commonUserConfig;
}
Config const *WorkFileSystemHelper::commonConfig() const {
	return &_commonUserConfig;
}
Config * WorkFileSystemHelper::dbKeyConfig() {
	return &_dbKeyConfig;
}

string WorkFileSystemHelper::createNewDataFolder( std::string const &) {
	return "";
}
string WorkFileSystemHelper::adDataFolder( std::string const &, std::string const &) {
	return "";
}
tWalletDescr WorkFileSystemHelper::activeWallet() const {
	return activeWallet( *commonConfig() );
}
list< tWalletDescr > WorkFileSystemHelper::localWallets( ) const {
	return localWallets( *commonConfig() );
}
bool WorkFileSystemHelper::changeCurrentWallet( int ) {
	return false;
}
void WorkFileSystemHelper::popBackWallet( ) {
	popBackWallet( *commonConfig() );
}
Logger *WorkFileSystemHelper::logger() const {
	return _logger;
}
Logger *WorkFileSystemHelper::userLogger() const {
	return _userLogger;
}
Logger *WorkFileSystemHelper::errLogger() const {
	return _errLogger;
}
CalLoger *WorkFileSystemHelper::calLogger() const {
	return _calLogger;
}
DateTime WorkFileSystemHelper::currentGMT() {
//cout << "Application::currentGMT 1" << endl;
	tScoped_lock scopedLock(_timeGMTMutex);
	if( !_isGMTSeted ) {
		string fname = fullFileName(config()->value(CSTR_GMT_STORE_PARAM));
		std::ifstream in(fname.c_str());
		if( in ) {
			string line;
			std::getline(in, line);
			line = AllTrim( line );

			DateTime dt;
			dt.load(line);

			int diff = DateTime::Current().diffSecs(dt);
			if( diff < 3600 * 24 ) {
				in >> _diffGMT;
				_isGMTSeted = true;
			}else{
				_diffGMT = 0;
			}
		}else{
			_diffGMT = 0;
		}
	}
	DateTime dt = DateTime::Current();
	dt.addSeconds(_diffGMT);
	return dt;
}
void WorkFileSystemHelper::currentGMT(DateTime const & dtGMT) {
	if( !dtGMT ) {
		return;
	}
	tScoped_lock scopedLock(_timeGMTMutex);
	int newDiff = dtGMT.diffSecs( DateTime::Current() );
	if(_isGMTSeted && abs( newDiff - _diffGMT ) < 2 ) {
		return;
	}
	_isGMTSeted = true;
	_diffGMT = newDiff;

	string fname = fullFileName(config()->value(CSTR_GMT_STORE_PARAM));
	std::ofstream out(fname.c_str());
	if( out ) {
		out << DateTime::Current().data() << std::endl;
		out << _diffGMT << std::endl;
	}
}
string WorkFileSystemHelper::socketMode() const {
	return _socketMode;
}
void WorkFileSystemHelper::setProxyMode() {
	_socketMode = CSTR_NOPROXY_FACTORY_ID;
	if( commonConfig()->exist(CSTR_PROXY_MODE_PARAM)){
		_socketMode = commonConfig()->value(CSTR_PROXY_MODE_PARAM);
	}
}



//