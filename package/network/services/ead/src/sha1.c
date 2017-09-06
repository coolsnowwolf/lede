/*
 * SHA transform algorithm, originally taken from code written by
 * Peter Gutmann, and placed in the public domain.
 */

static  uint32_t
rol32(uint32_t word, int shift)
{
	return (word << shift) | (word >> (32 - shift));
}

/* The SHA f()-functions.  */

#define f1(x,y,z)   (z ^ (x & (y ^ z)))		/* x ? y : z */
#define f2(x,y,z)   (x ^ y ^ z)			/* XOR */
#define f3(x,y,z)   ((x & y) + (z & (x ^ y)))	/* majority */

/* The SHA Mysterious Constants */

#define K1  0x5A827999L			/* Rounds  0-19: sqrt(2) * 2^30 */
#define K2  0x6ED9EBA1L			/* Rounds 20-39: sqrt(3) * 2^30 */
#define K3  0x8F1BBCDCL			/* Rounds 40-59: sqrt(5) * 2^30 */
#define K4  0xCA62C1D6L			/* Rounds 60-79: sqrt(10) * 2^30 */

/**
 * sha_transform - single block SHA1 transform
 *
 * @digest: 160 bit digest to update
 * @data:   512 bits of data to hash
 * @W:      80 words of workspace (see note)
 *
 * This function generates a SHA1 digest for a single 512-bit block.
 * Be warned, it does not handle padding and message digest, do not
 * confuse it with the full FIPS 180-1 digest algorithm for variable
 * length messages.
 *
 * Note: If the hash is security sensitive, the caller should be sure
 * to clear the workspace. This is left to the caller to avoid
 * unnecessary clears between chained hashing operations.
 */
static void sha_transform(uint32_t *digest, const unsigned char *in, uint32_t *W)
{
	uint32_t a, b, c, d, e, t, i;

	for (i = 0; i < 16; i++) {
		int ofs = 4 * i;

		/* word load/store may be unaligned here, so use bytes instead */
		W[i] =
			(in[ofs+0] << 24) |
			(in[ofs+1] << 16) |
			(in[ofs+2] << 8) |
			 in[ofs+3];
	}

	for (i = 0; i < 64; i++)
		W[i+16] = rol32(W[i+13] ^ W[i+8] ^ W[i+2] ^ W[i], 1);

	a = digest[0];
	b = digest[1];
	c = digest[2];
	d = digest[3];
	e = digest[4];

	for (i = 0; i < 20; i++) {
		t = f1(b, c, d) + K1 + rol32(a, 5) + e + W[i];
		e = d; d = c; c = rol32(b, 30); b = a; a = t;
	}

	for (; i < 40; i ++) {
		t = f2(b, c, d) + K2 + rol32(a, 5) + e + W[i];
		e = d; d = c; c = rol32(b, 30); b = a; a = t;
	}

	for (; i < 60; i ++) {
		t = f3(b, c, d) + K3 + rol32(a, 5) + e + W[i];
		e = d; d = c; c = rol32(b, 30); b = a; a = t;
	}

	for (; i < 80; i ++) {
		t = f2(b, c, d) + K4 + rol32(a, 5) + e + W[i];
		e = d; d = c; c = rol32(b, 30); b = a; a = t;
	}

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
	digest[4] += e;
}

/**
 * sha_init - initialize the vectors for a SHA1 digest
 * @buf: vector to initialize
 */
static void sha_init(uint32_t *buf)
{
	buf[0] = 0x67452301;
	buf[1] = 0xefcdab89;
	buf[2] = 0x98badcfe;
	buf[3] = 0x10325476;
	buf[4] = 0xc3d2e1f0;
}

