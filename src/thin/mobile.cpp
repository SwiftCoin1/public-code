#include <iostream>

#include "consts.h"
#include "thinwallet.h"
#include "wkeyrepository.h"
#include "application.h"
#include "log.h"
#include "logcalls.h"

#include "config.h"
#include "rndgen.h"
#include "network.h"
#include "walletaccount.h"
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
using SC::Money;
using SC::DateTime;
using SC::TimeInterval;
using SC::Error;
using SC::Config;
using SC::ASKeys;
using SC::FileSystemHelper;
using SC::RndGen;
using SC::WalletKey;
using SC::WalletKeyRepository;
using SC::NetAddress;
using SC::NetworkBase;
using SC::SerData;
using SC::Version;
using SC::Description;
using SC::OneTimeProvider;
using SC::ISign;
using SC::Tax;
using SC::SBInfo;
using SC::Attachment;
using SC::PProvider;
using SC::WalletAccount;

#define CSTR_FIELD_DELIMETER									"%"
#define CSTR_RECORD_DELIMETER									"$"

Mobile::Mobile( string const & wid ) : _id(wid), _lastUpdate(DateTime().oldest()), _lastAccUpdate(DateTime().oldest()), _pendingSbUpdates(0), _updatedOk( false ), _needStop(NULL) { }
Mobile::~Mobile() {}

Money Mobile::money() const {
	Money amnt;
	for( list<WalletAccount>::const_iterator j = _accs.begin(); j != _accs.end(); ++j) {
		if( j->isActual() ) {
			amnt = amnt + j->money();
		}
	}
	return amnt;
}
// only free acoount's sum - without pending outgoing transactions
Money Mobile::availableMoney() const {
	return money() - fees();
}
// sum of money in accounts
Money Mobile::fees() const {
	// fee calculate
	Money fee;
	for( list<WalletAccount>::const_iterator i = _accs.begin(); i != _accs.end(); ++i) {
		if( !i->isActual() ) {
			continue;
		}
		fee = Money();
		DateTime cGMT = FileSystemHelper::instance()->currentGMT();
		for( list<Tax>::const_iterator j = _taxes.begin(); j != _taxes.end(); ++j ) {
			fee = fee + j->fee( *i, cGMT );
		}
	}
	return fee;
}
SC::Money Mobile::pending() const {
	if( !_pendingTx.empty() ) {
		return _pendingTx.money();
	}
	return SC::Money();
}
DateTime Mobile::lastUpdateTime() const {
	return _lastUpdate;
}
void Mobile::lastUpdateTime( const SC::DateTime & dt ) {
	_lastUpdate = dt;
}
SPtr::shared<ASKeys> Mobile::key() {
	if(_wkey) {
		return _wkey->keys();
	}
	return SPtr::shared<ASKeys>();
}
	// the same as public key id
