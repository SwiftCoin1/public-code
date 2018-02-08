#ifndef _KEYS_H
#define _KEYS_H

#include <string>

#include "iblockcipher.h"
#include "isigncipher.h"
#include "ihashcipher.h"
#include "isign.h"
#include "sptr.h"
#include "sstring.h"
#include "serializable.h"
#include "parser.h"
#include "version.h"

#include "serids.h"

#define CINT_KEY_MAJOR_VERSION						0
#define CINT_KEY_MINOR_VERSION						1

namespace SC{

using SPtr::shared;

class PProvider;
class ASKeys;
class KeyBuilder;

//pair of keys for asymmetric cryptosystem
class Key {
public:
	Key(ASKeys * aparent, sstring const & key);
	virtual ~Key();

	//take key in the string form - vector of bytes
	sstring key() const;
	ASKeys * parent() const;

protected:
	ASKeys * _parent;
	sstring _key;
};

class PrivKey : public Key {
	//salt for block encryption algoriphm
	std::string _salt;
	bool _isOpen;

	bool decrypt(sstring const & passWord);
	void encrypt(sstring const & passWord);
	void checkSalt();
public:
	PrivKey(sstring const & secret, ASKeys * aparent);
	~PrivKey();

	bool open(PProvider const & provider);
	void close(PProvider const & provider);
	bool isOpen() const;
	void isOpen(bool a);

	bool changePassWord(const PProvider & provider);

	void salt(std::string const & asalt);
	std::string salt();
};

class PubKey : public Key {
public:
	PubKey(sstring const & pub, ASKeys * aparent);
	~PubKey();

	bool compare(PubKey const & r) const;
};

class ASKeys : public Serializable {
	Version _version;
	Version _workingVersion;

	std::string _id;
	shared<PubKey> _pub;
	shared<PrivKey> _priv;
	SPtr::shared<ISignCipher> _signer;
	SPtr::shared<IBlockCipher> _cipher;
	SPtr::shared<IHashCipher> _hasher;

	void isAllSeted();
	LoadPosition setPriv(LoadPosition const &, KeyBuilder & keyBuilder);
	LoadPosition setPub(LoadPosition const &, KeyBuilder & keyBuilder);

	ASKeys(ASKeys const & r);
	ASKeys & operator = (ASKeys const & r);
	void setVersion();

public:
	ASKeys(std::string const & aid, SPtr::shared<IHashCipher> ahash, SPtr::shared<ISignCipher> asign);
	ASKeys();

	~ASKeys();

	bool checkSign(Serializable const & data, ISign const & sign);
	ISign sign(Serializable const & data, PProvider const & pprovider);

	bool checkSign(std::string const & data, ISign const & sign);
	ISign sign(std::string const & data, PProvider const & pprovider);

	Error assymetricEncrypt(sstring const & text, std::string & cipher);
	Error assymetricDecrypt(std::string const & cipher, sstring & text, PProvider const & pprovider);

	// encrypt and decrypt for sender and reciever keys (not assymetric for sender)
	Error assymetricEncrypt( sstring const & text, std::string & cipher, sstring const & symmKey );
	Error assymetricDecrypt( std::string const & cipher, sstring & text, sstring const & symmKey );

	PubKey * pub();
	void pub(sstring const & key);
	PrivKey * priv() const;
	void priv(sstring const & key);

	std::string id() const;
	void id(std::string const & aid);
	void createId();

	std::string typeId() const;
	std::string data() const;
	LoadPosition load(LoadPosition const &in);
	std::string repData() const;
	LoadPosition repLoad(LoadPosition const &in);

	SPtr::shared<IBlockCipher> cipher() const;
	void cipher(SPtr::shared<IBlockCipher> in);
	SPtr::shared<ISignCipher> signer() const;
	void signer(SPtr::shared<ISignCipher> in);
	SPtr::shared<IHashCipher> hasher() const;
	void hasher(SPtr::shared<IHashCipher> in);

	SPtr::shared<ASKeys> clonePub() const;
	void copy(ASKeys const & r);

	bool comparePub(ASKeys const & r) const;

};


}; // SC
#endif
