#include <iostream>

#include "serializable.h"
#include "parser.h"
#include "code64.h"
#include "cipherFactory.h"
#include "ihashcipher.h"
#include "rndgen.h"
#include "datetime.h"
#include "application.h"

using namespace SC;
using std::string;
using std::cout;

Serializable::~Serializable() {
}

void Serializable::checkLoaded(LoadPosition const & in, string const & errorPlace) const {
	if(!in.size()){
		throw Exception("Serializable Error. Loaded vector is empty", errorPlace);
	}
}
Serializable::tLoaded Serializable::parse(string const & in, char delim) const{
	Parser parser;
	tLoaded v = parser.parseString(in, delim);
	return v;
}
string Serializable::bin2Code64(string const & bin) const {
	return Code64().Bin2Text(bin);
}
string Serializable::code2Bin(string const & text) const {
	return Code64().Text2Bin(text);
}
bool Serializable::load(std::string const & in){
	Serializable::tLoaded data = parse(in);
	LoadPosition loaded(data, 0);
	bool ret = load(loaded) != loaded;
	return ret;
}
bool Serializable::repLoad(std::string const & in){
	Serializable::tLoaded data = parse(in);
	LoadPosition loaded(data, 0);
	return repLoad(loaded) != loaded;
}
//void Serializable::makeId(std::string const & hashCipherId){
//	id(bin2Code64(CipherFactory::instance()->hash(hashCipherId)->hash(*this).hash()));
//}
string Serializable::makeIdWithDateTime( int rndLen, string const & dateTimeTmpl ) {
	DateTime dtGMT = FileSystemHelper::instance()->currentGMT();
	return dtGMT.data( dateTimeTmpl ) + (string)RndGen::code64( rndLen );
}
//================================ SerData

SerData::SerData() : _data("") {}
void SerData::clear(){
	_data.clear();
}
SerData & SerData::add(string const & field){
	_data.append(field + CSTR_SER_DELIMETER);
	return *this;
}
string SerData::data() const {
	return string(_data, 0, _data.size() ? _data.size() - 1: 0);
}

