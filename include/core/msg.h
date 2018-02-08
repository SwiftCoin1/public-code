#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <string>

namespace SC{

struct MessageState {
	enum State {
		Created = 1,
		OneProcessed,
		MultiProcessed,
		Error
	};
	State _state;
	std::string _err;
	
	MessageState() : _state( Created ) {}
	operator State() const {
		return _state;
	}
};

class Message{
public:
	enum Type {
		Empty = 1,
		NeedUpdateAppVersion,
		OtherServerStarted,
		SecondCopyStarted,
		IncomingTx,						// has got an incoming transaction
		WrongTx,							// when sending transactions can't be processed on server side
		UpdateBlocks,					// when incoming server recieves command to try get blocks from other node
		StartMakingBlock,				// when incoming server is available from other network
		StopMakingBlock,				// when incoming server isn't available from other network
		SendFirstBlock,				// send first block to all active nodes
		SendedToNodes,					// node successfuly received block (one message for one node)
		SendingFirstBlockEnded,
		HasActiveNodes,
		FoundNetworkNode,				// addreGetter sends if found node in IRC
		FoundSupportNode,				// addreGetter sends if found support node ( sw root now )
		DoForceSynchronize,			// start force sinchronize activity
		ForceSynchronizeStarted,
		ForceSynchronizeFinished,	// force sinchronize activity is finished
		NewBlockAdded2Chain,			// emited if new block is added to chain
		RefreshMainWindow,
		StartNodeServer,				// after client is shown we have to start node server
		NoInternetConnection,		// emit from nodeserver when can't get own ip adress

		LoadWalletId,					// when client is loading stored wallet - "restoring wallet" procedure
		ReprocessBlockNum,			// when reprocessed block in "restoring wallet" procedure
		
		ShowMovie,						// when user selects movie option
		ShowStaticImage,				// when user selects image option

		GetCheckInfo,					// request for Servers::Check results

		NewUserBonusSentOK,
		NewUserBonusAlredySent,
		NewUserBonusConfigError,

		ServerTimeError,
		GMTChanged,

		AddSbTxInQueue,				// SBProcessor send task to Server::Block to create  
		EraseSbTxFromQueue,			// SBProcessor remove task if there is transaction with processed SB (return SB payment from server to client)

		Progress,						// some processor sends messages about its progress in the text() will string with percents in form "01", "02" etc
		ModelHasSBConf,

		NeedInstallNewVersion,

		OurAddress
	};

	Message();
	Message(std::string const & text, Type const & t);
	virtual ~Message();

	std::string text() const;

	Type type() const;
	void type( Type const & t);

	std::string strType() const;

	bool needProcess( MessageState const & state ) const;
	bool processed( MessageState const & state ) const;
	MessageState process(MessageState const & state, bool processResult) const;

private:
	std::string _text;
	Type _type;
};

}; // SC

#endif // _MESSAGE_H
