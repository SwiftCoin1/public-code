#include <iostream>
#include <map>
#include <algorithm>

#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <cstdio>

#include "datetime.h"
#include "strutils.h"
#include "exception.h"
#include "application.h"
#include "log.h"

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace SC;
using std::string;
using std::cout;
using std::endl;

static boost::mutex _mutex;

static bool isDigitOrEmpty(string const &val){
	int l = val.size();
	bool ret = true;
	if(l){
		ret = l == std::count_if(val.begin(), val.end(), isdigit);
	}
	return ret;
}
bool checkTime(int y, int m, int d, int h = 0, int mi = 0, int s = 0){
	boost::mutex::scoped_lock scoped_lock(_mutex);
	m -= 1;
	y -= 1900;
	if(y < 70){
		y = 72 + y % 4;
	}
	struct tm ts;
	ts.tm_sec = s;
	ts.tm_min = mi;
	ts.tm_hour = h;
	ts.tm_mday = d;
	ts.tm_mon = m;
	ts.tm_year = y;
	ts.tm_isdst = -1;
	time_t t = mktime(&ts);
	tm * ptm = localtime(&t);
	if(!ptm)
		return false;
	return 	(s == ts.tm_sec)  && (mi == ts.tm_min) && (h == ts.tm_hour) &&
				(d == ts.tm_mday) && (m == ts.tm_mon) && (y == ts.tm_year) && (ts.tm_year == ptm->tm_year);
}
string transformTime(string const & input, string iformat, string oformat, bool & isTime){
//cout << "transformTime input=" << input << ", iformat=" << iformat << ", oformat=" << oformat << endl;
	if( input.size() != iformat.size() ) {
		isTime = false;
		FileSystemHelper::instance()->errLogger()->error("DateTime::transformTime input.size() != iformat.size() input=" + input + ", iformat=" + iformat + ", oformat=" + oformat + ", isTime=" + toString(isTime));
		FileSystemHelper::instance()->logger()->log("input.size() != iformat.size() input=" + input + ", iformat=" + iformat + ", oformat=" + oformat + ", isTime=" + toString(isTime), "DateTime::transformTime", "critical");
		return string();
	}

	iformat = ToUpper(iformat);
	oformat = ToUpper(oformat);
	std::map<char, string> p;
	string keys = "YMDHISO";
	for(size_t i = 0; i != iformat.size(); ++i){
		if(keys.find(iformat[i]) != string::npos){
			p[iformat[i]].append(1, input[i]);
		}else if(iformat[i] != input[i]){
			
			FileSystemHelper::instance()->errLogger()->error("DateTime::transformTime iformat[i] != input[i] i" + toString( (int)i ) + ", iformat=" + iformat + ", isTime=" + toString(isTime));
			
			isTime = false;
			return string();
		}
	}

	if(p['Y'].size() == 2){
		p['Y'] = "20" + p['Y'];
	}
	int iy = atoi(p['Y'].c_str());
	int im = atoi(p['M'].c_str());
	int id = atoi(p['D'].c_str());
	int ih = atoi(p['H'].c_str());
	int ii = atoi(p['I'].c_str());
	int is = atoi(p['S'].c_str());

	isTime = checkTime(iy, im, id, ih, ii, is) && isDigitOrEmpty(p['O']);

	string ret;
	for(size_t i = 0; i != oformat.size(); ++i){
		if(keys.find(oformat[i]) != string::npos){
			if(p[oformat[i]].size()){
				ret.append(p[oformat[i]], 0, 1);
				p[oformat[i]].erase(0, 1);
			}else{
				ret.append("0");
			}
		}else{
			ret += oformat[i];
		}
	}
	return ret;
}

DateTime::DateTime() {
	clear();
}
DateTime::DateTime(std::string const & strDate) {
	load(strDate);
}

