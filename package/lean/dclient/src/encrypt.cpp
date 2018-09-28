#include "encrypt.h"
#include "encrypt/md5.h"
#include "encrypt/rc4.h"
#include "encrypt/crc32.h"
#include "encrypt/base64.h"

unsigned char* Get_Md5_Digest(unsigned char* str, int len)
{
    static unsigned char digest[16];
    md5(str, len, digest);
    return digest;
}

unsigned char* Get_Hmac_Md5_Digest(unsigned char* text, int text_len, unsigned char* key, int key_len)
{
    static unsigned char digest[16];
    hmac_md5(text, text_len, key, key_len, digest);
    return digest;
}

unsigned char* Get_RC4(unsigned char *data, int data_len, unsigned char *key, int key_len)
{
    rc4_crypt(data, data_len, key, key_len);
    return data;
}

unsigned char* Get_CRC32(unsigned char *data, int data_len)
{
    static unsigned char checksum[4];
    unsigned int result;
    result = crc32(data, data_len);
    checksum[0] = result & 0xff;
    checksum[1] = (result >> 8) & 0xff;
    checksum[2] = (result >> 16) & 0xff;
    checksum[3] = (result >> 24) & 0xff;
    return checksum;
}

int Get_Base64_Encode(const unsigned char *data, int data_len, unsigned char * output)
{
    base64_encode(data, data_len, (char *)output);
    return BASE64_ENCODE_OUT_SIZE(data_len); // 返回加密后的字符串长度
}
