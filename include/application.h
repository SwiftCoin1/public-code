#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <string>
#include <utility>
#include <list>

#include "config.h"
#include "sptr.h"
#include "ihashcipher.h"
#include "version.h"
#include "pprovider.h"
#include "money.h"
#include "error.h"
#include "datetime.h"
#include "sstring.h"
#include "keys.h"

#include "msghandler.h"
#include "netaddress.h"

namespace SC{

//class Wallet;
class Logger;
class CalLoger;
class BlockChain;

namespace Models {
	class Txs;
}

typedef std::pair< std::string, std::string > tWalletDescr;
class FileSystemHelper {
public:
	virtual ~FileSystemHelper() {}

	virtual bool active() const = 0;

	static FileSystemHelper * instance(std::string const & confPath = "" );
	static std::string createNewUniqFolder( std::string const & existFolderPath );

	virtual std::string createNewDataFolder( std::string const & ) = 0;
	virtual std::string adDataFolder( std::string const &, std::string const & ) = 0;
	virtual tWalletDescr activeWallet() const = 0;
	virtual std::list< tWalletDescr > localWallets() const = 0;
	virtual bool changeCurrentWallet( int ) = 0;
	virtual void popBackWallet() = 0;
	virtual bool changeCurrentWallet( int, Config & ) = 0;
	virtual std::list< tWalletDescr > localWallets( Config const & ) const = 0;
	virtual std::string fullFileName(std::string const & ) const = 0;
	virtual std::string fileNameInCommon(std::string const &) const = 0;
	virtual std::string adDataFolderInternal( std::string const &, std::string const & ) = 0;
	virtual std::string createNewDataFolder( std::string const &, Config & ) = 0;
	virtual tWalletDescr activeWallet( Config const & ) const = 0;
	virtual void freeLoggers() = 0;
	virtual void loadUserConfigs() = 0;
	virtual void setupLogs() = 0;
	virtual void popBackWallet( Config & ) = 0;
	virtual std::string confPath() const = 0;
	virtual std::string commonConfPath() const = 0;
	virtual bool checkWalletPath( std::string ) = 0;
	virtual bool walletDataFolderMade() const = 0;
	virtual void walletDataFolderMade( bool) = 0;
	virtual void init( std::string const & ) = 0;

	virtual Config *config() = 0;
	virtual Config *localConfig() = 0;
	virtual Config *commonConfig() = 0;
	virtual Config const *commonConfig() const = 0;
	virtual Config *dbKeyConfig() = 0;
	virtual Logger * logger() const = 0;
	virtual Logger * userLogger() const = 0;
	virtual Logger * errLogger() const = 0;
	virtual CalLoger * calLogger() const = 0;

	virtual DateTime currentGMT() = 0;
	virtual void currentGMT(DateTime const & ) = 0;
	virtual std::string socketMode() const = 0;
};

class Application {
public:
	virtual ~Application() {}

	static Application * instance(std::string const & confPath = "", bool isEw = false );
	virtual char * dumpBuf() const = 0;

	virtual bool active() const = 0;

	virtual void init( std::string const &, bool ) = 0;

	virtual Config const * config() const = 0;
	virtual Config const * localConfig() const = 0;
	virtual Config const * commonConfig() const = 0;
	virtual SC::Models::Txs *modelTxs() const = 0;
	virtual Logger * logger() const = 0;
	virtual Logger * userLogger() const = 0;
	virtual Logger * errLogger() const = 0;
	virtual CalLoger * calLogger() const = 0;

	virtual std::string applicationUId() = 0;

	virtual void addHandler(const std::string &, MsgHandler *) = 0;
	virtual void eraseHandler(const std::string &) = 0;

	virtual void processMsg(Message const &) = 0;
	virtual std::string processMsgWithResult(Message const &) = 0;
	virtual DateTime currentGMT() = 0;
	virtual void currentGMT(DateTime const &) = 0;
	virtual NetAddress ownAddress() const = 0;
	virtual void ownAddress(NetAddress const &) = 0;
	virtual bool isActiveNode() const = 0;
	virtual void isActiveNode(bool) = 0;
	virtual void checkServerMode(bool) = 0;
	virtual bool checkServerMode() const = 0;
	virtual void checkDbEncrypted() = 0;
	virtual bool isDbEncrypted() const = 0;
	virtual sstring dbKey() const = 0;
	virtual Error setDbEncrypted( sstring const & ) = 0;
	virtual Error openDbKey( sstring const & ) = 0;
	virtual Error checkDbPw( sstring const & ) const = 0;
	virtual bool isDbKeyOpened() const = 0;
	virtual Error changeDbPassWord( sstring const &, sstring const & ) = 0;
	virtual bool isDb() const = 0;
	virtual Error freeDb() = 0;
	virtual Error initDb() = 0;
	virtual Error checkDbEncKey( sstring ) const = 0;
	virtual Error setDbEncKey( sstring const & ) = 0;
	virtual sstring encDbKey( sstring const & ) = 0;
	virtual std::string getStat() const = 0;
	virtual std::string createNewDataFolder( std::string const & ) = 0;
	virtual std::string adDataFolder( std::string const &, std::string const & ) = 0;
	virtual bool changeCurrentWallet( int ) = 0;
	virtual tWalletDescr activeWallet() const = 0;
	virtual std::list< tWalletDescr > localWallets() const = 0;
	virtual void popBackWallet() = 0;
};


}; // SC
#endif