string Mobile::id() const {
	return _id;
}
void Mobile::id( std::string const & wId ) {
	_id = wId;
}
string Mobile::data() const {
	string res;
	res =	_lastUpdate.data() + CSTR_RECORD_DELIMETER + 
			toString( _inTxs.begin(), _inTxs.end(), &ser2str<TxInfo>, CSTR_FIELD_DELIMETER ) + CSTR_RECORD_DELIMETER +
			toString( _outTxs.begin(), _outTxs.end(), &ser2str<TxInfo>, CSTR_FIELD_DELIMETER ) + CSTR_RECORD_DELIMETER +
			toString( _accs.begin(), _accs.end(), &ser2str<WalletAccount>, CSTR_FIELD_DELIMETER ) + CSTR_RECORD_DELIMETER +
			toString( _taxes.begin(), _taxes.end(), &ser2str<Tax>, CSTR_FIELD_DELIMETER ) + CSTR_RECORD_DELIMETER +
			toString( _sbs.begin(), _sbs.end(), &ser2str<SBInfo>, CSTR_FIELD_DELIMETER ) + CSTR_RECORD_DELIMETER +
			_pendingTx.data() + CSTR_RECORD_DELIMETER +
			_txStr;
	if( _pendingSb.valid() ) {
		res += CSTR_RECORD_DELIMETER + _pendingSb.data();
	}

	return res;
}
Error Mobile::getAccounts( NetAddress const & adr ) {
	NetworkBase net;
	net.setStop( _needStop );

	Error err;
	DateTime curDt = FileSystemHelper::instance()->currentGMT();
	curDt.addSeconds(3600 * 24);

	// need use getWAccCnt4Dates to find out all new accounts
	Network::tWAccounts accs = net.getWAccounts(adr, _id, DateTime().oldest(), curDt, 0, 0, err);
	if( !err && err.code() != Error::NoData ) {
		return err;
	}
//cout << "Wallet::getAccounts id()=" << id() << ", accs.size()=" << accs.size() << endl;
	_lastAccUpdate = FileSystemHelper::instance()->currentGMT();
	_accs = accs;
	return Error();
}
Error Mobile::getTx( list<TxInfo> & list, TxInfo &txinfo, NetAddress const & adr, NetworkBase & net, string const & txId ) const {
	Error err = net.getTxInfo(adr, txId, txinfo);
	if( err ) {
		list.push_back(txinfo);
	}
	return err;
}
Error Mobile::getTxs( NetAddress const & adr ) {
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
/*				AttachRepository atr;
				BOOST_FOREACH( Attachment att, tx.attachments() ) {
					if( !atr.find( att.id() ) ) {
						atr.save( att );
					}
				}
*/
				checkAndClearPendingTx( tx );
			}
		}
	}
	return err;
}
Error Mobile::getTaxes( NetAddress const & adr ) {
	NetworkBase net;
	net.setStop( _needStop );

	Error err;
	list<Tax> taxes;
	err = net.getTaxes(adr, taxes);
	if( !err ){
		return err;
	}
	_taxes = taxes;
	return Error();
}
Error Mobile::getSbs( NetAddress const & adr ) {
//cout << "Wallet::getSbs 1 id()=" << id() << endl;
	NetworkBase net;
	net.setStop( _needStop );

	++_pendingSbUpdates;

	Error err;
	list<SBInfo> sbs;
	err = net.getSBs( adr, _id, sbs );
	if( !err ){
//cout << "Wallet::getSbs 2 err.text()=" << err.text() << endl;
		return err;
	}
	_sbs.clear();

	BOOST_FOREACH( SBInfo sb, sbs ) {
		// copy only actual SBs
		if( sb.childSb().empty() ) {
			_sbs.push_back( sb );
		}
		checkAndClearPendingSb( sb );
//cout << "Wallet::getSbs 3 sb.amount()=" << sb.amount().toUser() << endl;
//sb.dump( cout );
	}
	return Error();
}
void Mobile::servers( const map<string, NetAddress> & adrs ) {
	_servers = adrs;
}
Error Mobile::update( std::string const & pw ) {
	string fName = "Wallet::update";

	NetAddress wServ = _servers[ CSTR_WORK_SERV ];
	NetAddress sbServ = _servers[ CSTR_SB_SERV ];

	// need to ping first
	NetworkBase net;
	net.setStop( _needStop );
//cout << "Wallet::update 1 id()=" << id() << endl;
	Error err;
	if( !net.pingAddr( wServ, err) && err ){
		return Error("Error in ping comand, err.text()=" + err.text(), fName);
	}
	PProvider::tProviderPtr pr( new OneTimeProvider( pw ) );
	PProvider::pwProvider( pr, _id );
//cout << "Wallet::update 1 wid=" << _id << endl;
	err = getAccounts( wServ );
	if( err ) {
//cout << "Wallet::update accounts - OK" << endl;
		if( checkStop() ) {
			return Error( "Stoping signal", fName );
		}
		err = getTxs( wServ );
		if( err ) {
			if( checkStop() ) {
				return Error( "Stoping signal", fName );
			}
//cout << "Wallet::update txs - OK" << endl;
			err = getTaxes( wServ );
			if( err ) {
				if( checkStop() ) {
					return Error( "Stoping signal", fName );
				}
//cout << "Wallet::update taxes - OK" << endl;
				if( sbServ.IsValid() ) {
					err = getSbs( sbServ );
					if( err ) {
//cout << "Wallet::update sbs - OK" << endl;
						_lastUpdate = FileSystemHelper::instance()->currentGMT();
						_updatedOk = true;
					}
				}
			}
		}
	}
	if( isTx() ) {
		SC::Thin::TxInfo tx = pendingTx();
		if( !tx.empty() ) {
			NetAddress wServ = _servers[ CSTR_WORK_SERV ];
			if( checkStop() ) 
				return Error( "Stoping signal", fName );
			passKeys( wServ, tx.receiver(), _needStop );
			err = sendTx( wServ );
		}
	}

	if( !err ) {
//		cout << "Wallet::update err.text()=" << err.text() << endl;
	}
	return err;
}

