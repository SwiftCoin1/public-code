#ifndef coder64_h
#define coder64_h

#include <string>

namespace SC{

class Code64{
	std::string _buf;
	static inline unsigned long GetNextChar(unsigned char *poBuf, bool &vobEof, int voiShift, int &viPos, int viLen);
public:
	Code64();
	~Code64();

	std::string Bin2Text(const std::string &vostrData);
	std::string Text2Bin(const std::string &vostrText);

	static std::string bin2Text(const std::string &vostrData);
	static std::string text2Bin(const std::string &vostrText);
};

}; //SC

#endif //coder64_h
