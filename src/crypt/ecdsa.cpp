#include <iostream>
#include <cassert>

#include "ecdsasw.h"
#include "exception.h"

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "cipherFactory.h"
#include "rndgen.h"
#include "strutils.h"

#include <boost/thread/recursive_mutex.hpp>

using std::string;
using std::cout;
using std::endl;

using namespace SC;

static boost::recursive_mutex _sslMutex;
typedef boost::recursive_mutex::scoped_lock tScoped_lock;

EC_KEY* getPKey(){
	tScoped_lock scoped_lock( _sslMutex );

	EC_KEY* ret = EC_KEY_new_by_curve_name(NID_secp256k1);
	if( !ret ){
		throw Exception("ECDSA module : getPKey() : pkey is NULL", "ECDSA module : getPKey()");
	}
	return ret;
}
static bool Sign(string const & hash, string & sign, EC_KEY* pkey) {
	tScoped_lock scoped_lock( _sslMutex );

	sign.clear();
	unsigned char pchSig[10000];
	unsigned int nSize = 0;
	if(!EC_KEY_check_key(pkey)){
		throw Exception("ECDSA module : Sign() : Error in pkey", "ECDSA module : Sign()");
	}
	if (!ECDSA_sign(0, (unsigned char*)hash.c_str(), hash.size(), pchSig, &nSize, pkey))
		return false;
	sign = string((char const *)pchSig, nSize);
	return true;
}
static bool Verify(string const & hash, string const & sign, EC_KEY* pkey) {
	// -1 = error, 0 = bad sig, 1 = good
	tScoped_lock scoped_lock( _sslMutex );

	if (ECDSA_verify(0, (unsigned char*)hash.c_str(), hash.size(), (unsigned char*)sign.c_str(), sign.size(), pkey) != 1)
		return false;
	return true;
}
static sstring GetPubKey(EC_KEY* pkey) {
	tScoped_lock scoped_lock( _sslMutex );

	int nSize = i2o_ECPublicKey(pkey, NULL);
	if (!nSize){
		throw Exception("ECDSA module : GetPubKey() : i2o_ECPublicKey failed", "ECDSA module ::GetPubKey()");
	}
	//!! 20120324
	sstring pubKey(nSize, 0);
//	sstring pubKey(nSize + 1, 0);
	unsigned char* pbegin = (unsigned char*)pubKey.data();
	if (i2o_ECPublicKey(pkey, &pbegin) != nSize){
		throw Exception("ECDSA module : GetPubKey() : i2o_ECPublicKey returned unexpected size", "ECDSA module ::GetPubKey()");
	}
	return pubKey;
}
static bool regenerate_key(EC_KEY *eckey, BIGNUM *priv_key) {
	tScoped_lock scoped_lock( _sslMutex );

	bool ret = false;
	if (!eckey) return ret;
	const EC_GROUP *group = EC_KEY_get0_group(eckey);

	BN_CTX *ctx = BN_CTX_new();

	if( ctx ) {
		EC_POINT * pub_key = EC_POINT_new(group);
		if( pub_key ) {
			if( EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx) ){

				EC_KEY_set_private_key(eckey, priv_key);
				EC_KEY_set_public_key(eckey, pub_key);
				ret = true;
			}
			EC_POINT_free(pub_key);
		}else{
			throw Exception("ECDSA module : regenerate_key() pub_key = NULL", "ECDSA module ::regenerate_key()");
		}
		BN_CTX_free(ctx);
	}else{
		throw Exception("ECDSA module : regenerate_key() ctx = NULL", "ECDSA module ::regenerate_key()");
	}
	return ret;
}
static bool SetPubKey(string const & pubKey, EC_KEY* pkey) {
	tScoped_lock scoped_lock( _sslMutex );
	
	const unsigned char* pbegin = (const unsigned char*)pubKey.c_str();
	if (!o2i_ECPublicKey(&pkey, &pbegin, pubKey.size()))
		return false;
	return true;
}
static bool SetSecret(sstring const& secret, EC_KEY*&pkey) {
	tScoped_lock scoped_lock( _sslMutex );
	
	EC_KEY_free(pkey);
	pkey = getPKey();
	if (secret.size() != 32){
		throw Exception("ECDSA module : SetSecret() : secret must be 32 bytes", "ECDSA module ::SetSecret()");
	}
	BIGNUM *bn = BN_bin2bn((const unsigned char*)secret.c_str(),32,BN_new());
	if (bn == NULL)
		throw Exception("ECDSA module : SetSecret() : BN_bin2bn failed", "ECDSA module ::SetSecret()");
	if (!regenerate_key(pkey,bn))
		throw Exception("ECDSA module : SetSecret() : EC_KEY_regenerate_key failed", "ECDSA module ::SetSecret()");
	BN_clear_free(bn);
	return EC_KEY_check_key(pkey);
}
static sstring GetSecret(EC_KEY* pkey) {
	tScoped_lock scoped_lock( _sslMutex );

	sstring ret(32, 0);
	const BIGNUM *bn = EC_KEY_get0_private_key(pkey);
	int nBytes = BN_num_bytes(bn);
	if (bn == NULL)
		throw Exception("ECDSA module : GetSecret() : EC_KEY_get0_private_key failed", "ECDSA module ::GetSecret()");
	int n=BN_bn2bin(bn, (unsigned char*)(ret.data() + 32 - nBytes));
	if (n != nBytes) 
		throw Exception("ECDSA module : GetSecret() : BN_bn2bin failed", "ECDSA module ::GetSecret()");
	return ret;
}

