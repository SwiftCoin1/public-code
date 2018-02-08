//#include <openssl/rand.h>
#include "rndgen.h"
#include <iostream>

#include "keys.h"
#include "keybuilder.h"
#include "pprovider.h"
#include "error.h"
#include "application.h"
#include "msg.h"
#include "cassert"

using namespace SC;
using std::string;
using std::cout;
using std::endl;

//============================= Key

Key::Key(ASKeys * aparent, sstring const & key) : _parent(aparent), _key(key) {}
Key::~Key() {}

sstring Key::key() const {
	return _key;
}
ASKeys * Key::parent() const {
	return _parent;
}

//============================= PrivKey

PrivKey::PrivKey(sstring const & secret, ASKeys * aparent) : Key(aparent, secret), _salt(""), _isOpen(true) {}
PrivKey::~PrivKey(){}

bool PrivKey::open(PProvider const & provider) {
	bool const ret = decrypt(provider.passWord());
	isOpen(true);
	return ret;
}
void PrivKey::close(PProvider const & provider) {
	encrypt(provider.passWord());
	isOpen(false);
}
bool PrivKey::changePassWord(PProvider const & provider) {
	//20120220
	// check old provider
	sstring oldPw = provider.passWord();
	sstring newPw = provider.newPassWord();
	if( oldPw.empty() || newPw.empty() ) {
		return false;
	}
	// decript key on old password 
	bool ret = decrypt(oldPw);
	if(ret) {
		isOpen(true);
		// encrypt key using new password
		encrypt(newPw);
		isOpen(false);
	}
	return ret;
}
bool PrivKey::decrypt(sstring const & passWord) {
	checkSalt();
	if(!parent()){
		throw Exception("Error - parent is NULL.", "ASKeys::decrypt");
	}
	if( !parent()->cipher() ){
		throw Exception("Error - Block cipher is NULL.", "PrivKey::decrypt");
	}
	if(!_salt.size()){
		throw Exception("Error - Salt is empty.", "PrivKey::decrypt");
	}
	parent()->cipher()->salt(_salt);
	_key = parent()->cipher()->decrypt(key(), passWord);
	return true;
}
void PrivKey::encrypt(sstring const & passWord) {
	checkSalt();
	if( !parent()->cipher() ){
		throw Exception("Error - doesn't have Block cipher.", "ASKeys::close");
	}
	parent()->cipher()->salt(_salt);
	_key = parent()->cipher()->encrypt(key(), passWord);
}
void PrivKey::salt(std::string const & asalt){
	_salt = asalt;
}
string PrivKey::salt(){
	return _salt;
}
void PrivKey::checkSalt(){
	if(!_salt.size()){
		// !! hack - need use constant for block cipher
		_salt = RndGen::rand(8);
//throw "PrivKey::checkSalt - Error Salt is empty";
	}
}
bool PrivKey::isOpen() const {
	return _isOpen;
}
void PrivKey::isOpen(bool a){
	_isOpen = a;
}

//============================= PubKey

PubKey::PubKey(sstring const & pub, ASKeys * aparent) : Key(aparent, pub) {}
PubKey::~PubKey(){
//	cout << "PubKey::~PubKey() this=" << (int)this << "\n";
}
bool PubKey::compare(PubKey const & r) const {
	string key = _key.c_str();
	string keyr = r._key.c_str();
//cout << "PubKey::compare key.size()=" << key.size() << "\n";
//cout << "PubKey::compare keyr.size()=" << keyr.size() << "\n";
	return key == keyr;
}

//============================= ASKeys

ASKeys::ASKeys(string const & aid, SPtr::shared<IHashCipher> ahash, SPtr::shared<ISignCipher> asign)
	: _id(aid), _pub(NULL), _priv(NULL), _signer(asign), _cipher(NULL), _hasher(ahash) {
	setVersion();
}
ASKeys::ASKeys() : _id(""), _pub(NULL), _priv(NULL), _signer(NULL), _cipher(NULL), _hasher(NULL) {
	setVersion();
}
ASKeys::ASKeys( ASKeys const & ){
	setVersion();
}
void ASKeys::setVersion() {
	_version.Major(CINT_KEY_MAJOR_VERSION);
	_version.Minor(CINT_KEY_MINOR_VERSION);
	_workingVersion = _version;
}
ASKeys & ASKeys::operator = ( ASKeys const & ){
//	cout << "ASKeys::operator =(ASKeys) this=" << (int)this << "\n";
	return *this;
}
ASKeys::~ASKeys(){}

