#ifndef _ISIGN_H
#define _ISIGN_H

#include "serializable.h"
#include <string>

namespace SC{

class ISign : public Serializable {
	//sign data string
	std::string _sign, _keyId, _cipherId;

public:
	ISign();
	ISign(std::string const & sign, std::string const & keyId, std::string const & cipherId);

	bool empty() const;

	std::string sign() const;
	std::string keyId() const;
	std::string cipherId() const;

	//serializable
	std::string data() const;
	LoadPosition load(LoadPosition const &in);
	std::string repData() const;
	LoadPosition repLoad(LoadPosition const &in);

	std::string typeId() const;
	std::string id() const;
	void id(std::string const & aid);

};

}; //SC
#endif
