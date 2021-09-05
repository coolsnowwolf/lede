/*
 * nixio - Linux I/O library for lua
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "nixio-tls.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static int nixio_crypto_hash__init(lua_State *L, int hmac) {
	const char *type = luaL_checkstring(L, 1);
	nixio_hash *hash = lua_newuserdata(L, sizeof(nixio_hash));

	if (!strcmp(type, "md5")) {
		hash->type = NIXIO_HASH_MD5;
		hash->digest_size = MD5_DIGEST_LENGTH;
		hash->block_size = 64;
		hash->ctx = malloc(sizeof(MD5_CTX));
		if (!hash->ctx) {
			return luaL_error(L, NIXIO_OOM);
		}
		MD5_Init((MD5_CTX*)hash->ctx);
		hash->init = (nixio_hash_initcb)MD5_Init;
		hash->update = (nixio_hash_updatecb)MD5_Update;
		hash->final = (nixio_hash_finalcb)MD5_Final;
	} else if (!strcmp(type, "sha1")) {
		hash->type = NIXIO_HASH_SHA1;
		hash->digest_size = SHA_DIGEST_LENGTH;
		hash->block_size = 64;
		hash->ctx = malloc(sizeof(SHA_CTX));
		if (!hash->ctx) {
			return luaL_error(L, NIXIO_OOM);
		}
		SHA1_Init((SHA_CTX*)hash->ctx);
		hash->init = (nixio_hash_initcb)SHA1_Init;
		hash->update = (nixio_hash_updatecb)SHA1_Update;
		hash->final = (nixio_hash_finalcb)SHA1_Final;
	} else {
		luaL_argerror(L, 1, "supported values: md5, sha1");
	}

	luaL_getmetatable(L, NIXIO_CRYPTO_HASH_META);
	lua_setmetatable(L, -2);

	if (hmac) {
		const char *key = luaL_checklstring(L, 2, &hash->key_size);
		if (hash->key_size > hash->block_size) {
			hash->update(hash->ctx, key, hash->key_size);
			hash->final(hash->digest, hash->ctx);
			hash->init(hash->ctx);
			hash->key_size = hash->digest_size;
			memcpy(hash->key, hash->digest, hash->key_size);
		} else {
			memcpy(hash->key, key, hash->key_size);
		}

		unsigned char pad[NIXIO_CRYPTO_BLOCK_SIZE];
		for (uint i = 0; i < hash->block_size; i++) {
			pad[i] = (i < hash->key_size) ? (0x36 ^ hash->key[i]) : 0x36;
		}
		hash->update(hash->ctx, pad, hash->block_size);
		hash->type |= NIXIO_HMAC_BIT;
	}

	return 1;
}

static int nixio_crypto_hash(lua_State *L) {
	return nixio_crypto_hash__init(L, 0);
}

static int nixio_crypto_hmac(lua_State *L) {
	return nixio_crypto_hash__init(L, 1);
}

static int nixio_crypto_hash_update(lua_State *L) {
	nixio_hash *hash = luaL_checkudata(L, 1, NIXIO_CRYPTO_HASH_META);
	if (hash->type) {
		size_t len;
		const char *chunk = luaL_checklstring(L, 2, &len);
		hash->update(hash->ctx, chunk, len);
		lua_pushvalue(L, 1);
		return 1;
	} else {
		return luaL_error(L, "Tried to update finalized hash object.");
	}
}

static int nixio_crypto_hash_final(lua_State *L) {
	nixio_hash *hash = luaL_checkudata(L, 1, NIXIO_CRYPTO_HASH_META);
	if (hash->type & NIXIO_HMAC_BIT) {
		hash->final(hash->digest, hash->ctx);
		hash->init(hash->ctx);

		unsigned char pad[NIXIO_CRYPTO_BLOCK_SIZE];
		for (uint i = 0; i < hash->block_size; i++) {
			pad[i] = (i < hash->key_size) ? (0x5c ^ hash->key[i]) : 0x5c;
		}

		hash->update(hash->ctx, pad, hash->block_size);
		hash->update(hash->ctx, hash->digest, hash->digest_size);
	}

	if (hash->type) {
		hash->type = NIXIO_HASH_NONE;
		hash->final(hash->digest, hash->ctx);
		free(hash->ctx);
	}

	char hashdigest[NIXIO_DIGEST_SIZE*2];
	for (uint i=0; i < hash->digest_size; i++) {
		hashdigest[2*i]   = nixio__bin2hex[(hash->digest[i] & 0xf0) >> 4];
		hashdigest[2*i+1] = nixio__bin2hex[(hash->digest[i] & 0x0f)];
	}

	lua_pushlstring(L, hashdigest, hash->digest_size * 2);
	memcpy(hashdigest, hash->digest, hash->digest_size);
	lua_pushlstring(L, hashdigest, hash->digest_size);

	return 2;
}

static int nixio_crypto_hash__gc(lua_State *L) {
	nixio_hash *hash = luaL_checkudata(L, 1, NIXIO_CRYPTO_HASH_META);
	if (hash->type) {
		hash->final(hash->digest, hash->ctx);
		free(hash->ctx);
		hash->type = NIXIO_HASH_NONE;
	}
	return 0;
}

static int nixio_crypto_hash__tostring(lua_State *L) {
	nixio_hash *hash = luaL_checkudata(L, 1, NIXIO_CRYPTO_HASH_META);
	lua_pushfstring(L, "nixio hash object: %p", hash);
	return 1;
}


/* module table */
static const luaL_reg R[] = {
	{"hash",		nixio_crypto_hash},
	{"hmac",		nixio_crypto_hmac},
	{NULL,			NULL}
};

/* hash table */
static const luaL_reg M[] = {
	{"update",		nixio_crypto_hash_update},
	{"final",		nixio_crypto_hash_final},
	{"__gc",		nixio_crypto_hash__gc},
	{"__tostring",	nixio_crypto_hash__tostring},
	{NULL,			NULL}
};



void nixio_open_tls_crypto(lua_State *L) {
	luaL_newmetatable(L, NIXIO_CRYPTO_HASH_META);
	luaL_register(L, NULL, M);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pop(L, 1);

	lua_newtable(L);
    luaL_register(L, NULL, R);

	lua_setfield(L, -2, "crypto");
}
