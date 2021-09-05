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

#include "nixio.h"
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>

/* Reads argument from given index and transforms it into a mode bitfield */
int nixio__check_mode(lua_State *L, int idx, int def) {
	if (lua_isnoneornil(L, idx) && def > 0) {
		return def;
	} else if (lua_isstring(L, idx) && lua_objlen(L, idx) == 9) {
		int mode = 0;
		const char *modestr = lua_tostring(L, idx);
		int i;
		for (i=0; i<9; i++) {
			if (i % 3 == 0) {			/* read flags */
				if (modestr[i] == 'r') {
					mode |= 1 << (8 - i);
				} else if (modestr[i] != '-') {
					break;
				}
			} else if (i % 3 == 1) {	/* write flags */
				if (modestr[i] == 'w') {
					mode |= 1 << (8 - i);
				} else if (modestr[i] != '-') {
					break;
				}
			} else if (i == 2) {
				if (modestr[i] == 'x') {
					mode |= 00100;
				} else if (modestr[i] == 's') {
					mode |= 04100;
				} else if (modestr[i] == 'S') {
					mode |= 04000;
				} else if (modestr[i] != '-') {
					break;
				}
			} else if (i == 5) {
				if (modestr[i] == 'x') {
					mode |= 00010;
				} else if (modestr[i] == 's') {
					mode |= 02010;
				} else if (modestr[i] == 'S') {
					mode |= 02000;
				} else if (modestr[i] != '-') {
					break;
				}
			} else if (i == 8) {
				if (modestr[i] == 'x') {
					mode |= 00001;
				} else if (modestr[i] == 't') {
					mode |= 01001;
				} else if (modestr[i] == 'T') {
					mode |= 01000;
				} else if (modestr[i] != '-') {
					break;
				}
			}
		}
		if (i == 9) {	/* successfully parsed */
			return mode;
		}
	} else if (lua_isnumber(L, idx)) {
		int decmode = lua_tointeger(L, idx);
		int s = (decmode % 10000)	/ 1000;
		int u = (decmode % 1000)	/ 100;
		int g = (decmode % 100)		/ 10;
		int o = (decmode % 10);

		if (s>=0 && s<=7 && u>=0 && u<=7 && g>=0 && g<=7 && o>=0 && o<=7) {
			return (s << 9) + (u << 6) + (g << 3) + o;
		}
	}

	return luaL_argerror(L, idx, "supported values: [0-7]?[0-7][0-7][0-7], "
				"[-r][-w][-xsS][-r][-w][-xsS][-r][-w][-xtT]");
}

/* Transforms a mode into the modestring */
int nixio__mode_write(int mode, char *modestr) {
	if (modestr) {
		modestr[0] = (mode & 00400) ? 'r' : '-';
		modestr[1] = (mode & 00200) ? 'w' : '-';
		modestr[2] = ((mode & 04100) == 04100) ? 's' :
			(mode & 04000) ? 'S' : (mode & 00100) ? 'x' : '-';
		modestr[3] = (mode & 00040) ? 'r' : '-';
		modestr[4] = (mode & 00020) ? 'w' : '-';
		modestr[5] = ((mode & 02010) == 02010) ? 's' :
			(mode & 02000) ? 'S' : (mode & 00010) ? 'x' : '-';
		modestr[6] = (mode & 00004) ? 'r' : '-';
		modestr[7] = (mode & 00002) ? 'w' : '-';
		modestr[8] = ((mode & 01001) == 01001) ? 't' :
			(mode & 01000) ? 'T' : (mode & 00001) ? 'x' : '-';
	}

	return (mode & 00007) + ((mode & 00070) >> 3) * 10 +
		((mode & 00700) >> 6) * 100 + ((mode & 07000) >> 9) * 1000;
}

static int nixio_access(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	int mode = F_OK;

	for (const char *s = luaL_optstring(L, 2, "f"); *s; s++) {
		if (*s == 'r') {
			mode |= R_OK;
		} else if (*s == 'w') {
			mode |= W_OK;
		} else if (*s == 'x') {
			mode |= X_OK;
		} else if (*s != 'f') {
			return luaL_argerror(L, 2, "supported values: [frwx]");
		}
	}

	return nixio__pstatus(L, !access(path, mode));
}

static int nixio_basename(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	char base[PATH_MAX];
	base[PATH_MAX-1] = 0;

	strncpy(base, path, PATH_MAX-1);
	lua_pushstring(L, basename(base));
	return 1;
}

