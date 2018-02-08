#include "code64.h"

using std::string;
using namespace SC;

#define conv_bin2ascii(a)	(bin2ascii[(a)&0x3f])
static unsigned char const bin2ascii[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz0123456789+/";

/* 0xF0 is a EOLN
 * 0xF1 is ignore but next needs to be 0xF0 (for \r\n processing).
 * 0xF2 is EOF
 * 0xE0 is ignore at start of line.
 * 0xFF is error
 */
#define B64_EOLN			0xF0
#define B64_CR				0xF1
#define B64_EOF			0xF2
#define B64_WS				0xE0
#define B64_DataEnd		0xE1
#define B64_ERROR      	0xFF
#define B64_NOT_BASE64(a)	(((a)|0x93) == 0xF3)

static unsigned char const ascii2bin[128]={
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xE0,0xF0,0xFF,0xFF,0xF1,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0x3E,0xFF,0xF2,0xFF,0x3F,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,
	0x3C,0x3D,0xFF,0xFF,0xFF,0xE1,0xFF,0xFF,
	0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
	0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
	0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,
	0x31,0x32,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,
};

unsigned long Code64::GetNextChar(unsigned char *poBuf, bool &vobEof, int voiShift, int &viPos, int viLen){
	unsigned long viRet;
	if(viPos >= viLen){
		vobEof = true;
		// 20121204
		return 0;
	}else{
		viRet = ascii2bin[poBuf[viPos]];
	}
	viPos++;
	while(B64_NOT_BASE64(viRet)){
		if((viRet == B64_DataEnd) || (viLen == viPos)){
			vobEof = true;
			return 0;
		}
		viRet = ascii2bin[poBuf[viPos]];
		viPos++;
	}
	viRet <<= (6 * voiShift);
	return viRet;
}

Code64::Code64() {}
Code64::~Code64(){}

string Code64::Bin2Text(const string &vostrData){
	int rem = vostrData.size() % 3;
	int viCnt = vostrData.size() / 3;
	int textLen = viCnt * 4 + (rem ? 4 : 0);
	_buf.resize(textLen + 1);
	unsigned char *pText = (unsigned char *)_buf.data(), *pData = (unsigned char *)(vostrData.c_str());
	for(int i(0); i < viCnt; i++){
		unsigned long viNum = (((unsigned long)pData[0]) << 16) | (((unsigned long)pData[1]) << 8) | pData[2];
		*(pText++) = conv_bin2ascii(viNum >> 18);
		*(pText++) = conv_bin2ascii(viNum >> 12);
		*(pText++) = conv_bin2ascii(viNum >> 6);
		*(pText++) = conv_bin2ascii(viNum);
		pData += 3;
	}
	if(rem){
		unsigned long viNum = (((unsigned long)pData[0]) << 16);
		if(rem == 2)
			viNum |= ((unsigned long)pData[1]<<8);
		*(pText++) = conv_bin2ascii(viNum >> 18);
		*(pText++) = conv_bin2ascii(viNum >> 12);
		*(pText++) = (rem == 1) ? '=' : conv_bin2ascii(viNum >> 6);
		*(pText++) = '=';
	}
	return string(_buf, 0, textLen);
}
string Code64::Text2Bin(const string &vostrText){
	size_t textLen = vostrText.size();
	int dataLen = (textLen / 4) * 3;
	_buf.resize(dataLen + 1);
	dataLen = 0;
	unsigned char *pText = (unsigned char*)vostrText.c_str();
	unsigned char *pData = (unsigned char*)_buf.data();;
	int viPos = 0;
	bool vbEof = false;
	while(!vbEof){
		unsigned long viNum = 0;
		int blockLen = 3;
		for(int i(4); (i--) > 0;){
			viNum += GetNextChar(pText, vbEof, i, viPos, textLen);
			if(vbEof){
				switch(i){
				case 3:
					blockLen = 0;
					break;
				case 2:
					blockLen = 0;
					break;
				case 1:
					blockLen = 1;
					break;
				case 0:
					blockLen = 2;
					break;
				}
				break;
			}
		}
		for(int i(0); i < blockLen; i++){
			*(pData++)=(unsigned char)(viNum >> ((2 - i) * 8));
		}
		dataLen += blockLen;
	}
	return string(_buf, 0, dataLen);
}

string Code64::bin2Text(const string &vostrData){
	int rem = vostrData.size() % 3;
	int viCnt = vostrData.size() / 3;
	int textLen = viCnt * 4 + (rem ? 4 : 0);
	string buf( textLen + 1, 0);
	unsigned char *pText = (unsigned char *)buf.data(), *pData = (unsigned char *)(vostrData.c_str());
	for(int i(0); i < viCnt; i++){
		unsigned long viNum = (((unsigned long)pData[0]) << 16) | (((unsigned long)pData[1]) << 8) | pData[2];
		*(pText++) = conv_bin2ascii(viNum >> 18);
		*(pText++) = conv_bin2ascii(viNum >> 12);
		*(pText++) = conv_bin2ascii(viNum >> 6);
		*(pText++) = conv_bin2ascii(viNum);
		pData += 3;
	}
	if(rem){
		unsigned long viNum = (((unsigned long)pData[0]) << 16);
		if(rem == 2)
			viNum |= ((unsigned long)pData[1]<<8);
		*(pText++) = conv_bin2ascii(viNum >> 18);
		*(pText++) = conv_bin2ascii(viNum >> 12);
		*(pText++) = (rem == 1) ? '=' : conv_bin2ascii(viNum >> 6);
		*(pText++) = '=';
	}
	return string( buf, 0, textLen );
}
string Code64::text2Bin(const string &vostrText){
	size_t textLen = vostrText.size();
	int dataLen = (textLen / 4) * 3;
	string buf( dataLen + 1, 0);
	dataLen = 0;
	unsigned char *pText = (unsigned char*)vostrText.c_str();
	unsigned char *pData = (unsigned char*)buf.data();;
	int viPos = 0;
	bool vbEof = false;
	while(!vbEof){
		unsigned long viNum = 0;
		int blockLen = 3;
		for(int i(4); (i--) > 0;){
			viNum += GetNextChar(pText, vbEof, i, viPos, textLen);
			if(vbEof){
				switch(i){
				case 3:
					blockLen = 0;
					break;
				case 2:
					blockLen = 0;
					break;
				case 1:
					blockLen = 1;
					break;
				case 0:
					blockLen = 2;
					break;
				}
				break;
			}
		}
		for(int i(0); i < blockLen; i++){
			*(pData++)=(unsigned char)(viNum >> ((2 - i) * 8));
		}
		dataLen += blockLen;
	}
	return string( buf, 0, dataLen );
}
