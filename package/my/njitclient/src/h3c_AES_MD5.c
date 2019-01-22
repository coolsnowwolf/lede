#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include "aes.h"
#include "h3c_dict.h"
#include "h3c_AES_MD5.h"

/* 大小端问题 */
//大小端检测，如果 ENDIANNESS=='l' 则为小端
static union { char c[4]; unsigned long mylong; } endian_test = {{ 'l', '?', '?', 'b' } }; 
#define ENDIANNESS ((char)endian_test.mylong)
//大小端转换
#define BigLittleSwap32(A) ((((uint32_t)(A)&0xff000000)>>24)|\
	(((uint32_t)(A)&0x00ff0000)>>8)|\
	(((uint32_t)(A)&0x0000ff00)<<8)|\
	(((uint32_t)(A)&0x000000ff)<<24))
//void main()
//{
//	test();
//}

int test() {
	unsigned char encrypt_data[32] = { 0xcf, 0xfe, 0x64, 0x73, 0xd5, 0x73, 0x3b, 0x1f, 0x9e, 0x9a, 0xee, 0x1a, 0x6b, 0x76, 0x47, 0xc8, 0x9e, 0x27, 0xc8, 0x92, 0x25, 0x78, 0xc4, 0xc8, 0x27, 0x03, 0x34, 0x50, 0xb6, 0x10, 0xb8, 0x35 };
	unsigned char decrypt_data[32];
	unsigned char i;

	// decrypt
	h3c_AES_MD5_decryption(decrypt_data, encrypt_data);

	// print
	printf("encrypted string = ");
	for (i = 0; i<32; ++i) {
		printf("%x%x", ((int)encrypt_data[i] >> 4) & 0xf,
			(int)encrypt_data[i] & 0xf);
	}
	printf("\n");
	printf("decrypted string = ");
	for (i = 0; i<32; ++i) {
		printf("%x%x", ((int)decrypt_data[i] >> 4) & 0xf,
			(int)decrypt_data[i] & 0xf);
	}
	// the decrypt_data should be 8719362833108a6e16b08e33943601542511372d8d1fb1ab31aa17059118a6ba
	getchar();
	return 0;
}

//参考 h3c_AES_MD5.md 文档中对算法的说明
int h3c_AES_MD5_decryption(unsigned char *decrypt_data, unsigned char *encrypt_data)
{
	const unsigned char key[16] = { 0xEC, 0xD4, 0x4F, 0x7B, 0xC6, 0xDD, 0x7D, 0xDE, 0x2B, 0x7B, 0x51, 0xAB, 0x4A, 0x6F, 0x5A, 0x22 };        // AES_BLOCK_SIZE = 16
	unsigned char iv1[16] = { 'a', '@', '4', 'd', 'e', '%', '#', '1', 'a', 's', 'd', 'f', 's', 'd', '2', '4' };        // init vector
	unsigned char iv2[16] = { 'a', '@', '4', 'd', 'e', '%', '#', '1', 'a', 's', 'd', 'f', 's', 'd', '2', '4' }; //每次加密、解密后，IV会被改变！因此需要两组IV完成两次“独立的”解密
	unsigned int length_1;
	unsigned int length_2;
	unsigned char tmp0[32];
	unsigned char sig[255];
	unsigned char tmp2[16];
	unsigned char tmp3[16];
	// decrypt
	AES128_CBC_decrypt_buffer(tmp0, encrypt_data, 32, key, iv1);
	memcpy(decrypt_data, tmp0, 16);
	length_1 = *(tmp0 + 5);
	get_sig(*(uint32_t *)tmp0, *(tmp0 + 4), length_1, sig);
	MD5(sig, length_1, tmp2);

	AES128_CBC_decrypt_buffer(tmp3, tmp0+16, 16, tmp2, iv2);

	memcpy(decrypt_data + 16, tmp3, 16);

	length_2 = *(tmp3 + 15);
	get_sig(*(uint32_t *)(tmp3 + 10), *(tmp3 + 14), length_2, sig + length_1);
	if (length_1 + length_2>32)
	{
		memcpy(decrypt_data, sig, 32);
	}
	else
	{
		memcpy(decrypt_data, sig, length_1 + length_2);
	}
	MD5(decrypt_data, 32, decrypt_data);//获取MD5摘要数据，将结果存到前16位中
	MD5(decrypt_data, 16, decrypt_data + 16);//将前一MD5的结果再做一次MD5，存到后16位
	return 0;
}


