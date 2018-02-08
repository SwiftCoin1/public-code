#include <iostream>

#include "consts.h"
#include "thinwallet.h"
#include "wkeyrepository.h"
#include "application.h"
#include "log.h"
#include "logcalls.h"

#include "waccrepository.h"
#include "strepository.h"
#include "config.h"
#include "rndgen.h"
#include "network.h"
//#include "descrepository.h"
#include "attachrepository.h"
#include "strutils.h"
#include "code64.h"
#include "attachment.h"
#include "pprovider.h"

#include <boost/foreach.hpp>

using namespace SC::Thin;

using std::cout;
using std::endl;
using std::string;
using std::list;
using std::map;
using SC::Thin::Wallet;
using SC::Money;
using SC::DateTime;
using SC::TimeInterval;
using SC::Error;
using SC::Config;
using SC::ASKeys;
using SC::Application;
using SC::FileSystemHelper;
using SC::RndGen;
using SC::WalletKey;
using SC::WalletKeyRepository;
using SC::NetAddress;
using SC::NetworkBase;
using SC::SerData;
using SC::WalletAccount;
using SC::Version;
using SC::Description;
using SC::OneTimeProvider;
using SC::ISign;
using SC::Tax;
//using SC::DescRepository;
using SC::SBInfo;
using SC::Attachment;
using SC::PProvider;

#define CSTR_FIELD_DELIMETER									"%"
#define CSTR_RECORD_DELIMETER									"$"

Wallet::Wallet( std::string const & wid ) : Mobile( wid ) {}
Wallet::~Wallet() {}

SC::Error Wallet::passKeys( NetAddress const & adr, std::string const & sendWId, std::string const & recWId ) {
	Error err;
	NetworkBase net;
	net.setStop( _needStop );

	// check recWId
	WalletKeyRepository wr;
	if( !wr.find(recWId) ) {
		WalletKey wk( recWId );
		err = net.getWkey(adr, recWId, wk);
		if( err ) {
			wr.save( wk );
		} else {
			return err;
		}
	}

	if( !net.hasWKey( adr, recWId, err ) ) {
		wr.find( recWId );
		WalletKeyRepository::tType wk = wr.value();
		err = net.sendWkey( adr, *wk );
		if( !err ) {
			return Error( "Error - could not send receiver's wallet keys to server", "Wallet::passKeys" );
		}
	}

	if( !net.hasWKey( adr, sendWId, err) ) {
		wr.find( sendWId );
		WalletKeyRepository::tType wk = wr.value();
		err = net.sendWkey( adr, *wk );
		if( !err ) {
			return Error( "Error - could not send sender's wallet keys to server", "Wallet::passKeys" );
		}
	}
	return err;
}
void updateAttachment( list<SC::Thin::TxInfo>::iterator begin, list<SC::Thin::TxInfo>::iterator end, string const & attId ){
	for( list<SC::Thin::TxInfo>::iterator i = begin; i != end; ++i ) {
		list<Attachment> atts = i->attachments();
		BOOST_FOREACH( Attachment att, atts ) {
			if( att.id() == attId ) {
				i->loadAttachmentFromRepository();
				return;
			}
		}
	}
}
void Wallet::attachmentChanged( string const & attId ) {
	updateAttachment( _inTxs.begin(), _inTxs.end(), attId );
	updateAttachment( _outTxs.begin(), _outTxs.end(), attId );
}
static string genWName() {
	Config const *conf = SC::FileSystemHelper::instance()->config();
	string oformat = string(CINT_WALLET_NAME_LEN, 'P');
	if(conf->exist(CSTR_CONFIG_WALLET_ID_FORMAT_PARAM)){
		oformat = conf->value(CSTR_CONFIG_WALLET_ID_FORMAT_PARAM);
	}
	string iformat = CSTR_DEFAULT_FORMAT + string(CINT_WALLET_NAME_LEN, 'P');
	string keys = "YMDHISOP";
	string input = DateTime::Current().data() + (string)RndGen::code16(CINT_WALLET_NAME_LEN / 2);
	return sTransform(input, iformat, oformat, keys);
}
Error createKey( WalletKey & outWKey, string const & pw ) {
	Error err;
	SC::PProvider::tProviderPtr pr( new SC::OneTimeProvider( pw ) );
	string wid = genWName();
	outWKey.id( wid );
	err = PProvider::pwProvider(pr, wid);
	if( !err ) {
		return err;
	}

	WalletKeyRepository wkRep;
	WalletKeyRepository::tType wk = wkRep.make( wid );
	outWKey = *wk;
	return err;
}
// all wallet's ids in local storage
list<string> Wallet::wallets() {
	StringRepository storedWallets( getDbName() );
	list<string> ret;
	StringRepository::tIter it = storedWallets.iterator();
	it->first();
	while(!it->isEnd()){
		ret.push_back( it->key() );
		it->next();
	}
	return ret;
}

string Wallet::getDbName() {
	string dbName = CSTR_THIN_WALLET_REPOSITORY_DBNAME;
	Config const * conf = SC::FileSystemHelper::instance()->config();

	if( conf->exist(dbName) ) {
		dbName = conf->value(dbName);
	}
	return dbName;
}

