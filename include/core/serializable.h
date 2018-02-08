#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

#include <string>
#include <vector>

#include "exception.h"
#include "consts.h"

namespace SC{


struct SerData{
	std::string _data;
	SerData();

	void clear();
	SerData & add(std::string const & field);
	std::string data() const;
};


class Serializable{
public:
	typedef std::vector<std::string> tLoaded;
	struct LoadPosition;

protected:
	void checkLoaded(LoadPosition const & in, std::string const & errorPlace) const;
	tLoaded parse(std::string const & in, char delim = CSTR_SER_DELIMETER) const;
	std::string bin2Code64(std::string const & bin) const;
	std::string code2Bin(std::string const & text) const;
//	void makeId(std::string const & hashCipherId);

public:
	virtual ~Serializable();

	virtual std::string data() const = 0;
	virtual bool load(std::string const & in);

	virtual std::string repData() const = 0;
	virtual bool repLoad(std::string const & in);

	virtual LoadPosition load(LoadPosition const &in) = 0;
	virtual LoadPosition repLoad(LoadPosition const &in) = 0;

	virtual std::string typeId() const = 0;
	virtual std::string id() const = 0;
	virtual void id(std::string const & aid) = 0;

	// dateTimeTmpl - "yyyymmddhh"
	static std::string makeIdWithDateTime( int rndLen, std::string const & dateTimeTmpl = "yyyymmddhh" );

};

struct Serializable::LoadPosition{
	Serializable::tLoaded & _data;
	int _begPos;
	int _curPos;
	mutable bool _isErr;
	LoadPosition(Serializable::tLoaded & data, int pos) : _data(data), _begPos(pos), _curPos(pos), _isErr(false) {
		if(pos >= (int)data.size()){
			throw Exception("LoadPosition::LoadPosition position is out of internal array size", "LoadPosition::LoadPosition");
		}
	}
	LoadPosition(LoadPosition const & r) : _data(r._data), _begPos(r._curPos), _curPos(r._curPos) {}

	bool operator ==(LoadPosition const & r) const {
		return (char *)&_data == (char *)&r._data && _begPos == r._begPos && _curPos == r._curPos;
	}
	bool operator !=(LoadPosition const & r) const {
		return !(*this == r);
	}
	operator std::string() const {
		if(_curPos < 0 || _curPos >= (int)_data.size()){
			_isErr = true;
			return "";
		}
		return _data[_curPos];
	}
	std::string toString() const {
		return this->operator std::string();
	}
	operator bool() const {
		return _curPos < (int)_data.size() && _curPos >= _begPos;
	}
	LoadPosition & operator++(){
		if(*this){
			++_curPos;
		}
		return *this;
	}
	LoadPosition operator++(int){
		LoadPosition ret(*this);
		++*this;
		return ret;
	}
	LoadPosition & operator--(){
		if(*this){
			--_curPos;
		}
		return *this;
	}
	LoadPosition operator--(int){
		LoadPosition ret(*this);
		--*this;
		return ret;
	}
	int distance() const {
		return _curPos - _begPos;
	}
	LoadPosition & operator+(int d){
		if(*this && _curPos + d < (int)_data.size()){
			_curPos += d;
		}
		return *this;
	}
	LoadPosition & operator-(int d){
		if(*this && _curPos - d >= _begPos){
			_curPos += d;
		}
		return *this;
	}
	LoadPosition & operator=(LoadPosition const & r){
		if(this != &r){
			_data = r._data;
			_begPos = r._begPos;
			_curPos = r._curPos;
		}
		return *this;
	}
	int begin() const {
		return _begPos;
	}
	int current() const {
		return _curPos;
	}
	int size() const {
		return _data.size();
	}
	int rest() const {
		return _data.size() - _curPos - 1;
	}
	bool isError() const {
		return _isErr;
	}
};


};

#endif // SERIALIZABLE_HPP
