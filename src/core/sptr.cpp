#include "sptr.h"

static boost::mutex _statMutex;
static unsigned long long _amount = 0;
static bool _shutDown = false;

namespace SPtr{

statistic::statistic() {
	if( !_shutDown ) {
		boost::mutex::scoped_lock scoped_lock( _statMutex );
		++_amount;
	}
}
statistic::~statistic() {
	if( !_shutDown ) {
		if( _amount > 1 ) {
			boost::mutex::scoped_lock scoped_lock( _statMutex );
		}
	--_amount;
	}
}
int statistic::amount() {
	if( !_shutDown ) {
		boost::mutex::scoped_lock scoped_lock( _statMutex );
		return _amount;
	}
	return 0;
}
void statistic::shutDown() {
	boost::mutex::scoped_lock scoped_lock( _statMutex );
	_shutDown = 1;
}

}		//namespace SPtr
