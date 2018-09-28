void rc4_crypt(unsigned char *data, int data_len, unsigned char *key, int key_len)
{
	int i, j, k, x, y;
	unsigned char a, b, s[256];

	for (i = 0; i < 256; i++){
		s[i] = i;
	}

	for (i = j = k = 0; i < 256; i++){
		a = s[i];
		j = (unsigned char)(j + a + key[k]);
		s[i] = s[j]; s[j] = a;
		if (++k >= key_len) k = 0;
	}

	for (i = x = y = 0; i < data_len; i++)
	{
		x = (unsigned char)(x + 1); a = s[x];
		y = (unsigned char)(y + a);
		s[x] = b = s[y];
		s[y] = a;
		data[i] ^= s[(unsigned char)(a + b)];
	}
}