static int nixio_dirname(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	char base[PATH_MAX];
	base[PATH_MAX-1] = 0;

	strncpy(base, path, PATH_MAX-1);
	lua_pushstring(L, dirname(base));
	return 1;
}

static int nixio_realpath(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	char real[PATH_MAX];

	if (!realpath(path, real)) {
		return nixio__perror(L);
	} else {
		lua_pushstring(L, real);
		return 1;
	}
}

static int nixio_remove(lua_State *L) {
	return nixio__pstatus(L, !remove(luaL_checkstring(L, 1)));
}

static int nixio_unlink(lua_State *L) {
	return nixio__pstatus(L, !unlink(luaL_checkstring(L, 1)));
}

static int nixio_rename(lua_State *L) {
	return nixio__pstatus(L,
			!rename(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
}

static int nixio_rmdir(lua_State *L) {
	return nixio__pstatus(L, !rmdir(luaL_checkstring(L, 1)));
}

static int nixio_mkdir(lua_State *L) {
	return nixio__pstatus(L,
			!mkdir(luaL_checkstring(L, 1), nixio__check_mode(L, 2, 0777)));
}

static int nixio_chmod(lua_State *L) {
	return nixio__pstatus(L,
			!chmod(luaL_checkstring(L, 1), nixio__check_mode(L, 2, -1)));
}

static int nixio_dir__gc(lua_State *L) {
	DIR **dirp = lua_touserdata(L, 1);
	if (dirp && *dirp) {
		closedir(*dirp);
		*dirp = NULL;
	}
	return 0;
}

static int nixio_dir__iter(lua_State *L) {
	DIR **dirp = lua_touserdata(L, lua_upvalueindex(1));
	struct dirent *entry;
	const char *n = NULL;

	if (*dirp) {
		do {
			entry = readdir(*dirp);
			n = (entry) ? entry->d_name : NULL;
		} while(n && n[0] == '.' && (n[1] == 0 || (n[1] == '.' && n[2] == 0)));
	}

	if (n) {
		lua_pushstring(L, n);
	} else {
		if (*dirp) {
			closedir(*dirp);
			*dirp = NULL;
		}
		lua_pushnil(L);
	}

	return 1;
}

static int nixio_dir(lua_State *L) {
	const char *path = luaL_optstring(L, 1, ".");
	DIR **dirp = lua_newuserdata(L, sizeof(DIR *));

	*dirp = opendir(path);
	if (!*dirp) {
		return nixio__perror(L);
	} else {
		luaL_getmetatable(L, NIXIO_DIR_META);
		lua_setmetatable(L, -2);
		lua_pushcclosure(L, nixio_dir__iter, 1);
		return 1;
	}
}

static int nixio_link(lua_State *L) {
	return nixio__pstatus(L,
			!link(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
}

static int nixio_utimes(lua_State *L) {
	const char *path = luaL_checkstring(L, 1);
	if (lua_gettop(L) < 2 || (lua_isnoneornil(L, 2) && lua_isnoneornil(L, 3))) {
		return nixio__pstatus(L, !utimes(path, NULL));
	} else {
		double atime = nixio__checknumber(L, 2);
		double mtime = nixio__optnumber(L, 3, atime);
		struct timeval times[2];

		times[0].tv_sec = atime;
		times[0].tv_usec = 0;
		times[1].tv_sec = mtime;
		times[1].tv_usec = 0;

		return nixio__pstatus(L, !utimes(path, times));
	}
}

int nixio__push_stat(lua_State *L, nixio_stat_t *buf) {
	lua_createtable(L, 0, 15);

	lua_pushinteger(L, buf->st_dev);
	lua_setfield(L, -2, "dev");

	lua_pushinteger(L, buf->st_ino);
	lua_setfield(L, -2, "ino");

	if (S_ISREG(buf->st_mode)) {
		lua_pushliteral(L, "reg");
	} else if (S_ISDIR(buf->st_mode)) {
		lua_pushliteral(L, "dir");
	} else if (S_ISCHR(buf->st_mode)) {
		lua_pushliteral(L, "chr");
	} else if (S_ISBLK(buf->st_mode)) {
		lua_pushliteral(L, "blk");
	} else if (S_ISFIFO(buf->st_mode)) {
		lua_pushliteral(L, "fifo");
	} else if (S_ISLNK(buf->st_mode)) {
		lua_pushliteral(L, "lnk");
	} else if (S_ISSOCK(buf->st_mode)) {
		lua_pushliteral(L, "sock");
	} else {
		lua_pushliteral(L, "unknown");
	}
	lua_setfield(L, -2, "type");

	char modestr[9];
	lua_pushinteger(L, nixio__mode_write(buf->st_mode, modestr));
	lua_setfield(L, -2, "modedec");

	lua_pushlstring(L, modestr, 9);
	lua_setfield(L, -2, "modestr");

	lua_pushinteger(L, buf->st_nlink);
	lua_setfield(L, -2, "nlink");

	lua_pushinteger(L, buf->st_uid);
	lua_setfield(L, -2, "uid");

	lua_pushinteger(L, buf->st_gid);
	lua_setfield(L, -2, "gid");

	lua_pushinteger(L, buf->st_rdev);
	lua_setfield(L, -2, "rdev");

	nixio__pushnumber(L, buf->st_size);
	lua_setfield(L, -2, "size");

	lua_pushinteger(L, buf->st_atime);
	lua_setfield(L, -2, "atime");

	lua_pushinteger(L, buf->st_mtime);
	lua_setfield(L, -2, "mtime");

	lua_pushinteger(L, buf->st_ctime);
	lua_setfield(L, -2, "ctime");

#ifndef __WINNT__
	lua_pushinteger(L, buf->st_blksize);
	lua_setfield(L, -2, "blksize");

	lua_pushinteger(L, buf->st_blocks);
	lua_setfield(L, -2, "blocks");
#endif

	return 1;
}

static int nixio_stat(lua_State *L) {
	nixio_stat_t buf;
	if (stat(luaL_checkstring(L, 1), &buf)) {
		return nixio__perror(L);
	} else {
		nixio__push_stat(L, &buf);
		if (lua_isstring(L, 2)) {
			lua_getfield(L, -1, lua_tostring(L, 2));
		}
		return 1;
	}
}

static int nixio_lstat(lua_State *L) {
	nixio_stat_t buf;
	if (lstat(luaL_checkstring(L, 1), &buf)) {
		return nixio__perror(L);
	} else {
		nixio__push_stat(L, &buf);
		if (lua_isstring(L, 2)) {
			lua_getfield(L, -1, lua_tostring(L, 2));
		}
		return 1;
	}
}

#ifndef __WINNT__

static int nixio_chown(lua_State *L) {
	return nixio__pstatus(L,
			!chown(
					luaL_checkstring(L, 1),
					lua_isnoneornil(L, 2) ? -1 : nixio__check_user(L, 2),
					lua_isnoneornil(L, 3) ? -1 : nixio__check_group(L, 3)
			)
	);
}

static int nixio_lchown(lua_State *L) {
	return nixio__pstatus(L,
			!lchown(
					luaL_checkstring(L, 1),
					lua_isnoneornil(L, 2) ? -1 : nixio__check_user(L, 2),
					lua_isnoneornil(L, 3) ? -1 : nixio__check_group(L, 3)
			)
	);
}

static int nixio_mkfifo(lua_State *L) {
	return nixio__pstatus(L,
			!mkfifo(luaL_checkstring(L, 1), nixio__check_mode(L, 2, -1)));
}

static int nixio_symlink(lua_State *L) {
	return nixio__pstatus(L,
			!symlink(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
}

static int nixio_readlink(lua_State *L) {
	char dest[PATH_MAX];
	ssize_t res = readlink(luaL_checkstring(L, 1), dest, sizeof(dest));
	if (res < 0) {
		return nixio__perror(L);
	} else {
		lua_pushlstring(L, dest, res);
		return 1;
	}
}

#include <glob.h>

typedef struct {
	glob_t gl;
	size_t pos;
	int	freed;
} nixio_glob_t;

static int nixio_glob__iter(lua_State *L) {
	nixio_glob_t *globres = lua_touserdata(L, lua_upvalueindex(1));
	if (!globres->freed && globres->pos < globres->gl.gl_pathc) {
		lua_pushstring(L, globres->gl.gl_pathv[(globres->pos)++]);
	} else {
		if (!globres->freed) {
			globfree(&globres->gl);
			globres->freed = 1;
		}
		lua_pushnil(L);
	}
	return 1;
}

static int nixio_glob__gc(lua_State *L) {
	nixio_glob_t *globres = lua_touserdata(L, 1);
	if (globres && !globres->freed) {
		globres->freed = 1;
		globfree(&globres->gl);
	}
	return 0;
}

static int nixio_glob(lua_State *L) {
	 const char *pattern = luaL_optstring(L, 1, "*");
	 nixio_glob_t *globres = lua_newuserdata(L, sizeof(nixio_glob_t));
	 if (!globres) {
		 return luaL_error(L, NIXIO_OOM);
	 }
	 globres->pos = 0;
	 globres->freed = 0;

	 int globstat = glob(pattern, 0, NULL, &globres->gl);
	 if (globstat == GLOB_NOMATCH) {
		 lua_pushcfunction(L, nixio__nulliter);
		 lua_pushinteger(L, 0);
	 } else if (globstat) {
		 return nixio__perror(L);
	 } else {
		 luaL_getmetatable(L, NIXIO_GLOB_META);
		 lua_setmetatable(L, -2);
		 lua_pushcclosure(L, nixio_glob__iter, 1);
		 lua_pushinteger(L, globres->gl.gl_pathc);
	 }
	 return 2;
}

#include <sys/statvfs.h>

static int nixio__push_statvfs(lua_State *L, struct statvfs *buf) {
	lua_createtable(L, 0, 12);

	nixio__pushnumber(L, buf->f_bavail);
	lua_setfield(L, -2, "bavail");

	nixio__pushnumber(L, buf->f_bfree);
	lua_setfield(L, -2, "bfree");

	nixio__pushnumber(L, buf->f_blocks);
	lua_setfield(L, -2, "blocks");

	nixio__pushnumber(L, buf->f_bsize);
	lua_setfield(L, -2, "bsize");

	nixio__pushnumber(L, buf->f_frsize);
	lua_setfield(L, -2, "frsize");

	nixio__pushnumber(L, buf->f_favail);
	lua_setfield(L, -2, "favail");

	nixio__pushnumber(L, buf->f_ffree);
	lua_setfield(L, -2, "ffree");

	nixio__pushnumber(L, buf->f_files);
	lua_setfield(L, -2, "files");

	nixio__pushnumber(L, buf->f_flag);
	lua_setfield(L, -2, "flag");

	nixio__pushnumber(L, buf->f_fsid);
	lua_setfield(L, -2, "fsid");

	nixio__pushnumber(L, buf->f_namemax);
	lua_setfield(L, -2, "namemax");

	return 1;
}

static int nixio_statvfs(lua_State *L) {
	struct statvfs buf;
	if (statvfs(luaL_optstring(L, 1, "."), &buf)) {
		return nixio__perror(L);
	} else {
		return nixio__push_statvfs(L, &buf);
	}
}

#endif /* !__WINNT__ */



/* module table */
static const luaL_reg R[] = {
#ifndef __WINNT__
	{"glob",		nixio_glob},
	{"mkfifo",		nixio_mkfifo},
	{"symlink",		nixio_symlink},
	{"readlink",	nixio_readlink},
	{"chown",		nixio_chown},
	{"lchown",		nixio_lchown},
	{"statvfs",		nixio_statvfs},
#endif
	{"chmod",		nixio_chmod},
	{"access",		nixio_access},
	{"basename",	nixio_basename},
	{"dir",			nixio_dir},
	{"dirname",		nixio_dirname},
	{"realpath",	nixio_realpath},
	{"mkdir",		nixio_mkdir},
	{"rmdir",		nixio_rmdir},
	{"link",		nixio_link},
	{"unlink",		nixio_unlink},
	{"utimes",		nixio_utimes},
	{"rename",		nixio_rename},
	{"remove",		nixio_remove},
	{"stat",		nixio_stat},
	{"lstat",		nixio_lstat},
	{NULL,			NULL}
};

void nixio_open_fs(lua_State *L) {
	lua_newtable(L);
	luaL_register(L, NULL, R);
	lua_setfield(L, -2, "fs");

	luaL_newmetatable(L, NIXIO_DIR_META);
	lua_pushcfunction(L, nixio_dir__gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);

#ifndef __WINNT__
	luaL_newmetatable(L, NIXIO_GLOB_META);
	lua_pushcfunction(L, nixio_glob__gc);
	lua_setfield(L, -2, "__gc");
	lua_pop(L, 1);
#endif
}
