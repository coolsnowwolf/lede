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
#include "t_read.h"

/* Master builtin parameter storage object.  The default that tphrase
uses is the last one. */

static struct pre_struct {
  struct t_preconf preconf;
  int state;    /* 0 == uninitialized/first time */
  unsigned char modbuf[MAXPARAMLEN];
  unsigned char genbuf[MAXPARAMLEN];
} pre_params[] = {
  { { "2iQzj1CagQc/5ctbuJYLWlhtAsPHc7xWVyCPAKFRLWKADpASkqe9djWPFWTNTdeJtL8nAhImCn3Sr/IAdQ1FrGw0WvQUstPx3FO9KNcXOwisOQ1VlL.gheAHYfbYyBaxXL.NcJx9TUwgWDT0hRzFzqSrdGGTN3FgSTA1v4QnHtEygNj3eZ.u0MThqWUaDiP87nqha7XnT66bkTCkQ8.7T8L4KZjIImrNrUftedTTBi.WCi.zlrBxDuOM0da0JbUkQlXqvp0yvJAPpC11nxmmZOAbQOywZGmu9nhZNuwTlxjfIro0FOdthaDTuZRL9VL7MRPUDo/DQEyW.d4H.UIlzp",
      "2",
      NULL }, 0 },
  { { "dUyyhxav9tgnyIg65wHxkzkb7VIPh4o0lkwfOKiPp4rVJrzLRYVBtb76gKlaO7ef5LYGEw3G.4E0jbMxcYBetDy2YdpiP/3GWJInoBbvYHIRO9uBuxgsFKTKWu7RnR7yTau/IrFTdQ4LY/q.AvoCzMxV0PKvD9Odso/LFIItn8PbTov3VMn/ZEH2SqhtpBUkWtmcIkEflhX/YY/fkBKfBbe27/zUaKUUZEUYZ2H2nlCL60.JIPeZJSzsu/xHDVcx",
      "2",
      NULL }, 0 },
  { { "3NUKQ2Re4P5BEK0TLg2dX3gETNNNECPoe92h4OVMaDn3Xo/0QdjgG/EvM.hiVV1BdIGklSI14HA38Mpe5k04juR5/EXMU0r1WtsLhNXwKBlf2zEfoOh0zVmDvqInpU695f29Iy7sNW3U5RIogcs740oUp2Kdv5wuITwnIx84cnO.e467/IV1lPnvMCr0pd1dgS0a.RV5eBJr03Q65Xy61R",
      "2",
      NULL }, 0 },
  { { "F//////////oG/QeY5emZJ4ncABWDmSqIa2JWYAPynq0Wk.fZiJco9HIWXvZZG4tU.L6RFDEaCRC2iARV9V53TFuJLjRL72HUI5jNPYNdx6z4n2wQOtxMiB/rosz0QtxUuuQ/jQYP.bhfya4NnB7.P9A6PHxEPJWV//////////",
      "5",
      "oakley prime 2" }, 0 },
  { { "Ewl2hcjiutMd3Fu2lgFnUXWSc67TVyy2vwYCKoS9MLsrdJVT9RgWTCuEqWJrfB6uE3LsE9GkOlaZabS7M29sj5TnzUqOLJMjiwEzArfiLr9WbMRANlF68N5AVLcPWvNx6Zjl3m5Scp0BzJBz9TkgfhzKJZ.WtP3Mv/67I/0wmRZ",
      "2",
      NULL }, 0 },
};

_TYPE( int )
t_getprecount()
{
  return (sizeof(pre_params) / sizeof(struct pre_struct));
}

static struct t_confent sysconf;

/* id is index origin 1 */

_TYPE( struct t_confent * )
gettcid
(id)
     int id;
{
	struct t_preconf *tcp;

	if (id <= 0 || id > t_getprecount()) {
		return NULL;
	}
	tcp = t_getpreparam(id - 1);
	sysconf.index = id;
	sysconf.modulus = tcp->modulus;
	sysconf.generator = tcp->generator;

	return &sysconf;
}

_TYPE( struct t_preconf * )
t_getpreparam(idx)
     int idx;
{
  if(pre_params[idx].state == 0) {
    /* Wire up storage */
    pre_params[idx].preconf.modulus.data = pre_params[idx].modbuf;
    pre_params[idx].preconf.generator.data = pre_params[idx].genbuf;

    /* Convert from b64 to t_num */
    pre_params[idx].preconf.modulus.len = t_fromb64(pre_params[idx].preconf.modulus.data, pre_params[idx].preconf.mod_b64);
    pre_params[idx].preconf.generator.len = t_fromb64(pre_params[idx].preconf.generator.data, pre_params[idx].preconf.gen_b64);

    pre_params[idx].state = 1;
  }
  return &(pre_params[idx].preconf);
}
