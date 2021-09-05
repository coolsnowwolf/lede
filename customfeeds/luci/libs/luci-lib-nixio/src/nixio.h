#ifndef NIXIO_H_
#define NIXIO_H_

#define NIXIO_OOM "out of memory"

#define NIXIO_META "nixio.socket"
#define NIXIO_FILE_META "nixio.file"
#define NIXIO_GLOB_META "nixio.glob"
#define NIXIO_DIR_META "nixio.dir"
#define _FILE_OFFSET_BITS 64

#define NIXIO_PUSH_CONSTANT(x) \
	lua_pushinteger(L, x); \
	lua_setfield(L, -2, #x);

/* uClibc: broken as always */
#define _LARGEFILE_SOURCE

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luaconf.h>

#define NIXIO_BUFFERSIZE 8192

typedef struct nixio_socket {
	int fd;
	int domain;
	int type;
	int protocol;
} nixio_sock;

typedef struct nixio_address {
	int family;
	char host[128];
	int port;
	int prefix;
} nixio_addr;

int nixio__perror(lua_State *L);
int nixio__pstatus(lua_State *L, int condition);

#if defined(LUA_NUMBER_DOUBLE) || defined(LNUM_DOUBLE) || defined(LNUM_LDOUBLE)
#define NIXIO_DOUBLE 1
#define nixio__checknumber luaL_checknumber
#define nixio__pushnumber  lua_pushnumber
#define nixio__optnumber   luaL_optnumber
#else
#define nixio__checknumber luaL_checkinteger
#define nixio__pushnumber  lua_pushinteger
#define nixio__optnumber   luaL_optinteger
#endif


#ifndef __WINNT__

#define NIXIO_API extern

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <netdb.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>

#define NIXIO_SEP "/"
#define NIXIO_PATHSEP ":"

#define nixio__perror_s nixio__perror
#define nixio__pstatus_s nixio__pstatus

int nixio__check_group(lua_State *L, int idx);
int nixio__check_user(lua_State *L, int idx);

typedef struct stat nixio_stat_t;

#else /* __WINNT__ */

#define NIXIO_API extern __declspec(dllexport)
#define NIXIO_SEP "\\"
#define NIXIO_PATHSEP ";"
#include "mingw-compat.h"

typedef struct _stati64 nixio_stat_t;

#endif

nixio_sock* nixio__checksock(lua_State *L);
int nixio__checksockfd(lua_State *L);
int nixio__checkfd(lua_State *L, int ud);
int nixio__tofd(lua_State *L, int ud);
int nixio__nulliter(lua_State *L);

int nixio__addr_parse(nixio_addr *addr, struct sockaddr *saddr);
int nixio__addr_write(nixio_addr *addr, struct sockaddr *saddr);

int nixio__check_mode(lua_State *L, int idx, int def);
int nixio__mode_write(int mode, char *modestr);

int nixio__push_stat(lua_State *L, nixio_stat_t *buf);

static const char nixio__bin2hex[16] = {
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/* Module functions */
void nixio_open_file(lua_State *L);
void nixio_open_socket(lua_State *L);
void nixio_open_sockopt(lua_State *L);
void nixio_open_bind(lua_State *L);
void nixio_open_address(lua_State *L);
void nixio_open_protoent(lua_State *L);
void nixio_open_poll(lua_State *L);
void nixio_open_io(lua_State *L);
void nixio_open_splice(lua_State *L);
void nixio_open_process(lua_State *L);
void nixio_open_syslog(lua_State *L);
void nixio_open_bit(lua_State *L);
void nixio_open_bin(lua_State *L);
void nixio_open_fs(lua_State *L);
void nixio_open_user(lua_State *L);

#ifndef NO_TLS
void nixio_open_tls_crypto(lua_State *L);
void nixio_open_tls_context(lua_State *L);
void nixio_open_tls_socket(lua_State *L);
#endif

/* Method functions */

#endif /* NIXIO_H_ */
