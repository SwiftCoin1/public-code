#ifndef _KEYBUILDER_H
#define _KEYBUILDER_H

#include "sptr.h"
#include "keys.h"
#include "sstring.h"
#include <string>

namespace SC{

#define CSTR_EMPTY_KEY_ID				"EMPTY_KEY"

#define CSTR_SIGN							"SIGN"
#define CSTR_HASH							"HASH"
#define CSTR_CIPHER						"CIPHER"


class IHashCipher;
class ISignCipher;
class IBlockCipher;
class CipherFactory;


// !!! not thread safe
class KeyBuilder {
	std::string _id;
	SPtr::shared<ASKeys> _key;
	SPtr::shared<IHashCipher> _hasher;
	SPtr::shared<ISignCipher> _signer;
	SPtr::shared<IBlockCipher> _cipher;

	sstring _priv, _pub, _salt;

public:
	KeyBuilder();
	~KeyBuilder();

	//create new ASKey instance
	//clear existing key
	void create(std::string const & id);
	void keys(SPtr::shared<ASKeys> key);

	//take builded instance
	SPtr::shared<ASKeys> get();
	SPtr::shared<ASKeys> getEmpty();

	KeyBuilder & pub(sstring const & pub);
	KeyBuilder & priv(sstring const & priv);
	KeyBuilder & salt(sstring const & asalt);
	KeyBuilder & signer(std::string const & id);
	KeyBuilder & hasher(std::string const & id);
	KeyBuilder & cipher(std::string const & id);
	KeyBuilder & id(std::string const & id);
	KeyBuilder & fromText(std::string const & fieldId, std::string const & val);

	// !!! may be method should be removed
	static KeyBuilder * instance();

};

}; // SC
#endif
