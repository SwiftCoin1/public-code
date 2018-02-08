#include <openssl/evp.h>
#include <sstream>

#include "exception.h"
#include "strutils.h"
#include "sstring.h"
#include "aescipher.h"

using std::string;

using namespace SC;

void aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx, int nrounds = 7) {
	if(!nrounds){
		throw Exception("::aes_init Error - nrounds is too small.", "::aes_init");
	}
	unsigned char key[32], iv[32];
	int i = EVP_BytesToKey(EVP_aes_256_ofb(), EVP_sha512(), salt, key_data, key_data_len, nrounds, key, iv);
	if( i != 32) {
		throw Exception("::aes_init Error Key size is " + toString(i)  + " bits, but should be 256 bits", "::aes_init");
	}
	if(e_ctx){
		EVP_CIPHER_CTX_init(e_ctx);
		EVP_EncryptInit_ex(e_ctx, EVP_aes_256_ofb(), NULL, key, iv);
	}
	if(d_ctx){
		EVP_CIPHER_CTX_init(d_ctx);
		EVP_DecryptInit_ex(d_ctx, EVP_aes_256_ofb(), NULL, key, iv);
	}
}
sstring aes_encrypt(EVP_CIPHER_CTX *e, sstring const & in){
	int len = in.size(), f_len = 0;
	sstring ret(len, 0);

	if(!EVP_EncryptUpdate(e, (unsigned char*)ret.data(), &len, (unsigned char*)in.c_str(), len)){
		throw Exception("::aes_encrypt ERROR in EVP_EncryptUpdate.", "::aes_encrypt");
	}
	if(!EVP_EncryptFinal_ex(e, (unsigned char*)ret.data() + len, &f_len)){
		throw Exception("aes_encrypt: ERROR in EVP_EncryptFinal_ex.", "::aes_encrypt");
	}
	return ret;
}
sstring aes_decrypt(EVP_CIPHER_CTX *e, sstring const &in){
	int len = in.size(), f_len = 0;
	sstring ret(len, 0);

	if(!EVP_DecryptUpdate(e, (unsigned char*)ret.data(), &len, (unsigned char*)in.c_str(), len)){
		throw Exception("::aes_decrypt ERROR in EVP_DecryptUpdate.", "::aes_decrypt");
	}
	if(!EVP_DecryptFinal_ex(e, (unsigned char*)ret.data() + len, &f_len)){
		throw Exception("::aes_decrypt ERROR in EVP_DecryptFinal_ex.", "::aes_decrypt");
	}
	return ret;
}

sstring AESCipher::encrypt(sstring const & text, sstring const & key) {
	EVP_CIPHER_CTX en;
	unsigned char *psalt = _salt.size() ? (unsigned char *)_salt.c_str() : NULL;
	aes_init((unsigned char *)key.c_str(), key.size(), psalt, &en, NULL, 10);

	sstring ret = aes_encrypt(&en, text);
	EVP_CIPHER_CTX_cleanup(&en);
	return ret;
}

sstring AESCipher::decrypt(sstring const & cipher, sstring const & key) {
	EVP_CIPHER_CTX de;
	unsigned char *psalt = _salt.size() ? (unsigned char *)_salt.c_str() : NULL;
	aes_init((unsigned char *)key.c_str(), key.size(), psalt, NULL, &de, 10);

	sstring ret = aes_decrypt(&de, cipher);
	EVP_CIPHER_CTX_cleanup(&de);
	return ret;
}

string AESCipher::id() const {
	return "aes_ofb";
}

int AESCipher::keyLength() const {
	return 32;
}
void AESCipher::salt(std::string const &salt) {
	_salt = salt;
}
int AESCipher::saltLength() const{
	return 8;
}

//