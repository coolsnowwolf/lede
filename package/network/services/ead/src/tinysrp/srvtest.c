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

#include <stdio.h>
#include "t_defines.h"
#include "t_pwd.h"
#include "t_server.h"

int
main(argc, argv)
     int argc;
     char * argv[];
{
  struct t_server * ts;
  struct t_pw * tpw;
  struct t_conf * tcnf;
  struct t_num * B;
  char username[MAXUSERLEN];
  char hexbuf[MAXHEXPARAMLEN];
  char buf[MAXPARAMLEN];
  struct t_num A;
  unsigned char * skey;
  unsigned char cbuf[20];
  FILE * fp;
  FILE * fp2;
  char confname[256];

  printf("Enter username: ");
  fgets(username, sizeof(username), stdin);
  username[strlen(username) - 1] = '\0';
  ts = t_serveropen(username);

  if(ts == NULL) {
    fprintf(stderr, "User %s not found\n", username);
    exit(1);
  }

#if 0
  printf("n: %s\n", t_tob64(hexbuf, ts->n.data, ts->n.len));
  printf("g: %s\n", t_tob64(hexbuf, ts->g.data, ts->g.len));
#endif
  printf("index (to client): %d\n", ts->index);
  printf("salt (to client): %s\n", t_tob64(hexbuf, ts->s.data, ts->s.len));

  B = t_servergenexp(ts);
  printf("Enter A (from client): ");
  fgets(hexbuf, sizeof(hexbuf), stdin);
  A.data = buf;
  A.len = t_fromb64(A.data, hexbuf);

  printf("B (to client): %s\n", t_tob64(hexbuf, B->data, B->len));

  skey = t_servergetkey(ts, &A);
  printf("Session key: %s\n", t_tohex(hexbuf, skey, 40));

  /* printf("[Expected response: %s]\n", t_tohex(hexbuf, cbuf, 16)); */

  printf("Enter response (from client): ");
  fgets(hexbuf, sizeof(hexbuf), stdin);
  hexbuf[strlen(hexbuf) - 1] = '\0';
  t_fromhex(cbuf, hexbuf);

  if(t_serververify(ts, cbuf) == 0) {
    printf("Authentication successful.\n");
    printf("Response (to client): %s\n",
      t_tohex(hexbuf, t_serverresponse(ts), RESPONSE_LEN));
  } else
    printf("Authentication failed.\n");

  t_serverclose(ts);

  return 0;
}
