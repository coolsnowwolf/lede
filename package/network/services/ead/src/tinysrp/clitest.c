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
#include "t_client.h"

int
main()
{
  int index;
  struct t_client * tc;
  struct t_preconf *tcp;
  struct t_num s;
  struct t_num B;
  char username[MAXUSERLEN];
  char hexbuf[MAXHEXPARAMLEN];
  char buf1[MAXPARAMLEN], buf2[MAXPARAMLEN], buf3[MAXSALTLEN];
  unsigned char cbuf[20];
  struct t_num * A;
  unsigned char * skey;
  char pass[128];

  printf("Enter username: ");
  fgets(username, sizeof(username), stdin);
  username[strlen(username) - 1] = '\0';
  printf("Enter index (from server): ");
  fgets(hexbuf, sizeof(hexbuf), stdin);
  index = atoi(hexbuf);
  tcp = t_getpreparam(index - 1);
  printf("Enter salt (from server): ");
  fgets(hexbuf, sizeof(hexbuf), stdin);
  s.data = buf3;
  s.len = t_fromb64(s.data, hexbuf);

  tc = t_clientopen(username, &tcp->modulus, &tcp->generator, &s);
  if (tc == 0) {
    printf("invalid n, g\n");
    exit(1);
  }

  A = t_clientgenexp(tc);
  printf("A (to server): %s\n", t_tob64(hexbuf, A->data, A->len));

  t_getpass(pass, 128, "Enter password:");
  t_clientpasswd(tc, pass);

  printf("Enter B (from server): ");
  fgets(hexbuf, sizeof(hexbuf), stdin);
  B.data = buf1;
  B.len = t_fromb64(B.data, hexbuf);

  skey = t_clientgetkey(tc, &B);
  printf("Session key: %s\n", t_tohex(hexbuf, skey, 40));
  printf("Response (to server): %s\n",
    t_tohex(hexbuf, t_clientresponse(tc), RESPONSE_LEN));

  printf("Enter server response: ");
  fgets(hexbuf, sizeof(hexbuf), stdin);
  hexbuf[strlen(hexbuf) - 1] = '\0';
  t_fromhex(cbuf, hexbuf);

  if (t_clientverify(tc, cbuf) == 0)
    printf("Server authentication successful.\n");
  else
    printf("Server authentication failed.\n");

  t_clientclose(tc);

  return 0;
}
