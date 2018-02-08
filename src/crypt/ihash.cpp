
#include "ihash.h"
#include "serids.h"

using std::string;
using namespace SC;

IHash::IHash(string const & hash, string const & cipherId) : _cipherId(cipherId), _hash(hash) {}

string IHash::hash() const {
	return _hash;
}
string IHash::textHash() const {
	return bin2Code64(_hash);
}
string IHash::cipherId() const {
	return _cipherId;
}
bool IHash::operator == (IHash const & r) const {
	return hash() == r.hash() && cipherId() == r.cipherId();
}
bool IHash::operator != (IHash const & r) const {
	return !(*this == r);
}
string IHash::data() const {
	return SerData().add(typeId()).add(_cipherId).add(bin2Code64(_hash)).data();
}
Serializable::LoadPosition IHash::load(Serializable::LoadPosition const &in){
	checkLoaded(in, "IHash::load");
	Serializable::LoadPosition ret(in);
	if( in.toString() == typeId()){
		_cipherId = (string)++ret;
		_hash = code2Bin((string)++ret);
	}
	return ret;
}
// !!! we would need to code special method
string IHash::repData() const {
	return data();
}
Serializable::LoadPosition IHash::repLoad(LoadPosition const &in){
	return load(in);
}
string IHash::typeId() const {
	return CSTR_HASH_TYPEID;
}
string IHash::id() const {
	return hash();
}
void IHash::id(std::string const & ){}