string ECDSA::id() const {
	return "ECDSA";
}
//internal operation to create sign data (string of bytes)
string ECDSA::createSign(string const & hash, sstring const & key){
	string ret;
	EC_KEY* pkey = getPKey();
	if(!SetSecret(key, pkey)){
		throw Exception("ECDSA module : createSign() : SetSecret error", "ECDSA module ::createSign()");
	}
	if(!Sign(hash, ret, pkey)){
		throw Exception("ECDSA module : createSign() : Sign creation error", "ECDSA module ::createSign()");
	}
	EC_KEY_free(pkey);
	return ret;
}
bool ECDSA::verifySign(string const & hash, string const & sign, string const & pubKey){
	EC_KEY* pkey = getPKey();
	if( !SetPubKey( pubKey, pkey ) ){
		throw Exception("ECDSA module : verifySign() : SetPubKey error", "ECDSA module ::verifySign()");
	}
	bool ret = Verify(hash, sign, pkey);
	EC_KEY_free(pkey);
	return ret;
}
void ECDSA::genKeyPair(sstring & priv, sstring & pub){
	tScoped_lock scoped_lock( _sslMutex );

	bool isOk = false;
	do{
		EC_KEY* pkey = getPKey();
		if (!EC_KEY_generate_key(pkey)){
			throw Exception("ECDSA module : genKeyPair() : Error in generation of key process", "ECDSA module ::genKeyPair()");
		}
		pub = GetPubKey(pkey);
		priv = GetSecret(pkey);
		try{
			isOk = SetPubKey( string(pub.c_str(), 0, pub.size()) , pkey ) && SetSecret( priv, pkey );
		}catch(...){
//cout << " ECDSA::genKeyPair throw" << endl;
			isOk = false;
		}
		isOk = isOk ? EC_KEY_check_key(pkey) : false;
		EC_KEY_free(pkey);
		pkey = NULL;
	}while(!isOk);
	createSign("11112222333344445555666677778888", priv);
}
ECDSA::ECDSA(){
//	cout << "ECDSA::ECDSA() this=" << (int)this << "\n";
}
ECDSA::~ECDSA(){
//	cout << "ECDSA::~ECDSA() this=" << (int)this << "\n";
}

