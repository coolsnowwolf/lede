#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "polarssl/havege.h"
#include "polarssl/bignum.h"
#include "polarssl/x509.h"
#include "polarssl/rsa.h"

#define PX5G_KEY_META "px5g.key"

typedef struct px5g_rsa {
	int stat;
	havege_state hs;
	rsa_context rsa;
} px5g_rsa;
