#ifndef _THIN_WALLET_HPP
#define _THIN_WALLET_HPP

#include <list>
#include <map>
#include <string>

#include "sptr.h"
//#include "strepository.h"
#include "money.h"
#include "error.h"
#include "datetime.h"
#include "txthininfo.h"
#include "walletkey.h"
#include "walletaccount.h"
#include "tax.h"
#include "sbinfo.h"
#include "network.h"

namespace SC{

class NetAddress;

namespace Thin {

class Mobile {
protected:
	// wallet id
	std::string _id;

	SC::DateTime _lastUpdate;
	SC::DateTime _lastAccUpdate;

	SPtr::shared<WalletKey> _wkey;

	std::list<Thin::TxInfo> _inTxs;
	std::list<Thin::TxInfo> _outTxs;

	TxInfo _pendingTx;
	SBInfo _pendingSb;

	int _pendingSbUpdates;

	std::list<WalletAccount> _accs;
	std::list<Tax> _taxes;

	std::list<SBInfo> _sbs;

	bool _updatedOk;
	std::string _txStr;
	mutable volatile bool *_needStop;

	std::map<std::string, SC::NetAddress > _servers;

	Error getAccounts( NetAddress const & adr );
	virtual Error getTxs( NetAddress const & adr );
	Error getTaxes( NetAddress const & adr );

	Error getSbs( NetAddress const & adr );

	std::string data() const;

	Error getTx( std::list<TxInfo> & list, SC::Thin::TxInfo &txinfo, NetAddress const & adr, NetworkBase & net, std::string const & txId ) const;

public:
	Mobile( std::string const & wid );
	virtual ~Mobile();

	// sum of money in accounts
	Money money() const;
	// money() - fees() - pending()
	Money availableMoney() const;

	SC::Money fees() const;
	SC::Money pending() const;

	void servers( const std::map<std::string, SC::NetAddress> & adrs );

	// get data about wallet: accounts, transactions
	SC::Error update( std::string const & pw );

	// send keys to server
	static SC::Error passKeys( NetAddress const & adr, std::string const & wId, volatile bool * needStop = NULL );
	SC::Error passKeys( std::string const & recWId );
	// send stored transaction to server
	SC::Error sendTx( NetAddress const & adr );

	// create transaction and store in _pendingTx
	SC::Error createTx( SC::Money m, std::string const & recWId, std::string const & descr, std::string const & pw, const std::list<SC::Attachment> & as );

	// get string for transaction
	SC::Error txString( std::string & outStr, SC::Money m, std::string const & recWId, std::string const & descr, std::string const & pw, const std::list<SC::Attachment> & as );
	SC::Error txString( std::string & outStr, SBInfo const & sb, std::string const & recWId, std::string const & descr, std::string const & pw );

	//create transaction using specified accounts for SB
	SC::Error sendSB( std::string const & recWId, const std::list<WalletAccount> & wacc2Process, SBInfo const & sb, std::string const & pw );

	bool isTx();
	void clearTx();

	SC::DateTime lastUpdateTime() const;
	void lastUpdateTime( const SC::DateTime & dt );

	SPtr::shared<SC::ASKeys> key();
	void key( SPtr::shared<SC::ASKeys> k );
	// the same as public key id
	std::string id() const;
	void id( std::string const & wId );

	std::list<SC::WalletAccount> const &walletAccounts() const;

	std::list<TxInfo> const &inTxs() const;
	std::list<TxInfo> const &outTxs() const;

	Error deleteOuTx( const std::string & txId );
	// check in out transaction and in pending
	bool hasOuTx( const std::string & txId ) const;

	std::list< SBInfo > sbs() const;

	TxInfo pendingTx() const;
	void pendingTx( const TxInfo & tx );
	void checkAndClearPendingTx( const TxInfo & tx );

	SBInfo pendingSb() const;
	void pendingSb( const SBInfo & sb );
	void checkAndClearPendingSb( const SBInfo & sb );
	void clearPendingSb();

	bool checkStop() const;
	void setStop( volatile bool * a );
};

class Wallet : public Mobile {
	static std::string getDbName();
	Error getTxs( NetAddress const & adr );
public:
	Wallet( std::string const & wid );
	virtual ~Wallet();

	// get and send keys for sender and receiver
	SC::Error passKeys( NetAddress const & adr, std::string const & sendWId, std::string const & recWId );

	bool canDoEmission( std::string & eaccId ) const;

	void walletAccounts( const std::list<SC::WalletAccount> & a );

	void inTxs( const std::list<TxInfo> & a );
	void outTxs( const std::list<TxInfo> & a );
	void taxes( const std::list<SC::Tax> & a );

	Error save();
	Error load( const std::string & data );

	// all wallet's ids in local storage
	static std::list<std::string> wallets();
	// get wallet by wid
	static SPtr::shared<Wallet> wallet( std::string const & wId );
	// create new wallet and set up password for key and send to servers
	static SPtr::shared<Wallet> create( std::string const & pw );
	// save wallet to local storage
	static Error save( SPtr::shared<Wallet> w );

	static Error erase( SPtr::shared<Wallet> w );
	static Error erase( std::string const & wId );

	static Error send( NetAddress const & adr, SPtr::shared<Wallet> w, volatile bool *needStop );

	void attachmentChanged( std::string const & attId );
};

}
}

#endif // _THIN_WALLET_HPP