static string extendKey(string key, int size) {
	if((int)key.size() != size) {
		CipherFactory *pCF = CipherFactory::instance();
		CipherFactory::tHash hasher = pCF->hash("SHA256");
		SC::IHash keyHash = hasher->hash(key);
		key = keyHash.hash();
		if( (int)key.size() < size) {
//add extra bytes using hash
			while ((int)key.size() < size){
				keyHash = hasher->hash(key, keyHash);
				key += keyHash.hash();
			}
		}
		key = key.substr(0, size);
	}
	assert(key.size() == size);
	return key;
}
BIGNUM *GenRndBN( EC_KEY* pkey, sstring const & symmKey ){
	tScoped_lock scoped_lock( _sslMutex );

	BIGNUM *pOrder = BN_new(), *pRet;
	const EC_GROUP *group = EC_KEY_get0_group(pkey);
	EC_GROUP_get_order(group, pOrder, NULL);

	if( symmKey.empty() ) {
		pRet = BN_new();
		do{
			if(!BN_rand_range(pRet, pOrder)){
				throw(":GenBN. Does not generate random number.");
			}
		}while(BN_is_zero(pRet));
	} else {
		int len = BN_num_bytes(pOrder);
		string key = extendKey( symmKey, len );
		pRet = BN_bin2bn((const unsigned char *)key.c_str(), len, NULL);
		len = BN_num_bits( pRet );
		while( BN_cmp( pRet, pOrder ) >= 0 ) {
			BN_clear_bit(pRet, len--);
		}
	}
	BN_free(pOrder);
	return pRet;
}
static Error crypt(string const & text, string & cipher, EC_KEY* pkey, sstring const & symmKey = "") {
	tScoped_lock scoped_lock( _sslMutex );
	Error err;
	const EC_GROUP *group = EC_KEY_get0_group(pkey);
	const EC_POINT *K = EC_KEY_get0_public_key(pkey);

	EC_POINT *L = EC_POINT_new(group);
	EC_POINT *LK = EC_POINT_new(group);

	BIGNUM *l = GenRndBN( pkey, symmKey );

	BN_CTX *ctx = BN_CTX_new();
	if( ctx ) {
		BN_CTX_start(ctx);
		if( EC_POINT_mul(group, LK, NULL, K, l, ctx) ) {
			if( EC_POINT_mul(group, L, l, NULL, NULL, ctx) ) {
				BIGNUM *LQ = EC_POINT_point2bn(group, L, POINT_CONVERSION_UNCOMPRESSED, NULL, ctx);
				BIGNUM *LKQ = EC_POINT_point2bn(group, LK, POINT_CONVERSION_UNCOMPRESSED, NULL, ctx);

				int viShortIntLen = sizeof(short int);
				string key(BN_num_bytes(LKQ), 0);

				BN_bn2bin(LKQ, (unsigned char *)key.c_str());

				CipherFactory *pCF = CipherFactory::instance();
				CipherFactory::tBlock chipper = pCF->block("AES");

				key = extendKey(key, chipper->keyLength());
				string chText = chipper->encrypt(text, key);

				cipher.resize(viShortIntLen + BN_num_bytes(LQ));
				*(short int *)cipher.c_str() = (short int)BN_num_bytes(LQ);

				BN_bn2bin( LQ, (unsigned char *)cipher.c_str() + viShortIntLen );

				cipher += chText;

				BN_free(LQ);
				BN_free(LKQ);
			} else {
				err = Error("Error: Can`t calculate the L.", "ecdsa::crypt");
			}
		} else {
			err = Error("Error: Can`t calculate the LK.", "ecdsa::crypt");
		}
		BN_CTX_end(ctx);
	} else {
		err = Error("Error: Can`t allocate the ctx.", "ecdsa::crypt");
	}
	BN_free(l);
	EC_POINT_free(L);
	EC_POINT_free(LK);

	BN_CTX_free(ctx);
	return err;
}
static Error dcrypt(string const & cipher, sstring & text, EC_KEY* pkey) {
	tScoped_lock scoped_lock( _sslMutex );

	Error err;

	const BIGNUM *pBNK = EC_KEY_get0_private_key(pkey);
	const EC_GROUP *group = EC_KEY_get0_group(pkey);

	EC_POINT *LKQ = EC_POINT_new(group);

	int viShortIntLen = sizeof(short int);
	short int *piBNLen = (short int *)cipher.c_str();

	int viCKLen = *piBNLen;
	if( viCKLen <= (int)cipher.size() ) {
		BIGNUM *pBNLQ = BN_bin2bn((const unsigned char *)(cipher.c_str() + viShortIntLen), viCKLen, NULL);
		BN_CTX *ctx = BN_CTX_new();
		if( ctx ) {
			BN_CTX_start(ctx);

			EC_POINT *LQ = EC_POINT_bn2point(group, pBNLQ, NULL, ctx);
			if( EC_POINT_mul(group, LKQ, NULL, LQ, pBNK, ctx) ) {
				BIGNUM *pBNLKQ = EC_POINT_point2bn(group, LKQ, POINT_CONVERSION_UNCOMPRESSED, NULL, ctx);

				string key(BN_num_bytes(pBNLKQ), 0);

				CipherFactory *pCF = CipherFactory::instance();
				CipherFactory::tBlock chipper = pCF->block("AES");

				BN_bn2bin(pBNLKQ, (unsigned char *)key.c_str());
				key = extendKey(key, chipper->keyLength());

				string cText(cipher, viShortIntLen + viCKLen, cipher.size());
				string otext = chipper->decrypt(cText, key);

				text = otext;
				BN_free(pBNLKQ);
			} else {
				err = Error("Error: Can`t calculate the LKQ.", "ecdsa::decrypt");
			}
			EC_POINT_free(LQ);
			BN_CTX_end(ctx);
		} else {
			err = Error("Error: Can`t allocate the ctx.", "ecdsa::decrypt");
		}
		BN_CTX_free(ctx);
		BN_free(pBNLQ);
	} else {
		err = Error("Error: The length of LGK big number is wrong.", "ecdsa::decrypt");
	}
	EC_POINT_free(LKQ);
	return err;
}
Error ECDSA::encrypt(sstring const & text, string & cipher, string const & pubKey) {
	EC_KEY* pkey = getPKey();
	if( !SetPubKey( pubKey, pkey ) ) {
		EC_KEY_free(pkey);
		return Error("Error: SetPubKey error.", "ecdsa::encrypt");
	}
	Error ret = crypt(text, cipher, pkey);
	EC_KEY_free(pkey);
	return ret;
}
Error ECDSA::decrypt(string const & cipher, sstring & text, sstring const & privKey) {
	EC_KEY* pkey = getPKey();
	if( !SetSecret(privKey, pkey) ) {
		EC_KEY_free(pkey);
		return Error("Error: SetSecret error.", "ecdsa::decrypt");
	}
	Error ret = dcrypt(cipher, text, pkey);
	EC_KEY_free(pkey);
	return ret;
}
Error ECDSA::encrypt(sstring const & text, std::string & cipher, std::string const & pubKey, sstring const & symmKey) {
//cout << "ECDSA::encrypt 1" << endl;
	EC_KEY* pkey = getPKey();
	if( !SetPubKey( pubKey, pkey ) ) {
		EC_KEY_free(pkey);
		return Error("Error: SetPubKey error.", "ecdsa::encrypt");
	}
	Error ret = crypt(text, cipher, pkey, symmKey);
	EC_KEY_free(pkey);
	return ret;
}
Error ECDSA::decryptSym(std::string const & cipher, sstring & text, std::string const & pubKey, sstring const & symmKey) {
	tScoped_lock scoped_lock( _sslMutex );

	Error err;

	EC_KEY* pkey = getPKey();
	if( SetPubKey( pubKey, pkey ) ) {
		const EC_GROUP *group = EC_KEY_get0_group(pkey);
		const EC_POINT *K = EC_KEY_get0_public_key(pkey);

		BN_CTX *ctx = BN_CTX_new();
		if( ctx ){
			BN_CTX_start(ctx);

			EC_POINT *LK = EC_POINT_new(group);
			BIGNUM *l = GenRndBN( pkey, symmKey );

			if( EC_POINT_mul(group, LK, NULL, K, l, ctx)) {

				BIGNUM *lk = EC_POINT_point2bn(group, LK, POINT_CONVERSION_UNCOMPRESSED, NULL, ctx);

				string key(BN_num_bytes(lk), 0);
				BN_bn2bin(lk, (unsigned char *)key.c_str());

				int keySize = *(short int *)cipher.c_str();;
				if( keySize <= (int)cipher.size() ) {
					CipherFactory *pCF = CipherFactory::instance();
					CipherFactory::tBlock chipper = pCF->block("AES");

					key = extendKey(key, chipper->keyLength());

					string cText(cipher, sizeof(short int) + keySize, cipher.size());
					string otext = chipper->decrypt(cText, key);

					text = otext;
				} else {
					err = Error("Error: The length of LG big number is wrong.", "ecdsa::decryptSym");
				}
				BN_free(lk);
			} else {
				err = Error("Error: Can`t calculate the LKQ.", "ecdsa::decryptSym");
			}
			BN_free(l);
			EC_POINT_free(LK);

			BN_CTX_end(ctx);
			BN_CTX_free(ctx);
		} else {
			err = Error("Error: Can`t allocate the ctx.", "ecdsa::decryptSym");
		}
	} else {
		err = Error("Error: SetPubKey error.", "ecdsa::decryptSym");
	}
	EC_KEY_free( pkey );
	return err;
}


//