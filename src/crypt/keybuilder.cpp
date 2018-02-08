
#include "keybuilder.h"
#include "ihashcipher.h"
#include "isigncipher.h"
#include "iblockcipher.h"
#include "cipherFactory.h"

#include <iostream>
#include <boost/thread/recursive_mutex.hpp>

using namespace SC;
using std::string;

static boost::recursive_mutex _createMutex;
typedef boost::recursive_mutex::scoped_lock tScoped_lock;

KeyBuilder::KeyBuilder() {
	// !!! use stac variable
	CipherFactory * pCF = CipherFactory::instance();
	string where = "KeyBuilder::KeyBuilder";
	if(!pCF){
		throw Exception("KeyBuilder::KeyBuilder CipherFactory wasn't created", where);
	}
	// get SignCipher
	_signer = pCF->sign("DSA");
	if(!_signer){
		throw Exception("KeyBuilder::KeyBuilder SignCipher wasn't created", where);
	}
	_hasher = pCF->hash("SHA256");
	if(!_hasher){
		throw Exception("KeyBuilder::KeyBuilder HashCipher wasn't created", where);
	}
	_cipher = pCF->block("AES");
	if(!_cipher){
		throw Exception("KeyBuilder::KeyBuilder BlockCipher wasn't created", where);
	}
}

KeyBuilder::~KeyBuilder(){}

//create new ASKey instance
//clear existing key
void KeyBuilder::create(string const & id) {
	_id = id;
}
void KeyBuilder::keys(SPtr::shared<ASKeys> key){
	_id = key->id();
	_key = key;
}
//take builded instance
SPtr::shared<ASKeys> KeyBuilder::get() {
	if(!_key){
		_key = getEmpty();
	}else{
		// !!
		_key->signer(_signer);
		_key->hasher(_hasher);
		_key->cipher(_cipher);
	}
	if(!_pub.size()){
		_signer->genKeyPair(_priv, _pub);
		_key->priv(_priv);
	}else{
		if(_priv.size())
			_key->priv(_priv);
	}
	_key->pub(_pub);
	if(_key->priv()){
		_key->priv()->salt(_salt);
	}
	return _key;
}
SPtr::shared<ASKeys> KeyBuilder::getEmpty(){
	_key = new ASKeys(_id, _hasher, _signer);
	_key->cipher(_cipher);
	return _key;
}
KeyBuilder & KeyBuilder::pub(sstring const & pub) {
	_pub = pub;
//	if(_pub[_pub.size() - 1] == 0) {
//		_pub.resize(_pub.size() - 1);
//	}
	return *this;
}
KeyBuilder & KeyBuilder::priv(sstring const & priv) {
	_priv = priv;
	return *this;
}
KeyBuilder & KeyBuilder::salt(sstring const & asalt){
	_salt = asalt;
	return *this;
}

KeyBuilder & KeyBuilder::signer(string const & id) {
	_signer = CipherFactory::instance()->sign(id);
	return *this;
}
KeyBuilder & KeyBuilder::hasher(string const & id) {
	_hasher = CipherFactory::instance()->hash(id);
	return *this;
}
KeyBuilder & KeyBuilder::cipher(string const & id) {
	_cipher = CipherFactory::instance()->block(id);
	return *this;
}
KeyBuilder & KeyBuilder::id(string const & id) {
	_id = id;
	return *this;
}
KeyBuilder & KeyBuilder::fromText(string const & , string const & ) {
	return *this;
}

KeyBuilder * KeyBuilder::instance(){
	tScoped_lock scopedLock(_createMutex);
	static KeyBuilder * ret = NULL;
	if( !ret ){
		ret = new KeyBuilder();
	}
	return ret;
}

