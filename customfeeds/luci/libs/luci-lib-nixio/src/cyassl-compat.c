#include "cyassl-compat.h"

int MD5_Init(MD5_CTX *md5) {
	InitMd5(md5);
	return 1;
}

int MD5_Update(MD5_CTX *md5, void *input, unsigned long sz) {
	Md5Update(md5, input, (word32)sz);
	return 1;
}

int MD5_Final(void *input, MD5_CTX *md5) {
	Md5Final(md5, input);
	return 1;
}

int SHA1_Init(SHA_CTX *sha) {
	InitSha(sha);
	return 1;
}

int SHA1_Update(SHA_CTX *sha, void *input, unsigned long sz) {
	ShaUpdate(sha, input, (word32)sz);
	return 1;
}

int SHA1_Final(void *input, SHA_CTX *sha) {
	ShaFinal(sha, input);
	return 1;
}
