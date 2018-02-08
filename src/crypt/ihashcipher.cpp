#include <iostream>

#include "ihashcipher.h"
#include "serializable.h"

using namespace SC;
using std::cout;

IHash IHashCipher::hash(const Serializable & data) {
	return hash(data.data());
}
IHash IHashCipher::hash(Serializable const & data, IHash const & l){
	return hash(data.data(), l);
}
IHash IHashCipher::hash(std::string const & data){
	IHash(createHash(data), id());
	IHash ret(createHash(data), id());
	return ret;
}
IHash IHashCipher::hash(std::string const & data, IHash const & l){
	return IHash(createHash(data, l), id());
}