void ASKeys::isAllSeted() {
	if( !_pub ){
		throw Exception("Error - doesn't have public key.", "ASKeys::isAllSeted");
	}
	if( !_signer ){
		throw Exception("Error - doesn't have Sign cipher.", "ASKeys::isAllSeted");
	}
	if( !_hasher ){
		throw Exception("Error - doesn't have Hash cipher.", "ASKeys::isAllSeted");
	}
}
bool ASKeys::checkSign(Serializable const & data, ISign const & sign) {
	return checkSign(data.data(), sign);
}
ISign ASKeys::sign(Serializable const & data, PProvider const & pprovider){
	return sign(data.data(), pprovider);
}

bool ASKeys::checkSign(std::string const & data, ISign const & sign){
	isAllSeted();
	return _signer->verify(_hasher->hash(data), sign, *pub());
}
ISign ASKeys::sign(std::string const & data, PProvider const & pprovider){
	isAllSeted();
	if( !_priv ){
		throw Exception("Error - doesn't have private key.", "ASKeys::sign");
	}
	if( !_cipher ){
		throw Exception("Error - doesn't have Block cipher.", "ASKeys::sign");
	}
	if( !_signer ){
		throw Exception("Error - doesn't have Sign cipher.", "ASKeys::sign");
	}
	if( !_hasher ){
		throw Exception("Error - doesn't have Hash cipher.", "ASKeys::sign");
	}
	if(!priv()->isOpen()){
		priv()->open(pprovider);
	}
	IHash hash = _hasher->hash(data);
	ISign ret = _signer->sign(hash, *priv());
	
	return ret;
}
string ASKeys::id() const {
	return _id;
}
void ASKeys::id(string const & aid){
	_id = aid;
}
string ASKeys::typeId() const {
	if(_priv){
		return CSTR_ASKEYS_FULL_TYPEID;
	}
	return CSTR_ASKEYS_SHORT_TYPEID;
}
string ASKeys::data() const{
	if( !_pub ){
		throw Exception("Error there is no public key", "ASKeys::data");
	}
	SerData sd;
	sd.add(typeId()).add(_version.data()).add(id()).add(bin2Code64(_pub->key())).add(signer()->id()).add(hasher()->id());
	if(_priv){
		sd.add(bin2Code64(_priv->key())).add(bin2Code64(_priv->salt())).add(cipher()->id());
	}
	return sd.data();
}
Serializable::LoadPosition ASKeys::setPub(Serializable::LoadPosition const & in, KeyBuilder & keyBuilder){
	Serializable::LoadPosition pos(in);
	keyBuilder.create((string)pos);
	keyBuilder.pub(code2Bin((string)++pos));
	keyBuilder.signer((string)++pos);
	keyBuilder.hasher((string)++pos);
	SPtr::shared<ASKeys> newKey = keyBuilder.get();
	copy(*newKey);
	return pos;
}
Serializable::LoadPosition ASKeys::setPriv(Serializable::LoadPosition const & in, KeyBuilder & keyBuilder){
	// check data in input structure
	if(in.rest() < 2){
		return in;
	}
	Serializable::LoadPosition pos(in);
	keyBuilder.priv(code2Bin((string)pos));
	keyBuilder.salt(code2Bin((string)++pos));
	string cipherName = (string)++pos;
	keyBuilder.cipher(cipherName);
	SPtr::shared<ASKeys> newKey = keyBuilder.get();
	copy(*newKey);
	return pos;
}
Serializable::LoadPosition ASKeys::load(Serializable::LoadPosition const & in){
	checkLoaded(in, "ASKeys::load");
	Serializable::LoadPosition ret(in);
	KeyBuilder keyBuilder;

	string typeId = (string)ret;
	_version.load((string)++ret);
	if(_workingVersion < _version){
		// send message to user
		Message msg("Programm can't process crypto keys - its version=" + _version.data() + ", _workingVersion=" + _workingVersion.data(), Message::NeedUpdateAppVersion);
#ifndef NO_MSG_PROCESSING
		Application::instance()->processMsg(msg);
#endif
		return in;
	}
	if(typeId == CSTR_ASKEYS_SHORT_TYPEID){
		ret = setPub(++ret, keyBuilder);
	}else if(typeId == CSTR_ASKEYS_FULL_TYPEID) {
		ret = setPub(++ret, keyBuilder);
		ret = setPriv(++ret, keyBuilder);
		priv()->isOpen(false);
	}else{
		throw Exception("Error - there is wrong key typeId", "ASKeys::load");
	}
	return ret;
}
// !!! we would need to code special method
string ASKeys::repData() const {
	return data();
}
Serializable::LoadPosition ASKeys::repLoad(LoadPosition const &in){
	return load(in);
}

