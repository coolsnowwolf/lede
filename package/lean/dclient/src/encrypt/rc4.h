#ifndef RC4_H_
#define RC4_H_

#ifdef __cplusplus
extern "C" {
#endif

void rc4_crypt(unsigned char *data, int data_len, unsigned char *key, int key_len);

#ifdef __cplusplus
}
#endif

#endif // RC4_H_
