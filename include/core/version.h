#ifndef _VERSION_H
#define _VERSION_H

#include <string>

namespace SC{

class Version {
	int _major;
	int _minor;
public:
	Version();
	Version(const std::string & str);

	bool operator<(const Version & r) const;
	bool operator>(const Version & r) const;
	bool operator==(const Version & r) const;
	bool operator!=(const Version & r) const;
	std::string toString() const;

	Version & AddMinor(int a);
	Version & AddMajor(int a);

	int Minor() const;
	void Minor(int a);

	int Major() const;
	void Major(int a);

	std::string data() const;
	void load(std::string const & in);

};

}; //SC
#endif