PubKey * ASKeys::pub() {
	return _pub.get();
}
void ASKeys::pub(sstring const & key) {
	_pub = new PubKey(key, this);
}
PrivKey * ASKeys::priv() const {
	return _priv.get();
}
void ASKeys::priv(sstring const & key) {
	_priv = new PrivKey(key, this);
}
shared<IBlockCipher> ASKeys::cipher() const {
	return _cipher;
}
void ASKeys::cipher(shared<IBlockCipher> in){
	assert(in);
	_cipher = in;
}
shared<ISignCipher> ASKeys::signer() const{
	return _signer;
}
void ASKeys::signer(shared<ISignCipher> in){
	assert(in);
	_signer = in;
}
shared<IHashCipher> ASKeys::hasher() const{
	return _hasher;
}
void ASKeys::hasher(shared<IHashCipher> in){
	assert(in);
	_hasher = in;
}
void ASKeys::createId(){
	string newId = bin2Code64(hasher()->hash(*this).hash());
	id(newId);
}
SPtr::shared<ASKeys> ASKeys::clonePub() const {
	SPtr::shared<ASKeys> ret(new ASKeys(id(), hasher(), signer()));
	ret->_pub = this->_pub;
	return ret;
}
void ASKeys::copy(ASKeys const & r){
	_id = r._id;
	_pub = new PubKey(r._pub->key(), this);
	_priv = NULL;
	if(r._priv){
		_priv = new PrivKey(r._priv->key(), this);
		_priv->salt(r._priv->salt());
	}
	_signer = r._signer;
	_cipher = r._cipher;
	_hasher = r._hasher;
}
bool ASKeys::comparePub(ASKeys const & r) const {
	bool ret = _version.data() == r._version.data() && id() == r.id() && signer()->id() == r.signer()->id() && clonePub()->pub()->compare(*(r.clonePub()->pub()));
//cout << "_version.data()=" << _version.data() << ", r._version.data()=" << r._version.data() << "\n";
//cout << "id()=" << id() << ", r.id()=" << r.id() << "\n";
//cout << "signer()->id()=" << signer()->id() << ", r.signer()->id()=" << r.signer()->id() << "\n";
	return ret;
}
Error ASKeys::assymetricEncrypt(sstring const & text, string & cipher) {
	if( !text.size() ) {
		return Error( "Input data is empty", "ASKeys::assymetricEncrypt" );
	}
	return signer()->encrypt(text, cipher, _pub->key().c_str());
}
Error ASKeys::assymetricDecrypt(string const & cipher, sstring & text, PProvider const & pprovider) {
	if( !cipher.size() ) {
		return Error( "Input data is empty", "ASKeys::assymetricDecrypt" );
	}

	if(!priv()->isOpen()) {
		if( !priv()->open(pprovider) ) {
			return Error( "Can't open private key", "ASKeys::assymetricDecrypt" );
		}
	}

	Error err = signer()->decrypt( cipher, text, priv()->key() );
	return err;
}
Error ASKeys::assymetricEncrypt( sstring const & text, std::string & cipher, sstring const & symmKey ) {
	return signer()->encrypt(text, cipher, _pub->key().c_str(), symmKey);
}
Error ASKeys::assymetricDecrypt( std::string const & cipher, sstring & text, sstring const & symmKey ) {
	return signer()->decryptSym( cipher, text, _pub->key().c_str(), symmKey );
}

//
