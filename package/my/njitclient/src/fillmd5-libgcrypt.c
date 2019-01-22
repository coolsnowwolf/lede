/* File: fillmd5.c
 * ---------------
 *
 */

#include <stdint.h>
#include <assert.h>

#include <gcrypt.h>	// GNU cryptographic function library (libgcrypt)


void FillMD5Area(uint8_t digest[], uint8_t id, const char passwd[], const uint8_t srcMD5[])
{
	uint8_t	msgbuf[128]; // msgbuf = ‘id‘ + ‘passwd’ + ‘srcMD5’
	size_t	msglen;
	size_t	passlen;

	passlen = strlen(passwd);
	msglen = 1 + passlen + 16;
	assert(sizeof(msgbuf) >= msglen);

	msgbuf[0] = id;
	memcpy(msgbuf+1,	 passwd, passlen);
	memcpy(msgbuf+1+passlen, srcMD5, 16);

	// Calls libgcrypt function for MD5 generation
	gcry_md_hash_buffer(GCRY_MD_MD5, digest, msgbuf, msglen);
}

