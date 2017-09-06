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

#include "t_defines.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "t_sha.h"

#ifndef NULL
#define NULL 0
#endif

static unsigned char randpool[SHA_DIGESTSIZE], randout[SHA_DIGESTSIZE];
static unsigned long randcnt = 0;
static unsigned int outpos = 0;
SHA1_CTX randctxt;

/*
 * t_envhash - Generate a 160-bit SHA hash of the environment
 *
 * This routine performs an SHA hash of all the "name=value" pairs
 * in the environment concatenated together and dumps them in the
 * output.  While it is true that anyone on the system can see
 * your environment, someone not on the system will have a very
 * difficult time guessing it, especially since some systems play
 * tricks with variable ordering and sometimes define quirky
 * environment variables like $WINDOWID or $_.
 */
extern char ** environ;

static void
t_envhash(out)
     unsigned char * out;
{
  char ** ptr;
  char ebuf[256];
  SHA1_CTX ctxt;

  SHA1Init(&ctxt);
  for(ptr = environ; *ptr; ++ptr) {
    strncpy(ebuf, *ptr, 255);
    ebuf[255] = '\0';
    SHA1Update(&ctxt, ebuf, strlen(ebuf));
  }
  SHA1Final(out, &ctxt);
}

/*
 * t_fshash - Generate a 160-bit SHA hash from the file system
 *
 * This routine climbs up the directory tree from the current
 * directory, running stat() on each directory until it hits the
 * root directory.  This information is sensitive to the last
 * access/modification times of all the directories above you,
 * so someone who lists one of those directories injects some
 * entropy into the system.  Obviously, this hash is very sensitive
 * to your current directory when the program is run.
 *
 * For good measure, it also performs an fstat on the standard input,
 * usually your tty, throws that into the buffer, creates a file in
 * /tmp (the inode is unpredictable on a busy system), and runs stat()
 * on that before deleting it.
 *
 * The entire buffer is run once through SHA to obtain the final result.
 */
static void
t_fshash(out)
     unsigned char * out;
{
  char dotpath[128];
  struct stat st;
  SHA1_CTX ctxt;
  int i, pinode;
  dev_t pdev;

  SHA1Init(&ctxt);
  if(stat(".", &st) >= 0) {
    SHA1Update(&ctxt, (unsigned char *) &st, sizeof(st));
    pinode = st.st_ino;
    pdev = st.st_dev;
    strcpy(dotpath, "..");
    for(i = 0; i < 40; ++i) {
      if(stat(dotpath, &st) < 0)
	break;
      if(st.st_ino == pinode && st.st_dev == pdev)
	break;
      SHA1Update(&ctxt, (unsigned char *) &st, sizeof(st));
      pinode = st.st_ino;
      pdev = st.st_dev;
      strcat(dotpath, "/..");
    }
  }

  if(fstat(0, &st) >= 0)
    SHA1Update(&ctxt, (unsigned char *) &st, sizeof(st));

  sprintf(dotpath, "/tmp/rnd.%d", getpid());
  if(creat(dotpath, 0600) >= 0 && stat(dotpath, &st) >= 0)
    SHA1Update(&ctxt, (unsigned char *) &st, sizeof(st));
  unlink(dotpath);

  SHA1Final(out, &ctxt);
}

/*
 * Generate a high-entropy seed for the strong random number generator.
 * This uses a wide variety of quickly gathered and somewhat unpredictable
 * system information.  The 'preseed' structure is assembled from:
 *
 *   The system time in seconds
 *   The system time in microseconds
 *   The current process ID
 *   The parent process ID
 *   A hash of the user's environment
 *   A hash gathered from the file system
 *   Input from a random device, if available
 *   Timings of system interrupts
 *
 * The entire structure (60 bytes on most systems) is fed to SHA to produce
 * a 160-bit seed for the strong random number generator.  It is believed
 * that in the worst case (on a quiet system with no random device versus
 * an attacker who has access to the system already), the seed contains at
 * least about 80 bits of entropy.  Versus an attacker who does not have
 * access to the system, the entropy should be slightly over 128 bits.
 */
static char initialized = 0;

static struct {
  unsigned int trand1;
  time_t sec;
  time_t usec;
  short pid;
  short ppid;
  unsigned char envh[SHA_DIGESTSIZE];
  unsigned char fsh[SHA_DIGESTSIZE];
  unsigned char devrand[20];
  unsigned int trand2;
} preseed;

unsigned long raw_truerand();