// 查找表函数，根据索引值、偏移量以及长度查找序列
char* get_sig(uint32_t index, int offset, int length, unsigned char* dst)
{
	uint32_t index_tmp;
	const unsigned char *base_address;
	// printf("index = %x\n" ,index);
	
	if (ENDIANNESS == 'l')
	{
		index_tmp = BigLittleSwap32(index); // 小端情况，如PC架构
	}
	else
	{
		index_tmp = index; // 大端序，如MIPS架构
	}
	switch (index_tmp) // this line works in mips.
	{
	case 0x15D0EADF:base_address = x15D0EADF; break;
	case 0x09F40DE7:base_address = x09F40DE7; break;
	case 0x189DF2CE:base_address = x189DF2CE; break;
	case 0x1A8AED5C:base_address = x1A8AED5C; break;
	case 0x2F40F9D8:base_address = x2F40F9D8; break;
	case 0x45941B98:base_address = x45941B98; break;
	case 0x359F23C3:base_address = x359F23C3; break;
	case 0x4167F618:base_address = x4167F618; break;
	case 0x4BEE2975:base_address = x4BEE2975; break;
	case 0x57F612DD:base_address = x57F612DD; break;
	case 0x36D426DD:base_address = x36D426DD; break;
	case 0x5E51B55F:base_address = x5E51B55F; break;
	case 0xF245C41D:base_address = xF245C41D; break;
	case 0x545CEFE0:base_address = x545CEFE0; break;
	case 0x5E08D0E0:base_address = x5E08D0E0; break;
	case 0x5E877108:base_address = x5E877108; break;
	case 0x7130F3A5:base_address = x7130F3A5; break;
	case 0x6DD9572F:base_address = x6DD9572F; break;
	case 0x67195BB6:base_address = x67195BB6; break;
	case 0x70886376:base_address = x70886376; break;
	case 0x7137CD3A:base_address = x7137CD3A; break;
	case 0xEF432FB9:base_address = xEF432FB9; break;
	case 0xED4B7E03:base_address = xED4B7E03; break;
	case 0xF1C07C91:base_address = xF1C07C91; break;
	case 0xE45C3124:base_address = xE45C3124; break;
	case 0xDAB58841:base_address = xDAB58841; break;
	case 0xDA58A32E:base_address = xDA58A32E; break;
	case 0xDF977247:base_address = xDF977247; break;
	case 0xEAE0E002:base_address = xEAE0E002; break;
	case 0xC3A46827:base_address = xC3A46827; break;
	case 0xB0F2918A:base_address = xB0F2918A; break;
	case 0xAF4ED407:base_address = xAF4ED407; break;
	case 0xBB9EC2E1:base_address = xBB9EC2E1; break;
	case 0xA8902F8B:base_address = xA8902F8B; break;
	case 0xA3747988:base_address = xA3747988; break;
	case 0x84E4BC95:base_address = x84E4BC95; break;
	case 0x763F4D5B:base_address = x763F4D5B; break;
	case 0x9F6C10A6:base_address = x9F6C10A6; break;
	case 0xA9407E26:base_address = xA9407E26; break;
	case 0xCCF59F07:base_address = xCCF59F07; break;
	default:
		printf("lookup dict failed.\n"); // 查表失败
		base_address = xCCF59F07;
		break;
	}
	memcpy(dst, base_address + offset, length);
	return dst;
}
