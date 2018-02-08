#ifndef _IHASHCIPHER_H
#define _IHASHCIPHER_H

#include <string>

#include "ihash.h"
#include "iblockcipher.h"

namespace SC{

class Serializable;

class IHashCipher : public SC::ICipher {
public:
	IHash hash(Serializable const & data);
	IHash hash(Serializable const & data, IHash const & l);

	IHash hash(std::string const & data);
	IHash hash(std::string const & data, IHash const & l);

protected:
	virtual std::string createHash(std::string const & data) = 0;
	virtual std::string createHash(std::string const & data, IHash const & l) = 0;

};

}; //SC
#endif
