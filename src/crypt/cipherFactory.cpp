#include <boost/thread/mutex.hpp>

#include "cipherFactory.h"
#include "aescipher.h"
#include "ecdsasw.h"
#include "sha256.h"
#include "ihash.h"
#include "strutils.h"
#include "code64.h"

using namespace SC;

using std::string;
using SPtr::shared;

static boost::mutex _createMutex;

template <typename T>
shared<T> newType(){
	shared<T> ret = new T();
	return ret;
}
CipherFactory::tSign CipherFactory::sign(string) {
	CipherFactory::tSign res( new ECDSA() );
	return res;
//	return newType<ECDSA>();
}
CipherFactory::tHash CipherFactory::hash(string) {
	CipherFactory::tHash res( new SHA256() );
	return res;
//	return newType<SHA256>();
}
CipherFactory::tBlock CipherFactory::block(string) {
	CipherFactory::tBlock res( new AESCipher() );
	return res;
//	return newType<AESCipher>();
}

CipherFactory *CipherFactory::instance() {
	boost::mutex::scoped_lock scoped_lock(_createMutex);
	static CipherFactory *ret = NULL;
	if(!ret){
		ret = new CipherFactory();
	}
	return ret;
}
CipherFactory::CipherFactory() {}
CipherFactory::~CipherFactory(){}

string SC::getHash(string const & text) {
	CipherFactory *cf = CipherFactory::instance();
	CipherFactory::tHash hasher = cf->hash("SHA256");
	return hasher->hash(text).hash();
}
string SC::getCode16Hash(std::string const & text) {
	return toHexString( getHash(text) );
}
string SC::getCode64Hash(std::string const & text) {
	return Code64().Bin2Text( getHash(text) );
}
