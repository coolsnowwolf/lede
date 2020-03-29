/*
   Copyright 1996, Christopher Osburn, Lunar Outreach Services,

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

   moon.h

   Headers and function defs for moon phase routines
*/

#if !defined __MOON_H
#define __MOON__H

#include <time.h>

extern double moonphasebylunation(int lun, int phi);  /* moonphase.c */
extern double moonphase(double k, int phi);  /* moonphase.c */
extern double torad(double x);              /* misc.c      */

#endif /* __MOON_H */
