#include <iostream>
#include <string>
#include <list>

#ifdef WIN32
	#include <windows.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif

#include "pprovider.h"
#include "rndgen.h"
#include "exception.h"
#include "application.h"
#include "log.h"
#include "wkeyrepository.h"
#include "code64.h"
#include "cipherFactory.h"
#include "strutils.h"
#include "wallet.h"

using std::cout;
using std::endl;
using std::list;
using std::string;
using namespace SC;

static void SetStdinEcho(bool enable = true) {
#ifdef WIN32
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	if( !enable )
		mode &= ~ENABLE_ECHO_INPUT;
	else
		mode |= ENABLE_ECHO_INPUT;
	SetConsoleMode(hStdin, mode );
#else
	struct termios tty;
	tcgetattr(STDIN_FILENO, &tty);
	if( !enable )
		tty.c_lflag &= ~ECHO;
	else
		tty.c_lflag |= ECHO;
	(void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

//bool SC::checkPresentWallets(SC::PProvider::tProviderPtr pr) {
static bool checkLocalWalletsPassWord(SC::PProvider::tProviderPtr pr) {
//cout << "checkLocalWalletsPassWord pr 1 pw=" << pr->passWord() << endl;
	list<string> wIds =  Wallet::wallets();
	for(list<string>::const_iterator i = wIds.begin(); i != wIds.end(); ++i){
		SC::Error err = Wallet::checkPProvider(pr, *i);
		if(!err) {
			return false;
		}
	}
	return true;
}
static bool checkLocalWalletsPassWord(std::string const &pw) {
//cout << "checkLocalWalletsPassWord 1 pw=" << pw << endl;
	if( pw.empty() )
		return false;
	SC::PProvider::tProviderPtr pr( new SC::OneTimeProvider() );
	pr->passWord(pw);
	return checkLocalWalletsPassWord(pr);
}
bool SC::setWalletPassWord(std::string const &wName, std::string const &pw) {
//cout << "SC::setWalletPassWord 1 pw=" << pw << endl;
	Application *app = Application::instance();
	SC::PProvider::tProviderPtr pr( new SC::OneTimeProvider() );
	pr->passWord(pw);
	if( checkLocalWalletsPassWord( pw ) ) {
		SC::Error err = PProvider::pwProvider(pr, wName);
		return err;
	}else{
		app->userLogger()->add("SC::setWalletPassWord Can't set password if it is different from other passwords for existing wallets");
	}
	return false;
}
bool SC::setLocalPassWord(std::string const &pw) {
//cout << "SC::setLocalPassWord 1 pw=" << pw << endl;
	Application * app = Application::instance();
	if( pw.empty() ) {
		app->errLogger()->add( "SC::setLocalPassWord Password is empty" );
		return false;
	}
	if( app->isDbEncrypted() && (!app->openDbKey(pw)) ) {
		app->errLogger()->add( "SC::setLocalPassWord Can't decrypt DB key" );
		return false;
	}
	list<string> wIds =  Wallet::wallets();
	bool ret = true;
	for(list<string>::const_iterator i = wIds.begin(); i != wIds.end(); ++i){
		if( ! setWalletPassWord(*i, pw) ) {
			app->errLogger()->add( "SC::setLocalPassWord Can't decrypt password for wallet:" + *i );
			ret = false;
		}
	}
	return ret;
}
Error SC::changePassWord(std::string const & oldPw, std::string const & newPw) {
//cout << "SC::changePassWord oldPw=" << oldPw << ", newPw=" << newPw << "\n";
	Error err;
	SC::PProvider::tProviderPtr pr( new SC::OneTimeProvider(oldPw) );
	if( checkLocalWalletsPassWord(pr) ) {
		Application *app = Application::instance();
		list<string> wIds =  Wallet::wallets();
		WalletKeyRepository wkr;
		Store::tStoreTx dbtx = wkr.tx();
		for(list<string>::const_iterator i = wIds.begin(); i != wIds.end(); ++i){
			SPtr::shared<Wallet> w = Wallet::wallet(*i);
			if(w) {
				err = w->changePassword(oldPw, newPw);
				if(! err ) {
					break;
				}
			} else {
				err = Error("::changePassWord Error - wallet is null id=" + *i, "::changePassword");
			}
		}
		err = app->changeDbPassWord(oldPw, newPw);
		if( err ) {
			dbtx->commit();
		} else {
		}
	} else {
		err = Error("::changePassWord Error - old password is wrong", "::changePassword");
	}
	return err;
}
sstring SC::getConsolePassword() {
	sstring a;
	cout << "Enter passWord:";
	SetStdinEcho(false);
	getline(std::cin, a);
	cout << endl;
	SetStdinEcho(true);
	return a;
}
sstring SC::getPassword(string const & arg) {
	if(arg == "-pc") {
		return getConsolePassword();
	} else {
		if(arg.substr(0, 2) == "-p")
			return arg.substr(2, arg.size());
	}
	return "";
}
void SC::printPWParams(string const & prgName) {
	printPWParams( prgName, cout );
}
void SC::printPWParams( std::string const & prgName, std::ostream &a ) {
	a << "password key = -pPASSWORD or -pc " << endl;
	a << "PASSWORD - user password for wallets in local storage" << endl;
	a << "-p - user enters password as parameter in command line" << endl;
	a << "\texample of command is: " << prgName << " -pPaSsWoRd" << endl;
	a << "-pc - user enters password direct to application after a special request" << endl;
	a << "\texample of command is: " << prgName << " -pc" << endl;
}

//
