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
#include "log.h"
#include "code64.h"
#include "cipherFactory.h"
#include "strutils.h"

#include <boost/thread/recursive_mutex.hpp>

typedef boost::recursive_mutex::scoped_lock tScoped_lock;

using std::cout;
using std::endl;
using std::list;
using std::string;
using namespace SC;

static boost::recursive_mutex _pwProvidersMutex;
static PProvider::tProviderPtr _defPProvider;
static std::map<std::string, PProvider::tProviderPtr> _providers;

class TestPProvider : public PProvider {
public:
	sstring passWord() const{
//		cout << "Attension - using test password provider!\n";
		return "12345";
	}
};

PProvider::~PProvider(){}

sstring PProvider::passWord() const {
	return sstring();
}
sstring PProvider::newPassWord() const {
	return sstring();
}
PProvider::tProviderPtr PProvider::create(std::string const & id){
	tProviderPtr ret;
	if(id == CSTR_SIMPLE_PROVIDER){
		ret = new SimplePProvider();
	}else if(id == CSTR_RND_PROVIDER){
		ret = new RNDProvider();
	}else if(id == CSTR_TEST_PROVIDER){
		ret = new TestPProvider();
	}else if(id == CSTR_ONE_TIME_PROVIDER){
		ret = new OneTimeProvider();
	}else{
		throw Exception("PProvider::create Error - unknown provider id.", "PProvider::create");
	}
	return ret;
}
void PProvider::passWord(sstring const & ) {
}
bool PProvider::empty() const {
	return true;
}


PProvider::tProviderPtr PProvider::pwProvider() {
	return _defPProvider;
}
void PProvider::pwProvider(PProvider::tProviderPtr pprovider){
	tScoped_lock scopedLock( _pwProvidersMutex );
	_defPProvider = pprovider;
}
PProvider::tProviderPtr PProvider::pwProvider(std::string const & wName) {
	tScoped_lock scopedLock( _pwProvidersMutex );
	PProvider::tProviderPtr ret = _providers[wName];
	return ret;
}
Error PProvider::pwProvider(PProvider::tProviderPtr pprovider, std::string const & wName){
	tScoped_lock scopedLock( _pwProvidersMutex );
	_providers[wName] = pprovider;
	return Error();
}

//====================================================== SimplePProvider

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

sstring SimplePProvider::passWord() const {
	sstring a;
	cout << "Enter passWord:";
	SetStdinEcho(false);
	getline(std::cin, a);
	SetStdinEcho(true);
	return a;
}

//====================================================== RNDProvider

RNDProvider::RNDProvider(){
	_psw = RndGen::code16(6);
}
sstring RNDProvider::passWord() const{
	return _psw;
}
bool RNDProvider::empty() const {
	return false;
}

//====================================================== OneTimeProvider

OneTimeProvider::OneTimeProvider(){
}
OneTimeProvider::OneTimeProvider(sstring const & pw) : _psw(pw) {}

sstring OneTimeProvider::passWord() const {
	if(!_psw.size()){
		_psw = SimplePProvider::passWord();
	}
	return _psw;
}
void OneTimeProvider::passWord(sstring const & pw) {
	_psw = pw;
}
bool OneTimeProvider::empty() const {
	return _psw.size();
}

//====================================================== PwChanger

PwChanger::PwChanger(sstring const & oldPw, sstring const & newPw) : OneTimeProvider(oldPw), _newPw(newPw) {}
sstring PwChanger::newPassWord() const {
	return _newPw;
}


//