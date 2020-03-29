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
/* Cache results for some years for faster response */
#ifndef YEARCACHE_H
#define YEARCACHE_H

#ifndef NO_NAMESPACE
#include <vector>
typedef std::vector<double> vdouble;
#else
#include <vector.h>
typedef vector<double> vdouble;
#endif

class YearCache
{
public:
    /* Input:
       year
       Returns:
       true -- year is in range of cache, false -- otherwise
    */
    static bool IsInRange(short int year);
    /* Inputs:
       year
       Outputs:
       vterms, vector of solarterm times for the given year
       lastnew, julian day of last new moon for previous year
       lastmon, month number for that started on last new moon day of
                previous year
       vmoons, vector of new moon times for the given year
       vmonth, vector of lunar month numbers for the given year with half
               advance for leap month, i.e. 7.5 for leap 7th month
       nextnew, julian day of first new moon for next year
    */
    static void GetYear(short int year, vdouble& vterms, double& lastnew,
        double& lastmon, vdouble& vmoons, vdouble& vmonth, double& nextnew);
};

#endif /*YEARCACHE_H*/
