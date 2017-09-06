/*
 * Copyright (c) 1997-2000  The Stanford SRP Authentication Project
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * In addition, the following conditions apply:
 *
 * 1. Any software that incorporates the SRP authentication technology
 *    must display the following acknowlegment:
 *    "This product uses the 'Secure Remote Password' cryptographic
 *     authentication system developed by Tom Wu (tjw@CS.Stanford.EDU)."
 *
 * 2. Any software that incorporates all or part of the SRP distribution
 *    itself must also display the following acknowledgment:
 *    "This product includes software developed by Tom Wu and Eugene
 *     Jhong for the SRP Distribution (http://srp.stanford.edu/srp/)."
 *
 * 3. Redistributions in source or binary form must retain an intact copy
 *    of this copyright notice and list of conditions.
 */

#include "t_defines.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef USE_HOMEDIR
#include <pwd.h>
#endif
#ifdef WIN32
#include <io.h>
#endif

#include "t_pwd.h"
#include "t_read.h"
#include "t_sha.h"
#include "t_server.h"

static struct t_pw * syspw = NULL;
static struct t_passwd tpass;

_TYPE( struct t_server * )
t_serveropen(username)
     const char * username;
{
  struct t_passwd * p;
  p = gettpnam(username);
  if(p == NULL) {
    return NULL;
  } else {
    return t_serveropenraw(&p->tp, &p->tc);
  }
}


/* t_openpw(NULL) is deprecated - use settpent()/gettpnam() instead */

_TYPE( struct t_pw * )
t_openpw(fp)
     FILE * fp;
{
  struct t_pw * tpw;
  char close_flag = 0;

  if(fp == NULL) { /* Deprecated */
    if((fp = fopen(DEFAULT_PASSWD, "r")) == NULL)
      return NULL;
    close_flag = 1;
  }
  else
    close_flag = 0;

  if((tpw = malloc(sizeof(struct t_pw))) == NULL)
    return NULL;
  tpw->instream = fp;
  tpw->close_on_exit = close_flag;
  tpw->state = FILE_ONLY;

  return tpw;
}

_TYPE( struct t_pw * )
t_openpwbyname(pwname)
     const char * pwname;
{
  FILE * fp;
  struct t_pw * t;

  if(pwname == NULL)            /* Deprecated */
    return t_openpw(NULL);

  if((fp = fopen(pwname, "r")) == NULL)
    return NULL;

  t = t_openpw(fp);
  t->close_on_exit = 1;
  return t;
}

_TYPE( void )
t_closepw(tpw)
     struct t_pw * tpw;
{
  if(tpw->close_on_exit)
    fclose(tpw->instream);
  free(tpw);
}

_TYPE( void )
t_rewindpw(tpw)
     struct t_pw * tpw;
{
#ifdef ENABLE_YP
  if(tpw->state == IN_NIS)
    tpw->state = FILE_NIS;
#endif
  rewind(tpw->instream);
}

#ifdef ENABLE_YP
static void
savepwent(tpw, pwent)
     struct t_pw * tpw;
     struct t_pwent *pwent;
{
  tpw->pebuf.name = tpw->userbuf;
  tpw->pebuf.password.data = tpw->pwbuf;
  tpw->pebuf.salt.data = tpw->saltbuf;
  strcpy(tpw->pebuf.name, pwent->name);
  tpw->pebuf.password.len = pwent->password.len;
  memcpy(tpw->pebuf.password.data, pwent->password.data, pwent->password.len);
  tpw->pebuf.salt.len = pwent->salt.len;
  memcpy(tpw->pebuf.salt.data, pwent->salt.data, pwent->salt.len);
  tpw->pebuf.index = pwent->index;
}
#endif /* ENABLE_YP */

_TYPE( struct t_pwent * )
t_getpwbyname(tpw, user)
     struct t_pw * tpw;
     const char * user;
{
  char indexbuf[16];
  char passbuf[MAXB64PARAMLEN];
  char saltstr[MAXB64SALTLEN];
  char username[MAXUSERLEN];
#ifdef ENABLE_YP
  struct t_passwd * nisent;
#endif

  t_rewindpw(tpw);

  while(t_nextfield(tpw->instream, username, MAXUSERLEN) > 0) {
#ifdef ENABLE_YP
    if(tpw->state == FILE_NIS && *username == '+') {
      if(strlen(username) == 1 || strcmp(user, username+1) == 0) {
	nisent = _yp_gettpnam(user);    /* Entry is +username or + */
	if(nisent != NULL) {
	  savepwent(tpw, &nisent->tp);
	  return &tpw->pebuf;
	}
      }
    }
#endif
    if(strcmp(user, username) == 0)
      if(t_nextfield(tpw->instream, passbuf, MAXB64PARAMLEN) > 0 &&
	 (tpw->pebuf.password.len = t_fromb64(tpw->pwbuf, passbuf)) > 0 &&
	 t_nextfield(tpw->instream, saltstr, MAXB64SALTLEN) > 0 &&
	 (tpw->pebuf.salt.len = t_fromb64(tpw->saltbuf, saltstr)) > 0 &&
	 t_nextfield(tpw->instream, indexbuf, 16) > 0 &&
	 (tpw->pebuf.index = atoi(indexbuf)) > 0) {
	strcpy(tpw->userbuf, username);
	tpw->pebuf.name = tpw->userbuf;
	tpw->pebuf.password.data = tpw->pwbuf;
	tpw->pebuf.salt.data = tpw->saltbuf;
	t_nextline(tpw->instream);
	return &tpw->pebuf;
      }
    if(t_nextline(tpw->instream) < 0)
      return NULL;
  }
  return NULL;
}

/* System password file accessors */

static int
pwinit()
{
  if(syspw == NULL) {
    if((syspw = t_openpwbyname(DEFAULT_PASSWD)) == NULL)
      return -1;
    syspw->state = FILE_NIS;
  }
  return 0;
}

static void
pwsetup(out, tpwd, tcnf)
     struct t_passwd * out;
     struct t_pwent * tpwd;
     struct t_confent * tcnf;
{
  out->tp.name = tpwd->name;
  out->tp.password.len = tpwd->password.len;
  out->tp.password.data = tpwd->password.data;
  out->tp.salt.len = tpwd->salt.len;
  out->tp.salt.data = tpwd->salt.data;
  out->tp.index = tpwd->index;

  out->tc.index = tcnf->index;
  out->tc.modulus.len = tcnf->modulus.len;
  out->tc.modulus.data = tcnf->modulus.data;
  out->tc.generator.len = tcnf->generator.len;
  out->tc.generator.data = tcnf->generator.data;
}

_TYPE( struct t_passwd * )
gettpnam
(user)
     const char * user;
{
  struct t_pwent * tpptr;
  struct t_confent * tcptr;

  if(pwinit() < 0)
    return NULL;
  tpptr = t_getpwbyname(syspw, user);
  if(tpptr == NULL)
    return NULL;
  tcptr =
    gettcid
    (tpptr->index);
  if(tcptr == NULL)
    return NULL;
  pwsetup(&tpass, tpptr, tcptr);
  return &tpass;
}
