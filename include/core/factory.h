#ifndef _FACTORY_H
#define _FACTORY_H

#include <string>
#include <map>
#include <boost/function.hpp>

#include "exception.h"
#include "sptr.h"

namespace SC{

/*
template<typename T, typename A>
class Factory {
public:
	typedef typename boost::function<T*(A)> tCtr;
	typedef typename SPtr::shared<T> tResult;
private:
	typedef typename std::map<std::string, tCtr> tCont;
	tCont _ctrs;
	tCtr _defCTR;
public:
	virtual ~Factory(){}

	void addCtr(std::string const & id, tCtr const & ctr) {
		_ctrs[id] = ctr;
	}
	void defCTR(tCtr const & ctr) {
		_defCTR = ctr;
	}
	tResult get(std::string const & id, A a) {
		tResult ret;
		typename tCont::const_iterator fValue = _ctrs.find(id);
		if(fValue != _ctrs.end()){
			tCtr ctr = fValue->second;
			ret = ctr(a);
		}else if(_defCTR){
			ret = _defCTR(a);
		}else{
			throw Exception("Factory<T>::get(string id) Error - here is not defaut CTR, id=" + id, "Factory<T>::get(string id)");
		}
		return ret;
	}
};*/

template<typename T>
class Factory {
public:
	typedef typename boost::function<T*(void)> tCtr;
	typedef typename SPtr::shared<T> tResult;
private:
	typedef typename std::map<std::string, tCtr> tCont;
	tCont _ctrs;
	tCtr _defCTR;
public:
	virtual ~Factory(){}

	void addCtr(std::string const & id, tCtr const & ctr) {
		_ctrs[id] = ctr;
	}
	void defCTR(tCtr const & ctr) {
		_defCTR = ctr;
	}
	tResult get(std::string const & id) {
		tResult ret;
		typename tCont::const_iterator fValue = _ctrs.find(id);
		if(fValue != _ctrs.end()){
			tCtr ctr = fValue->second;
			ret = ctr();
		}else if(_defCTR){
			ret = _defCTR();
		}else{
			throw Exception("Factory<T>::get(string id) Error - here is not defaut CTR, id=" + id, "Factory<T>::get(string id)");
		}
		return ret;
	}
};

}; // SC

#endif
