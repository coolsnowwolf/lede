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

   solarsystem.h: Header file for solsys1.c, solsys2.c, & solsys3.c

   U. S. Naval Observatory
   Astronomical Applications Dept.
   Washington, DC
   http://www.usno.navy.mil/USNO/astronomical-applications
*/

#ifndef _SOLSYS_
   #define _SOLSYS_

/*
   Function prototypes
*/

   short int solarsystem (double tjd, short int body, short int origin,

                          double *position, double *velocity);

#endif