int DateTime::Years() const {
	return _data[0];
}
int DateTime::Months() const {
	return _data[1];
}
int DateTime::Days() const {
	return _data[2];
}
int DateTime::Hours() const {
	return _data[3];
}
int DateTime::Minutes() const {
	return _data[4];
}
int DateTime::Seconds() const {
	return _data[5];
}
int DateTime::MSeconds() const {
	return _data[6];
}
bool DateTime::operator==(const DateTime & r) const {
	for(int i = 0; i < 7; ++i){
		if(_data[i] != r._data[i]) {
			return false;
		}
	}
	return true;
//	return (_years == r._years) && (_months == r._months) && (_days == r._days) && (_hours == r._hours) && (_minutes == r._minutes) && (_seconds == r._seconds) && (_mseconds == r._mseconds);
}
bool DateTime::operator>(const DateTime & r) const {
	for(int i = 0; i < 7; ++i){
		if(_data[i] > r._data[i]){
			return true;
		}else if(_data[i] < r._data[i]){
			return false;
		}
	}
	return false;
}
bool DateTime::operator!=(const DateTime & r) const {
	return !(*this == r);
}
bool DateTime::operator>=(const DateTime & r) const {
	return *this > r || *this == r;
}
bool DateTime::operator<(const DateTime & r) const {
	return r > *this;
}
bool DateTime::operator<=(const DateTime & r) const {
	return r >= *this;
}
string DateTime::data(string const & format) const{
	string date =	::toString(Months(), 2) + "/" + ::toString(Days(), 2) + "/" + ::toString(Years(), 4) + "#" + 
						::toString(Hours(), 2) + "-" + ::toString(Minutes(), 2) + "-" + ::toString(Seconds(), 2) + "-" +
						::toString(MSeconds(), 3);
	bool isTime;
	date = transformTime(date, CSTR_DEFAULT_FORMAT, format, isTime);
	return date;
}
DateTime & DateTime::operator=(const DateTime & r) {
	for(int i = 0; i < 7; ++i){
		_data[i] = r._data[i];
	}
	return *this;
}

