#ifndef _SHA256_H
#define _SHA256_H

#include "ihashcipher.h"
#include <string>

namespace SC{

class SHA256 : public IHashCipher {
public:
	std::string id() const;

protected:
	std::string createHash(std::string const & data);
	std::string createHash(std::string const & data, IHash const & l);

};

};//SC
#endif
