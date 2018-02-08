#ifndef parser_h
#define parser_h

#include <vector>
#include <string>

namespace SC{

// parser interface is used to hide boost::spirit
class Parser{
public:
	typedef std::vector<std::string> StringVector;
	typedef StringVector::iterator StringVectorIterator;

	StringVector & parseString(std::string const & str, char sep);
private:
	StringVector _values;
	void clear();
};

}; // SC

#endif	//parser_h
