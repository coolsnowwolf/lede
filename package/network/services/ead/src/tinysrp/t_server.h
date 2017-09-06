/*
 * Copyright (c) 1997-1999  The Stanford SRP Authentication Project
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

#ifndef T_SERVER_H
#define T_SERVER_H

#include "t_sha.h"

#if     !defined(P)
#ifdef  __STDC__
#define P(x)    x
#else
#define P(x)    ()
#endif
#endif

#ifndef _DLLDECL
#define _DLLDECL

#ifdef MSVC15   /* MSVC1.5 support for 16 bit apps */
#define _MSVC15EXPORT _export
#define _MSVC20EXPORT
#define _DLLAPI _export _pascal
#define _TYPE(a) a _MSVC15EXPORT
#define DLLEXPORT 1

#elif MSVC20
#define _MSVC15EXPORT
#define _MSVC20EXPORT _declspec(dllexport)
#define _DLLAPI
#define _TYPE(a) _MSVC20EXPORT a
#define DLLEXPORT 1

#else                   /* Default, non-dll.  Use this for Unix or DOS */
#define _MSVC15DEXPORT
#define _MSVC20EXPORT
#define _DLLAPI
#define _TYPE(a) a
#endif
#endif

#define BLEN 32

struct t_server {
  int index;
  struct t_num n;
  struct t_num g;
  struct t_num v;
  struct t_num s;

  struct t_num b;
  struct t_num B;

  SHA1_CTX oldhash, hash, oldckhash, ckhash;

  unsigned char session_key[SESSION_KEY_LEN];
  unsigned char session_response[RESPONSE_LEN];

  unsigned char nbuf[MAXPARAMLEN], gbuf[MAXPARAMLEN], vbuf[MAXPARAMLEN];
  unsigned char saltbuf[MAXSALTLEN], bbuf[BLEN], Bbuf[MAXPARAMLEN];
};

/*
 * SRP server-side negotiation
 *
 * This code negotiates the server side of an SRP exchange.
 * "t_serveropen" accepts a username (sent by the client), a pointer
 *   to an open password file, and a pointer to an open configuration
 *   file.  The server should then call...
 * "t_servergenexp" will generate a random 256-bit exponent and
 *   raise g (from the configuration file) to that power, returning
 *   the result.  This result should be sent to the client as y(p).
 * "t_servergetkey" accepts the exponential w(p), which should be
 *   sent by the client, and computes the 256-bit session key.
 *   This data should be saved before the session is closed.
 * "t_serverresponse" computes the session key proof as SHA(w(p), K).
 * "t_serverclose" closes the session and frees its memory.
 *
 * Note that authentication is not performed per se; it is up
 * to either/both sides of the protocol to now verify securely
 * that their session keys agree in order to establish authenticity.
 * One possible way is through "oracle hashing"; one side sends
 * r, the other replies with H(r,K), where H() is a hash function.
 *
 * t_serverresponse and t_serververify now implement a version of
 * the session-key verification described above.
 */
_TYPE( struct t_server * )
  t_serveropen P((const char *));
_TYPE( struct t_server * )
  t_serveropenfromfiles P((const char *, struct t_pw *, struct t_conf *));
_TYPE( struct t_server * )
  t_serveropenraw P((struct t_pwent *, struct t_confent *));
_TYPE( struct t_num * ) t_servergenexp P((struct t_server *));
_TYPE( unsigned char * ) t_servergetkey P((struct t_server *, struct t_num *));
_TYPE( int ) t_serververify P((struct t_server *, unsigned char *));
_TYPE( unsigned char * ) t_serverresponse P((struct t_server *));
_TYPE( void ) t_serverclose P((struct t_server *));

#endif
