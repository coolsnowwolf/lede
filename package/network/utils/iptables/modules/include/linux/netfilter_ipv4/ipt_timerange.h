/*  timerange --	An iptables extension to match multiple timeranges within a week
 *  			Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2009 by Eric Bishop <eric@gargoyle-router.com>
 * 
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef _IPT_TIMERANGE_H
#define _IPT_TIMERANGE_H


#define RANGE_LENGTH 51

#define HOURS 1
#define WEEKDAYS 2
#define DAYS_HOURS (HOURS+WEEKDAYS)
#define WEEKLY_RANGE 4


struct ipt_timerange_info
{
	long ranges[RANGE_LENGTH];
	char days[7];
	char type;
	unsigned char invert;
};
#endif /*_IPT_TIMERANGE_H*/
