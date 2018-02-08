#ifndef _MONEY_H
#define _MONEY_H

#include <string>


// amount of fractional part in decimal represintation
//#define CINT_FRACTIONAL_PART			6
// 20120206 - to finish bug reporting system
#define CINT_FRACTIONAL_PART			3

namespace SC{

class Money {
	long long _value;
	std::string _currency;

public:
	Money();
	explicit Money(long long val);

	std::string Currency() const;
	void Currency(std::string const & currency);
	bool TheSameCurrency(Money const & r) const;

	// to show in swiftconsole and graphical console
	std::string toUser() const;
	void fromUser(std::string const & m);

	std::string Value() const;

	Money operator+(Money const & r) const;
	Money operator-(Money const & r) const;
	bool operator==(Money const & r) const;
	bool operator!=(Money const & r) const;

	Money operator+(int r) const;
	Money operator-(int r) const;

	Money operator*(double r) const;

	bool operator>(Money const & r) const;
	bool operator<(Money const & r) const;
	bool operator>=(Money const & r) const;
	bool operator<=(Money const & r) const;

	operator bool() const;

	Money Divide(int parts, Money & rest) const;
	Money Divide(int parts) const;

	void clear();

	std::string data( const std::string & delim = ":") const;
	void load(std::string const & in, const std::string & delim = ":");
};

extern const SC::Money oneCoin;
extern const SC::Money zeroCoin;

}; //SC
#endif
