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
/* Computes Beijing time of solarterms for a given year */

#include "solarterm.h"
#include "tt2ut.h"
#include "novas.h"
#include <stdio.h>

#ifndef NO_NAMESPACE
using namespace std;
#endif

const double PI = 3.141592653589793;
const double errlimit = 360.0 / 365.2422 / 86400.0; /* One second */

typedef vector<double> vdouble;

/* Computes the angle in degrees of given UTC time,
   with 0 being winter solstice
*/
double timeangle(double t)
{
    double sposg[3], spos[3];
    double eposb[3], evelb[3], eposh[3], evelh[3];
    double y, angle;
    double dummy, secdiff, tdb, lighttime;

    tdb2tt(t, &dummy, &secdiff);
    tdb = t + secdiff / 86400.0;
    solarsystem(tdb, 3, HELIOC, eposh, evelh);
    solarsystem(tdb, 3, BARYC, eposb, evelb);
    /* Convert to geocentric sun position */
    spos[0] = -eposh[0];
    spos[1] = -eposh[1];
    spos[2] = -eposh[2];
    lighttime = sqrt(spos[0]*spos[0] + spos[1]*spos[1] + spos[2]*spos[2]) / C_AUDAY;
    aberration(spos, evelb, lighttime, sposg);
    precession(T0, sposg, tdb, spos);
    nutation(tdb, FN0, 0, spos, sposg);
    y = sqrt(sposg[1] * sposg[1] + sposg[2] * sposg[2]);
    y *= (sposg[1] > 0.0) ? 1.0 : -1.0;
    angle = atan2(y, sposg[0]) + PI / 2.0;
    if (angle < 0.0)
        angle += 2.0 * PI;
    angle *= 180.0 / PI;
    return angle;
}

void chklowvalue(double& tl, double& vl, double& vu, double termang)
{
    /* For discontinuity at 0 */
    if (vl > vu)
    	vl -= 360.0;
    /* In case the range start time is not early enough */
    /* As Gregorian year is slightly longer than tropical year, term time shifts backwards */
    if (vl > errlimit) {
    	tl -= 4;
        vl = timeangle(tl) - termang;
        chklowvalue(tl, vl, vu, termang);
    }
}

/* Computes the UTC time for given solar term:
   tstart:  search start time
   tend:    search end time
   termang: angle value of the given term
   returns the UTC time for the solar term
*/
double termtime(double tstart, double tend, double termang)
{
    double tl, tu, t, f;
    double v, vl, vu;

    tl = tstart;
    tu = tend;
    vl = timeangle(tl) - termang;
    vu = timeangle(tu) - termang;
    chklowvalue(tl, vl, vu, termang);
    do
    {
    	if (fabs(vu - vl) < errlimit)
    	{
    		t = (tu + tl) / 2.0;
    		v = (vu + vl) / 2.0;
    		break;
    	}
	    f = (tu - tl) / (vu - vl);
	    t = tl - f * vl;
	    v = timeangle(t) - termang;
	    if (v < 0 || v > vu)
	    {
	        chklowvalue(t, v, vu, termang);
	    	vl = v;
	    	tl = t;
	    }
	    else
	    {
	    	vu = v;
	    	tu = t;
	    }
    } while (fabs(v) > errlimit);
    return tt2ut(t);
}

/* Given year, computes the julian date (Beijing Time) of the winter solstice
   of previous year (jdpws) and all solarterms of the given year (vjdterms)
*/
void solarterm(short int year, double& jdpws, vdouble& vjdterms)
{
    int dstart, dend;
    short int j, month, day;
    double angle, offs;

    if (year < 1928)
        offs = (116.0 + 25.0 / 60.0) / 360.0;
    else
        offs = 120.0 / 360.0;
    /* Determine the time of winter solstice of previous year */
    dstart = (int) julian_date(year - 1, 12, 12, 12.0);
    dend = (int) julian_date(year - 1, 12, 25, 12.0);
    jdpws = termtime(dstart, dend, 0.0) + offs;
    vjdterms.resize(24);
    for (j = 0; j < 24; j++)
    {
        month = j / 2 + 1;
        day = (j % 2) * 14;
        dstart = (int) julian_date(year, month, 1 + day, 12.0);
        dend = (int) julian_date(year, month, 11 + day, 12.0);
        angle = ((j + 1) % 24) * 15.0;
        vjdterms[j] = termtime(dstart, dend, angle) + offs;
    }
}
