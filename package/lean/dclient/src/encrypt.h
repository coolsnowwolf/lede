#ifndef ENCRYPT_H_
#define ENCRYPT_H_

unsigned char* Get_Md5_Digest(unsigned char* str, int len);
unsigned char* Get_Hmac_Md5_Digest(unsigned char* text, int text_len, unsigned char* key, int key_len);
unsigned char* Get_RC4(unsigned char *data, int data_len, unsigned char *key, int key_len);
unsigned char* Get_CRC32(unsigned char *data, int data_len);
int Get_Base64_Encode(const unsigned char *data, int data_len, unsigned char * output);

#endif // ENCRYPT_H_
