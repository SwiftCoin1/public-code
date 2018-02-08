#ifndef _CIPHERFACTORY_H
#define _CIPHERFACTORY_H

#include <string>

#include "isign.h"
#include "ihash.h"
#include "iblockcipher.h"
#include "ihashcipher.h"
#include "isigncipher.h"
#include "sptr.h"


#define CSTR_HASH_SHA256_ID		"SHA256"

namespace SC{

class CipherFactory {
public:
	typedef SPtr::shared<ISignCipher> tSign;
	typedef SPtr::shared<IHashCipher> tHash;
	typedef SPtr::shared<IBlockCipher> tBlock;
	
	tSign sign(std::string id);
	tHash hash(std::string id);
	tBlock block(std::string id);
	static CipherFactory* instance();
	virtual ~CipherFactory();
private:
	CipherFactory();
};

std::string getHash(std::string const & text);
std::string getCode16Hash(std::string const & text);
std::string getCode64Hash(std::string const & text);

}; // SC

#endif