static string LMonths[12] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
static string SMonths[12] = {"Jan.","Feb.","Mar.","Apr.","May","Jun.","Jul.","Aug.","Sep.","Oct.","Nov.","Dec."};
static string GMTMonths[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

string strMonth(int m) {
	if(m < 1 || m > 12){
		return LMonths[0];
	}
	return LMonths[m -1];
}

string DateTime::dataStrMonth(std::string const & format) const {
	string ret = data(format);
	size_t pos = format.find("m");
	if(pos == string::npos){
		pos = format.find("M");
	}
	if(pos == string::npos){
		return ret;
	}
	ret = ret.substr(0, pos) + strMonth(_data[1]) + ret.substr(pos + 2, ret.size());
	return ret;
}
const DateTime & DateTime::load(string const & in, string const & format){
	bool isTime;
	string inDT = transformTime(in, format, CSTR_DEFAULT_FORMAT, isTime);
	if(!isTime){
		*this = oldest();
//		throw Exception("DateTime::load Error in input data=" + in, "DateTime::load");
	}
	sscanf(inDT.c_str(), CSTR_DEFAULT_PARSE_FORMAT, &_data[1], &_data[2], &_data[0], &_data[3], &_data[4], &_data[5], &_data[6]);
	return *this;
}
string DateTime::Binary() const{
	string ret;
	ret.append(1, (char)(_data[0] >> 8));
	ret.append(1, (char)_data[0]);
	ret.append(1, (char)_data[1]);
	ret.append(1, (char)_data[2]);
	ret.append(1, (char)_data[3]);
	ret.append(1, (char)_data[4]);
	ret.append(1, (char)_data[5]);
	ret.append(1, (char)(_data[6] >> 8));
	ret.append(1, (char)_data[6]);
	return ret;
}
void DateTime::LoadBinary(std::string const & in){
	if(in.size() != 9){
		throw Exception("DateTime::LoadBinary Error - wrong date binary format", "DateTime::LoadBinary");
	}
	_data[0] = (((int)(unsigned char)in[0]) << 8) + (unsigned char)in[1];
	_data[1] = in[2];
	_data[2] = in[3];
	_data[3] = in[4];
	_data[4] = in[5];
	_data[5] = in[6];
	_data[6] = (((int)(unsigned char)in[7]) << 8) + (unsigned char)in[8];
}
DateTime & DateTime::setCurrent(){
	boost::mutex::scoped_lock scoped_lock(_mutex);
	time_t current = time(NULL);
	tm * ptm = localtime(&current);

	_data[5] = ptm->tm_sec;
	_data[4] = ptm->tm_min;
	_data[3] = ptm->tm_hour;
	_data[2] = ptm->tm_mday;
	_data[1] = ptm->tm_mon + 1;
	_data[0] = ptm->tm_year + 1900;

	boost::posix_time::ptime t(boost::posix_time::microsec_clock::local_time());
	boost::posix_time::time_duration td = t.time_of_day();
	_data[6] = td.total_milliseconds() % 1000;

	if( _data[6] < 0 ) {
		_data[6] = 0;
	}
	return *this;
}
DateTime DateTime::Current(){
	DateTime dt;
	return dt.setCurrent();
}
DateTime DateTime::oldest() {
	DateTime dt;
	dt.load(CSTR_OLDEST_TIME);
	return dt;
}
bool DateTime::Check(string const & in, string const & format){
	try{
		DateTime dt;
		dt.load(in, format);
	}catch(Exception const & ex){
		return false;
	}
	return true;
}
time_t getTime(int y, int m, int d, int h = 0, int mi = 0, int s = 0){
	boost::mutex::scoped_lock scoped_lock(_mutex);
	m -= 1;
	y -= 1900;
	struct tm ts;
	ts.tm_sec = s;
	ts.tm_min = mi;
	ts.tm_hour = h;
	ts.tm_mday = d;
	ts.tm_mon = m;
	ts.tm_year = y;
	ts.tm_isdst = -1;
	return mktime(&ts);
}
int DateTime::diffDays(const DateTime & r) const {
	return diffSecs(r) / (3600 * 24);
}
int DateTime::diffSecs(const DateTime & r) const {
	time_t lt = getTime(_data[0], _data[1], _data[2], _data[3], _data[4], _data[5]);
	time_t rt = getTime(r._data[0], r._data[1], r._data[2], r._data[3], r._data[4], r._data[5]);

//cout << "DateTime::diffSecs lt=" << lt << ", rt=" << rt << "\n";
//cout << "lt - rt=" << lt - rt << "\n";

	return difftime (lt, rt);
}
bool DateTime::fromGMT(string const & in) {
	if(in.empty()){
		return false;
	}
	clear();
	string time = AllTrim(in);
	string t = GetField(time, " ", 1);
	_data[2] = toInt(t);
	if(!_data[2]){
		return false;
	}
	_data[1] = 0;
	t = GetField(time, " ", 2);
	for(int i = 0; i < 12; ++i){
		if(t == GMTMonths[i]){
			_data[1] = i + 1;
			break;
		}
	}
	t = GetField(time, " ", 3);
	_data[0] = toInt(t);
	if(!_data[0]){
		return false;
	}
	time = GetField(time, " ", 4);
	if(time.empty()){
		return false;
	}
	t = GetField(time, ":", 0);
	_data[3] = toInt(t);
	t = GetField(time, ":", 1);
	_data[4] = toInt(t);
	t = GetField(time, ":", 2);
	_data[5] = toInt(t);

	_data[6] = 0;
	return true;
}
DateTime::operator bool() const {
	return _data[0] && _data[2];
}
void DateTime::clear() {
	_data[0] = 0;
	_data[1] = 0;
	_data[2] = 0;
	_data[3] = 0;
	_data[4] = 0;
	_data[5] = 0;
	_data[6] = 0;
}
void DateTime::addSeconds(int amnt) {
	time_t lt = getTime(_data[0], _data[1], _data[2], _data[3], _data[4], _data[5]);
	lt += amnt;

	boost::mutex::scoped_lock scoped_lock(_mutex);
	tm * ptm = localtime(&lt);

	_data[5] = ptm->tm_sec;
	_data[4] = ptm->tm_min;
	_data[3] = ptm->tm_hour;
	_data[2] = ptm->tm_mday;
	_data[1] = ptm->tm_mon + 1;
	_data[0] = ptm->tm_year + 1900;
}
void DateTime::addDays( int a ) {
	addSeconds( a * 3600 * 24  );
}
void DateTime::addWeek( int a ) {
	addSeconds( a * 3600 * 24 * 7 );
}
void DateTime::addMonth( int a ) {

	int y = a / 12;
	int m = a % 12;

	_data[1] += m;
	if( _data[1] > 12 ) {
		++_data[ 0 ];
		_data[1] = _data[1] % 12;
	}
	addYear( y ); 
}
void DateTime::addYear( int a ) {
	_data[ 0 ] += a;
}
DateTime & DateTime::setBeginDay() {
	_data[3] = 0;
	_data[4] = 0;
	_data[5] = 0;
	_data[6] = 1;
	return *this;
}
DateTime & DateTime::setEnDay() {
	_data[3] = 23;
	_data[4] = 59;
	_data[5] = 59;
	_data[6] = 999;
	return *this;
}

//===============================================

TimeInterval::TimeInterval(DateTime const & begin, DateTime const & end) : _begin(begin), _end(end) {}

int TimeInterval::periodSec(TimeInterval r){
	TimeInterval l = *this;
	if(_begin > r._begin){
		l = r;
		r = *this;
	}
	if(l._end < r._begin)
		return 0;
	DateTime lt = r._begin;
	DateTime rt = l._end < r._end ? l._end : r._end;
	return rt.diffSecs(lt);
}
void TimeInterval::reSetIntervalByEnd(DateTime const & dt) {
	int diff = _end.diffSecs(dt);
	_begin.addSeconds(diff);
	_end.addSeconds(diff);
}


//
