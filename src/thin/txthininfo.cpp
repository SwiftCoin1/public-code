#include <iostream>

#include "txthininfo.h"
#include "serids.h"
#include "keys.h"
#include "strutils.h"
#include "consts.h"
#include "attachrepository.h"
#include "application.h"
#include "wkeyrepository.h"

#include <boost/foreach.hpp>

using SC::Thin::TxInfo;
using SC::Serializable;
using SC::Money;
using SC::DateTime;
using SC::Description;
using SC::WalletKeyRepository;

using std::cout;
using std::endl;
using std::string;
using std::list;

TxInfo::TxInfo() : _dt( FileSystemHelper::instance()->currentGMT() ), _processeDT( SC::DateTime::oldest() ), _isSb( false ) {
	_version.Major( CINT_TX_MAJOR_VERSION );
	_version.Minor( CINT_TX_MINOR_VERSION );
	_workingVersion = _version;
}
string TxInfo::sender() const {
	return _sender;
}
void TxInfo::sender(string const & a) {
	_sender = a;
}
string TxInfo::receiver() const {
	return _receiver;
}
void TxInfo::receiver(std::string const & a) {
	_receiver = a;
}
Money TxInfo::money() const {
	return _amount;
}
void TxInfo::money(Money const & m) {
	_amount = m;
}
Money TxInfo::fee() const {
	return _fee;
}
void TxInfo::fee(Money const & m) {
	_fee = m;
}
SC::DateTime TxInfo::creationDT() const {
	return _dt;
}
void TxInfo::creationDT( SC::DateTime const & dt ) {
	_dt = dt;
}
DateTime TxInfo::processeDT() const {
	return _processeDT;
}
void TxInfo::processeDT(DateTime const & dt) {
	_processeDT = dt;
}
bool TxInfo::isProcessed() const {
	return _processeDT != SC::DateTime::oldest();
}
string TxInfo::inDescription(SC::ASKeys & key, string const & pw) const {
	_isSb = false;
	string ret;
	BOOST_FOREACH( SC::Description d, _descrs ) {
		Error err;
		string descr = d.text( key, pw, err );
		_isSb = _isSb || Description::isSb( descr );
		descr = Description::filter( descr );
		if( err && descr.size() ) {
			ret += descr + ";";
		} else if( d.hasMsg() && descr.empty() ) {
			ret += "Error: Can't decrypt message.";
		}
	}
	return RTrimStr(ret, ";");
}
string TxInfo::outDescription(SC::ASKeys & key, string const & pw) const {
	_isSb = false;

	string secret = creationDT().data();

	string errText = "Error: Can't decrypt message";
	if( key.priv() ) {
		SC::PProvider::tProviderPtr pr( new SC::OneTimeProvider( pw ) );
		if( !pr ) {
			return errText;
		}
		if( !key.priv()->isOpen() ) {
			key.priv()->open( *pr );
		}

		secret += key.priv()->key();
		secret = key.hasher()->hash(secret).data();
	} else {
		return errText;
	}

	string ret;
	BOOST_FOREACH( SC::Description d, _descrs ) {
		SPtr::shared<ASKeys> keys = WalletKeyRepository::key( _receiver );
		if( keys ){
			Error err;
			string descr = d.text4Sender( *keys, secret, err );
			if( d.hasMsg() && descr.empty() ) {
				descr = "Error: Can't decrypt message.";
			}
			_isSb = _isSb || Description::isSb( descr );
			descr = Description::filter( descr );
			if( descr.size() ) {
				ret = ret + descr + ";";
			}
		}
	}
	return RTrimStr(ret, ";");
}
bool TxInfo::isSb() const {
	return _isSb;
}
void TxInfo::addDescription(SC::Description const &descr) {
	_descrs.push_back(descr);
}
std::list<SC::Attachment> TxInfo::attachments() const {
	return _atts;
}
void TxInfo::attachments( const std::list<SC::Attachment> & a ) {
	_atts = a;
}
bool TxInfo::hasAttachment() const {
	return _atts.size();
}
void TxInfo::loadAttachmentFromRepository() {
	AttachRepository atr;
	std::list<Attachment> atts;
	BOOST_FOREACH( Attachment a, _atts ) {
		if( atr.find( a.id() ) )
			a = *(atr.value());
		atts.push_back( a );
	}
	_atts = atts;
}
// to show all data 
void TxInfo::dump(std::ostream & out) const {
	out << "Thin tx info: tx id=" << _id << endl;
	out << "Sender=" << _sender << ", Receiver=" << _receiver << endl;
	out << "Money=" << (_amount - _fee).data() << ", fee=" << _fee.data() << ", Date=" << _processeDT.data() << ", BlockId=" << _blockId << endl;
	out << (_descrs.size() ? "has description" : "does not have description") << endl;
}
bool TxInfo::operator==( const TxInfo & r ) const {
	return _id == r._id && _dt == r._dt && _sender == r._sender && _receiver == r._receiver && _amount == r._amount && _fee == r._fee && _processeDT == r._processeDT;
}
bool TxInfo::operator!=( const TxInfo & r ) const {
	return !( *this == r );
}
bool TxInfo::empty() const {
	return _id.empty();
}
void TxInfo::clear() {
	_id.clear();
	_dt = DateTime::oldest();
	_sender.clear();
	_receiver.clear();
	_amount = SC::Money();
	_fee = SC::Money();
	_processeDT = DateTime::oldest();
	_blockId.clear();
	_descrs.clear();
	_atts.clear();
	_isSb = false;
	_sbId = "";
}
string TxInfo::status( int lastBlockN ) const {
	string ret;
	if( isProcessed() ){
		ret = toString( distance( lastBlockN ) );
	}else{
		ret = "not processed";
	}
	return ret;
}
string TxInfo::sbId() const {
	return _sbId;
}
void TxInfo::sbId( const string & id ) {
	_sbId = id;
}
// Serializable interface
string TxInfo::data() const {
	SerData sd;
	sd.add(typeId()).add(_id).add(_dt.data()).add(_version.data()).add(_sender).add(_receiver).add(_amount.data()).add(_fee.data()).add(_processeDT.data()).add(_blockId);

	sd.add( toString( (int)_atts.size() ) );
	BOOST_FOREACH( SC::Attachment atmnt, _atts ) {
		sd.add( atmnt.data() );
	}

	for( list<Description>::const_iterator it = _descrs.begin(); it != _descrs.end(); ++it){
		sd.add( it->data() );
	}
	return sd.data();
}
Serializable::LoadPosition TxInfo::load(LoadPosition const & in) {
	checkLoaded(in, "WalletAccount::load");
	Serializable::LoadPosition ret(in);
	if( ret.toString() == typeId() ) {
		clear();
		_id = (string)++ret;
		_dt.load( (string)++ret );

		if(ret.rest() > 0) {
			_version.load((string)++ret);
		} else {
			return ret;
		}

		if(_workingVersion < _version){
			// send message to user
			Message msg("Programm can't process transaction - its version=" + _version.data() + ", _workingVersion=" + _workingVersion.data(), Message::NeedUpdateAppVersion);
#ifndef NO_MSG_PROCESSING
			Application::instance()->processMsg(msg);
#endif
			return in;
		}

		_sender = (string)++ret;
		_receiver = (string)++ret;

		string mStr = (string)++ret + ":";
		_amount.load( mStr + (string)++ret );

		mStr = (string)++ret + ":";
		_fee.load( mStr + (string)++ret );

		_processeDT.load( (string)++ret );
		_blockId = (string)++ret;

		int cnt = toInt( (string)++ret );
		AttachRepository atr;
		for( int i = 0; i < cnt; ++i ) {
			Attachment a;
			ret = a.load( ++ret );
			// when attachment is already in repository we have to use stored one
			if( atr.find( a.id() ) )
				a = *(atr.value());
			_atts.push_back( a );
		}

		while(ret.rest() > 0){
			Description descr;
			ret = descr.load(++ret);
			addDescription(descr);
		}
	}
	return ret;
}
string TxInfo::repData() const {
	return data();
}
Serializable::LoadPosition TxInfo::repLoad(LoadPosition const & in) {
	return load( in );
}
string TxInfo::typeId() const {
	return CSTR_THIN_TX_TYPEID;
}
string TxInfo::id() const {
	return _id;
}
void TxInfo::id(const std::string & aid) {
	_id = aid;
}
int TxInfo::distance( int lastBlockN ) const {
	if(_blockId.empty())
		return -1;
	return lastBlockN - toInt(_blockId);
}
void TxInfo::blockId( string const & a ) {
	_blockId = a;
}
string TxInfo::blockId() const {
	return _blockId;
}

//
