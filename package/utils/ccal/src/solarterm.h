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
#ifndef SOLARTERM_H
#define SOLARTERM_H

#ifndef NO_NAMESPACE
#include <vector>
typedef std::vector<double> vdouble;
#else
#include <vector.h>
typedef vector<double> vdouble;
#endif

/* Given year, computes the julian date (Beijing Time) of the winter solstice
   of previous year (jdpws) and all solarterms of the given year (vjdterms)
*/
void solarterm(short int year, double& jdpws, vdouble& vjdterms);

#endif /* SOLARTERM_H */
