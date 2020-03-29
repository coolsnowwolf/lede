/*
   Copyright 1996, Christopher Osburn, Lunar Outreach Services,

   As part of the ccal program by Zhuo Meng, this version is
   distributed under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   moonphase.cpp
   1996/02/11

   calculate phase of the moon per Meeus Ch. 47
   updated with parameters from Ch. 49 of 2005 printing of 2nd Edition

   Parameters:
      int lun:  phase parameter.  This is the number of lunations
                since the New Moon of 2000 January 6.

      int phi:  another phase parameter, selecting the phase of the
                moon.  0 = New, 1 = First Qtr, 2 = Full, 3 = Last Qtr

   Return:  Apparent JD of the needed phase
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "moon.h"

extern double moonphasebylunation(int lun, int phi)
{
  double k;

  k = lun + phi / 4.0;
  return moonphase(k, phi);
}

extern double moonphase(double k, int phi)
{
  int i;                       /* iterator to be named later.  Every
                                  program needs an i */
  double T;                    /* time parameter, Julian Centuries since
                                  J2000 */
  double JDE;                  /* Julian Ephemeris Day of phase event */
  double E;                    /* Eccentricity anomaly */
  double M;                    /* Sun's mean anomaly */
  double M1;                   /* Moon's mean anomaly */
  double F;                    /* Moon's argument of latitude */
  double O;                    /* Moon's longitude of ascenfing node */
  double A[15];                /* planetary arguments */
  double W;                    /* added correction for quarter phases */

  T = k / 1236.85;                            /* (47.3) */

  /* this is the first approximation.  all else is for style points! */
  JDE = 2451550.09766 + (29.530588861 * k)    /* (47.1) */
        + T * T * (0.00015437 + T * (-0.000000150 + 0.00000000073 * T));

  /* these are correction parameters used below */
  E = 1.0                                     /* (45.6) */
      + T * (-0.002516 + -0.0000074 * T);
  M = 2.5534 + 29.10535670 * k                /* (47.4) */
      + T * T * (-0.0000014 + -0.00000011 * T);
  M1 = 201.5643 + 385.81693528 * k            /* (47.5) */
       + T * T * (0.0107582 + T * (0.00001238 + -0.000000058 * T));
  F = 160.7108 + 390.67050284 * k             /* (47.6) */
      + T * T * (-0.0016118 * T * (-0.00000227 + 0.000000011 * T));
  O = 124.7746 - 1.56375588 * k               /* (47.7) */
      + T * T * (0.0020672 + 0.00000215 * T);

  /* planetary arguments */
  A[0]  = 0; /* unused! */
  A[1]  = 299.77 +  0.107408 * k - 0.009173 * T * T;
  A[2]  = 251.88 +  0.016321 * k;
  A[3]  = 251.83 + 26.651886 * k;
  A[4]  = 349.42 + 36.412478 * k;
  A[5]  =  84.66 + 18.206239 * k;
  A[6]  = 141.74 + 53.303771 * k;
  A[7]  = 207.14 +  2.453732 * k;
  A[8]  = 154.84 +  7.306860 * k;
  A[9]  =  34.52 + 27.261239 * k;
  A[10] = 207.19 +  0.121824 * k;
  A[11] = 291.34 +  1.844379 * k;
  A[12] = 161.72 + 24.198154 * k;
  A[13] = 239.56 + 25.513099 * k;
  A[14] = 331.55 +  3.592518 * k;

  /* all of the above crap must be made into radians!!! */
  /* except for E... */

  M = torad(M);
  M1 = torad(M1);
  F = torad(F);
  O = torad(O);

  /* all those planetary arguments, too! */
  for (i=1; i<=14; i++)
    A[i] = torad(A[i]);

  /* ok, we have all the parameters, let's apply them to the JDE.
    (remember the JDE?  this is a program about the JDE...)        */

  switch(phi)
  {
    /* a special case for each different phase.  NOTE!,
       I'm not treating these in a 0123 order!!!  Pay
       attention, there,  you!                         */

    case 0: /* New Moon */
      JDE = JDE
          - 0.40720         * sin (M1)
          + 0.17241 * E     * sin (M)
          + 0.01608         * sin (2.0 * M1)
          + 0.01039         * sin (2.0 * F)
          + 0.00739 * E     * sin (M1 - M)
          - 0.00514 * E     * sin (M1 + M)
          + 0.00208 * E * E * sin (2.0 * M)
          - 0.00111         * sin (M1 - 2.0 * F)
          - 0.00057         * sin (M1 + 2.0 * F)
          + 0.00056 * E     * sin (2.0 * M1 + M)
          - 0.00042         * sin (3.0 * M1)
          + 0.00042 * E     * sin (M + 2.0 * F)
          + 0.00038 * E     * sin (M - 2.0 * F)
          - 0.00024 * E     * sin (2.0 * M1 - M)
          - 0.00017         * sin (O)
          - 0.00007         * sin (M1 + 2.0 * M)
          + 0.00004         * sin (2.0 * M1 - 2.0 * F)
          + 0.00004         * sin (3.0 * M)
          + 0.00003         * sin (M1 + M - 2.0 * F)
          + 0.00003         * sin (2.0 * M1 + 2.0 * F)
          - 0.00003         * sin (M1 + M + 2.0 * F)
          + 0.00003         * sin (M1 - M + 2.0 * F)
          - 0.00002         * sin (M1 - M - 2.0 * F)
          - 0.00002         * sin (3.0 * M1 + M)
          + 0.00002         * sin (4.0 * M1);

          break;

    case 2: /* Full Moon */
      JDE = JDE
          - 0.40614         * sin (M1)
          + 0.17302 * E     * sin (M)
          + 0.01614         * sin (2.0 * M1)
          + 0.01043         * sin (2.0 * F)
          + 0.00734 * E     * sin (M1 - M)
          - 0.00515 * E     * sin (M1 + M)
          + 0.00209 * E * E * sin (2.0 * M)
          - 0.00111         * sin (M1 - 2.0 * F)
          - 0.00057         * sin (M1 + 2.0 * F)
          + 0.00056 * E     * sin (2.0 * M1 + M)
          - 0.00042         * sin (3.0 * M1)
          + 0.00042 * E     * sin (M + 2.0 * F)
          + 0.00038 * E     * sin (M - 2.0 * F)
          - 0.00024 * E     * sin (2.0 * M1 - M)
          - 0.00017         * sin (O)
          - 0.00007         * sin (M1 + 2.0 * M)
          + 0.00004         * sin (2.0 * M1 - 2.0 * F)
          + 0.00004         * sin (3.0 * M)
          + 0.00003         * sin (M1 + M - 2.0 * F)
          + 0.00003         * sin (2.0 * M1 + 2.0 * F)
          - 0.00003         * sin (M1 + M + 2.0 * F)
          + 0.00003         * sin (M1 - M + 2.0 * F)
          - 0.00002         * sin (M1 - M - 2.0 * F)
          - 0.00002         * sin (3.0 * M1 + M)
          + 0.00002         * sin (4.0 * M1);

          break;

    case 1: /* First Quarter */
    case 3: /* Last Quarter */
      JDE = JDE
          - 0.62801         * sin (M1)
          + 0.17172 * E     * sin (M)
          - 0.01183 * E     * sin (M1 + M)
          + 0.00862         * sin (2.0 * M1)
          + 0.00804         * sin (2.0 * F)
          + 0.00454 * E     * sin (M1 - M)
          + 0.00204 * E * E * sin (2.0 * M)
          - 0.00180         * sin (M1 - 2.0 * F)
          - 0.00070         * sin (M1 + 2.0 * F)
          - 0.00040         * sin (3.0 * M1)
          - 0.00034 * E     * sin (2.0 * M1 - M)
          + 0.00032 * E     * sin (M + 2.0 * F)
          + 0.00032 * E     * sin (M - 2.0 * F)
          - 0.00028 * E * E * sin (M1 + 2.0 * M)
          + 0.00027 * E     * sin (2.0 * M1 + M)
          - 0.00017         * sin (O)
          - 0.00005         * sin (M1 - M - 2.0 * F)
          + 0.00004         * sin (2.0 * M1 + 2.0 * F)
          - 0.00004         * sin (M1 + M + 2.0 * F)
          + 0.00004         * sin (M1 - 2.0 * M)
          + 0.00003         * sin (M1 + M - 2.0 * F)
          + 0.00003         * sin (3.0 * M)
          + 0.00002         * sin (2.0 * M1 - 2.0 * F)
          + 0.00002         * sin (M1 - M + 2.0 * F)
          - 0.00002         * sin (3.0 * M1 + M);

      W = 0.00306
        - 0.00038 * E * cos(M)
        + 0.00026 * cos(M1)
        - 0.00002 * cos(M1 - M)
        + 0.00002 * cos(M1 + M)
        + 0.00002 * cos(2.0 * F);
      if (phi == 3)
        W = -W;
      JDE += W;

      break;

    default: /* oops! */
      fprintf(stderr, "The Moon has exploded!\n");
      exit(1);
      break; /* unexecuted code */
  }
      /* now there are some final correction to everything */
  JDE = JDE
      + 0.000325 * sin(A[1])
      + 0.000165 * sin(A[2])
      + 0.000164 * sin(A[3])
      + 0.000126 * sin(A[4])
      + 0.000110 * sin(A[5])
      + 0.000062 * sin(A[6])
      + 0.000060 * sin(A[7])
      + 0.000056 * sin(A[8])
      + 0.000047 * sin(A[9])
      + 0.000042 * sin(A[10])
      + 0.000040 * sin(A[11])
      + 0.000037 * sin(A[12])
      + 0.000035 * sin(A[13])
      + 0.000023 * sin(A[14]);

  return JDE;
}
