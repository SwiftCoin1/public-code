#include <openssl/rand.h>
#include <cstdio>
#include <iostream>

#include "rndgen.h"
#include "code64.h"

using namespace SC;
using std::cout;

sstring RndGen::rand(int byteCnt)
{
	sstring ret;
	ret.resize(byteCnt);
	RAND_bytes((unsigned char*)ret.data(), byteCnt);
	return ret;
}
sstring RndGen::code64(int byteCnt)
{
	Code64 coder;
	return sstring(coder.Bin2Text(rand(byteCnt)));
}
sstring RndGen::code16(int byteCnt)
{
	sstring ret, val = rand(byteCnt);
	char buf[sizeof(int) * 2 + 1];
	for(int i = 0; i < byteCnt; ++i){
		int a = (unsigned char)val[i];
		int const len = sprintf(buf, "%x", a);
		if(len == 1){
			ret.append(1, '0');
		}
		ret.append(buf, len);
	}
	return ret;
}

