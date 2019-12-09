/*  weburl --	A netfilter module to match URLs in HTTP requests 
 *  		This module can match using string match or regular expressions
 *  		Originally designed for use with Gargoyle router firmware (gargoyle-router.com)
 *
 *
 *  Copyright © 2008 by Eric Bishop <eric@gargoyle-router.com>
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




#ifndef _IPT_WEBURL_H
#define _IPT_WEBURL_H


#define MAX_TEST_STR 1024

#define WEBURL_CONTAINS_TYPE 1
#define WEBURL_REGEX_TYPE 2
#define WEBURL_EXACT_TYPE 3
#define WEBURL_ALL_PART 4
#define WEBURL_DOMAIN_PART 5
#define WEBURL_PATH_PART 6

struct ipt_weburl_info
{
	char test_str[MAX_TEST_STR];
	unsigned char match_type;
	unsigned char match_part;
	unsigned char invert;
};
#endif /*_IPT_WEBURL_H*/
