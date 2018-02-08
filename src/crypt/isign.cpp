
#include "isign.h"
#include "code64.h"
#include "serids.h"

using std::string;
using namespace SC;

// filds amount in data() method
static int const serLenght = 4;

ISign::ISign() : _sign(), _keyId(), _cipherId() {}
ISign::ISign(string const & sign, string const & keyId, string const & cipherId) : _sign(sign), _keyId(keyId), _cipherId(cipherId) {}

string ISign::sign() const {
	return _sign;
}
string ISign::keyId() const {
	return _keyId;
}
string ISign::cipherId() const {
	return _cipherId;
}
string ISign::data() const {
	return typeId() + ":" + _cipherId + ":" + _keyId + ":" + bin2Code64(sign());
}

Serializable::LoadPosition ISign::load(Serializable::LoadPosition const &in){
	checkLoaded(in, "ISign::load");
	Serializable::LoadPosition ret(in);
	if( in.toString() == typeId() ) {
		_cipherId = (string)++ret;
		_keyId = (string)++ret;
		_sign = code2Bin((string)++ret);
	}
	return ret;
}
// !!! we would need to code special method
string ISign::repData() const {
	return data();
}
Serializable::LoadPosition ISign::repLoad(LoadPosition const &in){
	return load(in);
}

string ISign::typeId() const {
	return CSTR_SIGN_TYPEID;
}
string ISign::id() const {
	return keyId();
}
void ISign::id(std::string const & aid){
	_keyId = aid;
}
bool ISign::empty() const {
	return _sign.empty() || _keyId.empty() || _cipherId.empty();
}