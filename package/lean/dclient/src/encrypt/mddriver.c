/* MDDRIVER.C - test driver for MD2, MD4 and MD5 */

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* The following makes MD default to MD5 if it has not already been
  defined with C compiler flags. */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "md5.h"

/* Length of test block, number of test blocks. */
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000

static void MD5String(char *);
static void MD5TimeTrial(void);
static void MD5TestSuite(void);
static void MD5File(char *);
static void MD5Filter(void);
static void MD5Print(unsigned char [16]);

/* Main driver.

Arguments (may be any combination):
  -sstring - digests string
  -t       - runs time trial
  -x       - runs test script
  filename - digests file
  (none)   - digests standard input
 */
int main (int argc, char *argv[])
{
  int i;

  if (argc > 1)
 for (i = 1; i < argc; i++)
   if (argv[i][0] == '-' && argv[i][1] == 's')
     MD5String(argv[i] + 2);
   else if (strcmp (argv[i], "-t") == 0)
     MD5TimeTrial ();
   else if (strcmp (argv[i], "-x") == 0)
     MD5TestSuite ();
   else
     MD5File (argv[i]);
  else
 MD5Filter ();

  return (0);
}

/* Digests a string and prints the result.
 */
static void MD5String(char *string)
{
  MD5_CTX context;
  unsigned char digest[16];
  int len = strlen ((const char*)string);

  MD5Init (&context);
  MD5Update (&context, (unsigned char *)string, len);
  MD5Final (digest, &context);

  printf ("MD5 (\"%s\") = ", string);
  MD5Print (digest);
  printf ("\n");
}

/* Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte
  blocks.
 */
static void MD5TimeTrial ()
{
  MD5_CTX context;
  time_t endTime, startTime;
  unsigned char block[TEST_BLOCK_LEN], digest[16];
  unsigned int i;
  printf("MD5 time trial. Digesting %d %d-byte blocks ...",
            TEST_BLOCK_LEN, TEST_BLOCK_COUNT);

  /* Initialize block */
  for (i = 0; i < TEST_BLOCK_LEN; i++)
 block[i] = (unsigned char)(i & 0xff);

  /* Start timer */
  time (&startTime);

  /* Digest blocks */
  MD5Init (&context);
  for (i = 0; i < TEST_BLOCK_COUNT; i++)
 MD5Update (&context, block, TEST_BLOCK_LEN);
  MD5Final (digest, &context);

  /* Stop timer */
  time (&endTime);

  printf(" done\n");
  printf("Digest = ");
  MD5Print(digest);
  printf("\nTime = %ld seconds\n", (long)(endTime-startTime));
  printf("Speed = %ld bytes/second\n",
        (long)TEST_BLOCK_LEN * (long)TEST_BLOCK_COUNT/(endTime-startTime));
}

/* Digests a reference suite of strings and prints the results.
 */
static void MD5TestSuite ()
{
  printf ("MD5 test suite:\n");

  MD5String("");
  MD5String("a");
  MD5String("abc");
  MD5String("message digest");
  MD5String("abcdefghijklmnopqrstuvwxyz");
  MD5String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  MD5String("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
}

/* Digests a file and prints the result. */
static void MD5File (char * filename)
{
  FILE *file;
  MD5_CTX context;
  int len;
  unsigned char buffer[1024], digest[16];

  if ((file = fopen (filename, "rb")) == NULL)
 printf ("%s can't be opened\n", filename);

  else {
 MD5Init (&context);
 while (len = fread (buffer, 1, 1024, file))
   MD5Update (&context, buffer, len);
 MD5Final (digest, &context);

 fclose (file);

 printf ("MD5 (%s) = ", filename);
 MD5Print (digest);
 printf ("\n");
  }
}

/* Digests the standard input and prints the result.
 */
static void MD5Filter ()
{
  MD5_CTX context;
  int len;
  unsigned char buffer[16], digest[16];

  MD5Init (&context);
  while (len = fread (buffer, 1, 16, stdin))
 MD5Update (&context, buffer, len);
  MD5Final (digest, &context);

  MD5Print (digest);
  printf ("\n");
}

/* Prints a message digest in hexadecimal.
 */
static void MD5Print (unsigned char *digest)
{
    unsigned int i;

    for (i = 0; i < 16; i++)
        printf ("%02x", digest[i]);
}

/*

   The MD5 test suite (driver option "-x") should print the following
   results:

MD5 test suite:
MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") =
d174ab98d277d9f5a5611c2c9f419d9f
MD5 ("123456789012345678901234567890123456789012345678901234567890123456
78901234567890") = 57edf4a22be3c955ac49da2e2107b67a
*/


