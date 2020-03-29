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
/* Converts TT to UT */
#include "tt2ut.h"
#include "novas.h"

/* Give TT in julian day, computes UT in julian day
   Algorithm is from NASA website
   http://eclipse.gsfc.nasa.gov/SEcat5/deltatpoly.html
*/
double tt2ut(double jd)
{
    short int year, month, day;
    double hour;
    double y, t, t2, t3, t4, t5, t6, t7, dt;

    cal_date(jd, &year, &month, &day, &hour);
    y = year + (month - 0.5) / 12;
    /* Year >= 1645 */
    if (y < 1700)
    {
        t = y - 1600;
        t2 = t * t;
        t3 = t2 * t;
        dt = 120 - 0.9808 * t - 0.01532 * t2 + t3 / 7129;
    }
    else if (y < 1800)
    {
        t = y - 1700;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t3 * t;
        dt = 8.83 + 0.1603 * t - 0.0059285 * t2 + 0.00013336 * t3 - t4 / 1174000;
    }
    else if (y < 1860)
    {
        t = y - 1800;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t3 * t;
        t5 = t4 * t;
        t6 = t5 * t;
        t7 = t6 * t;
        dt = 13.72 - 0.332447 * t + 0.0068612 * t2 + 0.0041116 * t3 - 0.00037436 * t4 + 0.0000121272 * t5 - 0.0000001699 * t6 + 0.000000000875 * t7;
    }
    else if (y < 1900)
    {
        t = y - 1860;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t3 * t;
        t5 = t4 * t;
        dt = 7.62 + 0.5737 * t - 0.251754 * t2 + 0.01680668 * t3 -0.0004473624 * t4 + t5 / 233174;
    }
    else if (y < 1920)
    {
        t = y - 1900;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t3 * t;
        dt = -2.79 + 1.494119 * t - 0.0598939 * t2 + 0.0061966 * t3 - 0.000197 * t4;
    }
    else if (y < 1941)
    {
        t = y - 1920;
        t2 = t * t;
        t3 = t2 * t;
        dt = 21.20 + 0.84493 * t - 0.076100 * t2 + 0.0020936 * t3;
    }
    else if (y < 1961)
    {
        t = y - 1950;
        t2 = t * t;
        t3 = t2 * t;
        dt = 29.07 + 0.407 * t - t2 / 233 + t3 / 2547;
    }
    else if (y < 1986)
    {
        t = y - 1975;
        t2 = t * t;
        t3 = t2 * t;
        dt = 45.45 + 1.067 * t - t2 / 260 - t3 / 718;
    }
    else if (y < 2005)
    {
        t = y - 2000;
        t2 = t * t;
        t3 = t2 * t;
        t4 = t3 * t;
        t5 = t4 * t;
        dt = 63.86 + 0.3345 * t - 0.060374 * t2 + 0.0017275 * t3 + 0.000651814 * t4 + 0.00002373599 * t5;
    }
    else if (y < 2050)
    {
        t = y - 2000;
        t2 = t * t;
        dt = 62.92 + 0.32217 * t + 0.005589 * t2;
    }
    else if (y < 2150)
    {
        t = (y - 1820) / 100;
        t2 = t * t;
        dt = -20 + 32 * t2 - 0.5628 * (2150 - y);
    }
    else
    {
        t = (y - 1820) / 100;
        t2 = t * t;
        dt = -20 + 32 * t2;
    }
    return (jd - dt / 86400);
}

