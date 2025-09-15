
#ifndef HEADER_DH_KEY_H
#define HEADER_DH_KEY_H

#define NN_DIGIT_BITS		32
#define DH_KEY_LEN			192
#define NN_DIGIT_LEN		4
#define MAX_NN_DIGITS		49

extern void GenerateDHPublicKey(unsigned char *memPtr, unsigned char *ran_buf, int ran_len,
								unsigned char *dhkey, int *dhkey_len);

extern void GenerateDHSecreteKey(unsigned char *memPtr, unsigned char *ran_buf, int ran_len,
								 unsigned char *peer_dhkey, int peer_dhkey_len,
								 unsigned char *secrete_dhkey, int *secrete_dhkey_len);

extern void DH_freeall(void);

#endif /*HEADER_DH_KEY_H */
