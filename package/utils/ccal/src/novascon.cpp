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
 
   novascon.c: Constants for use with NOVAS 
 
   U. S. Naval Observatory
   Astronomical Applications Dept.
   Washington, DC 
   http://www.usno.navy.mil/USNO/astronomical-applications
*/

#ifndef _CONSTS_
   #include "novascon.h"
#endif

   const short int FN0 = 0;

/*
   TDB Julian date of epoch J2000.0.
*/

   const double T0 = 2451545.00000000;

/*
   Speed of light in meters/second is a defining physical constant.
*/

   const double C = 299792458.0;

/*
   Light-time for one astronomical unit (AU) in seconds, from DE-405.
*/

   const double AU_SEC = 499.0047838061;

/*
   Speed of light in AU/day.  Value is 86400 / AU_SEC.
*/

   const double C_AUDAY = 173.1446326846693;

/*
   Astronomical unit in meters.  Value is AU_SEC * C.
*/

   const double AU = 1.4959787069098932e+11;

/*
   Astronomical Unit in kilometers.
*/

   const double AU_KM = 1.4959787069098932e+8;

/*
   Heliocentric gravitational constant in meters^3 / second^2, from
   DE-405.
*/

   const double GS = 1.32712440017987e+20;

/*
   Geocentric gravitational constant in meters^3 / second^2, from
   DE-405.
*/

   const double GE = 3.98600433e+14;

/*
   Radius of Earth in meters from IERS Conventions (2003).
*/

   const double ERAD = 6378136.6;

/*
   Earth ellipsoid flattening from IERS Conventions (2003).
   Value is 1 / 298.25642.
*/

   const double F = 0.003352819697896;

/*
   Rotational angular velocity of Earth in radians/sec from IERS
   Conventions (2003).
*/

   const double ANGVEL = 7.2921150e-5;

/*
   Reciprocal masses of solar system bodies, from DE-405
   (Sun mass / body mass).
   MASS[0] = Earth/Moon barycenter, MASS[1] = Mercury, ...,
   MASS[9] = Pluto, MASS[10] = Sun, MASS[11] = Moon.
*/

   const double RMASS[12] = {328900.561400, 6023600.0, 408523.71,
      332946.050895, 3098708.0, 1047.3486, 3497.898, 22902.98,
      19412.24, 135200000.0, 1.0, 27068700.387534};

/*
   Value of 2 * pi in radians.
*/

   const double TWOPI = 6.283185307179586476925287;

/*
   Number of arcseconds in 360 degrees.
*/

   const double ASEC360 = 1296000.0;

/*
   Angle conversion constants.
*/

   const double ASEC2RAD = 4.848136811095359935899141e-6;
   const double DEG2RAD = 0.017453292519943296;
   const double RAD2DEG = 57.295779513082321;











