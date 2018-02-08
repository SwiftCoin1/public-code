#ifndef _IHASH_H
#define _IHASH_H

#include "serializable.h"
#include <string>

namespace SC{

class IHash : public Serializable {
	//id of hash cipher
	std::string _cipherId;
	std::string _hash;

public:
	IHash(std::string const & hash, std::string const & cipherId);
	std::string hash() const;
	std::string textHash() const;
	std::string cipherId() const;

	bool operator == (IHash const & r) const;
	bool operator != (IHash const & r) const;

	//serializable
	std::string data() const;
	LoadPosition load(LoadPosition const &in);
	std::string repData() const;
	LoadPosition repLoad(LoadPosition const &in);

	std::string typeId() const;
	std::string id() const;
	void id(std::string const & aid);

};

}; // SC
#endif