void
t_initrand()
{
  SHA1_CTX ctxt;
#ifdef USE_FTIME
  struct timeb t;
#else
  struct timeval t;
#endif
  int i, r=0;

  if(initialized)
    return;

  initialized = 1;

  i = open("/dev/urandom", O_RDONLY);
  if(i > 0) {
    r += read(i, preseed.devrand, sizeof(preseed.devrand));
    close(i);
  }

  /* Resort to truerand only if desperate for some Real entropy */
  if(r == 0)
    preseed.trand1 = raw_truerand();

#ifdef USE_FTIME
  ftime(&t);
#else
  gettimeofday(&t, NULL);
#endif

#ifdef USE_FTIME
  preseed.sec = t.time;
  preseed.usec = t.millitm;
#else
  preseed.sec = t.tv_sec;
  preseed.usec = t.tv_usec;
#endif
  preseed.pid = getpid();
  preseed.ppid = getppid();
  t_envhash(preseed.envh);
  t_fshash(preseed.fsh);

  if(r == 0)
    preseed.trand2 = raw_truerand();

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, (unsigned char *) &preseed, sizeof(preseed));
  SHA1Final(randpool, &ctxt);
  outpos = 0;
  memset((unsigned char *) &preseed, 0, sizeof(preseed));
  memset((unsigned char *) &ctxt, 0, sizeof(ctxt));
}

#define NUM_RANDOMS 12

/*
 * The strong random number generator.  This uses a 160-bit seed
 * and uses SHA-1 in a feedback configuration to generate successive
 * outputs.  If S[0] is set to the initial seed, then:
 *
 *         S[i+1] = SHA-1(i || S[i])
 *         A[i] = SHA-1(S[i])
 *
 * where the A[i] are the output blocks starting with i=0.
 * Each cycle generates 20 bytes of new output.
 */
_TYPE( void )
t_random(data, size)
     unsigned char * data;
     unsigned size;
{
  if(!initialized)
    t_initrand();

  if(size <= 0)         /* t_random(NULL, 0) forces seed initialization */
    return;

  while(size > outpos) {
    if(outpos > 0) {
      memcpy(data, randout + (sizeof(randout) - outpos), outpos);
      data += outpos;
      size -= outpos;
    }

    /* Recycle */
    SHA1Init(&randctxt);
    SHA1Update(&randctxt, randpool, sizeof(randpool));
    SHA1Final(randout, &randctxt);
    SHA1Init(&randctxt);
    SHA1Update(&randctxt, (unsigned char *) &randcnt, sizeof(randcnt));
    SHA1Update(&randctxt, randpool, sizeof(randpool));
    SHA1Final(randpool, &randctxt);
    ++randcnt;
    outpos = sizeof(randout);
  }

  if(size > 0) {
    memcpy(data, randout + (sizeof(randout) - outpos), size);
    outpos -= size;
  }
}

/*
 * The interleaved session-key hash.  This separates the even and the odd
 * bytes of the input (ignoring the first byte if the input length is odd),
 * hashes them separately, and re-interleaves the two outputs to form a
 * single 320-bit value.
 */
_TYPE( unsigned char * )
t_sessionkey(key, sk, sklen)
     unsigned char * key;
     unsigned char * sk;
     unsigned sklen;
{
  unsigned i, klen;
  unsigned char * hbuf;
  unsigned char hout[SHA_DIGESTSIZE];
  SHA1_CTX ctxt;

  while(sklen > 0 && *sk == 0) {        /* Skip leading 0's */
    --sklen;
    ++sk;
  }

  klen = sklen / 2;
  if((hbuf = malloc(klen * sizeof(char))) == 0)
    return 0;

  for(i = 0; i < klen; ++i)
    hbuf[i] = sk[sklen - 2 * i - 1];
  SHA1Init(&ctxt);
  SHA1Update(&ctxt, hbuf, klen);
  SHA1Final(hout, &ctxt);
  for(i = 0; i < sizeof(hout); ++i)
    key[2 * i] = hout[i];

  for(i = 0; i < klen; ++i)
    hbuf[i] = sk[sklen - 2 * i - 2];
  SHA1Init(&ctxt);
  SHA1Update(&ctxt, hbuf, klen);
  SHA1Final(hout, &ctxt);
  for(i = 0; i < sizeof(hout); ++i)
    key[2 * i + 1] = hout[i];

  memset(hout, 0, sizeof(hout));
  memset(hbuf, 0, klen);
  free(hbuf);
  return key;
}