// get wallet by wid
// static method
SPtr::shared<Wallet> Wallet::wallet( string const & wId ) {
//cout << "Wallet::wallet wId=" << wId << endl;
	StringRepository storedWallets( getDbName() );
	SPtr::shared<Wallet> w;
	if( storedWallets.find( wId ) ) {
		w = new Wallet( wId );
		WalletKeyRepository wkRep;
		if( !wkRep.find( wId ) ) {
			return SPtr::shared<Wallet>();
		}
		w->_wkey = wkRep.value();
		string data = storedWallets.getString();
		Error err = w->load( data );
		if( !err ) {
		}
	}
	return w;
}

	// create new wallet and set up password for key
SPtr::shared<Wallet> Wallet::create( string const & pw ) {
	SPtr::shared<SC::WalletKey> wk( new WalletKey( "" ) );
	Error err = createKey( *wk, pw );
	SPtr::shared<Wallet> ret;
	if( err ) {
		ret = new Wallet( wk->id() );
		ret->_wkey = wk;
	}
	return ret;
}
// save wallet to local storage
Error Wallet::save() {
	StringRepository storedWallets( getDbName() );
//cout << "Wallet::save() id()=" << id() << ", data().size()=" << data().size() << endl;
	storedWallets.saveAsKey( id(), data() );

	WalletKeyRepository wkRep;
	if( !wkRep.find( id() ) ) {
		if( !wkRep.save( _wkey ) ) {
			storedWallets.erase( id() );
			return Error( "Could not save wallet key to repository", "Wallet::save" );
		}
	} else {
		SPtr::shared<SC::WalletKey> wkey = wkRep.value();
		if( wkey->keys()->data() != _wkey->keys()->data() ) {
			return Error( "Wrong key for wallet id=" + id(), "Wallet::save" );
		}
	}

	return Error();
}
Error Wallet::load( const std::string & data ) {
//cout << "Wallet::load() id()=" << id() << ", data().size()=" << data.size() << endl;
	_updatedOk = false;

	list<string> strs = split( data, CSTR_RECORD_DELIMETER );
	list<string>::const_iterator i = strs.begin();
	if( i != strs.end() ) {
		_lastUpdate.load( *i );
	}
	++i;

	if( i != strs.end() ) {
		AttachRepository atr;
		list<string> strs = split( *i, CSTR_FIELD_DELIMETER );
		BOOST_FOREACH( string txStr, strs ) {
			if( txStr.empty() ) {
				continue;
			}
			TxInfo tx;
			if( tx.Serializable::load( txStr ) ) {
				list< Attachment > atts;
				BOOST_FOREACH( Attachment a, tx.attachments() ) {
					if( atr.find( a.id() ) ) {
						atts.push_back( *(atr.value()) );
					}
				}
				tx.attachments( atts );
				_inTxs.push_back( tx );
			} else {
				return Error( "Can't load input transaction's data=" + txStr, "Thin::Wallet::load" );
			}
		}
	}
	++i;

	if( i != strs.end() ) {
		list<string> strs = split( *i, CSTR_FIELD_DELIMETER );
		BOOST_FOREACH( string txStr, strs ) {
			if( txStr.empty() ) {
				continue;
			}
			TxInfo tx;
			if( tx.Serializable::load( txStr ) ) {
				_outTxs.push_back( tx );
			} else {
				return Error( "Can't load output transaction's data=" + txStr, "Thin::Wallet::load" );
			}
		}
	}
	++i;

	//walletaccount
	if( i != strs.end() ) {
		list<string> strs = split( *i, CSTR_FIELD_DELIMETER );
		BOOST_FOREACH( string txStr, strs ) {
			if( txStr.empty() ) {
				continue;
			}
			WalletAccount acc;
			if( acc.Serializable::load( txStr ) ) {
				_accs.push_back( acc );
			} else {
				return Error( "Can't load wallet's account data=" + txStr, "Thin::Wallet::load" );
			}
		}
	}
	++i;

	//tax
	if( i != strs.end() ) {
		list<string> strs = split( *i, CSTR_FIELD_DELIMETER );
		BOOST_FOREACH( string txStr, strs ) {
			if( txStr.empty() ) {
				continue;
			}
			Tax t;
			if( t.Serializable::load( txStr ) ) {
				_taxes.push_back( t );
			} else {
				return Error( "Can't load tax's data=" + txStr, "Thin::Wallet::load" );
			}
		}
	}
	++i;

	//SBInfo
	if( i != strs.end() ) {
		list<string> strs = split( *i, CSTR_FIELD_DELIMETER );
		BOOST_FOREACH( string sbStr, strs ) {
			if( sbStr.empty() ) {
				continue;
			}
			SBInfo v;
			if( v.Serializable::load( sbStr ) ) {
				_sbs.push_back( v );
			} else {
				return Error( "Can't load SBInfo's data=" + sbStr, "Thin::Wallet::load" );
			}
		}
	}
	++i;

	if( i != strs.end() ) {
		if( !_pendingTx.Serializable::load( *i ) ) {
			return Error( "Can't load pending transaction's data=" + *i, "Thin::Wallet::load" );
		}
	}
	++i;
	//load completed transaction into _txStr
//cout << "Wallet::load 1 id=" << id() << endl;
	if( i != strs.end() ) {
		_txStr = *i;
		++i;
		if( i != strs.end() ) {
			if( !_pendingSb.Serializable::load( *i ) ) {
				return Error( "Can't load pending SB's data=" + *i, "Thin::Wallet::load" );
			}
			if( _pendingSb.valid() ) {
				_pendingSbUpdates = 0;
			}
		}
	}

	_updatedOk = true;
	return Error();
}
Error Wallet::save( SPtr::shared<Wallet> w ) {
	if( !w ) {
		return Error( "Input wallet is NULL", "Wallet::save" );
	}
	return w->save();
}
Error Wallet::send( NetAddress const & adr, SPtr::shared<Wallet> w, volatile bool *needStop ) {
	if( !w || !w->_wkey ) {
		return Error( "Input wallet is NULL", "Wallet::send" );
	}
	NetworkBase net;
	net.setStop( needStop );

	Error err = net.sendWkey( adr, *w->_wkey);
	if( !err ) {
		return Error( "Error is in net.sendWkey(), err.text()=" + err.text(), "Wallet::send" );
	}
	return Error();
}
Error Wallet::erase( SPtr::shared<Wallet> w ) {
	if( !w ) {
		return Error( "Input wallet is NULL", "Wallet::save" );
	}
	return erase( w->id() );
}
Error Wallet::erase( std::string const & wId ) {
	StringRepository storedWallets( getDbName() );
	storedWallets.erase( wId );
	return Error();
}
Error Wallet::getTxs( NetAddress const & adr ) {
	NetworkBase net;
	net.setStop( _needStop );

	Error err;
	std::set< string > inIds, outIds;

	BOOST_FOREACH( SC::Thin::TxInfo tx, _inTxs ) {
		inIds.insert( tx.id() );
	}
	BOOST_FOREACH( SC::Thin::TxInfo tx, _outTxs ) {
		outIds.insert( tx.id() );
	}

	AttachRepository atr;

	for( list<WalletAccount>::const_iterator i = _accs.begin(); i != _accs.end(); ++i ) {
		// eleminate output transactions (return remainder from closed account)
		if( checkStop() ) {
			return Error( "Stoping signal", "Wallet::getTxs");
		}
		TxInfo tx;
//		if( !i->isReturn() && !i->isEmission() && !i->isSB() ) {
		if( !i->isReturn() && !i->isEmission() ) {
			if( !inIds.count( i->inTx() ) ) {
				inIds.insert( i->inTx() );
				err = net.getTxInfo( adr, i->inTx(), tx );
				if( !err ) {
					return err;
				}

				list< Attachment > atts;
				BOOST_FOREACH( Attachment att, tx.attachments() ) {
					if( !atr.find( att.id() ) ) {
						//save attachments
						atr.save( att );
						atts.push_back( att );
					} else {
						atts.push_back( *(atr.value()) );
					}
				}
				tx.attachments( atts );
				_inTxs.push_back( tx );
			}
		}
		// outgoing txs
		if( i->outTx().size() ) {
			if( !outIds.count( i->outTx() ) ) {
				outIds.insert( i->outTx() );
				err = getTx( _outTxs, tx, adr, net, i->outTx() );
				if( !err ) {
					return err;
				}
				//save attachments
				AttachRepository atr;
				BOOST_FOREACH( Attachment att, tx.attachments() ) {
					if( !atr.find( att.id() ) ) {
						atr.save( att );
					}
				}
				checkAndClearPendingTx( tx );
			}
		}
		if( i->isEmission() ) {
			if( !_pendingTx.empty() && !outIds.count( _pendingTx.id() ) ) {
				outIds.insert( _pendingTx.id() );
				err = getTx( _outTxs, tx, adr, net, _pendingTx.id() );
				if( !err ) {
					return err;
				}
				//save attachments
				AttachRepository atr;
				BOOST_FOREACH( Attachment att, tx.attachments() ) {
					if( !atr.find( att.id() ) ) {
						atr.save( att );
					}
				}
				checkAndClearPendingTx( tx );
			}
		}
	}
	return err;
}
bool Wallet::canDoEmission( std::string & eaccId ) const {
	BOOST_FOREACH( WalletAccount a, _accs ) {
		if( a.isEmission() ) {
			eaccId = a.accId();
			return true;
		}
	}
	return false;
}
void Wallet::walletAccounts( const std::list<SC::WalletAccount> & a ) {
	_accs = a;
}
void Wallet::inTxs( const std::list<SC::Thin::TxInfo> & a ) {
	_inTxs = a;
}
void Wallet::outTxs( const std::list<SC::Thin::TxInfo> & a ) {
	_outTxs = a;
}
void Wallet::taxes( const std::list<SC::Tax> & a ) {
	_taxes = a;
}


//

