/*  bandwidth --	An iptables extension for bandwidth monitoring/control
 *  			Can be used to efficiently monitor bandwidth and/or implement bandwidth quotas
 *  			Can be queried using the iptbwctl userspace library
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

#ifndef _IPT_BANDWIDTH_H
#define _IPT_BANDWIDTH_H

/*flags -- first three don't map to parameters the rest do */
#define BANDWIDTH_INITIALIZED		   1
#define BANDWIDTH_REQUIRES_SUBNET	   2
#define BANDWIDTH_SUBNET		   4
#define BANDWIDTH_CMP			   8
#define BANDWIDTH_CURRENT		  16
#define BANDWIDTH_RESET_INTERVAL	  32
#define BANDWIDTH_RESET_TIME		  64
#define BANDWIDTH_LAST_BACKUP		 128


/* parameter defs that don't map to flag bits */
#define BANDWIDTH_TYPE			  70
#define BANDWIDTH_ID			  71
#define BANDWIDTH_GT			  72
#define BANDWIDTH_LT			  73
#define BANDWIDTH_MONITOR		  74
#define BANDWIDTH_CHECK			  75
#define BANDWIDTH_CHECK_NOSWAP		  76
#define BANDWIDTH_CHECK_SWAP		  77
#define BANDWIDTH_NUM_INTERVALS		  78

/* possible reset intervals */
#define BANDWIDTH_MINUTE		  80
#define BANDWIDTH_HOUR			  81
#define BANDWIDTH_DAY			  82
#define BANDWIDTH_WEEK			  83
#define BANDWIDTH_MONTH			  84
#define BANDWIDTH_NEVER			  85

/* possible monitoring types */
#define BANDWIDTH_COMBINED 		  90
#define BANDWIDTH_INDIVIDUAL_SRC	  91
#define BANDWIDTH_INDIVIDUAL_DST 	  92
#define BANDWIDTH_INDIVIDUAL_LOCAL	  93
#define BANDWIDTH_INDIVIDUAL_REMOTE	  94



/* socket id parameters (for userspace i/o) */
#define BANDWIDTH_SET 			2048
#define BANDWIDTH_GET 			2049

/* max id length */
#define BANDWIDTH_MAX_ID_LENGTH		  50

/* 4 bytes for total number of entries, 100 entries of 12 bytes each, + 1 byte indicating whether all have been dumped */
#define BANDWIDTH_QUERY_LENGTH		1205 
#define BANDWIDTH_ENTRY_LENGTH		  12


struct ipt_bandwidth_info
{
	char id[BANDWIDTH_MAX_ID_LENGTH];
	unsigned char type;
	unsigned char check_type;
	uint32_t local_subnet;
	uint32_t local_subnet_mask;

	unsigned char cmp;
	unsigned char reset_is_constant_interval;
	time_t reset_interval; //specific fixed type (see above) or interval length in seconds
	time_t reset_time; //seconds from start of month/week/day/hour/minute to do reset, or start point of interval if it is a constant interval
	uint64_t bandwidth_cutoff;
	uint64_t current_bandwidth;
	time_t next_reset;
	time_t previous_reset;
	time_t last_backup_time;

	uint32_t num_intervals_to_save;


	unsigned long hashed_id;
	void* iam;
	uint64_t* combined_bw;
	struct ipt_bandwidth_info* non_const_self;
	unsigned long* ref_count;


};
#endif /*_IPT_BANDWIDTH_H*/