struct ThinDebit {
	WalletAccount _wa;
	Money _fee;
	Money _possible;
	Money _payment;
	Money _return;
	string _retId;
};

struct ThinTx{
	SerData _needSign;
	SerData _data;

	string _crOper;
	list<ThinDebit> _dopers;

	string _txId;
	string _sendWId;
	DateTime _dtGMT;

	list<SC::Attachment> _atts;
	SC::Thin::TxInfo _tx;

	bool _encDescr;

	ThinTx() {
		_encDescr = true;
		_dtGMT = FileSystemHelper::instance()->currentGMT();
		_txId = _dtGMT.data("yyyymmddhh") + (string)RndGen::code64(24);

		Version version;
		version.Major(CINT_TX_MAJOR_VERSION);
		version.Minor(CINT_TX_MINOR_VERSION);

		_needSign.add( _txId ).add( version.data() ).add( _dtGMT.data() );

		_tx.id( _txId );
		_tx.creationDT( _dtGMT );
		_data.add( CSTR_TX_TYPEID ).add( _txId ).add( version.data() ).add( _dtGMT.data() );
	}
	void attachments( const list<SC::Attachment> & atts ) {
		_atts = atts;
		_tx.attachments( atts );

		_data.add( toString( (int)atts.size() ) );

		BOOST_FOREACH( SC::Attachment a, atts ) {
			_data.add( a.data() );
		}
	}
	void sender( const string & wId ) {
		_sendWId = wId;
		_tx.sender( wId );
	}
	Error adDebit( list<ThinDebit> & dopers ) {
		_dopers = dopers;
		// add return credit operation 
		for( list<ThinDebit>::iterator j = dopers.begin(); j != dopers.end(); ++j ) {
			// check if return > 0
			if( j->_return > Money() ) {
				Error err = addCredit2SD( _needSign, j->_wa.walletId(), j->_return, "", j->_retId, CSTR_RETURN_OPER_TID);
				if( !err )
					return err;
				addCredit2SD( _data, j->_wa.walletId(), j->_return, "", j->_retId, CSTR_RETURN_OPER_TID);
			}
		}
		Money fee;
		// add fees
		for( list<ThinDebit>::iterator j = dopers.begin(); j != dopers.end(); ++j ) {
			// check if fee > 0
			if( j->_fee > Money() ) {
				Error err = addFee2SD( _needSign, j->_fee, j->_wa);
				if( !err )
					return err;
				addFee2SD( _data, j->_fee, j->_wa);
				fee = fee + j->_fee;
			}
		}
		_tx.fee( fee );
		// add debit operation to _needSign
		for( list<ThinDebit>::iterator j = dopers.begin(); j != dopers.end(); ++j ) {
			addDebit2SD( _needSign, j->_wa.money(), j->_wa );
		}
		return Error();
	}
	Error addFee2SD( SerData & cd, Money m, WalletAccount const & wa ) {
		cd.add( CSTR_FEE_TID ).add( wa.accId() ).add( m.data() );
		return Error();
	}
	Error addDebit2SD( SerData & cd, Money m, WalletAccount const & wa ) {
		cd.add( CSTR_DEBIT_OPER_TID ).add( m.data() ).add( CSTR_ACCOUNT_TYPEID ).
			add( wa.accId() ).add( wa.walletId() ).add( wa.inBlock() ).add( wa.CreationTime().data() ).add( wa.money().data() );
		return Error();
	}
	Error encryptDescr( string const & recWId, string const & descr, string & res ) {
		string fName = "ThinTx::encryptDescr";
		res = "";
		WalletKeyRepository wr;
		if( !wr.find(recWId) ) {
			return Error("ThinTx::encryptDescr Receiver's wallet is not in repository, wallet id=" + recWId, fName );
		}
		WalletKeyRepository::tType wk = wr.value();
		SPtr::shared<ASKeys> pubKey = wk->keys()->clonePub();

		Description idescr;
		if( descr.size() ) {

			if( _sendWId.empty() ) {
				return Error("ThinTx::encryptDescr Sender's wallet name is empty", fName );
			}

			if( !wr.find( _sendWId ) ) {
				return Error("ThinTx::encryptDescr Sender's wallet is not in repository, wallet id=" + _sendWId, fName );
			}
			//use one wallet to get its private key to decrypt description
			// creating secret
			string secret = _dtGMT.data();
			// get sender private key
			SPtr::shared<ASKeys> privSenderKey = WalletKeyRepository::key( _sendWId );
			if( privSenderKey && privSenderKey->priv() ) {
				SC::PProvider::tProviderPtr pr = SC::PProvider::pwProvider( _sendWId );
				if( !pr ) {
					return Error("ThinTx::encryptDescr Password provider for wallet=" + _sendWId + " is wrong", fName );
				}
				if( !privSenderKey->priv()->isOpen() ) {
					privSenderKey->priv()->open( *pr );
				}
				secret += privSenderKey->priv()->key();
			} else {
				return Error("ThinTx::encryptDescr Sender's private key is wrong", fName );
			}
			secret = privSenderKey->hasher()->hash(secret).data();
			// use secret in description
			idescr.text(descr, *pubKey, secret);
			_tx.addDescription( idescr );
			res = idescr.data();
		}
		return Error();
	}
	string operId() {
		DateTime dtGMT = FileSystemHelper::instance()->currentGMT();
		return dtGMT.data("yyyymmddhh") + (string)RndGen::code64(12);
	}

