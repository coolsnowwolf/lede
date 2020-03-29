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

   novas.c: Main library

   U. S. Naval Observatory
   Astronomical Applications Dept.
   Washington, DC
   http://www.usno.navy.mil/USNO/astronomical-applications
*/

#ifndef _NOVAS_
   #include "novas.h"
#endif

#include <math.h>

/*
   Global variables.

   'PSI_COR' and 'EPS_COR' are celestial pole offsets for high-
   precision applications.  See function 'cel_pole' for more details.
*/

static double PSI_COR = 0.0;
static double EPS_COR = 0.0;



/********e_tilt */

void e_tilt (double jd_tdb, short int accuracy,

             double *mobl, double *tobl, double *ee, double *dpsi,
             double *deps)
/*
------------------------------------------------------------------------

   PURPOSE:
      Computes quantities related to the orientation of the Earth's
      rotation axis at Julian date 'jd_tdb'.

   REFERENCES:
      None.

   INPUT
   ARGUMENTS:
      jd_tdb (double)
         TDB Julian Date.
      accuracy (short int)
         Selection for accuracy
            = 0 ... full accuracy
            = 1 ... reduced accuracy

   OUTPUT
   ARGUMENTS:
      *mobl (double)
         Mean obliquity of the ecliptic in degrees at 'jd_tdb'.
      *tobl (double)
         True obliquity of the ecliptic in degrees at 'jd_tdb'.
      *ee (double)
         Equation of the equinoxes in seconds of time at 'jd_tdb'.
      *dpsi (double)
         Nutation in longitude in arcseconds at 'jd_tdb'.
      *deps (double)
         Nutation in obliquity in arcseconds at 'jd_tdb'.

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      PSI_COR, EPS_COR   novas.c
      T0, ASEC2RAD       novascon.c
      DEG2RAD            novascon.c

   FUNCTIONS
   CALLED:
      nutation_angles    novas.c
      ee_ct              novas.c
      mean_obliq         novas.c
      fabs               math.h
      cos                math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/08-93/WTH (USNO/AA) Translate Fortran.
      V1.1/06-97/JAB (USNO/AA) Incorporate IAU (1994) and IERS (1996)
                               adjustment to the "equation of the
                               equinoxes".
      V1.2/10-97/JAB (USNO/AA) Implement function that computes
                               arguments of the nutation series.
      V1.3/07-98/JAB (USNO/AA) Use global variables 'PSI_COR' and
                               'EPS_COR' to apply celestial pole offsets
                               for high-precision applications.
      V2.0/10-03/JAB (USNO/AA) Update function for IAU 2000 resolutions.
      V2.1/12-04/JAB (USNO/AA) Add 'mode' argument.
      V2.2/01-06/WKP (USNO/AA) Changed 'mode' to 'accuracy'.

   NOTES:
      1. Values of the celestial pole offsets 'PSI_COR' and 'EPS_COR'
      are set using function 'cel_pole', if desired.  See the prolog
      of 'cel_pole' for details.
      2. This function is the C version of NOVAS Fortran routine
      'etilt'.

------------------------------------------------------------------------
*/
{
   static short int accuracy_last = 0;
   short int acc_diff;

   static double jd_last = 0.0;
   static double dp, de, c_terms;
   double t, d_psi, d_eps, mean_ob, true_ob, eq_eq;

/*
   Compute time in Julian centuries from epoch J2000.0.
*/

   t = (jd_tdb - T0) / 36525.0;

/*
   Check for difference in accuracy mode from last call.
*/

   acc_diff = accuracy - accuracy_last;

/*
   Compute the nutation angles (arcseconds) if the input Julian date
   is significantly different from the last Julian date, or the
   accuracy mode has changed from the last call.
*/

   if (((fabs (jd_tdb - jd_last)) > 1.0e-8) || (acc_diff != 0))
   {
      nutation_angles (t,accuracy, &dp,&de);

/*
   Obtain complementary terms for equation of the equinoxes in
   arcseconds.
*/

      c_terms = ee_ct (jd_tdb,0.0,accuracy) / ASEC2RAD;

/*
   Reset the values of the last Julian date and last mode.
*/

      jd_last = jd_tdb;
      accuracy_last = accuracy;
   }

/*
   Apply observed celestial pole offsets.
*/

   d_psi = dp + PSI_COR;
   d_eps = de + EPS_COR;

/*
   Compute mean obliquity of the ecliptic in arcseconds.
*/

   mean_ob = mean_obliq (jd_tdb);

/*
   Compute true obliquity of the ecliptic in arcseconds.
*/

   true_ob = mean_ob + d_eps;

/*
   Convert obliquity values to degrees.
*/

   mean_ob /= 3600.0;
   true_ob /= 3600.0;

/*
   Compute equation of the equinoxes in seconds of time.
*/

   eq_eq = d_psi * cos (mean_ob * DEG2RAD) + c_terms;
   eq_eq /= 15.0;

/*
   Set output values.
*/

   *dpsi = d_psi;
   *deps = d_eps;
   *ee   = eq_eq;
   *mobl = mean_ob;
   *tobl = true_ob;

   return;
}

/********ee_ct */

