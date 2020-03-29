/*
   As part of the ccal program by Zhuo Meng, this version is
   distributed under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   The whole package can be obtained from
   http://aa.usno.navy.mil/software/novas/novas_c/novasc_info.php

   Adapted from:

   Naval Observatory Vector Astrometry Software (NOVAS)
   C Edition, Version 3.1

   novas.h: Header file for novas.c

   U. S. Naval Observatory
   Astronomical Applications Dept.
   Washington, DC
   http://www.usno.navy.mil/USNO/astronomical-applications
*/

#ifndef _NOVAS_
   #define _NOVAS_

   #ifndef __STDIO__
      #include <stdio.h>
   #endif

   #ifndef __MATH__
      #include <math.h>
   #endif

   #ifndef __STRING__
      #include <string.h>
   #endif

   #ifndef __STDLIB__
      #include <stdlib.h>
   #endif

   #ifndef __CTYPE__
      #include <ctype.h>
   #endif

   #ifndef _CONSTS_
      #include "novascon.h"
   #endif

   #ifndef _SOLSYS_
      #include "solarsystem.h"
   #endif

   #ifndef _NUTATION_
      #include "nutation.h"
   #endif

/*
   Define "origin" constants.
*/

   #define BARYC  0
   #define HELIOC 1

/*
   Function prototypes
*/
   void e_tilt (double jd_tdb, short int accuracy,

                double *mobl, double *tobl, double *ee, double *dpsi,
                double *deps);

   double ee_ct (double jd_high, double jd_low, short int accuracy);

   void aberration (double *pos, double *ve, double lighttime,

                    double *pos2);

   short int precession (double jd_tdb1, double *pos1, double jd_tdb2,

                         double *pos2);

   void nutation (double jd_tdb, short int direction, short int accuracy,
                  double *pos,

                  double *pos2);

   void nutation_angles (double t, short int accuracy,

                         double *dpsi, double *deps);

   void fund_args (double t,

                   double a[5]);

   double mean_obliq (double jd_tdb);

   void radec2vector (double ra, double dec, double dist,

                      double *vector);

   void tdb2tt (double tdb_jd,

                double *tt_jd, double *secdiff);

   double julian_date (short int year, short int month, short int day,
                       double hour);

   void cal_date (double tjd,

                  short int *year, short int *month, short int *day,
                  double *hour);

   double norm_ang (double angle);

#endif
