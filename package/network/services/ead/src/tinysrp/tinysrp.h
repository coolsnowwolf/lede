/* Simple API for the tinysrp library. */

#ifndef T_PWD_H
#define MAXUSERLEN      32
#define SESSION_KEY_LEN 40      /* 320-bit session key */
#endif

typedef struct {
	char username[MAXUSERLEN + 1];
	unsigned char key[SESSION_KEY_LEN];
} TSRP_SESSION;

/* These functions are passed a connected socket, and return true for a
successful authentication.  If tsrp is not NULL, the username and key
fields are filled in. */

extern int tsrp_server_authenticate(int s, TSRP_SESSION *tsrp);
extern int tsrp_client_authenticate(int s, char *user, char *pass, TSRP_SESSION *tsrp);
