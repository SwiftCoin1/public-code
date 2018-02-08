#ifndef _ISIGNCIPHER_H
#define _ISIGNCIPHER_H

#include <string>

#include "sstring.h"
#include "isign.h"
#include "iblockcipher.h"
#include "sptr.h"
#include "error.h"

namespace SC{

class IHash;
class PrivKey;
class PubKey;

class ISignCipher : public ICipher {
protected:
	//internal operation to create sign data (string of bytes)
	virtual std::string createSign(std::string const & hash, sstring const & key) = 0;
	virtual bool verifySign(std::string const & hash, std::string const & sign, std::string const & pubKey) = 0;

public:
	ISign sign(IHash const & hash, PrivKey const & prkey);
	bool verify(IHash const & hash, ISign const & sign, PubKey const & pubKey);
	//to generate new keys pair
	virtual void genKeyPair(sstring & priv, sstring & pub) = 0;

	virtual Error encrypt(sstring const & text, std::string & cipher, std::string const & pubKey) = 0;
	virtual Error decrypt(std::string const & cipher, sstring & text, sstring const & privKey) = 0;

	virtual Error encrypt( sstring const & text, std::string & cipher, std::string const & pubKey, sstring const & symmKey ) = 0;
	// using only symmetric encryption system
	// pubKey is the same as in encrypt
	virtual Error decryptSym( std::string const & cipher, sstring & text, std::string const & pubKey, sstring const & symmKey ) = 0;

};

}; //SC
#endif
