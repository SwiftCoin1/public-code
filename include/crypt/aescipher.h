#ifndef _AESCHIPHER_H
#define _AESCHIPHER_H

#include <string>
#include "iblockcipher.h"

namespace SC{

class AESCipher : public IBlockCipher {
	std::string _salt;
public:
	sstring encrypt(sstring const & text, sstring const & key);
	sstring decrypt(sstring const & cipher, sstring const & key);
	std::string id() const;

	//length of key in bytes
	int keyLength() const;
	//set salt bo block cipher
	void salt(std::string const & salt);
	//max lengthh of salt string in byte
	int saltLength() const;

};

} //SC
#endif	//_AESCHIPHER_H
