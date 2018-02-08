#ifndef SSTRING_HPP
#define SSTRING_HPP

#include <cstring>
#include <string>

//typedef std::string sstring;

#if defined _WIN32 || defined _WIN32_
#include <Windows.h>
// This is used to attempt to keep keying material out of swap
// Note that VirtualLock does not provide this as a guarantee on Windows,
// but, in practice, memory that has been VirtualLock'd almost never gets written to
// the pagefile except in rare circumstances where memory is extremely low.
#define mlock(p, n) VirtualLock((p), (n));
#define munlock(p, n) VirtualUnlock((p), (n));
#else
#include <sys/mman.h>
#include <limits.h>
// This comes from limits.h if it's not defined there set a sane default 
#ifndef PAGESIZE
#include <unistd.h>
#define PAGESIZE sysconf(_SC_PAGESIZE)
#endif
#define mlock(a,b) \
  mlock(((void *)(((size_t)(a)) & (~((PAGESIZE)-1)))),\
  (((((size_t)(a)) + (b) - 1) | ((PAGESIZE) - 1)) + 1) - (((size_t)(a)) & (~((PAGESIZE) - 1))))
#define munlock(a,b) \
  munlock(((void *)(((size_t)(a)) & (~((PAGESIZE)-1)))),\
  (((((size_t)(a)) + (b) - 1) | ((PAGESIZE) - 1)) + 1) - (((size_t)(a)) & (~((PAGESIZE) - 1))))
#endif


template<typename T>
struct secure_allocator : public std::allocator<T>{
	// MSVC8 default copy constructor is broken
	typedef std::allocator<T> base;
	typedef typename base::size_type size_type;
//	typedef typename base::difference_type  difference_type;
	typedef typename base::pointer pointer;
	typedef typename base::const_pointer const_pointer;
	typedef typename base::reference reference;
	typedef typename base::const_reference const_reference;
	typedef typename base::value_type value_type;

	secure_allocator() throw() {}
	secure_allocator(const secure_allocator& a) throw() : base(a) {}

	template <typename U>
	secure_allocator(const secure_allocator<U>& a) throw() : base(a) {}

	~secure_allocator() throw() {}

	template<typename _Other>
	struct rebind{
		typedef secure_allocator<_Other> other;
	};

	T* allocate(std::size_t n, const void *hint = 0){
		T *p;
		p = std::allocator<T>::allocate(n, hint);
		if (p)
			mlock(p, sizeof(T) * n);
		return p;
	}

	void deallocate(T* p, std::size_t n) {
		if (p) {
			memset(p, 0, sizeof(T) * n);
			munlock(p, sizeof(T) * n);
		}
		std::allocator<T>::deallocate(p, n);
	}
};

typedef std::basic_string<char, std::char_traits<char>, secure_allocator<char> > SstringBase;

class sstring : public SstringBase{
public:
	sstring(std::string const & r) : SstringBase(r.c_str(), r.size()) {}
	sstring() : SstringBase() {}
	sstring(sstring const & r) : SstringBase(r.c_str(), r.size()) {}
	sstring(unsigned int size, char s) : SstringBase(size, s) {}
	sstring(char const * r) : SstringBase(r) {}

	sstring & operator=(std::string const & r){
		*this = sstring(r);
		return *this;
	}
	operator std::string()const{
		return std::string(this->c_str(), this->size());
	}
	bool operator == (std::string const & r)const{
		return std::string(*this) == r;
	}
	bool operator != (std::string const & r)const{
		return std::string(*this) != r;
	}
};

inline bool operator == (std::string const & l, sstring const &r){
	return r == l;
}
inline bool operator != (std::string const & l, sstring const &r){
	return r != l;
}


#endif // SSTRING_HPP

//