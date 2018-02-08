#ifndef _PPROVIDER_H
#define _PPROVIDER_H

#include <string>

#include "sstring.h"
#include "sptr.h"
#include "consts.h"
#include "error.h"

namespace SC{

class PProvider {
public:
	virtual ~PProvider();
	
	typedef SPtr::shared<PProvider> tProviderPtr;

	virtual sstring passWord() const;
	virtual sstring newPassWord() const;
	virtual void passWord(sstring const & pw);
	virtual bool empty() const;

	static tProviderPtr create(std::string const & id);

	static PProvider::tProviderPtr pwProvider();
	static void pwProvider(PProvider::tProviderPtr);
	static PProvider::tProviderPtr pwProvider(std::string const &);
	static Error pwProvider(PProvider::tProviderPtr, std::string const &);

};

//YubiKey 2.0 static password provider
class YubiPProvider : public PProvider {
};


class SimplePProvider : public PProvider {
public:
	virtual sstring passWord() const;

};

class RNDProvider : public PProvider {
	sstring _psw;
public:
	RNDProvider();
	sstring passWord() const;
	bool empty() const;
};

class OneTimeProvider : public SimplePProvider {
	mutable sstring _psw;
public:
	OneTimeProvider();
	OneTimeProvider(sstring const & pw);
	sstring passWord() const;
	void passWord(sstring const & pw);
	bool empty() const;
};

class PwChanger : public OneTimeProvider {
	sstring _newPw;
public:
	PwChanger(sstring const & oldPw, sstring const & newPw);
	sstring newPassWord() const;
};

bool setWalletPassWord(std::string const &wName, std::string const &pw);
bool setLocalPassWord(std::string const &pw);

Error changePassWord(std::string const & oldPw, std::string const & newPw);

void printPWParams( std::string const & prgName );
void printPWParams( std::string const & prgName, std::ostream &a );

sstring getConsolePassword();
sstring getPassword(std::string const & arg);

}; //SC

#endif