double ee_ct (double jd_high, double jd_low, short int accuracy)
/*
------------------------------------------------------------------------

   PURPOSE:
      To compute the "complementary terms" of the equation of the
      equinoxes.

   REFERENCES:
      Capitaine, N., Wallace, P.T., and McCarthy, D.D. (2003). Astron. &
         Astrophys. 406, p. 1135-1149. Table 3.
      IERS Conventions (2010), Chapter 5, p. 60, Table 5.2e.
         (Table 5.2e presented in the printed publication is a truncated
         series. The full series, which is used in NOVAS, is available
         on the IERS Conventions Center website in file tab5.2e.txt.)
         ftp://tai.bipm.org/iers/conv2010/chapter5/

   INPUT
   ARGUMENTS:
      jd_high (double)
         High-order part of TT Julian date.
      jd_low (double)
         Low-order part of TT Julian date.
      accuracy (short int)
         Selection for accuracy
            = 0 ... full accuracy
            = 1 ... reduced accuracy

   OUTPUT
   ARGUMENTS:
      None

   RETURNED
   VALUE:
      (double)
         Complementary terms, in radians.

   GLOBALS
   USED:
      T0, ASEC2RAD       novascon.c
      TWOPI              novascon.c

   FUNCTIONS
   CALLED:
      norm_ang           novas.c
      fund_args          novas.c
      fmod               math.h
      sin                math.h
      cos                math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/09-03/JAB (USNO/AA)
      V1.1/12-04/JAB (USNO/AA) Added low-accuracy formula.
      V1.2/01-06/WKP (USNO/AA) Changed 'mode' to 'accuracy'.
      V1.3/11-10/JAB (USNO/AA) Updated reference and notes.
      V1.4/03-11/WKP (USNO/AA) Added braces to 2-D array initialization
                               to quiet gcc warnings.

   NOTES:
      1. The series used in this function was derived from the first
      reference.  This same series was also adopted for use in the IAU's
      Standards of Fundamental Astronomy (SOFA) software (i.e.,
      subroutine eect00.for and function eect00.c).
      2. The low-accuracy series used in this function is a simple
      implementation derived from the first reference, in which terms
      smaller than 2 microarcseconds have been omitted.
      3. This function is based on NOVAS Fortran routine 'eect2000',
      with the low-accuracy formula taken from NOVAS Fortran routine
      'etilt'.

------------------------------------------------------------------------
*/
{
   short int i, j;

   double t, fa[14], fa2[5], s0, s1, a, c_terms;

/*
   Argument coefficients for t^0.
*/

   const short int ke0_t[33][14] = {
      {0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2, -2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2, -2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  0,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1,  2, -2,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1,  2, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  4, -4,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  1, -1,  1,  0, -8, 12,  0,  0,  0,  0,  0,  0},
      {0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0,  2,  0,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0,  2,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2, -2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1, -2,  2, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1, -2,  2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  0,  0,  0,  0,  8,-13,  0,  0,  0,  0,  0, -1},
      {0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {2,  0, -2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0,  0, -2,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  1,  2, -2,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0,  0, -2, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  4, -2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {0,  0,  2, -2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0, -2,  0, -3,  0,  0,  0,  0,  0,  0,  0,  0,  0},
      {1,  0, -2,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0}};

/*
   Argument coefficients for t^1.
*/

   const short int ke1[14] =
      {0,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0};

/*
   Sine and cosine coefficients for t^0.
*/

   const double se0_t[33][2] = {
      {+2640.96e-6,          -0.39e-6},
      {  +63.52e-6,          -0.02e-6},
      {  +11.75e-6,          +0.01e-6},
      {  +11.21e-6,          +0.01e-6},
      {   -4.55e-6,          +0.00e-6},
      {   +2.02e-6,          +0.00e-6},
      {   +1.98e-6,          +0.00e-6},
      {   -1.72e-6,          +0.00e-6},
      {   -1.41e-6,          -0.01e-6},
      {   -1.26e-6,          -0.01e-6},
      {   -0.63e-6,          +0.00e-6},
      {   -0.63e-6,          +0.00e-6},
      {   +0.46e-6,          +0.00e-6},
      {   +0.45e-6,          +0.00e-6},
      {   +0.36e-6,          +0.00e-6},
      {   -0.24e-6,          -0.12e-6},
      {   +0.32e-6,          +0.00e-6},
      {   +0.28e-6,          +0.00e-6},
      {   +0.27e-6,          +0.00e-6},
      {   +0.26e-6,          +0.00e-6},
      {   -0.21e-6,          +0.00e-6},
      {   +0.19e-6,          +0.00e-6},
      {   +0.18e-6,          +0.00e-6},
      {   -0.10e-6,          +0.05e-6},
      {   +0.15e-6,          +0.00e-6},
      {   -0.14e-6,          +0.00e-6},
      {   +0.14e-6,          +0.00e-6},
      {   -0.14e-6,          +0.00e-6},
      {   +0.14e-6,          +0.00e-6},
      {   +0.13e-6,          +0.00e-6},
      {   -0.11e-6,          +0.00e-6},
      {   +0.11e-6,          +0.00e-6},
      {   +0.11e-6,          +0.00e-6}};
/*
   Sine and cosine coefficients for t^1.
*/

   const double se1[2] =
      {   -0.87e-6,          +0.00e-6};

/*
   Interval between fundamental epoch J2000.0 and current date.
*/

      t = ((jd_high - T0) + jd_low) / 36525.0;

/*
   High accuracy mode.
*/

   if (accuracy == 0)
   {

/*
   Fundamental Arguments.

   Mean Anomaly of the Moon.
*/

      fa[0] = norm_ang ((485868.249036 +
                         (715923.2178 +
                         (    31.8792 +
                         (     0.051635 +
                         (    -0.00024470)
                         * t) * t) * t) * t) * ASEC2RAD
                         + fmod (1325.0*t, 1.0) * TWOPI);

/*
   Mean Anomaly of the Sun.
*/

      fa[1] = norm_ang ((1287104.793048 +
                         (1292581.0481 +
                         (     -0.5532 +
                         (     +0.000136 +
                         (     -0.00001149)
                         * t) * t) * t) * t) * ASEC2RAD
                         + fmod (99.0*t, 1.0) * TWOPI);

/*
   Mean Longitude of the Moon minus Mean Longitude of the Ascending
   Node of the Moon.
*/

      fa[2] = norm_ang (( 335779.526232 +
                         ( 295262.8478 +
                         (    -12.7512 +
                         (     -0.001037 +
                         (      0.00000417)
                         * t) * t) * t) * t) * ASEC2RAD
                         + fmod (1342.0*t, 1.0) * TWOPI);

/*
   Mean Elongation of the Moon from the Sun.
*/

      fa[3] = norm_ang ((1072260.703692 +
                         (1105601.2090 +
                         (     -6.3706 +
                         (      0.006593 +
                         (     -0.00003169)
                         * t) * t) * t) * t) * ASEC2RAD
                         + fmod (1236.0*t, 1.0) * TWOPI);

/*
   Mean Longitude of the Ascending Node of the Moon.
*/

      fa[4] = norm_ang (( 450160.398036 +
                         (-482890.5431 +
                         (      7.4722 +
                         (      0.007702 +
                         (     -0.00005939)
                         * t) * t) * t) * t) * ASEC2RAD
                         + fmod (-5.0*t, 1.0) * TWOPI);

      fa[ 5] = norm_ang (4.402608842 + 2608.7903141574 * t);
      fa[ 6] = norm_ang (3.176146697 + 1021.3285546211 * t);
      fa[ 7] = norm_ang (1.753470314 +  628.3075849991 * t);
      fa[ 8] = norm_ang (6.203480913 +  334.0612426700 * t);
      fa[ 9] = norm_ang (0.599546497 +   52.9690962641 * t);
      fa[10] = norm_ang (0.874016757 +   21.3299104960 * t);
      fa[11] = norm_ang (5.481293872 +    7.4781598567 * t);
      fa[12] = norm_ang (5.311886287 +    3.8133035638 * t);
      fa[13] =          (0.024381750 +    0.00000538691 * t) * t;

/*
   Evaluate the complementary terms.
*/

      s0 = 0.0;
      s1 = 0.0;

      for (i = 32; i >= 0; i--)
      {
         a = 0.0;

         for (j = 0; j < 14; j++)
         {
            a += (double) ke0_t[i][j] * fa[j];
         }

         s0 += (se0_t[i][0] * sin (a) + se0_t[i][1] * cos (a));
      }

      a = 0.0;

      for (j = 0; j < 14; j++)
      {
         a += (double) (ke1[j]) * fa[j];
      }

      s1 += (se1[0] * sin (a) + se1[1] * cos (a));

      c_terms = (s0 + s1 * t);
   }

    else

/*
   Low accuracy mode: Terms smaller than 2 microarcseconds omitted.
*/

   {
      fund_args (t, fa2);
      c_terms =
          2640.96e-6 * sin (fa2[4])
         +  63.52e-6 * sin (2.0 * fa2[4])
         +  11.75e-6 * sin (2.0 * fa2[2] - 2.0 * fa2[3] + 3.0 * fa2[4])
         +  11.21e-6 * sin (2.0 * fa2[2] - 2.0 * fa2[3] +       fa2[4])
         -   4.55e-6 * sin (2.0 * fa2[2] - 2.0 * fa2[3] + 2.0 * fa2[4])
         +   2.02e-6 * sin (2.0 * fa2[2]                + 3.0 * fa2[4])
         +   1.98e-6 * sin (2.0 * fa2[2]                +       fa2[4])
         -   1.72e-6 * sin (3.0 * fa2[4])
         -   0.87e-6 * t * sin (fa2[4]);
   }

   return (c_terms *= ASEC2RAD);
}

/********aberration */

void aberration (double *pos, double *ve, double lighttime,

                 double *pos2)
/*
------------------------------------------------------------------------

   PURPOSE:
      Corrects position vector for aberration of light.  Algorithm
      includes relativistic terms.

   REFERENCES:
      Murray, C. A. (1981) Mon. Notices Royal Ast. Society 195, 639-648.
      Kaplan, G. H. et. al. (1989). Astron. Journ. 97, 1197-1210.

   INPUT
   ARGUMENTS:
      pos[3] (double)
         Position vector, referred to origin at center of mass of the
         Earth, components in AU.
      ve[3] (double)
         Velocity vector of center of mass of the Earth, referred to
         origin at solar system barycenter, components in AU/day.
      lighttime (double)
         Light time from object to Earth in days.

   OUTPUT
   ARGUMENTS:
      pos2[3] (double)
         Position vector, referred to origin at center of mass of the
         Earth, corrected for aberration, components in AU

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      C_AUDAY            novascon.c

   FUNCTIONS
   CALLED:
      sqrt               math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/01-93/TKB (USNO/NRL Optical Interfer.) Translate Fortran.
      V1.1/08-93/WTH (USNO/AA) Update to C Standards.
      V1.2/12-02/JAB (USNO/AA) Improve efficiency by removing calls to
                               'pow' and replacing final 'for' loop
                               with direct assignment.
      V1.3/11-03/JAB (USNO/AA) Remove returned value.
      V1.4/02-06/WKP (USNO/AA) Changed C to C_AUDAY.

   NOTES:
      1. This function is the C version of NOVAS Fortran routine
      'aberat'.
      2. If 'lighttime' = 0 on input, this function will compute it.

------------------------------------------------------------------------
*/
{
   double p1mag, vemag, beta, dot, cosd, gammai, p, q, r;

   if (lighttime == 0.0)
   {
      p1mag = sqrt (pos[0] * pos[0] + pos[1] * pos[1] + pos[2] *
         pos[2]);
      lighttime = p1mag / C_AUDAY;
   }
    else
      p1mag = lighttime * C_AUDAY;

   vemag = sqrt (ve[0] * ve[0] + ve[1] * ve[1] + ve[2] * ve[2]);
   beta = vemag / C_AUDAY;
   dot = pos[0] * ve[0] + pos[1] * ve[1] + pos[2] * ve[2];

   cosd = dot / (p1mag * vemag);
   gammai = sqrt (1.0 - beta * beta);
   p = beta * cosd;
   q = (1.0 + p / (1.0 + gammai)) * lighttime;
   r = 1.0 + p;

   pos2[0] = (gammai * pos[0] + q * ve[0]) / r;
   pos2[1] = (gammai * pos[1] + q * ve[1]) / r;
   pos2[2] = (gammai * pos[2] + q * ve[2]) / r;

   return;
}

/********precession */

short int precession (double jd_tdb1, double *pos1, double jd_tdb2,

                      double *pos2)
/*
------------------------------------------------------------------------

   PURPOSE:
      Precesses equatorial rectangular coordinates from one epoch to
      another.  One of the two epochs must be J2000.0.  The coordinates
      are referred to the mean dynamical equator and equinox of the two
      respective epochs.

   REFERENCES:
      Explanatory Supplement To The Astronomical Almanac, pp. 103-104.
      Capitaine, N. et al. (2003), Astronomy And Astrophysics 412,
         pp. 567-586.
      Hilton, J. L. et al. (2006), IAU WG report, Celest. Mech., 94,
         pp. 351-367.

   INPUT
   ARGUMENTS:
      jd_tdb1 (double)
         TDB Julian date of first epoch.  See Note 1 below.
      pos1[3] (double)
         Position vector, geocentric equatorial rectangular coordinates,
         referred to mean dynamical equator and equinox of first epoch.
      jd_tdb2 (double)
         TDB Julian date of second epoch.  See Note 1 below.

   OUTPUT
   ARGUMENTS:
      pos2[3] (double)
         Position vector, geocentric equatorial rectangular coordinates,
         referred to mean dynamical equator and equinox of second epoch.

   RETURNED
   VALUE:
      (short int)
         = 0 ... everything OK.
         = 1 ... Precession not to or from J2000.0; 'jd_tdb1' or 'jd_tdb2'
                 not 2451545.0.

   GLOBALS
   USED:
      T0, ASEC2RAD       novascon.c

   FUNCTIONS
   CALLED:
      fabs               math.h
      sin                math.h
      cos                math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/01-93/TKB (USNO/NRL Optical Interfer.) Translate Fortran.
      V1.1/08-93/WTH (USNO/AA) Update to C Standards.
      V1.2/03-98/JAB (USNO/AA) Change function type from 'short int' to
                               'void'.
      V1.3/12-99/JAB (USNO/AA) Precompute trig terms for greater
                               efficiency.
      V2.0/10-03/JAB (USNO/AA) Update function for consistency with
                               IERS (2000) Conventions.
      V2.1/01-05/JAB (USNO/AA) Update expressions for the precession
                               angles (extra significant digits).
      V2.2/04-06/JAB (USNO/AA) Update model to 2006 IAU convention.
                               This is model "P03" of second reference.
      V2.3/03-10/JAB (USNO/AA) Implement 'first-time' to fix bug when
                                'jd_tdb2' is 'T0' on first call to
                                function.

   NOTES:
      1. Either 'jd_tdb1' or 'jd_tdb2' must be 2451545.0 (J2000.0) TDB.
      2. This function is the C version of NOVAS Fortran routine
      'preces'.

------------------------------------------------------------------------
*/
{
   static short int first_time = 1;
   short int error = 0;

   static double t_last = 0.0;
   static double xx, yx, zx, xy, yy, zy, xz, yz, zz;
   double eps0 = 84381.406;
   double  t, psia, omegaa, chia, sa, ca, sb, cb, sc, cc, sd, cd;

/*
   Check to be sure that either 'jd_tdb1' or 'jd_tdb2' is equal to T0.
*/

   if ((jd_tdb1 != T0) && (jd_tdb2 != T0))
      return (error = 1);

/*
   't' is time in TDB centuries between the two epochs.
*/

   t = (jd_tdb2 - jd_tdb1) / 36525.0;

   if (jd_tdb2 == T0)
      t = -t;

   if ((fabs (t - t_last) >= 1.0e-15) || (first_time == 1))
   {

/*
   Numerical coefficients of psi_a, omega_a, and chi_a, along with
   epsilon_0, the obliquity at J2000.0, are 4-angle formulation from
   Capitaine et al. (2003), eqs. (4), (37), & (39).
*/

      psia   = ((((-    0.0000000951  * t
                   +    0.000132851 ) * t
                   -    0.00114045  ) * t
                   -    1.0790069   ) * t
                   + 5038.481507    ) * t;

      omegaa = ((((+    0.0000003337  * t
                   -    0.000000467 ) * t
                   -    0.00772503  ) * t
                   +    0.0512623   ) * t
                   -    0.025754    ) * t + eps0;

      chia   = ((((-    0.0000000560  * t
                   +    0.000170663 ) * t
                   -    0.00121197  ) * t
                   -    2.3814292   ) * t
                   +   10.556403    ) * t;

      eps0 = eps0 * ASEC2RAD;
      psia = psia * ASEC2RAD;
      omegaa = omegaa * ASEC2RAD;
      chia = chia * ASEC2RAD;

      sa = sin (eps0);
      ca = cos (eps0);
      sb = sin (-psia);
      cb = cos (-psia);
      sc = sin (-omegaa);
      cc = cos (-omegaa);
      sd = sin (chia);
      cd = cos (chia);
/*
   Compute elements of precession rotation matrix equivalent to
   R3(chi_a) R1(-omega_a) R3(-psi_a) R1(epsilon_0).
*/

      xx =  cd * cb - sb * sd * cc;
      yx =  cd * sb * ca + sd * cc * cb * ca - sa * sd * sc;
      zx =  cd * sb * sa + sd * cc * cb * sa + ca * sd * sc;
      xy = -sd * cb - sb * cd * cc;
      yy = -sd * sb * ca + cd * cc * cb * ca - sa * cd * sc;
      zy = -sd * sb * sa + cd * cc * cb * sa + ca * cd * sc;
      xz =  sb * sc;
      yz = -sc * cb * ca - sa * cc;
      zz = -sc * cb * sa + cc * ca;

      t_last = t;
      first_time = 0;
   }

   if (jd_tdb2 == T0)
   {

/*
   Perform rotation from epoch to J2000.0.
*/
      pos2[0] = xx * pos1[0] + xy * pos1[1] + xz * pos1[2];
      pos2[1] = yx * pos1[0] + yy * pos1[1] + yz * pos1[2];
      pos2[2] = zx * pos1[0] + zy * pos1[1] + zz * pos1[2];
   }
    else
   {

/*
   Perform rotation from J2000.0 to epoch.
*/

      pos2[0] = xx * pos1[0] + yx * pos1[1] + zx * pos1[2];
      pos2[1] = xy * pos1[0] + yy * pos1[1] + zy * pos1[2];
      pos2[2] = xz * pos1[0] + yz * pos1[1] + zz * pos1[2];
   }

   return (error = 0);
}

/********nutation */

void nutation (double jd_tdb, short int direction, short int accuracy,
               double *pos,

               double *pos2)
/*
------------------------------------------------------------------------

   PURPOSE:
      Nutates equatorial rectangular coordinates from mean equator and
      equinox of epoch to true equator and equinox of epoch. Inverse
      transformation may be applied by setting flag 'direction'.

   REFERENCES:
      Explanatory Supplement To The Astronomical Almanac, pp. 114-115.

   INPUT
   ARGUMENTS:
      jd_tdb (double)
         TDB Julian date of epoch.
      direction (short int)
         Flag determining 'direction' of transformation;
            direction  = 0 transformation applied, mean to true.
            direction != 0 inverse transformation applied, true to mean.
      accuracy (short int)
         Selection for accuracy
            = 0 ... full accuracy
            = 1 ... reduced accuracy
      pos[3] (double)
         Position vector, geocentric equatorial rectangular coordinates,
         referred to mean equator and equinox of epoch.

   OUTPUT
   ARGUMENTS:
      pos2[3] (double)
         Position vector, geocentric equatorial rectangular coordinates,
         referred to true equator and equinox of epoch.

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      DEG2RAD, ASEC2RAD  novascon.c

   FUNCTIONS
   CALLED:
      e_tilt             novas.c
      cos                math.h
      sin                math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/01-93/TKB (USNO/NRL Optical Interfer.) Translate Fortran.
      V1.1/08-93/WTH (USNO/AA) Update to C Standards.
      V1.2/11-03/JAB (USNO/AA) Remove returned value.
      V1.3/01-06/WKP (USNO/AA) Changed 'mode' to 'accuracy'.

   NOTES:
      1. This function is the C version of NOVAS Fortran routine
      'nutate'.

------------------------------------------------------------------------
*/
{
   double cobm, sobm, cobt, sobt, cpsi, spsi, xx, yx, zx, xy, yy, zy,
      xz, yz, zz, oblm, oblt, eqeq, psi, eps;

/*
   Call 'e_tilt' to get the obliquity and nutation angles.
*/

   e_tilt (jd_tdb,accuracy, &oblm,&oblt,&eqeq,&psi,&eps);

   cobm = cos (oblm * DEG2RAD);
   sobm = sin (oblm * DEG2RAD);
   cobt = cos (oblt * DEG2RAD);
   sobt = sin (oblt * DEG2RAD);
   cpsi = cos (psi * ASEC2RAD);
   spsi = sin (psi * ASEC2RAD);

/*
   Nutation rotation matrix follows.
*/

   xx = cpsi;
   yx = -spsi * cobm;
   zx = -spsi * sobm;
   xy = spsi * cobt;
   yy = cpsi * cobm * cobt + sobm * sobt;
   zy = cpsi * sobm * cobt - cobm * sobt;
   xz = spsi * sobt;
   yz = cpsi * cobm * sobt - sobm * cobt;
   zz = cpsi * sobm * sobt + cobm * cobt;

   if (!direction)
   {

/*
   Perform rotation.
*/

      pos2[0] = xx * pos[0] + yx * pos[1] + zx * pos[2];
      pos2[1] = xy * pos[0] + yy * pos[1] + zy * pos[2];
      pos2[2] = xz * pos[0] + yz * pos[1] + zz * pos[2];
   }
    else
   {

/*
   Perform inverse rotation.
*/

      pos2[0] = xx * pos[0] + xy * pos[1] + xz * pos[2];
      pos2[1] = yx * pos[0] + yy * pos[1] + yz * pos[2];
      pos2[2] = zx * pos[0] + zy * pos[1] + zz * pos[2];
   }

   return;
}

/********nutation_angles */

void nutation_angles (double t, short int accuracy,

                      double *dpsi, double *deps)
/*
------------------------------------------------------------------------

   PURPOSE:
      This function returns the values for nutation in longitude and
      nutation in obliquity for a given TDB Julian date.  The nutation
      model selected depends upon the input value of 'accuracy'.  See
      notes below for important details.

   REFERENCES:
      Kaplan, G. (2005), US Naval Observatory Circular 179.

   INPUT
   ARGUMENTS:
      t (double)
         TDB time in Julian centuries since J2000.0
      accuracy (short int)
         Selection for accuracy
            = 0 ... full accuracy
            = 1 ... reduced accuracy

   OUTPUT
   ARGUMENTS:
      dpsi (double)
         Nutation in longitude in arcseconds.
      deps (double)
         Nutation in obliquity in arcseconds.

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      T0, ASEC2RAD       novascon.c

   FUNCTIONS
   CALLED:
      iau2000a           nutation.c
      iau2000b           nutation.c
      nu2000k            nutation.c

   VER./DATE/
   PROGRAMMER:
      V1.0/12-04/JAB (USNO/AA)
      V1.1/01-06/WKP (USNO/AA): Changed 'mode' to 'accuracy'.
      V1.2/02-06/WKP (USNO/AA): Fixed units bug.
      V1.3/01-07/JAB (USNO/AA): Implemented 'low_acc_choice' construct.

   NOTES:
      1. This function selects the nutation model depending first upon
      the input value of 'accuracy'.  If 'accuracy' = 0 (full accuracy),
      the IAU 2000A nutation model is used.  If 'accuracy' = 1 (reduced
      accuracy, the model used depends upon the value of local
      variable 'low_acc_choice', which is set below.
      2. If local variable 'low_acc_choice' = 1 (the default), a
      specially truncated version of IAU 2000A, called 'NU2000K' is
      used.  If 'low_acc_choice' = 2, the IAU 2000B nutation model is
      used.
      3.  See the prologs of the nutation functions in file 'nutation.c'
      for details concerning the models.
      4. This function is the C version of NOVAS Fortran routine
      'nod'.

------------------------------------------------------------------------
*/
{

/*
   Set the value of 'low_acc_choice' according to the rules explained
   under NOTES in the prolog.
*/

   double t1;

   t1 = t * 36525.0;

/*
   High accuracy mode -- use IAU 2000A.
*/
   iau2000a (T0,t1, dpsi,deps);

/*
   Convert output to arcseconds.
*/

   *dpsi /= ASEC2RAD;
   *deps /= ASEC2RAD;

   return;
}

/********fund_args */

void fund_args (double t,

                double a[5])
/*
------------------------------------------------------------------------

   PURPOSE:
      To compute the fundamental arguments (mean elements) of the Sun
      and Moon.

   REFERENCES:
      Simon et al. (1994) Astronomy and Astrophysics 282, 663-683,
         esp. Sections 3.4-3.5.

   INPUT
   ARGUMENTS:
      t (double)
         TDB time in Julian centuries since J2000.0

   OUTPUT
   ARGUMENTS:
      a[5] (double)
         Fundamental arguments, in radians:
          a[0] = l (mean anomaly of the Moon)
          a[1] = l' (mean anomaly of the Sun)
          a[2] = F (mean argument of the latitude of the Moon)
          a[3] = D (mean elongation of the Moon from the Sun)
          a[4] = Omega (mean longitude of the Moon's ascending node);
                 from Simon section 3.4(b.3),
                 precession = 5028.8200 arcsec/cy)

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      ASEC2RAD, ASEC360  novascon.c

   FUNCTIONS
   CALLED:
      fmod               math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/10-97/JAB (USNO/AA)
      V1.1/07-98/JAB (USNO/AA): Place arguments in the range 0-TWOPI
                                radians.
      V1.2/09-03/JAB (USNO/AA): Incorporate function 'norm_ang'.
      V1.3/11-03/JAB (USNO/AA): Update with Simon et al. expressions.
      V1.4/01-06/JAB (USNO/AA): Remove function 'norm_ang'; rewrite for
                                consistency with Fortran.
      V1.5/02-11/WKP (USNO/AA): Clarified a[4] description in prolog.

   NOTES:
      1. This function is the C version of NOVAS Fortran routine
      'funarg'.

------------------------------------------------------------------------
*/
{

   a[0] = fmod (485868.249036 +
             t * (1717915923.2178 +
             t * (        31.8792 +
             t * (         0.051635 +
             t * (       - 0.00024470)))), ASEC360) * ASEC2RAD;

   a[1] = fmod (1287104.79305 +
             t * ( 129596581.0481 +
             t * (       - 0.5532 +
             t * (         0.000136 +
             t * (       - 0.00001149)))), ASEC360) * ASEC2RAD;

   a[2] = fmod (335779.526232 +
             t * (1739527262.8478 +
             t * (      - 12.7512 +
             t * (      -  0.001037 +
             t * (         0.00000417)))), ASEC360) * ASEC2RAD;

   a[3] = fmod (1072260.70369 +
             t * (1602961601.2090 +
             t * (       - 6.3706 +
             t * (         0.006593 +
             t * (       - 0.00003169)))), ASEC360) * ASEC2RAD;

   a[4] = fmod (450160.398036 +
             t * ( - 6962890.5431 +
             t * (         7.4722 +
             t * (         0.007702 +
             t * (       - 0.00005939)))), ASEC360) * ASEC2RAD;

   return;
}

/********mean_obliq */

double mean_obliq (double jd_tdb)
/*
------------------------------------------------------------------------

   PURPOSE:
      To compute the mean obliquity of the ecliptic.

   REFERENCES:
      Capitaine et al. (2003), Astronomy and Astrophysics 412, 567-586.

   INPUT
   ARGUMENTS:
      jd_tdb (double)
         TDB Julian Date.

   OUTPUT
   ARGUMENTS:
      None.

   RETURNED
   VALUE:
      (double)
         Mean obliquity of the ecliptic in arcseconds.

   GLOBALS
   USED:
      T0                 novascon.c

   FUNCTIONS
   CALLED:
      None.

   VER./DATE/
   PROGRAMMER:
      V1.0/12-04/JAB (USNO/AA)
      V2.0/04-06/JAB (USNO/AA) Update the expression for mean obliquity
                               using data from the reference.

   NOTES:
      None.

------------------------------------------------------------------------
*/
{
   double t, epsilon;

/*
   Compute time in Julian centuries from epoch J2000.0.
*/

   t = (jd_tdb - T0) / 36525.0;

/*
   Compute the mean obliquity in arcseconds.  Use expression from the
   reference's eq. (39) with obliquity at J2000.0 taken from eq. (37)
   or Table 8.
*/

   epsilon = (((( -  0.0000000434   * t
                  -  0.000000576  ) * t
                  +  0.00200340   ) * t
                  -  0.0001831    ) * t
                  - 46.836769     ) * t + 84381.406;

   return (epsilon);
}

/********radec2vector */

void radec2vector (double ra, double dec, double dist,

                   double *vector)
/*
------------------------------------------------------------------------

   PURPOSE:
      Converts equatorial spherical coordinates to a vector (equatorial
      rectangular coordinates).

   REFERENCES:
      None.

   INPUT
   ARGUMENTS:
      ra (double)
         Right ascension (hours).
      dec (double)
         Declination (degrees).
      dist (double)
         Distance (AU)

   OUTPUT
   ARGUMENTS:
      vector[3] (double)
         Position vector, equatorial rectangular coordinates (AU).

   RETURNED
   VALUE:
      (short int)
         = 0 ... Everything OK.

   GLOBALS
   USED:
      DEG2RAD            novascon.c

   FUNCTIONS
   CALLED:
      cos                math.h
      sin                math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/05-92/TKB (USNO/NRL Optical Interfer.) Translate Fortran.
      V1.1/08-93/WTH (USNO/AA) Update to C Standards.
      V1.2/08-09/JLB (USNO/AA) Documented "dist" in prolog

   NOTES:
      None.

------------------------------------------------------------------------
*/
{

   vector[0] = dist * cos (DEG2RAD * dec) * cos (DEG2RAD * 15.0 * ra);
   vector[1] = dist * cos (DEG2RAD * dec) * sin (DEG2RAD * 15.0 * ra);
   vector[2] = dist * sin (DEG2RAD * dec);

   return;
}

/********tdb2tt */

void tdb2tt (double tdb_jd,

             double *tt_jd, double *secdiff)
/*
------------------------------------------------------------------------

   PURPOSE:
      Computes the Terrestrial Time (TT) or Terrestrial Dynamical Time
      (TDT) Julian date corresponding to a Barycentric Dynamical Time
      (TDB) Julian date.

   REFERENCES:
      Fairhead, L. & Bretagnon, P. (1990) Astron. & Astrophys. 229, 240.
      Kaplan, G. (2005), US Naval Observatory Circular 179.

   INPUT
   ARGUMENTS:
      tdb_jd (double)
         TDB Julian date.

   OUTPUT
   ARGUMENTS:
      *tt_jd (double)
         TT Julian date.
      *secdiff (double)
         Difference 'tdb_jd'-'tt_jd', in seconds.

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      T0                 novascon.c

   FUNCTIONS
   CALLED:
      sin                math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/07-92/TKB (USNO/NRL Optical Interfer.) Translate Fortran.
      V1.1/08-93/WTH (USNO/AA) Update to C Standards.
      V1.2/06-98/JAB (USNO/AA) Adopt new model (Explanatory Supplement
                               to the Astronomical Almanac, pp. 42-44
                               and p. 316.)
      V1.3/11-03/JAB (USNO/AA) Changed variable names of the input
                               Julian dates to make more descriptive.
      V1.4/01-07/JAB (USNO/AA) Adopt Fairhead & Bretagnon expression.

   NOTES:
      1. Expression used in this function is a truncated form of a
      longer and more precise series given in the first reference.  The
      result is good to about 10 microseconds.
      2. This function is the C version of NOVAS Fortran routine
      'times'.
------------------------------------------------------------------------
*/
{
   double t;

   t = (tdb_jd - T0) / 36525.0;

/*
   Expression given in USNO Circular 179, eq. 2.6.
*/

   *secdiff = 0.001657 * sin ( 628.3076 * t + 6.2401)
            + 0.000022 * sin ( 575.3385 * t + 4.2970)
            + 0.000014 * sin (1256.6152 * t + 6.1969)
            + 0.000005 * sin ( 606.9777 * t + 4.0212)
            + 0.000005 * sin (  52.9691 * t + 0.4444)
            + 0.000002 * sin (  21.3299 * t + 5.5431)
            + 0.000010 * t * sin ( 628.3076 * t + 4.2490);

   *tt_jd = tdb_jd - *secdiff / 86400.0;

    return;
}


/********julian_date */

double julian_date (short int year, short int month, short int day,
                    double hour)
/*
------------------------------------------------------------------------

   PURPOSE:
      This function will compute the Julian date for a given calendar
      date (year, month, day, hour).

   REFERENCES:
      Fliegel, H. & Van Flandern, T.  Comm. of the ACM, Vol. 11, No. 10,
         October 1968, p. 657.

   INPUT
   ARGUMENTS:
      year (short int)
         Year.
      month (short int)
         Month number.
      day (short int)
         Day-of-month.
      hour (double)
         Hour-of-day.

   OUTPUT
   ARGUMENTS:
      None.

   RETURNED
   VALUE:
      (double)
         Julian date.

   GLOBALS
   USED:
      None.

   FUNCTIONS
   CALLED:
      None.

   VER./DATE/
   PROGRAMMER:
      V1.0/06-98/JAB (USNO/AA)
      V1.1/03-08/WKP (USNO/AA) Updated prolog.

   NOTES:
      1. This function is the C version of NOVAS Fortran routine
      'juldat'.
      2. This function makes no checks for a valid input calendar
      date.
      3. Input calendar date must be Gregorian.
      4. Input time value can be based on any UT-like time scale
      (UTC, UT1, TT, etc.) - output Julian date will have the same basis.
------------------------------------------------------------------------
*/
{
   long int jd12h;

   double tjd;

   jd12h = (long) day - 32075L + 1461L * ((long) year + 4800L
      + ((long) month - 14L) / 12L) / 4L
      + 367L * ((long) month - 2L - ((long) month - 14L) / 12L * 12L)
      / 12L - 3L * (((long) year + 4900L + ((long) month - 14L) / 12L)
      / 100L) / 4L;
   tjd = (double) jd12h - 0.5 + hour / 24.0;

   return (tjd);
}

/********cal_date */

void cal_date (double tjd,

               short int *year, short int *month, short int *day,
               double *hour)
/*
------------------------------------------------------------------------

   PURPOSE:
      This function will compute a date on the Gregorian calendar given
      the Julian date.

   REFERENCES:
      Fliegel, H. & Van Flandern, T.  Comm. of the ACM, Vol. 11, No. 10,
         October 1968, p. 657.

   INPUT
   ARGUMENTS:
      tjd (double)
         Julian date.

   OUTPUT
   ARGUMENTS:
      *year (short int)
         Year.
      *month (short int)
         Month number.
      *day (short int)
         Day-of-month.
      *hour (double)
         Hour-of-day.

   RETURNED
   VALUE:
      None.

   GLOBALS
   USED:
      None.

   FUNCTIONS
   CALLED:
      fmod               math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/06-98/JAB (USNO/AA)

   NOTES:
      1. This routine valid for any 'jd' greater than zero.
      2. Input Julian date can be based on any UT-like time scale
      (UTC, UT1, TT, etc.) - output time value will have same basis.
      3. This function is the C version of NOVAS Fortran routine
      'caldat'.


------------------------------------------------------------------------
*/
{
   long int jd, k, m, n;

   double djd;

   djd = tjd + 0.5;
   jd = (long int) djd;

   *hour = fmod (djd,1.0) * 24.0;

   k     = jd + 68569L;
   n     = 4L * k / 146097L;

   k     = k - (146097L * n + 3L) / 4L;
   m     = 4000L * (k + 1L) / 1461001L;
   k     = k - 1461L * m / 4L + 31L;

   *month = (short int) (80L * k / 2447L);
   *day   = (short int) (k - 2447L * (long int) *month / 80L);
   k      = (long int) *month / 11L;

   *month = (short int) ((long int) *month + 2L - 12L * k);
   *year  = (short int) (100L * (n - 49L) + m + k);

   return;
}


/********norm_ang */

double norm_ang (double angle)
/*
------------------------------------------------------------------------

   PURPOSE:
      Normalize angle into the range 0 <= angle < (2 * pi).

   REFERENCES:
      None.

   INPUT
   ARGUMENTS:
      angle (double)
         Input angle (radians).

   OUTPUT
   ARGUMENTS:
      None.

   RETURNED
   VALUE:
      (double)
          The input angle, normalized as described above (radians).

   GLOBALS
   USED:
      TWOPI              novascon.c

   FUNCTIONS
   CALLED:
      fmod               math.h

   VER./DATE/
   PROGRAMMER:
      V1.0/09-03/JAB (USNO/AA)

   NOTES:
      None.

------------------------------------------------------------------------
*/
{
   double a;

   a = fmod (angle,TWOPI);
   if (a < 0.0)
         a += TWOPI;

   return (a);
}
