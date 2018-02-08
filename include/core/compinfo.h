#ifndef _COMPUTER_INFO_H
#define _COMPUTER_INFO_H

#include <string>
#include <list>

#include "serializable.h"

namespace SC{

class ComputerInfo : public Serializable {
	std::list< std::string > _macs;
	std::string _hdd;

public:
	ComputerInfo();
	~ComputerInfo();

	void fill();

	std::string hdd() const;
	std::list< std::string > macs() const;

	// Serializable inteface
	std::string data() const;
	LoadPosition load(LoadPosition const &in);
	std::string repData() const;
	LoadPosition repLoad(LoadPosition const &in);

	std::string typeId() const;
	std::string id() const;
	void id(std::string const  & aid);

	void dump() const;

};

}; // SC

#endif //_COMPUTER_INFO_H
