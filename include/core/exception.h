#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <stdexcept>
#include <string>

namespace SC{

class Exception : public std::logic_error {
	std::string _place;
public:
	Exception(std::string const & what, std::string const & place);
	~Exception() throw();

	void place(std::string const & p);
	std::string place() const;

	std::string text() const;

};

}; // SC

#endif
