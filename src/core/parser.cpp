#include "parser.h"
#include "spirit.h"

#include <iostream>

namespace SC {

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	template <typename Iterator>
	struct SepStringParser : qi::grammar<Iterator, std::vector<std::string>()> {
		SepStringParser(char sep) : SepStringParser::base_type(result){
			using qi::lit;
			using ascii::char_;
			using boost::spirit::ascii::cntrl;
			using boost::spirit::qi::eol;
			// adds to substring any symbol except the sep and the control symbols
			t_string %= *(char_ - char_(sep) - cntrl);
			// field is the t_string followed sep symbol
			field = lit(std::string(&sep, 1)) >> t_string;
			// result is the string vector
			result = *field >> *eol;
		}

		qi::rule<Iterator, std::string() > t_string;
		qi::rule<Iterator, std::string() > field;
		qi::rule<Iterator, std::vector<std::string>() > result;
	};

	void Parser::clear(){
		_values.clear();
	}
	// parses string in form of "delimited list" to vector of substrings
	Parser::StringVector & Parser::parseString(std::string const & str, char sep){
		// clear resulting vector
		clear();
		if(str.empty()){
			return _values;
		}
		// add separator to the beginning of the string
		std::string pstr;
		pstr = sep + str;
		// get start iterator
		std::string::const_iterator begin = pstr.begin();
		// get end iterator
		std::string::const_iterator end = pstr.end();
		typedef std::string::const_iterator iterator_type;
		typedef SepStringParser<iterator_type> SepStringParser;
		SepStringParser parser(sep);
		// start parsing
		// result puts in the _values
		bool r = parse(begin, end, parser, _values);
		if (!r) {
			throw "Error is in the separated string parsing process";
		}
		return _values;
	}
	
}


