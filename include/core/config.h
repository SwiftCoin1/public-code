#pragma once

#include <string>
#include <map>

namespace SC {

class Config {
protected:
	// Add string to internal tree
	void addstr(std::string str);
	std::map<std::string, std::string> _values;
	std::string _fname;

public:
	static Config * instance(std::string const & confPath = "");

	virtual ~Config();
	// Loads data from the file fName
	virtual bool load(std::string const & fName );

	// Cleans up internal structures
	void clear();
	// Get value from the configuration
	std::string value(std::string const & key) const;
	// returns value of key if it exists in other case returns defValue
	std::string value(std::string const & key, std::string const & defValue) const;
	std::string operator [] (std::string const & key) const;

	// Checks where key has value or not
	bool exist(std::string const & key) const;
	void saveValue(std::string const &key, std::string const &value);

	// strIcmp( val, CSTR_YES )
	bool boolValue( std::string const & key, bool defValue ) const;

	// insert into storage and save to file
	void insert( Config const & r);
};

class MemConfig : public Config {
public:
	// Loads data from the string key=value;key1=value1;....
	bool load(std::string const &);
	std::string data() const;

	void keepValue(std::string const &key, std::string const &value);
	void saveKept();
};
} //SC


