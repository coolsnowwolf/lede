/*
   Copyright (c) 2000-2012, by Zhuo Meng (zxm8@case.edu).
   All rights reserved.

   Distributed under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* Computes the julian dates (Beijing Time) of a given moon phase (0-3, for
   new moon to last quarter) between two given time marks
*/
#ifndef MPHASES_H
#define MPHASES_H

#ifndef NO_NAMESPACE
#include <vector>
typedef std::vector<double> vdouble;
#else
#include <vector.h>
typedef vector<double> vdouble;
#endif

/* Inputs:
   tstart, start of the period in julian date (Beijing Time)
   tend, end of the period in julian date (Beijing Time)
   phase, the desired phase, 0-3 for new moon to last quarter
   Output:
   vjdphases, the julian dates (Beijing Time) of all occurings of the
              desired phase
*/
void mphases(double tstart, double tend, int phase, vdouble& vjdphases);

#endif /* MPHASES_H */
