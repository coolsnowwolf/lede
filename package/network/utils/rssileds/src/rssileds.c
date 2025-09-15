/*
 * configurable RSSI LED control daemon for OpenWrt
 *  (c) 2012 Allnet GmbH, Daniel Golle <dgolle@allnet.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * The author may be reached as dgolle@allnet.de, or
 * ALLNET GmbH
 * Maistr. 2
 * D-82110 Germering
 * Germany
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <syslog.h>

#include "iwinfo.h"

#define RUN_DIR			"/var/run"
#define LEDS_BASEPATH		"/sys/class/leds/"
#define BACKEND_RETRY_DELAY	500000

char *ifname;
int qual_max;

struct led {
	char *sysfspath;
	FILE *controlfd;
	unsigned char state;
};

typedef struct rule rule_t;
struct rule {
	struct led *led;
	int minq;
	int maxq;
	int boffset;
	int bfactor;
	rule_t *next;
};

void log_rules(rule_t *rules)
{
	rule_t *rule = rules;
	while (rule)
	{
		syslog(LOG_INFO, " %s r: %d..%d, o: %d, f: %d\n",
			rule->led->sysfspath,
			rule->minq, rule->maxq,
			rule->boffset, rule->bfactor);
		rule = rule->next;
	}
}

int set_led(struct led *led, unsigned char value)
{
	char buf[8];

	if ( ! led )
		return -1;

	if ( ! led->controlfd )
		return -1;

	if ( led->state == value )
		return 0;

	snprintf(buf, 8, "%d", value);

	rewind(led->controlfd);

	if ( ! fwrite(buf, sizeof(char), strlen(buf), led->controlfd) )
		return -2;

	fflush(led->controlfd);
	led->state=value;

	return 0;
}

int init_led(struct led **led, char *ledname)
{
	struct led *newled;
	struct stat statbuffer;
	int status;
	char *bp;
	FILE *bfp;

	bp = calloc(sizeof(char), strlen(ledname) + strlen(LEDS_BASEPATH) + 12);
	if ( ! bp )
		goto return_error;

	sprintf(bp, "%s%s/brightness", LEDS_BASEPATH, ledname);

	status = stat(bp, &statbuffer);
	if ( status )
		goto cleanup_fname;

	bfp = fopen( bp, "w" );
	if ( !bfp )
		goto cleanup_fname;

	if ( ferror(bfp) )
		goto cleanup_fp;

	/* sysfs path exists and, allocate LED struct */
	newled = calloc(sizeof(struct led),1);
	if ( !newled )
		goto cleanup_fp;

	newled->sysfspath = bp;
	newled->controlfd = bfp;

	*led = newled;

	if ( set_led(newled, 255) )
		goto cleanup_fp;

	if ( set_led(newled, 0) )
		goto cleanup_fp;

	return 0;

cleanup_fp:
	fclose(bfp);
cleanup_fname:
	free(bp);
return_error:
	syslog(LOG_CRIT, "can't open LED %s\n", ledname);
	*led = NULL;
	return -1;
}

void close_led(struct led **led)
{
	fclose((*led)->controlfd);
	free((*led)->sysfspath);
	free((*led));
	(*led)=NULL;
}


int quality(const struct iwinfo_ops *iw, const char *ifname)
{
	int qual;

	if ( ! iw ) return -1;

	if (qual_max < 1)
		if (iw->quality_max(ifname, &qual_max))
			return -1;

	if (iw->quality(ifname, &qual))
		return -1;

	return ( qual * 100 ) / qual_max ;
}

int open_backend(const struct iwinfo_ops **iw, const char *ifname)
{
	*iw = iwinfo_backend(ifname);

	if (!(*iw))
		return 1;

	return 0;
}

void update_leds(rule_t *rules, int q)
{
	rule_t *rule = rules;
	while (rule)
	{
		int b;
		/* offset and factore correction according to rule */
		b = ( q + rule->boffset ) * rule->bfactor;
		if ( b < 0 )
			b=0;
		if ( b > 255 )
			b=255;

		if ( q >= rule->minq && q <= rule->maxq )
			set_led(rule->led, (unsigned char)b);
		else
			set_led(rule->led, 0);

		rule = rule->next;
	}
}

int main(int argc, char **argv)
{
	int i,q,q0,r,s;
	const struct iwinfo_ops *iw = NULL;
	rule_t *headrule = NULL, *currentrule = NULL;

	if (argc < 9 || ( (argc-4) % 5 != 0 ) )
	{
		printf("syntax: %s (ifname) (refresh) (threshold) (rule) [rule] ...\n", argv[0]);
		printf("  rule: (sysfs-name) (minq) (maxq) (offset) (factore)\n");
		return 1;
	}

	ifname = argv[1];

	/* refresh interval */
	if ( sscanf(argv[2], "%d", &r) != 1 )
		return 1;

	/* sustain threshold */
	if ( sscanf(argv[3], "%d", &s) != 1 )
		return 1;

	openlog("rssileds", LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "monitoring %s, refresh rate %d, threshold %d\n", ifname, r, s);

	currentrule = headrule;
	for (i=4; i<argc; i=i+5) {
		if (! currentrule)
		{
			/* first element in the list */
			currentrule = calloc(sizeof(rule_t),1);
			headrule = currentrule;
		}
		else
		{
			/* follow-up element */
			currentrule->next = calloc(sizeof(rule_t),1);
			currentrule = currentrule->next;
		}

		if ( init_led(&(currentrule->led), argv[i]) )
			return 1;
		
		if ( sscanf(argv[i+1], "%d", &(currentrule->minq)) != 1 )
			return 1;

		if ( sscanf(argv[i+2], "%d", &(currentrule->maxq)) != 1 )
			return 1;
		
		if ( sscanf(argv[i+3], "%d", &(currentrule->boffset)) != 1 )
			return 1;
		
		if ( sscanf(argv[i+4], "%d", &(currentrule->bfactor)) != 1 )
			return 1;
	}
	log_rules(headrule);

	q0 = -1;
	do {
		q = quality(iw, ifname);
		if ( q < q0 - s || q > q0 + s ) {
			update_leds(headrule, q);
			q0=q;
		};
		// re-open backend...
		if ( q == -1 && q0 == -1 ) {
			if (iw) {
				iwinfo_finish();
				iw=NULL;
				usleep(BACKEND_RETRY_DELAY);
			}
			while (open_backend(&iw, ifname))
				usleep(BACKEND_RETRY_DELAY);
		}
		usleep(r);
	} while(1);

	iwinfo_finish();

	return 0;
}
