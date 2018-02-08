#include "msg.h"

using namespace SC;
using std::string;

Message::Message() : _text(""), _type(Empty) {}
Message::Message(string const & text, Type const & t) : _text(text), _type(t) {}
Message::~Message() {}

string Message::text() const {
	return _text;
}
Message::Type Message::type() const {
	return _type;
}
void Message::type( Type const & t) {
	_type = t;
}
string Message::strType() const {
	switch( _type) {
	case Empty:
		return "Empty";
	case NeedUpdateAppVersion:
		return "NeedUpdateAppVersion";
	case OtherServerStarted:
		return "OtherServerStarted";
	case SecondCopyStarted:
		return "SecondCopyStarted";
	case IncomingTx:
		return "IncomingTx";
	case UpdateBlocks:
		return "UpdateBlocks";
	case StartMakingBlock:
		return "StartMakingBlock";
	case StopMakingBlock:
		return "StopMakingBlock";
	case SendFirstBlock:
		return "SendFirstBlock";
	case SendedToNodes:
		return "SendedToNodes";
	case SendingFirstBlockEnded:
		return "SendingFirstBlockEnded";
	case HasActiveNodes:
		return "HasActiveNodes";
	case FoundNetworkNode:
		return "FoundNetworkNode";
	case DoForceSynchronize:
		return "DoForceSynchronize";
	case ForceSynchronizeStarted:
		return "ForceSynchronizeStarted";
	case ForceSynchronizeFinished:
		return "ForceSynchronizeFinished";
	case NewBlockAdded2Chain:
		return "NewBlockAdded2Chain";
	case RefreshMainWindow:
		return "RefreshMainWindow";
	case OurAddress:
		return "OurAddress";
	case StartNodeServer:
		return "StartNodeServer";
	case NoInternetConnection:
		return "NoInternetConnection";

	case LoadWalletId:
		return "LoadWalletId";
	case ReprocessBlockNum:
		return "ReprocessBlockNum";

	case WrongTx:
	case ShowMovie:
	case ShowStaticImage:
	case GetCheckInfo:
	case NewUserBonusSentOK:
	case NewUserBonusAlredySent:
	case NewUserBonusConfigError:
	case ServerTimeError:
	case GMTChanged:
	case FoundSupportNode:
	case AddSbTxInQueue:
	case EraseSbTxFromQueue:
	case Progress:
	case ModelHasSBConf:
		return "DefValue";
	}
	return "Ok";
}
bool Message::needProcess( MessageState const & state ) const {
	bool ret = state == MessageState::Created;
	switch(_type){
		case ShowMovie:
		case ShowStaticImage:
			ret = ret || state == MessageState::OneProcessed || state == MessageState::MultiProcessed;
		case Empty:
		case NeedUpdateAppVersion:
		case OtherServerStarted:
		case SecondCopyStarted:
		case IncomingTx:
		case WrongTx:
		case UpdateBlocks:
		case StartMakingBlock:
		case StopMakingBlock:
		case SendFirstBlock:
		case SendedToNodes:
		case SendingFirstBlockEnded:
		case HasActiveNodes:
		case FoundNetworkNode:
		case DoForceSynchronize:
		case ForceSynchronizeStarted:
		case ForceSynchronizeFinished:
		case NewBlockAdded2Chain:
		case RefreshMainWindow:
		case StartNodeServer:
		case NoInternetConnection:
		case LoadWalletId:
		case ReprocessBlockNum:
		case GetCheckInfo:
		case NewUserBonusSentOK:
		case NewUserBonusAlredySent:
		case NewUserBonusConfigError:
		case ServerTimeError:
		case GMTChanged:
		case OurAddress:
		case FoundSupportNode:
		case AddSbTxInQueue:
		case EraseSbTxFromQueue:
		case Progress:
		case ModelHasSBConf:
		{}
	}
	return ret;
}
bool Message::processed( MessageState const & state ) const {
	bool ret = state != MessageState::Created;
	switch( _type ){
		case ShowMovie:
		case ShowStaticImage:
			ret = ret || state == MessageState::Created;

		case Empty:
		case NeedUpdateAppVersion:
		case OtherServerStarted:
		case SecondCopyStarted:
		case IncomingTx:
		case WrongTx:
		case UpdateBlocks:
		case StartMakingBlock:
		case StopMakingBlock:
		case SendFirstBlock:
		case SendedToNodes:
		case SendingFirstBlockEnded:
		case HasActiveNodes:
		case FoundNetworkNode:
		case DoForceSynchronize:
		case ForceSynchronizeStarted:
		case ForceSynchronizeFinished:
		case NewBlockAdded2Chain:
		case RefreshMainWindow:
		case StartNodeServer:
		case NoInternetConnection:
		case LoadWalletId:
		case ReprocessBlockNum:
		case GetCheckInfo:
		case NewUserBonusSentOK:
		case NewUserBonusAlredySent:
		case NewUserBonusConfigError:
		case ServerTimeError:
		case GMTChanged:
		case OurAddress:
		case FoundSupportNode:
		case AddSbTxInQueue:
		case EraseSbTxFromQueue:
		case Progress:
		case ModelHasSBConf:
		{}
	}
	return ret;
}
MessageState Message::process(MessageState const & state, bool processResult) const {
	MessageState ret;
	switch( (MessageState::State)state ) {
		case MessageState::Created:
			if( processResult )
				ret._state = MessageState::OneProcessed;
		case MessageState::OneProcessed: {
			if( processResult )
				ret._state = MessageState::MultiProcessed;
		}
		case MessageState::MultiProcessed:
		case MessageState::Error:
		{}
	}
	return ret;
}

//