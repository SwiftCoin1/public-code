#pragma once

// Version 2 

#include <boost/thread/mutex.hpp>

namespace SPtr{

struct statistic {
	statistic();
	~statistic();

	static int amount();
	static void shutDown();
};

template <typename T>
class shared : public statistic {
	typedef boost::mutex::scoped_lock tScoped_lock;
	mutable boost::mutex _m;
public:
	typedef void(*tdeleter)(T*);
	static void deleter(T *ptr){
		delete ptr;
	}
	typedef shared<T> self;
	shared() : _intrnl( NULL ) {}
	explicit shared(T * ptr, tdeleter adeleter = &deleter) : _intrnl( NULL ) {
		assignment( ptr, adeleter );
	}
	shared(const shared &r) : _intrnl(NULL) {
		operator=(r);
	}
	~shared() {
		tScoped_lock m( _m );
		clear();
	}

	T& operator*() const throw(){
		return *operator->();
	}
	T* operator->() const throw(){
		tScoped_lock m( _m );
		if( _intrnl ){
			return _intrnl->ptr();
		}else{
			return NULL;
		}
	}
	T* get() const throw(){
		return operator->();
	}
	self& operator=(const shared &r){
		tScoped_lock m( _m );
		if( this != &r ){
			clear();
			_intrnl = r._intrnl;
			if( _intrnl )
				_intrnl->inc();
		}
		return *this;
	}
	self& operator=( T *ptr ) {
		return assignment( ptr );
	}
	self& assignment(T *ptr, tdeleter adeleter = &deleter) {
		tScoped_lock m( _m );
		clear();
		if( ptr ) {
			_intrnl = new(std::nothrow) Internal( ptr, adeleter );
		}
		return *this;
	}
	operator bool() const{
		tScoped_lock m( _m );
		return _intrnl;
	}
	int refCnt() const {
		tScoped_lock m( _m );
		if( _intrnl ){
			return _intrnl->cnt();
		}
		return 0;
	}
	void forceClear(){
		tScoped_lock m( _m );
		if( _intrnl ) {
			_intrnl->forceClear();
		}
	}
	void forceReset(T *ptr){
		tScoped_lock m( _m );
		if(_intrnl){
			_intrnl->clear();
			_intrnl->reSet( ptr );
		}
	}
	
	bool operator == (const shared &r) const {
		return get() == r.get();
	}
private:
	template<typename O> friend class shared;

	void clear(){
		if( _intrnl ) {
			_intrnl->dec();
			if( _intrnl->cnt() <= 0 ) {
				delete _intrnl;
			}
		}
		_intrnl = NULL;
	}
	class Internal{
		T *_ptr;
		tdeleter _deleter;
		int _cnt;
		mutable boost::mutex _im;
	public:
		Internal(T * ptr, tdeleter adeleter = &deleter) : _ptr( ptr ), _deleter( adeleter ), _cnt( _ptr ? 1 : 0 ) {}
		~Internal(){
			clear();
		}
		T *ptr() const {
			tScoped_lock m( _im );
			return _ptr;
		}
		int cnt() const {
			tScoped_lock m( _im );
			return _cnt;
		}
		void clear() {
			tScoped_lock m( _im );
			if( _deleter && _ptr)
				(*_deleter)(_ptr);
			_ptr = NULL;
		}
		void forceClear() {
			tScoped_lock m( _im );
			_ptr = NULL;
		}
		void inc(){
			tScoped_lock m( _im );
			++_cnt;
		}
		void dec(){
			tScoped_lock m( _im );
			--_cnt;
		}
		void reSet( T * ptr ) {
			tScoped_lock m( _im );
			_ptr = ptr;
		}
	};
	Internal *_intrnl;
};

};		//namespace SPtr

//
