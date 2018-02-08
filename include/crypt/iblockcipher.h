#ifndef _IBLOCKCIPHER_H
#define _IBLOCKCIPHER_H

#include <string>
#include "sstring.h"

namespace SC{

class ICipher {
public:
	virtual ~ICipher();

	virtual std::string id() const = 0;

};

class IBlockCipher : public ICipher {
public:
	virtual sstring encrypt(sstring const &text, sstring const &key) = 0;
	virtual sstring decrypt(sstring const &cipher, sstring const &key) = 0;
	//length of key in bytes
	virtual int keyLength() const = 0;
	//set salt bo block cipher
	virtual void salt(std::string const &salt);
	//max lengthh of salt string in byte
	virtual int saltLength() const = 0;

};

}; //SC
#endif