	Error addCredit2SD( SerData & cd, string const & recWId, Money m, string const & descr, string & opId, 
								string const & operTId = CSTR_CREDIT_OPER_TID, const DateTime & start = DateTime::oldest() ){
		string fName = "ThinTx::addCredit2SD";
		if( opId.empty() ) {
			opId = operId();
		}

		string encDescr;
		if( descr.size() ) {
			if( _encDescr ) {
				Error err = encryptDescr( recWId, descr, encDescr );
				if( !err )
					return err;
			} else {
				encDescr = SC::Code64::bin2Text( descr );
			}
		}

		WalletKeyRepository wr;
		if( !wr.find(recWId) ) {
			return Error("ThinTx::addCredit2SD Receiver's wallet is not in repository, wallet id=" + recWId, fName );
		}
		WalletKeyRepository::tType wk = wr.value();
		SPtr::shared<ASKeys> pubKey = wk->keys()->clonePub();

		cd.add( operTId ).add( opId ).add( encDescr ).add( recWId ).add( pubKey->data() ).add( m.data() );

		if( start != DateTime::oldest() ) {
			cd.add( CSTR_CREDIT_START_FLAG ).add( start.data() );
		}
		return Error();
	}
	Error addCredit( string const & recWId, Money m, string const & descr ) {
		SerData cd;
		string retId;
		Error err = addCredit2SD(cd, recWId, m, descr, retId);
		if( err ) {
			_crOper = cd.data();
			_needSign.add(_crOper);
			_data.add(_crOper);

			_tx.receiver( recWId );
			_tx.money( m );
		}
		return err;
	}
	Error addCredits( string const & recWId, const list<WalletAccount> & wacc2Process, SBInfo const & sb ) {
		Error err;
		SerData sd;

		SBInfo sbTmp = sb;
		SBInfo newSb = SC::SbCreator::create( sbTmp, recWId );
		// create new accounts and save them in SB
		list<string> newAccIds;
		for( int i = 0; i < (int)wacc2Process.size(); ++i ) {
			newAccIds.push_back( operId() );
		}

		newSb.accIds( newAccIds );

		SC::Code64 coder;
		string descr = CSTR_SB_SEND_DESCR_PREF ":";
		descr += coder.Bin2Text( newSb.data() );

		list<string>::const_iterator accI = newAccIds.begin();
		BOOST_FOREACH( WalletAccount wa, wacc2Process ) {
			string retId = *accI;
			err = addCredit2SD( sd, recWId, wa.money(), descr, retId, CSTR_CREDIT_OPER_TID, wa.start() );
			if( !err ) {
				return err;
			}
			descr = "";
			++accI;
		}
		_crOper = sd.data();
		_needSign.add( _crOper );
		_data.add( _crOper );

		_tx.receiver( recWId );
		_tx.money( newSb.payments() );

		return Error();
	}
	string data() {
		return _data.data();
	}
	// key - is full key
	Error sign( SPtr::shared<ASKeys> key, string const & pw) {
		OneTimeProvider pr(pw);
		ISign _sign = key->sign( _needSign.data(), pr );
		if( !key->checkSign(_needSign.data(), _sign) ) {
			return Error("Wrong sign", "sign");
		}
		for( list<ThinDebit>::iterator j = _dopers.begin(); j != _dopers.end(); ++j ) {
			addDebit2SD( _data, j->_wa.money(), j->_wa );
			_data.add( _sign.data() );
		}
		return Error();
	}
	string id() {
		return _txId;
	}
	SC::Thin::TxInfo tx() {
		return _tx;
	}
};
static list<ThinDebit> calcDebOpers( list<WalletAccount> const & waccs, list<Tax> const & taxes, Money const & payment, const SC::DateTime & atDate ) {
//cout << "calcDebOpers 1 atDate=" << atDate.data() << ", payment=" << payment.toUser() << endl;
	list<ThinDebit> ret;
	Money needPay = payment;

	for( list<WalletAccount>::const_iterator i = waccs.begin(); i != waccs.end(); ++i) {
		if( !i->isActualOnDate( atDate ) ) {
			continue;
		}
		if( !i->isEmission() ) {
			Money fee = Money();
			for( list<Tax>::const_iterator j = taxes.begin(); j != taxes.end(); ++j ) {
				fee = fee + j->fee(*i, FileSystemHelper::instance()->currentGMT() );
			}
			ThinDebit d;
			d._wa = *i;
			d._fee = fee;
			d._possible = d._wa.money() - fee;
			if( d._possible > Money() ) {
				if( d._possible > needPay ) {
					d._payment = needPay;
					d._return = d._possible - needPay;
					needPay = Money( 0 );
				} else {
					d._payment = d._possible;
					d._return = Money( 0 );
					needPay = needPay - d._payment;
				}
			}
			ret.push_back( d );
			if( needPay == Money() ) {
				break;
			}
		} else {
		// emission account
			ret.clear();
			ThinDebit d;
			d._wa = *i;
			d._wa.money( needPay );
			d._fee = Money( 0 );
			d._possible = needPay;
			d._payment = needPay;
			d._return = Money( 0 );

			ret.push_back( d );
			return ret;
		}
	}
	if( needPay > Money() ) {
		return list<ThinDebit>();
	}
	return ret;
}
// create new transaction in thin client
static Error createTransaction( string & outTxStr,
								SC::Thin::TxInfo & outTx,
								list<WalletAccount> const & waccs,
								list<Tax> const & taxes,
								Money m,
								string const & sendWId,
								string const & recWId,
								string const & descr,
								SPtr::shared<ASKeys> key,
								string const & pw,
								list<Attachment> const & atts,
								SC::DateTime const & atDate ) {
//cout << "createTransaction 1" << endl;
	string fName = "createTransaction";
	CallsPusher cp( fName );
	addLineCallStack(__LINE__);

	// create debit, fee and return data
	list<ThinDebit> opers = calcDebOpers( waccs, taxes, m, atDate );

	addLineCallStack(__LINE__);

	if( opers.empty() ) {
		addLineCallStack(__LINE__);
		// fixed text for Daniel's request at 20140929'
//		return Error("There is no enough money in wallet", "createTransaction");
		return Error("Insufficient funds", fName );
	}

	ThinTx ttx;
	addLineCallStack(__LINE__);

	// add attachments 
	// must be here ! (acording to order from transaction.data() )
	ttx.attachments( atts );

	ttx.sender( sendWId );
	addLineCallStack(__LINE__);
	//add credit oper
	Error err = ttx.addCredit( recWId, m, descr );
	addLineCallStack(__LINE__);
	if( err ) {
		addLineCallStack(__LINE__);
		//add debit opers
		err = ttx.adDebit( opers );
		addLineCallStack(__LINE__);
		if( err ) {
			addLineCallStack(__LINE__);
			//create signs
			err = ttx.sign( key, pw );
			addLineCallStack(__LINE__);
			if( err ) {
				addLineCallStack(__LINE__);
				// get tx string
				outTxStr = ttx.data();
				//get pending tx
				outTx = ttx.tx();
			}
		}
	}
	addLineCallStack(__LINE__);
	return err;
}
// create transaction and store in _pendingTx
Error Mobile::createTx( Money m, string const & recWId, string const & descr, string const & pw, const list<Attachment> & as ) {
	string fName = "Wallet::txString";
	CallsPusher cp( fName );

	Error err;

	if( !_pendingTx.empty() ) {
		return Error( "There is pending transaction", fName );
	}

	PProvider::tProviderPtr pr( new OneTimeProvider( pw ) );
	PProvider::pwProvider( pr, _id );

	if( !_wkey ) {
		return Error( "_wkey is NULL", fName );
	}

	err = createTransaction( _txStr, _pendingTx, _accs, _taxes, m, _id, recWId, descr, _wkey->keys(), pw, as, FileSystemHelper::instance()->currentGMT() );

	//key is opened so we need to reload closed key
	WalletKeyRepository wr;
	if( wr.find( id( )) ) {
		key( wr.value()->keys() );
	} else {
		clearTx();
		return Error( "Can't load closed keys for wallet", fName );
	}

	if( !err ) {
		clearTx();
		return err;
	}

	//w commetnted 20150910
//	LocalDescription ldescr( _pendingTx.id() );
//	ldescr.text( descr );

//	DescRepository dr;
//	if( !dr.save( ldescr ) ) {
//		return Error("Could not save description in repository", fName );
//	}
	_lastUpdate = DateTime::oldest();
	return err;
}
Error Mobile::txString( string & outStr, SC::Money m, std::string const & recWId, std::string const & descr, std::string const & pw, const list<Attachment> & as ) {
	string fName = "Wallet::txString";
	CallsPusher cp( fName );
//cout << "Wallet::txString 1" << endl;
	Error err;

	if( !_pendingTx.empty() ) {
		return Error( "There is pending transaction", fName );
	}
	PProvider::tProviderPtr pr( new OneTimeProvider( pw ) );
	PProvider::pwProvider( pr, _id );

	addLineCallStack(__LINE__);

	TxInfo tmp;
	if( !_wkey ) {
		return Error( "_wkey is NULL", fName );
	}
	_wkey->keys();
	err = createTransaction( outStr, tmp, _accs, _taxes, m, _id, recWId, descr, _wkey->keys(), pw, as, FileSystemHelper::instance()->currentGMT() );

	addLineCallStack(__LINE__);
	//key is opened so we need to reload closed key
	WalletKeyRepository wr;
	if( wr.find( id( )) ) {
		key( wr.value()->keys() );
	} else {
		return Error( "Can't load closed keys for wallet", fName );
	}
	addLineCallStack(__LINE__);
	if( !err ) {
		return err;
	}
	addLineCallStack(__LINE__);
	return err;
}
Error Mobile::txString( string & outStr, SBInfo const & sb, string const & recWId, string const & descr, string const & pw ) {
	string fName = "Wallet::txString for SB";
	CallsPusher cp( fName );
//cout << fName << " 1" << endl;
	if( !sb.valid() ) {
		return Error( "Checking SB is invalid", fName );
	}

	if( !sb.isProcessed() ) {
		return Error( "Checking SB is not processed", fName );
	}

	Error err;
	if( !_pendingTx.empty() ) {
		return Error( "There is pending transaction", fName );
	}
	PProvider::tProviderPtr pr( new OneTimeProvider( pw ) );
	PProvider::pwProvider( pr, _id );

	TxInfo tmp;
	if( !_wkey ) {
		return Error( "_wkey is NULL", fName );
	}
	_wkey->keys();

	std::list<WalletAccount> accs;
	SC::Money m = zeroCoin;
	list< string > sbaccs = sb.accIds();

	DateTime curTime = FileSystemHelper::instance()->currentGMT();
	BOOST_FOREACH( WalletAccount a, _accs ) {
		// select accounts in future only
		if( find( sbaccs.begin(), sbaccs.end(), a.accId() ) != sbaccs.end() ) {
			DateTime start = a.start();
			start.setEnDay();
			if( a.outTx().empty() && start > curTime ) {
				m = m + a.money();
				accs.push_back( a );
			}
		}
	}

	DateTime sbEndDate = sb.stop();
	sbEndDate.addDays( 1 );

	list<Tax> taxes; // empty taxes for acciunts in future;
	list<Attachment> as;
	err = createTransaction( outStr, tmp, accs, taxes, m, _id, recWId, descr, _wkey->keys(), pw, as, sbEndDate );
	//key is opened so we need to reload closed key
	
	WalletKeyRepository wr;
	if( wr.find( id( )) ) {
		key( wr.value()->keys() );
	} else {
		return Error( "Can't load closed keys for wallet", fName );
	}

	if( !err ) {
		return err;
	}
//	LocalDescription ldescr( _pendingTx.id() );
//	ldescr.text( descr );
	return err;
}
SC::Error Mobile::sendSB( string const & recWId, const list<WalletAccount> & wacc2Process, SBInfo const & sb, string const & pw ) {
//cout << "Wallet::sendSB 1 this=" << this << endl;
	string fName = "Wallet::sendSB";
	Error err;
	if( !wacc2Process.size() ) {
		return Error( "there are no coins to transfer", fName );
	}

	list<ThinDebit> debits;
	BOOST_FOREACH( WalletAccount wa, wacc2Process ) {
		ThinDebit d;
		d._wa = wa;
		d._fee = zeroCoin;
		d._possible = wa.money();
		d._payment = wa.money();
		d._return = zeroCoin;
		debits.push_back( d );
	}

	if( debits.empty() ) {
		return Error( "There is no enough money in wallet", fName );
	}
	PProvider::tProviderPtr pr( new OneTimeProvider( pw ) );
	PProvider::pwProvider( pr, _id );

	ThinTx ttx;
	ttx._encDescr = false;
	ttx.attachments( list<Attachment>() );
	ttx.sender( _id );

	//add credit oper
	err = ttx.addCredits( recWId, wacc2Process, sb );
	if( !err ) {
		clearTx();
		return err;
	}
	if( err ) {
		//add debit opers
		err = ttx.adDebit( debits );
		if( err ) {
			//create signs
			err = ttx.sign( _wkey->keys(), pw );
			if( err ) {
				// get tx string
				_txStr = ttx.data();
				//get pending tx
				_pendingTx = ttx.tx();
				if( _pendingTx.empty() ) {
//					cout << "Wallet::sendSB _pendingTx is empty" << endl;
				}
				_pendingTx.sbId( sb.id() );
			}
		}
	}

	WalletKeyRepository wr;
	if( wr.find( id() )) {
		key( wr.value()->keys() );
	} else {
		err = Error( "Can't load closed keys for wallet", fName );
	}

	if( !err ) {
		clearTx();
		return err;
	}

	_lastUpdate = DateTime::oldest();
	return err;
}
Error Mobile::sendTx( NetAddress const & adr ) {
	if( !isTx() || _pendingTx.empty() ) {
		return Error( "There is no complited transaction", "Thin::Wallet::sendTx" );
	}
	NetworkBase net;
	net.setStop( _needStop );
	Error err = net.sendTxStr( adr, _pendingTx.id(), _txStr, "2" );
	if( err ) {
		_lastUpdate = DateTime::oldest();
	}
	return err;
}
bool Mobile::isTx() {
	return _txStr.size();
}
void Mobile::clearTx() {
	_pendingTx.clear();
	_txStr.clear();
	clearPendingSb();
}
void Mobile::clearPendingSb() {
	if( _pendingSb.valid() ) {
		list<SBInfo>::iterator i = _sbs.begin();
		for(; i != _sbs.end(); ++i) {
			if( *i == _pendingSb ) {
				_sbs.erase( i );
				break;
			}
		}
		_pendingSb = SBInfo();
	}
}
list<WalletAccount> const & Mobile::walletAccounts() const {
	return _accs;
}
list<SC::Thin::TxInfo> const & Mobile::inTxs() const {
	return _inTxs;
}
list<SC::Thin::TxInfo> const & Mobile::outTxs() const {
	return _outTxs;
}
Error Mobile::deleteOuTx( const std::string & txId ) {
	for( list<SC::Thin::TxInfo>::iterator i = _outTxs.begin(); i != _outTxs.end(); ++i ) {
		if( i->id() == txId ) {
			if( i->isProcessed() ) {
				return Error( "Can't delete processed transaction", "Wallet::deleteOuTx" );
			}
			_outTxs.erase( i );
			break;
		}
	}
	return Error();
}
bool Mobile::hasOuTx( const std::string & txId ) const {
	if( _pendingTx.id() == txId )
		return true;
	BOOST_FOREACH( SC::Thin::TxInfo tx, _outTxs ) {
		if( tx.id() == txId ) {
			return true;
		}
	}
	return false;
}
list< SBInfo > Mobile::sbs() const {
	list< SBInfo > res = _sbs;
	if( pendingSb().valid() ) {
		res.push_front( pendingSb() );
	}
	return res;
}
SC::Thin::TxInfo Mobile::pendingTx() const {
	return _pendingTx;
}
void Mobile::pendingTx( const SC::Thin::TxInfo & tx ) {
	_pendingTx = tx;
}
void Mobile::checkAndClearPendingTx( const SC::Thin::TxInfo & tx ) {
	if( _pendingTx.id() == tx.id() ) {
		if( _pendingTx.sbId().size() ) {
			for( list< SBInfo >::iterator i = _sbs.begin(); i != _sbs.end(); ++i ) {
				if( i->id() == _pendingTx.sbId() ) {
					_sbs.erase( i );
					break;
				}
			}
		}
		clearTx();
	}
}
SBInfo Mobile::pendingSb() const {
	return _pendingSb;
}
void Mobile::pendingSb( const SBInfo & sb ) {
	_pendingSb = sb;
	_pendingSbUpdates = 0;
}
void Mobile::checkAndClearPendingSb( const SBInfo & sb ) {
	if( sb.id() == _pendingSb.id() || _pendingSbUpdates >= CINT_SB_UPDATE_MAX_COUNTER ) {
		_pendingSb = SBInfo();
	}
}
void Mobile::key( SPtr::shared<SC::ASKeys> k ) {
	if( !_wkey ) {
		_wkey = new WalletKey( _id );
	}
	_wkey->keys( k );
}
bool Mobile::checkStop() const {
	if( !_needStop )
		return false;
	return *_needStop;
}
void Mobile::setStop( volatile bool * a ) {
	_needStop = a;
}
Error Mobile::passKeys( NetAddress const & adr, std::string const & wId, volatile bool * needStop ) {
	string fName = "Mobile::passKeys";
	Error err;

	bool localFalse = false;
	if(!needStop){
		needStop = &localFalse;
	}
	NetworkBase net;
	net.setStop( needStop );

	// check wId
	WalletKeyRepository wr;
	if( !wr.find( wId ) ) {
		return Error( "Error - Wrong wallet id, id is nit in local repository", fName );
	}

	if( !net.hasWKey( adr, wId, err ) ) {
		WalletKeyRepository::tType wk = wr.value();
		err = net.sendWkey( adr, *wk );
		if( !err ) {
			return Error( "Error - could not send wallet keys to server", fName );
		}
	}
	return err;
}
SC::Error Mobile::passKeys( std::string const & recWId ) {
	Error err;
	NetworkBase net;
	net.setStop( _needStop );

	NetAddress wServ = _servers[ CSTR_WORK_SERV ];

	// check recWId
	WalletKeyRepository wr;
	if( !wr.find(recWId) ) {
		WalletKey wk( recWId );
		err = net.getWkey(wServ, recWId, wk);
		if( err ) {
			wr.save( wk );
		} else {
			return err;
		}
	}

	if( !net.hasWKey( wServ, recWId, err ) ) {
		wr.find( recWId );
		WalletKeyRepository::tType wk = wr.value();
		err = net.sendWkey( wServ, *wk );
		if( !err ) {
			return Error( "Error - could not send receiver's wallet keys to server", "Mobile::passKeys" );
		}
	}

	if( !net.hasWKey( wServ, _id, err) ) {
		wr.find( _id );
		WalletKeyRepository::tType wk = wr.value();
		err = net.sendWkey( wServ, *wk );
		if( !err ) {
			return Error( "Error - could not send sender's wallet keys to server", "Mobile::passKeys" );
		}
	}
	return err;
}

//

