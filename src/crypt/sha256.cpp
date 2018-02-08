#include <iostream>

#include <openssl/sha.h>
#include "sha256.h"
#include "exception.h"

#define SHA256_BYTE_LENGTH					32

using std::string;
using namespace SC;

string SHA256::id() const {
	return "SHA256";
}

string SHA256::createHash(string const & data) {
	if(!data.size()){
		throw Exception("SHA256::createHash - empty data", "SHA256::createHash");
	}
	string ret(SHA256_BYTE_LENGTH, 0);
	::SHA256((unsigned char*)data.c_str(), data.size(), (unsigned char*)ret.data());
	return ret;
}
string SHA256::createHash(string const & data, IHash const & l) {
	if(!data.size()){
		throw Exception("SHA256::createHash - empty data", "SHA256::createHash");
	}
	string ret(SHA256_BYTE_LENGTH, 0);
	string hdata = l.hash() + data;
	::SHA256((unsigned char*)hdata.c_str(), hdata.size(), (unsigned char*)ret.data());
	return ret;
}
