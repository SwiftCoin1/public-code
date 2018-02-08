#include <iostream>

#include "isigncipher.h"
#include "ihash.h"
#include "keys.h"
#include "sstring.h"

using std::string;
using std::cout;

using namespace SC;

ISign ISignCipher::sign(IHash const & hash, PrivKey const & prkey) {
	sstring skey = prkey.key();
	string shash = hash.hash();
	string signText = createSign(shash, skey);
	ISign ret (signText, prkey.parent()->id(), id());
	return ret;
}
bool ISignCipher::verify(IHash const & hash, ISign const & sign, PubKey const & pubKey) {
//	return verifySign(hash.hash(), sign.sign(), pubKey.key());
	return verifySign(hash.hash(), sign.sign(), pubKey.key().c_str());
}