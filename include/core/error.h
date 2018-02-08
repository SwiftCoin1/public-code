#ifndef _ERROR_H
#define _ERROR_H

#include <string>

namespace SC{

class Error{
public:
	enum Code {
		NoError = 1,
		WalletIsInRepository,
		WalletKeysIsInRepository,
		NotInRepository,
		WrongWalletPassWord,
		WrongPassWordInDbConfig,
		NoStartedSW,
		WrongDB,
		DBIsNotEncrypted,
		DBisRecovering,
		BadProxyServer,
		AttachmentIsErased,
		WrongTransaction,
		NoData,
		TimeOut,
		Warning
	};

	Error();
	Error(std::string const & text, std::string const & place);
	~Error() throw();

	std::string place() const;
	std::string text() const;

	Error & place( std::string const & v );
	Error & text( std::string const & v );

	// true if does not have text
	operator bool() const;
	Error operator &&(Error const & r);

	Code code() const;
	void code(Code const & c);

private:
	std::string _place;
	std::string _text;
	Code _code;
};

}; // SC

#endif // _ERROR_H
