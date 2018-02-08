#ifndef _TX_THIN_INFO_H
#define _TX_THIN_INFO_H

#include <string>
#include <list>
#include <ostream>

#include "money.h"
#include "datetime.h"
#include "serializable.h"
#include "description.h"
#include "attachment.h"
#include "version.h"

namespace SC {

class ASKeys;

namespace Thin {

class TxInfo : public SC::Serializable {
	std::string _id;
	// creation date - when transaction was created
	DateTime _dt;

	// sender's wallet id
	std::string _sender;
	std::string _receiver;
	SC::Money _amount;
	SC::Money _fee;
	SC::DateTime _processeDT;
	std::string _blockId;
	std::list<SC::Description> _descrs;

	std::list<Attachment> _atts;

	Version _version;
	Version _workingVersion;

	mutable bool _isSb;
	std::string _sbId;

public:
	TxInfo();

	std::string sender() const;
	void sender(std::string const & a);

	std::string receiver() const;
	void receiver(std::string const & a);

	// payment + fees
	// payment = money() - fees()
	SC::Money money() const;
	void money(SC::Money const & m);

	SC::Money fee() const;
	void fee(SC::Money const & m);

	SC::DateTime creationDT() const;
	void creationDT( SC::DateTime const & dt );

	SC::DateTime processeDT() const;
	void processeDT(SC::DateTime const & dt);

	bool isProcessed() const;

	int distance( int lastBlockN ) const;
	void blockId( std::string const & a );
	std::string blockId() const;

	// description if it is incoming transaction
	std::string inDescription(ASKeys & key, std::string const & pw) const;
	// description if it is outgoing transaction
	std::string outDescription(ASKeys & key, std::string const & pw) const;

	bool isSb() const;

	void addDescription(SC::Description const &descr);

	std::list<SC::Attachment> attachments() const;
	void attachments( const std::list<SC::Attachment> & a );

	bool hasAttachment() const;
	void loadAttachmentFromRepository();

	// to show all data 
	void dump(std::ostream & out) const;

	bool operator!=( const TxInfo & r ) const;
	bool operator==( const TxInfo & r ) const;

	bool empty() const;
	void clear();

	std::string status( int lastBlockN ) const;

	std::string sbId() const;
	void sbId( const std::string & id );

	// Serializable interface
	std::string data() const;
	LoadPosition load(LoadPosition const & in);

	std::string repData() const;
	LoadPosition repLoad(LoadPosition const & in);

	std::string typeId() const;
	std::string id() const;
	void id(const std::string & aid);
};

}
} //SC
#endif // _TX_THIN_INFO_H
