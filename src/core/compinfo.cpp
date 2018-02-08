#include <iostream>

#ifdef WIN32
	#include <windows.h>
	#include <winsock2.h>
	#include <iphlpapi.h>

#else
	#include <cstdio>

#endif

#include "compinfo.h"
#include "serids.h"
#include "strutils.h"

using namespace SC;
using std::cout;
using std::endl;
using std::string;
using std::list;

ComputerInfo::ComputerInfo() {}
ComputerInfo::~ComputerInfo() {}

//CINFOTID:MAC1,MAC2:HDD
string ComputerInfo::data() const {
	SerData sd;
	sd.add(typeId()).add(toString( _macs, ",")).add(_hdd);
	return sd.data();
}
Serializable::LoadPosition ComputerInfo::load(LoadPosition const &in) {
	checkLoaded(in, "ComputerInfo::load");
	Serializable::LoadPosition ret(in);
	if( ret.toString() == typeId() ) {
		string macs = (string)++ret;
		_macs = split( macs, "," );
		_hdd = (string)++ret;
	}
	return ret;
}
string ComputerInfo::repData() const {
	return "";
}
Serializable::LoadPosition ComputerInfo::repLoad(Serializable::LoadPosition const &in) {
	return in;
}
string ComputerInfo::typeId() const {
	return CSTR_COMP_INFO_TYPEID;
}
string ComputerInfo::id() const {
	return "";
}
void ComputerInfo::id( string const &  ) {
}

#ifdef WIN32
void ComputerInfo::fill() {
	unsigned long ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	std::string buf( ulOutBufLen, 0 );
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)buf.data();

	if (pAdapterInfo == NULL) {
		return;
	}

	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW) {
		buf = std::string( ulOutBufLen, 0 );
		pAdapterInfo = (IP_ADAPTER_INFO *)buf.data();
		if (pAdapterInfo == NULL) {
			return;
		}
	}
	unsigned int dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen );
	if (dwRetVal == NO_ERROR) {
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while( pAdapter ) {
			string adr;
			for ( int i = 0; i < pAdapter->AddressLength; ++i) {
				adr += toHexString( (int) pAdapter->Address[i]);
			}
			if( adr.size() ) {
				_macs.push_back( adr );
			}
			pAdapter = pAdapter->Next;
		}
	}

	string path = "C:\\";
	string name( 255, 0 );
	DWORD serNum, len, flags;
	string nameBuf( 255, 0 );
	if( GetVolumeInformation( path.c_str(), (char*)name.data(), name.size(), &serNum, &len, &flags, (char*)nameBuf.data(), nameBuf.size() )) {
		_hdd = toHexString(serNum);
	}
}
#else
void ComputerInfo::fill() {
	char line[133];
	string cmd = "ip -o link  2>/dev/null";
	FILE *fp = popen(cmd.c_str(), "r");
	if(fp){
		bool now = false;
		while( fscanf(fp, "%s", line) != EOF ) {
			if( now ) {
				string mac = line;
				if( mac.size() ) {
					mac = strReplaceAll( mac, ":", "");
					_macs.push_back( mac );
				}
				now = false;
			}
			string cur = line;
			if( cur == "link/ether" )
				now = true;
		}
		pclose(fp);
	}

	_hdd = "";
	cmd = "udisks --show-info /dev/sda 2>/dev/null";
	fp = popen(cmd.c_str(), "r");
	if(fp){
		bool now = false;
		while( fscanf(fp, "%s", line) != EOF ) {
			if( now ) {
				_hdd += line;
				now = false;
			}
			string cur = line;
//			if( cur == "serial:" || cur == "WWN:" )
			if( cur == "WWN:" )
				now = true;
		}
		pclose(fp);
	}

}
#endif
void ComputerInfo::dump() const {
	cout << "MACs:" + toString( _macs, " , ") << endl;
	cout << "HDD:" + _hdd << endl;
}
string ComputerInfo::hdd() const {
	return _hdd;
}
list< string > ComputerInfo::macs() const {
	return _macs;
}

