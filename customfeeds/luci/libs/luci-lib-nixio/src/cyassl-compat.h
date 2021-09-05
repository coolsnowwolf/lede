#define WITH_CYASSL
#define WITHOUT_OPENSSL
#include <openssl/ssl.h>

typedef unsigned int word32;


#define MD5_DIGEST_LENGTH 16
typedef struct MD5_CTX {
    int dummy[24];
} MD5_CTX;

void InitMd5(MD5_CTX*);
void Md5Update(MD5_CTX*, void*, word32);
void Md5Final(MD5_CTX*, void*);


#define SHA_DIGEST_LENGTH 20
typedef struct SHA_CTX {
    int dummy[24];
} SHA_CTX;

void InitSha(SHA_CTX*);
void ShaUpdate(SHA_CTX*, void*, word32);
void ShaFinal(SHA_CTX*, void*);

int MD5_Init(MD5_CTX *md5);
int MD5_Update(MD5_CTX *md5, void *input, unsigned long sz);
int MD5_Final(void *input, MD5_CTX *md5);
int SHA1_Init(SHA_CTX *md5);
int SHA1_Update(SHA_CTX *sha, void *input, unsigned long sz);
int SHA1_Final(void *input, SHA_CTX *sha);
