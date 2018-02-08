#ifndef _DATETIME_H
#define _DATETIME_H

#include <string>

namespace SC{

#define CSTR_DEFAULT_FORMAT					"mm/dd/yyyy#hh-ii-ss-ooo"
#define CSTR_OLDEST_TIME						"01/01/1980#00-00-01-001"
#define CSTR_DEFAULT_PARSE_FORMAT			"%d/%d/%d#%d-%d-%d-%d"

class DateTime {
private:
//	int _years, _months, _days, _hours, _minutes, _seconds, _mseconds;
	int _data[7];

public:
	DateTime();
	DateTime(std::string const & strDate);

	int Years() const;
	int Months() const;
	int Days() const;

	int Hours() const;
	int Minutes() const;
	int Seconds() const;

	int MSeconds() const;

	bool operator==(const DateTime & r) const;
	bool operator!=(const DateTime & r) const;
	bool operator>(const DateTime & r) const;
	bool operator>=(const DateTime & r) const;
	bool operator<(const DateTime & r) const;
	bool operator<=(const DateTime & r) const;
	operator bool() const;

	DateTime & operator=(const DateTime & r);

	DateTime & setCurrent();

	std::string data(std::string const & format = CSTR_DEFAULT_FORMAT) const;
	const DateTime & load(std::string const & in, std::string const & format = CSTR_DEFAULT_FORMAT);

	std::string dataStrMonth(std::string const & format = CSTR_DEFAULT_FORMAT) const;

	std::string Binary() const;
	void LoadBinary(std::string const & in);

	// this > r, result > 0
	int diffDays(const DateTime & r) const;
	int diffSecs(const DateTime & r) const;

	void addSeconds(int amnt);

	void addDays( int a);
	void addWeek( int a);
	void addMonth( int a);
	void addYear( int a);

	DateTime & setBeginDay();
	DateTime & setEnDay();

	bool fromGMT(std::string const & in);
	void clear();

	static DateTime Current();
	static bool Check(std::string const & in, std::string const & format = CSTR_DEFAULT_FORMAT);
	static DateTime oldest();
};

struct TimeInterval{
	DateTime _begin, _end;

	TimeInterval(DateTime const & begin, DateTime const & end);

	int periodSec(TimeInterval r);
	void reSetIntervalByEnd(DateTime const & dt);
};

}; //SC

#endif
