#ifndef _ECDSASW_H
#define _ECDSASW_H

#include "isigncipher.h"
#include <string>

#include "sstring.h"

namespace SC{

class ECDSA : public ISignCipher {
public:
	ECDSA();
	~ECDSA();

	std::string id() const;
	void genKeyPair(sstring & priv, sstring & pub);

	Error encrypt(sstring const & text, std::string & cipher, std::string const & pubKey);
	Error decrypt(std::string const & cipher, sstring & text, sstring const & privKey);

	Error encrypt( sstring const & text, std::string & cipher, std::string const & pubKey, sstring const & symmKey );
	
	// using only symmetric encryption system
	// pubKey is the same as in encrypt
	Error decryptSym( std::string const & cipher, sstring & text, std::string const & pubKey, sstring const & symmKey );

protected:
	//internal operation to create sign data (string of bytes)
	std::string createSign(std::string const & hash, sstring const & key);
	bool verifySign(std::string const & hash, std::string const & sign, std::string const & pubKey);

};

} //SC
#endif	//_ECDSASW_H
