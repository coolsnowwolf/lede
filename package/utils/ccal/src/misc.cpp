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

   misc.cpp
   1996/02/11

   Miscellaneous routines for moon phase programs
*/

#include "moon.h"
#include <math.h>

extern double torad(double x)
/* convert x to radians */
{
  x = fmod(x, 360.0); /* normalize the angle */
  return ((x) * 0.01745329251994329576);
                    /* and return the result */
}

