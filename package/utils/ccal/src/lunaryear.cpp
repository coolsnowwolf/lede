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
/* Determines the lunar month numbers for a given year */

#include "lunaryear.h"
#include "solarterm.h"
#include "mphases.h"
#ifdef USE_YEARCACHE
#include "yearcache.h"
#endif
#include "novas.h"
#include <assert.h>

#ifndef NO_NAMESPACE
using namespace std;
#endif

/* Inputs:
   jdws, Time of winter solstice of given year
   vmoons, vector of new moon times since day after winter solstice of
           previous year
   Returns:
   true -- normal year, false -- leap year
*/
bool IsNormalYear(double jdws, vdouble& vmoons)
{
    int i = 0, nmonth = 0;
    double jend = jdws + 0.5;
    jend = int(jend) + 0.5;
    while (i < int(vmoons.size()) && vmoons[i++] < jend)
        nmonth++;
    assert(nmonth == 12 || nmonth == 13);
    if (nmonth == 12) /* Normal year */
      return true;
    return false;
}

/* Inputs:
   jstart, time of new moon for the starting of the month
   jend, time of new moon for the starting of the next month
   vterms, vector of solarterms of the given year
   Returns:
   true -- a Zhongqi is in, false -- no Zhongqi
*/
bool IsZhongQiInMonth(double jstart, double jend, vdouble& vterms)
{
    jstart -= 0.5;
    jstart = int(jstart) + 0.5;
    jend -= 0.5;
    jend = int(jend) + 0.5;
    int i;
    for (i = 1; i < int(vterms.size()); i += 2)
    {
        if (vterms[i] >= jstart && vterms[i] < jend)
            return true;
        if (vterms[i] >= jend)
            return false;
    }
    return false;
}

/* Inputs:
   vjds, vector of julian dates to have the hour portion trimmed
   Outputs:
   vjds, vector of julian dates with the hour portion trimmed
*/
void TrimHour(vdouble& vjds)
{
    int i, t;
    for (i = 0; i < int(vjds.size()); i++)
    {
        t = int(vjds[i] + 0.5);
        vjds[i] = double(t);
    }
}

/* Inputs:
   year
   Outputs:
   vterms, vector of solarterm days for the given year
   lastnew, julian day for the last new moon of previous year
   lastmon, month number for that started on last new moon day of previous year
   vmoons, vector of new moon days for the given year
   vmonth, vector of lunar month numbers for the given year with half
           advance for leap month, i.e. 7.5 for leap 7th month
   nextnew, julian day for the first new moon of next year
   Returns:
   calendar month number in which a leap lunar month begins with 0.5 added if
   this leap lunar month runs into next calendar month
*/
double lunaryear(short int year, vdouble& vterms, double& lastnew,
                 double& lastmon, vdouble& vmoons, vdouble& vmonth,
                 double& nextnew)
{
#ifdef USE_YEARCACHE
    /* Use cache if in range */
    if (YearCache::IsInRange(year)) /* Cached year */
    {
        YearCache::GetYear(year, vterms, lastnew, lastmon, vmoons, vmonth,
                           nextnew);
    }
    else
#endif
    {
        /* Determine solar terms */
        double jdpws;
        solarterm(year, jdpws, vterms);
        /* Determine new moons since day after previous winter solstice */
        double jstart = jdpws + 0.5;
        jstart = int(jstart) + 0.5;
        double jend = julian_date(year, 12, 31, 23.9999);
        mphases(jstart, jend, 0, vmoons);
        /* Determine the month numbers */
        vmonth.resize(vmoons.size());
        int i;
        if (IsNormalYear(vterms[23], vmoons)) /* Normal year */
        {
            int n = 12;
            for (i = 0; i < int(vmoons.size()); i++)
            {
                vmonth[i] = n;
                if (n == 12)
                    n = 0;
                n++;
            }
            if (n == 1) /* Involves next lunar year */
            {
                double jdnwsp;
                vdouble vjdnterms;
                solarterm(year + 1, jdnwsp, vjdnterms);
                vdouble vnmoons;
                jstart = jdnwsp + 0.5;
                jstart = int(jstart) + 0.5;
                jend = julian_date(year + 1, 12, 31, 23.999);
                mphases(jstart, jend, 0, vnmoons);
                if (!IsNormalYear(vjdnterms[23], vnmoons))
                {
                    /* Check if a zhongqi falls inside the month */
                    if (!IsZhongQiInMonth(vnmoons[0], vnmoons[1], vjdnterms))
                        vmonth.back() = 11.5;
                }
            }
        }
        else /* Leap year */
        {
            bool bleaped = false;
            int n = 11;
            for (i = 0; i < int(vmoons.size()) - 1; i++)
            {
                /* Check if a zhongqi falls inside the month */
                if (bleaped || IsZhongQiInMonth(vmoons[i], vmoons[i + 1], vterms))
                    vmonth[i] = ++n;
                else
                {
                	if (n == 0)
                		vmonth[i] = 12.5;
                	else
                		vmonth[i] = n + 0.5;
                    bleaped = true;
                }
                if (n == 12)
                    n = 0;
            }
            vmonth.back() = n + 1;
        }
        if (vmoons[0] < julian_date(year, 1, 1, 0.0))
        {
            lastnew = floor(vmoons.front() + 0.5);
            lastmon = vmonth.front();
            vmoons.erase(vmoons.begin());
            vmonth.erase(vmonth.begin());
        }
        else /* Need to find the last new moon for previous year */
        {
            vdouble vlastnew;
            mphases(vmoons[0] - 35.0, vmoons[0] - 25.0, 0, vlastnew);
            TrimHour(vlastnew);
            lastnew = vlastnew.back();
            lastmon = 11.0;
        }
        /* Need to find the first new moon for next year */
        vdouble vnextnew;
        mphases(vmoons.back() + 25.0, vmoons.back() + 35.0, 0, vnextnew);
        TrimHour(vnextnew);
        nextnew = vnextnew.back();
        /* Convert to whole day numbers */
        TrimHour(vmoons);
        TrimHour(vterms);
        /* Apply correction from DE405 */
    	if (year == 1774)
    		vterms[2]++;
    	if (year == 1878)
    		vterms[8]++;
    	if (year == 1951)
    		vterms[23]++;
    }
    /* Scan for leap month and return the calendar month */
    if (int(lastmon + 0.9) != int(lastmon)) /* lastmon is a leap month */
    {
        if (julian_date(year, 1, 1, 12.0) < vmoons[0]) /* runs into new year */
            return 0.5;
    }
    short int monnum = 2;
	int i;
    for (i = 0; i < int(vmoons.size()); i++)
    {
        if (int(vmonth[i] + 0.9) != int(vmonth[i])) /* found leap month */
        {
            double jdfirst;
            while ((jdfirst = julian_date(year, monnum, 1, 12.0)) < vmoons[i] && monnum <= 12)
                monnum++;
            if (monnum == 13)
                return 12.0;
            /* See if leap month runs into next month */
            if (i != int(vmoons.size()) - 1 && jdfirst < vmoons[i + 1])
                return (monnum - 0.5); /* Yes */
            else
                return (monnum - 1.0); /* No */
        }
    }
    return 0.0;
}

