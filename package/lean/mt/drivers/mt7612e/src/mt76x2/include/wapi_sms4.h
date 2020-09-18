

/* SMS4 encryption/decryption definition */
/* Parameter : */
/*		Input 	- the incoming message packet */
/*		Ouput 	- the result ouput */
/*		rk		- key */
void SMS4Crypt(unsigned char *Input, unsigned char *Output, unsigned int *rk);

/* SMS4 key extend algorithm */
void SMS4KeyExt(unsigned char *Key, unsigned int *rk, unsigned int CryptFlag);

