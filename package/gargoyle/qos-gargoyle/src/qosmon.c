/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*  qosmon - An active QoS monitor for gargoyle routers. 
 *           Created By Paul Bixel
 *           http://www.gargoyle-router.com
 *        
 *  Copyright © 2010 by Paul Bixel <pbix@bigfoot.com>
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
 *
*/
#define _GNU_SOURCE 1
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "utils.h"
#include "tc_util.h"
#include "tc_common.h"

#include <netdb.h>
#include <signal.h>
#include <netinet/ip_icmp.h>

#ifndef ONLYBG
#include <ncurses.h>
#endif


#define MAXPACKET   100   /* max packet size */
#define BACKGROUND  3     /* Detact and run in the background */
#define ADDENTITLEMENT 4

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  64
#endif

//The number of arguments needed for two of our kernel calls changed
//in iproute2 after v2.6.29 (not sure when).  We will use the new define
//RTNL_FAMILY_MAX to tell us that we are linking against a version of iproute2 
//after then and define dump_filter and talk accordingly.
#ifdef RTNL_FAMILY_MAX
  #define dump_filter(a,b,c) rtnl_dump_filter(a,b,c)
  #ifdef IFLA_STATS_RTA
    #define talk(a,b,c) rtnl_talk(a,b,c)
  #else
    #define talk(a,b,c,d,e) rtnl_talk(a,b,c,NULL,NULL)
  #endif
#else
#define dump_filter(a,b,c) rtnl_dump_filter(a,b,c,NULL,NULL)
#define talk(a,b,c,d,e) rtnl_talk(a,b,c,NULL,NULL,NULL,NULL)
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))

/* use_names is required when linking to tc_util.o */
bool use_names = false;


u_char  packet[MAXPACKET];
int pingflags, options;

#define DEAMON (pingflags & BACKGROUND)

struct rtnl_handle rth;

int s;              /* Socket file descriptor */
struct hostent *hp; /* Pointer to host info */
struct timezone tz; /* leftover */

struct sockaddr_in whereto;/* Who to ping */
int datalen=64-8;   /* How much data */

const char usage[] =
"Gargoyle active congestion controller version 2.4\n\n"
"Usage:  qosmon [options] pingtime pingtarget bandwidth [pinglimit]\n" 
"              pingtime   - The ping interval the monitor will use when active in ms.\n"
"              pingtarget - The URL or IP address of the target host for the monitor.\n"
"              bandwidth  - The maximum download speed the WAN link will support in kbps.\n"
"              pinglimit  - Optional pinglimit to use for control, otherwise measured.\n"
"              Options:\n"
"                     -b  - Run in the background\n"
"                     -a  - Add entitlement to pinglimt, enable auto ACTIVE/MINRTT mode switching.\n\n"
"        SIGUSR1 can be used to reset the link bandwidth at anytime.\n";

char *hostname;
char hnamebuf[MAXHOSTNAMELEN];

uint16_t ntransmitted = 0;   /* sequence # for outbound packets = #sent */
uint16_t ident;
uint16_t nreceived = 0;      /* # of packets we got back */

// For our digital filters we use Y = Y(-1) + alpha * (X - Y(-1))
// where alpha = Sample_Period / (TC + Sample_Period)

int fil_triptime;           //Filter ping times in uS 
int alpha;                  //Actually alpha * 1000
int period;                 //PING period In milliseconds
int rawfltime;              //Trip time in milliseconds
int rawfltime_max;          //The maximum measured ping time we have seen in uS.
char nopingresponse;        //Set to true when ping response is dropped.


// Struct of data we keep on our classes
struct CLASS_STATS {
   int        ID;          //Class leaf ID
   __u64      bytes;       //Work bytes last query
   u_char     rtclass;     //True if class is realtime.
   u_char     backlog;     //Number of packets waiting
   u_char     actflg;      //True if class is active.
   long int   cbw_flt;     //Class bandwidth subject to filter. (bps)
   long int   cbw_flt_rt;  //Class realtime bandwidth subject to filter. (bps)
   long       bwtime;      //Timestamp of last byte reading.
};

#define STATCNT 30
struct CLASS_STATS dnstats[STATCNT];
struct CLASS_STATS *classptr;
u_char classcnt;
u_char errorflg;
u_char firstflg=1;       //First pass flag

u_char DCA;              //Number of download classes active
u_char RTDCA;            //Number of realtime download classes active
u_char pingon=0;         //Set to one when pinger becomes active.
int    pinglimit=0;      //MinRTT mode ping time. 
int    pinglimit_cl=0;   //Ping limit entered on the commandline.
int    plimit;           //Currently enforce ping limit

float BWTC;              //Time constant of the bandwidth filter
int DBW_UL;              //This the absolute limit of the link passed in as a parameter.
int dbw_ul;              //This is the last value of the limit sent to the kernel.
int new_dbw_ul;          //The new link limit proposed by the state machine.
int saved_active_limit;  //The new link limit last known to work with active mode.
int saved_realtime_limit;//The new link limit last known to work with realtime mode.
long int dbw_fil;        //Filtered total download load (bps).

#define QMON_CHK   0
#define QMON_INIT  1
#define QMON_ACTIVE 2
#define QMON_REALTIME 3
#define QMON_IDLE  4
#define QMON_EXIT  5
char *statename[]= {"CHECK","INIT","ACTIVE","MINRTT","IDLE","DISABLED"};
unsigned char qstate=QMON_CHK;

u_short cnt_mismatch=0;
u_short cnt_errorflg=0;
u_short last_errorflg=0;

FILE *statusfd;          //Filestream for updating our status to.              
char sigterm=0;          //Set when we get a signal to terminal   
int sel_err=0;           //Last error code returned by select

#define DAEMON_NAME "qosmon"

#ifndef DEVICE
#define DEVICE "imq0"
#endif

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

/* In a world were size is everything we can avoid linking to libm
   if we can come up with replacements for rint() and ceil().  This will
   save around 64k of RAM.

   rint() is used in tc_util.c in five places.  By code inspection I can see that
   the parameter x will always be less than or equal to LONG_MAX so the following
   simplified rint() will work for us.
*/
double rint(double x)
{
   long i;
   if (x > LONG_MAX) i = LONG_MAX; else i = x+.5;
   return i;
}

/*  ceil() is used in q_hfsc.c in three places.  There I can see that x is always
    positive and less than LONG_MAX.  This leads to a much simplified routine.
*/
double ceil(double x)
{
   long i;

   if (x > LONG_MAX) x = LONG_MAX;
   i = x;
   if ((double)i != x) i++;
   return i;
}



/*
 *          F I N I S H
 *
 * Sets a global to cause the main loop to terminate.
 */
void finish(int parm)
{
    sigterm=parm;
}

/*
 *          I N _ C K S U M
 *
 * Checksum routine for Internet Protocol family headers (C Version)
 *
 */
int in_cksum(u_short *addr, int len)
{
    int nleft = len;
    u_short *w = addr;
    u_short answer;
    int sum = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) {
        u_short u = 0;

        *(u_char *)(&u) = *(u_char *)w ;
        sum += u;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}

/*
 *          P I N G E R
 * 
 * Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
void pinger(void)
{
    static u_char outpack[MAXPACKET];
    struct icmp *icp = (struct icmp *) outpack;
    int i, cc;
    struct timeval *tp = (struct timeval *) &outpack[8];
    u_char *datap = &outpack[8+sizeof(struct timeval)];

    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    icp->icmp_seq = ++ntransmitted;
    icp->icmp_id = ident;       /* ID */

    cc = datalen+8;         /* skips ICMP portion */

    gettimeofday( tp, &tz );

    for( i=8; i<datalen; i++)   /* skip 8 for time */
        *datap++ = i;

    /* Compute ICMP checksum here */
    icp->icmp_cksum = in_cksum( (u_short *) icp, cc );

    /* cc = sendto(s, msg, len, flags, to, tolen) */
    i = sendto( s, outpack, cc, 0, (const struct sockaddr *)  &whereto, sizeof(whereto) );
    
}


/*
 *          T V S U B
 * 
 * Subtract 2 timeval structs:  out = out - in.
 * 
 * Out is assumed to be >= in.
 */
void tvsub(register struct timeval *out, register struct timeval *in)
{
    if( (out->tv_usec -= in->tv_usec) < 0 )   {
        out->tv_sec--;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

/*
 *          P R _ P A C K
 *
 * Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
char pr_pack( void *buf, int cc, struct sockaddr_in *from )
{
    struct ip *ip;
    struct icmp *icp;
    struct timeval tv;
    struct timeval *tp;
    int hlen,triptime;
    struct in_addr tip;

    from->sin_addr.s_addr = ntohl( from->sin_addr.s_addr );
    gettimeofday( &tv, &tz );

    ip = (struct ip *) buf;
    hlen = ip->ip_hl << 2;
    if (cc < hlen + ICMP_MINLEN) {
        tip.s_addr = ntohl(*(uint32_t *) &from->sin_addr);
        return 0;
    }

    icp = (struct icmp *)(buf + hlen);
    if( icp->icmp_type != ICMP_ECHOREPLY )  {
        tip.s_addr = ntohl(*(uint32_t *) &from->sin_addr);
        return 0;
    }

    if( icp->icmp_id != ident )
        return 0;           /* 'Twas not our ECHO */

    nreceived++;

    //If it was not the packet we are looking for return now.
    if (icp->icmp_seq != ntransmitted) return 0;
    
    tp = (struct timeval *)&icp->icmp_data[0];
    tvsub( &tv, tp );
    triptime = tv.tv_sec*1000+(tv.tv_usec/1000);
            
    //We are now ready to update the filtered round trip time.
    //Check for some possible errors first.
    if (triptime > period) triptime = period; 

    //If this was the most recent one we sent then update the rawfltime.
    rawfltime=triptime;

    //Is this a new maximum?
    if (rawfltime > rawfltime_max/1000) rawfltime_max = rawfltime*1000;

    //return 1 if we got a valid time.
    return 1;

}

//These variables referenced but not used by the tc code we link to.
int filter_ifindex;
int use_iec = 0;
int resolve_hosts = 0;


int print_class(struct nlmsghdr *n, void *arg)
{
    struct tcmsg *t = NLMSG_DATA(n);
    int len = n->nlmsg_len;
    struct rtattr * tb[TCA_MAX+1];
    int leafid;
    u_char actflg=0;
    unsigned long long work=0;
    struct timespec newtime;

    if (n->nlmsg_type != RTM_NEWTCLASS && n->nlmsg_type != RTM_DELTCLASS) {
        fprintf(stderr, "Not a class\n");
        return 0;
    }
    len -= NLMSG_LENGTH(sizeof(*t));
    if (len < 0) {
        fprintf(stderr, "Wrong len %d\n", len);
        return -1;
    }

    memset(tb, 0, sizeof(tb));
    parse_rtattr(tb, TCA_MAX, TCA_RTA(t), len);
    clock_gettime(CLOCK_MONOTONIC,&newtime);

    if (tb[TCA_KIND] == NULL) {
        fprintf(stderr, "print_class: NULL kind\n");
        return -1;
    }

    if (n->nlmsg_type == RTM_DELTCLASS) return 0;

    //We only deal with hfsc classes.
    if (strcmp((char*)RTA_DATA(tb[TCA_KIND]),"hfsc")) return 0;
 
    //Reject the root node
    if (t->tcm_parent == TC_H_ROOT) return 0;

    //A previous error backs us out.
    if (errorflg) return 0;

    //If something has changed about the class structure or we reached the 
    //end of the array we need to reset and back out.
    if (classcnt >= STATCNT) {
       errorflg=1;
       return 0;
    }

    //Get the leafid or set to -1 if parent.
    if (t->tcm_info) leafid = t->tcm_info>>16;
     else leafid = -1;

    //If this is not the first pass and the leafid does not
    //match then the class list changed so backout.
    if ((!firstflg) && (leafid != classptr->ID) ) {
       errorflg=1;
       return 0;
    }
 
    //First time through so record the ID.
    if (firstflg) {
       classptr->ID = leafid;      
    }  
 
    //Pickup some hfsc basic stats
    if (tb[TCA_STATS2]) {

        struct tc_stats st;

        /* handle case where kernel returns more/less than we know about */
        memset(&st, 0, sizeof(st));
        memcpy(&st, RTA_DATA(tb[TCA_STATS]), MIN(RTA_PAYLOAD(tb[TCA_STATS]), sizeof(st)));
        work = st.bytes;
        classptr->backlog = st.qlen;

		/*Checkout if this class will trigger realtime mode by looking to see if either
		  the realtime or fair service curves are two part. */
		if (firstflg) {

			struct tc_service_curve *sc = NULL;
			struct rtattr *tbs[TCA_STATS_MAX + 1];

			classptr->rtclass=0;
			parse_rtattr_nested(tbs, TCA_HFSC_MAX, tb[TCA_OPTIONS]);
			if (tbs[TCA_HFSC_RSC] && (RTA_PAYLOAD(tbs[TCA_HFSC_RSC]) >= sizeof(*sc))) {
				sc = RTA_DATA(tbs[TCA_HFSC_RSC]);
				classptr->rtclass |= (sc && sc->m1);
    	    }

			if (tbs[TCA_HFSC_FSC] && (RTA_PAYLOAD(tbs[TCA_HFSC_FSC]) >= sizeof(*sc))) {
				sc = RTA_DATA(tbs[TCA_HFSC_FSC]);
				classptr->rtclass |= (sc && sc->m1);
    	    }

		}

    } else {
        errorflg=1;
        return 0;
    }

         
    //Avoid a big jolt on the first pass.
    if (firstflg) {
		classptr->bytes = work;
	}

    //Update the filtered bandwidth based on what happened unless a rollover occured.
    if (work >= classptr->bytes) {
        long int bw;
        long bperiod;

        //Calculate an accurate time period for the bps calculation.
        bperiod=(newtime.tv_nsec-classptr->bwtime)/1000000;
        if (bperiod<period/2) bperiod=period;
        bw = (work - classptr->bytes)*8000/bperiod;  //bps per second x 1000 here

        //Convert back to bps as part of the filter calculation 
        classptr->cbw_flt=(bw-classptr->cbw_flt)*BWTC/1000+classptr->cbw_flt;

        //A class is considered active if its BW exceeds 4000bps 
        if ((leafid != -1) && (classptr->cbw_flt > 4000)) {
            DCA++;actflg=1;
            if (classptr->rtclass) RTDCA++;
        }

        //Calculate the total link load by adding up all the classes.
        if (leafid == -1) {
            dbw_fil = 0;
        } else {
            dbw_fil += classptr->cbw_flt;
        } 

    }

    classptr->bwtime=newtime.tv_nsec;
    classptr->bytes = work;
    classptr->actflg = actflg;

    classptr++;
    classcnt++;
    return 0;
}

/*Gather stats for classes attached to device d */
int class_list(char *d)
{
    struct tcmsg t;

    RTDCA=DCA =0;
    memset(&t, 0, sizeof(t));
    t.tcm_family = AF_UNSPEC;

    ll_init_map(&rth);

    if (d[0]) {
        if ((t.tcm_ifindex = ll_name_to_index(d)) == 0) {
            fprintf(stderr, "Cannot find device \"%s\"\n", d);
            return 1;
        }
        filter_ifindex = t.tcm_ifindex;
    }

    if (rtnl_dump_request(&rth, RTM_GETTCLASS, &t, sizeof(t)) < 0) {
        perror("Cannot send dump request");
        return 1;
    }

    if (dump_filter(&rth, print_class, stdout) < 0) {
        fprintf(stderr, "Dump terminated\n");
        return 1;
    }

    return 0;
}


/*
 *       tc_class_modify
 *
 * This function changes the upper limit rate of the 'DEVICE' class to match
 * the rate passed in as the sole parameter.  This is the throttle means
 * we will use to maintian the QoS performance as the link becomes saturated.
 *
 * The structure of this code is gleaned from the source code of 'tc' and is
 * specific the the gargoyle QoS design.
 */
int tc_class_modify(__u32 rate)
{
    struct {
        struct nlmsghdr     n;
        struct tcmsg        t;
        char            buf[4096];
    } req;

    char  k[16];
    __u32 handle;

    if (dbw_ul == rate) return 0;
    dbw_ul=rate;

    memset(&req, 0, sizeof(req));
    memset(k, 0, sizeof(k));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_NEWTCLASS;
    req.t.tcm_family = AF_UNSPEC;

    //We are only going to modify the upper limit rate of the parent class.
    if (get_tc_classid(&handle, "1:1")) {
         fprintf(stderr,"invalid class ID");
         return 1;
    }
    req.t.tcm_handle = handle;

    if (get_tc_classid(&handle, "1:0")) {
        fprintf(stderr,"invalid parent ID");
        return 1;
    }
    req.t.tcm_parent = handle;
     
    strcpy(k,"hsfc");
    addattr_l(&req.n, sizeof(req), TCA_KIND, k, strlen(k)+1);

    {
        struct tc_service_curve usc;
        struct rtattr *tail;

        memset(&usc, 0, sizeof(usc));

        usc.m2 = rate/8;

        tail = NLMSG_TAIL(&req.n);

        addattr_l(&req.n, 1024, TCA_OPTIONS, NULL, 0);
        addattr_l(&req.n, 1024, TCA_HFSC_USC, &usc, sizeof(usc));

        tail->rta_len = (void *) NLMSG_TAIL(&req.n) - (void *) tail;
    }


    //Communicate our change to the kernel.
    ll_init_map(&rth);

    if ((req.t.tcm_ifindex = ll_name_to_index(DEVICE)) == 0) {
            fprintf(stderr, "Cannot find device %s\n",DEVICE);
            return 1;
    }


    if (talk(&rth, &req.n, NULL) < 0)
        return 2;

    return 0;
}

/*
    This function is periodically called and updates the
    status file for the deamon.  The status file can then
    be viewed by other processes to tell what is going on.
*/
void update_status( FILE* fd )
{

    struct CLASS_STATS *cptr=dnstats;
    u_char i;
    char nstr[10];
    int dbw;

    //Link load includes the ping traffic when the pinger is on.
    if (pingon) dbw = dbw_fil + 64 * 8 * 1000/period;
           else dbw = dbw_fil; 

    //Update the status file.
    rewind(fd);
    fprintf(fd,"State: %s\n",statename[qstate]);
    fprintf(fd,"Link limit: %d (kbps)\n",dbw_ul/1000);
    fprintf(fd,"Fair Link limit: %d (kbps)\n",new_dbw_ul/1000);
    fprintf(fd,"Link load: %d (kbps)\n",dbw/1000);

    if (pingon) {
        if (nopingresponse) fprintf(fd,"Ping: Dropped, assume %d mS\n",rawfltime_max/1000);
        else fprintf(fd,"Ping: %d (ms)\n",rawfltime);
    }
    else
        fprintf(fd,"Ping: off\n");

    fprintf(fd,"Filtered/Max recent RTT: %d/%d (ms)\n",fil_triptime/1000,rawfltime_max/1000);
    fprintf(fd,"RTT time limit: %d (ms) [%d/%d]\n",plimit/1000,pinglimit/1000,(pinglimit+135*pinglimit_cl/100)/1000);
    fprintf(fd,"Classes Active: %u\n",DCA);

    fprintf(fd,"Errors: (mismatch,errors,last err,selerr): %u,%u,%u,%i\n", cnt_mismatch, cnt_errorflg,last_errorflg,sel_err); 
	

    i=0;
    while ((i++<STATCNT) && (cptr->ID != 0)) {
        fprintf(fd,"ID %4X, Active %u, Backlog %u, BW bps (filtered): %ld\n",
              (short unsigned) cptr->ID,
              cptr->actflg,
              cptr->backlog,
              cptr->cbw_flt);
        cptr++;
    }

    fflush(fd);

#ifndef ONLYBG
    if (DEAMON) return;

    //Home the cursor
    mvprintw(0,0,"");
    printw("\nqosmon status\n");

    if (pingon) {
        sprintf(nstr,"%d",rawfltime);
    } else {
        strcpy(nstr,"*");
    }

    printw("ping (%s/%d) DCA=%d, RTDCA=%d, plim=%d, plim2=%d, state=%s\n",nstr,fil_triptime/1000,
		DCA,RTDCA,pinglimit/1000,plimit/1000,statename[qstate]);
    printw("Link Limit=%6d, Fair Limit=%6d, Current Load=%6d (kbps)\n", 
		dbw_ul/1000,new_dbw_ul/1000,dbw/1000);
    printw("Saved Active Limit=%6d, Saved Realtime Limit=%6d\n",saved_active_limit/1000,saved_realtime_limit/1000);
    printw("pings sent=%d, pings received=%d\n", 
		ntransmitted,nreceived);

    printw("Defined classes for %s\n",DEVICE); 
    printw("Errors: (mismatches,errors,last err,selerr): %u,%u,%u,%i\n", cnt_mismatch, cnt_errorflg,last_errorflg,sel_err); 
    cptr=dnstats;
    i=0; 
    while ((i++<STATCNT) && (cptr->ID != 0)) {
        printw("ID %4X, Active %u, Realtime %u. Backlog %u, BW (filtered kbps): %ld\n",
              (short unsigned) cptr->ID,
              cptr->actflg,
			  cptr->rtclass,
              cptr->backlog,
              cptr->cbw_flt/1000);
        cptr++;
    }

    refresh();
#endif

}

/* v2.3 feature allows reseting the link limit to the initial value by sending the process SIGUSR1. */
sig_atomic_t resetbw=1;
void resetsig(int parm)
{
    resetbw=1;
}


/*
 *          M A I N
 */
int main(int argc, char *argv[])
{
    struct sockaddr_in from;
    char **av = argv;
    struct sockaddr_in *to = &whereto;
    int on = 1;
    struct protoent *proto;
    float err;


    argc--, av++;
    while (argc > 0 && *av[0] == '-') {
        while (*++av[0]) switch (*av[0]) {
            case 'b':
                pingflags |= BACKGROUND;
                break;

            case 'a':
                pingflags |= ADDENTITLEMENT;
                break;
        }
        argc--, av++;
    }
    if ((argc < 3) || (argc >4))  {
        printf(usage);
        exit(1);
    }

#ifdef ONLYBG
    if (!(pingflags & BACKGROUND)) {
        fprintf(stderr, "Must use the -b switch\n");
        exit(1);
    }
#endif

    //The first parameter is the ping time in ms.
    period = atoi( av[0] );
    if ((period > 2000) || (period < 100)) {
        fprintf(stderr, "Invalid ping interval '%s'\n", av[0]);
        exit(1);
    }

    bzero((char *)&whereto, sizeof(whereto) );
    to->sin_family = AF_INET;
    to->sin_addr.s_addr = inet_addr(av[1]);
    if(to->sin_addr.s_addr != (unsigned)-1) {
        strcpy(hnamebuf, av[1]);
        hostname = hnamebuf;
    } else {
        hp = gethostbyname(av[1]);
        if (hp) {
            to->sin_family = hp->h_addrtype;
            bcopy(hp->h_addr, (caddr_t)&to->sin_addr, hp->h_length);
            hostname = hp->h_name;
        } else {
            fprintf(stderr, "%s: unknown host %s\n", argv[1], av[1]);
            exit(1);
        }
    }

    //The third parameter is the maximum download speed in kbps.
    DBW_UL = atoi( av[2] );
    if ((DBW_UL < 100) || (DBW_UL >= INT_MAX/1000)) {
        fprintf(stderr, "Invalid download bandwidth '%s'\n", av[2]);
        exit(1);
    }

    //Convert kbps to bps.
    dbw_ul = DBW_UL = DBW_UL*1000;

    //The fourth optional parameter is the ping limit in ms.
    if (argc == 4) {
        pinglimit_cl = pinglimit = atoi( av[3] )*1000;
    }


    ident = getpid() & 0xFFFF;

    if ((proto = getprotobyname("icmp")) == NULL) {
        fprintf(stderr, "icmp: unknown protocol\n");
        exit(10);
    }

    // where alpha = Sample_Period / (TC + Sample_Period)
    // TC needs to be not less than 3 times the sample period
    alpha = (period*1000. / (period*4 + period)); 

    //Class bandwidth filter time constants  
    BWTC= (period*1000. / (7500. + period));

    //Check that we have access to tc functions.
    tc_core_init();
    if (rtnl_open(&rth, 0) < 0) {
        fprintf(stderr, "Cannot open rtnetlink\n");
        exit(1);
    }

    //Make sure the device is present and that we can scan it.
    classptr=dnstats;
    errorflg=0;       
    class_list(DEVICE);
    if (errorflg) {
        fprintf(stderr, "Cannot scan ingress device %s\n",DEVICE);
        exit(1);
    }

   //If running in the background fork()
    if (DEAMON) {

        /* Ignore most signals in background */  
        signal( SIGINT,  SIG_IGN );
        signal( SIGQUIT, SIG_IGN );
        signal( SIGCHLD, SIG_IGN );
        signal( SIGALRM, SIG_IGN );
        signal( SIGUSR1, SIG_IGN );
        signal( SIGUSR2, SIG_IGN );
        signal( SIGHUP,  SIG_IGN );
        signal( SIGTSTP, SIG_IGN );
        signal( SIGPIPE, (sighandler_t) finish );
        signal( SIGSEGV, (sighandler_t) finish );
        signal( SIGILL, (sighandler_t) finish );
        signal( SIGFPE, (sighandler_t) finish );
        signal( SIGSYS, (sighandler_t) finish );
        signal( SIGURG, (sighandler_t) finish );
        signal( SIGTTIN, (sighandler_t) finish );
        signal( SIGTTOU, (sighandler_t) finish );

    	//daemonize();
        if ( daemon( 0, 0) < 0 )
	   	{
            fprintf(stderr,"deamon() failed with %i\n",errno);
            exit( 1 );
	    }

        /* Initialize the logging interface */
        openlog( DAEMON_NAME, LOG_PID, LOG_LOCAL5 );
    }

    //SIGTERM is what we expect to kill us.
    signal( SIGTERM, (sighandler_t) finish );

    //SIGUSR1 resets the link speed.
    signal( SIGUSR1, (sighandler_t) resetsig );

    //Create the status file and ping socket
    //These are called here because the above daemon() call closes
    //open files.
    statusfd = fopen("/tmp/qosmon.status","w");
    s = socket(AF_INET, SOCK_RAW, proto->p_proto);


    //Check that things opened correctly.
    if (DEAMON) {
        if (statusfd == NULL) {
            syslog( LOG_CRIT, "Cannot open /tmp/qosmon.status - %i",errno );
            exit(EXIT_FAILURE);
        }
  
        if (s < 0) {
            syslog( LOG_CRIT, "Cannot open ping socket - %i",errno );
            exit(EXIT_FAILURE);
        }

        syslog(LOG_INFO, "starting socketfd = %i, statusfd = %i",s,fileno(statusfd));
    }

#ifndef ONLYBG
    else {
        if (statusfd == NULL) {
	        fprintf(stderr, "Cannot open /tmp/qosmon.status - %i",errno );
            exit(EXIT_FAILURE);
        }
  
        if (s < 0) {
	        fprintf( stderr, "Cannot open ping socket - %i",errno );
            exit(EXIT_FAILURE);
        }

        //Ctrl-C terminates       
        signal( SIGINT, (sighandler_t) finish );

        //Close terminal terminates       
        signal( SIGHUP, (sighandler_t) finish );

        setlinebuf( stdout );

        //Bring up ncurses
        initscr();

    }
#endif

    //Clear all initial stats.
    memset((void *)&dnstats,0,sizeof(dnstats));

    //Initialize the max ping to something reasonable.
    //We will fix it later.
    rawfltime_max = period*1000;

    while (!sigterm) {
        int len = sizeof (packet);
        socklen_t fromlen = sizeof (from);
        int cc;
        u_char chill;
        struct timeval timeout;
        fd_set fdmask;
        FD_ZERO(&fdmask);
        FD_SET(s , &fdmask);

        //rawfltime variable will be set in pr_pack() if we get a pong that matched
        //our ping, but clearing it here will let us know we did not get a response to our
        //ping.
        rawfltime=0;

        //Send the next ping
        if (pingon) pinger();
        
        //Wait for the pong(s).
        timeout.tv_sec = 0;
        timeout.tv_usec = period*1000;
    
        //Need a loop here to clean out any old pongs that show up.
        //select() returns 1 if there is data to read.
        //                 0 if the time has expired.
        //                -1 if a signal arrived. 
        while  (sel_err=select(s+1, &fdmask, NULL, NULL, &timeout)) {

              //Signal arrived, just loop and keep waiting.
              if (sel_err == -1) continue;
        
              //If we got here then data must be waiting, try to read the whole packet
              if ( (cc=recvfrom(s,packet,len,0,(struct sockaddr *) &from, &fromlen)) < 0) {
                  continue;
              }
              
              //OK there is a whole packet, get it and record the triptime. 
              pr_pack( packet, cc, &from );      

        }

        //Gather new statistics
        classptr=dnstats;
        cc=classcnt;
        classcnt=0;
        errorflg=0;       
        class_list(DEVICE);

        //If there was an error or the number of classes changed then reset everything
        if (errorflg || (!firstflg && (cc !=classcnt))) {

            if (errorflg) {cnt_errorflg++; last_errorflg=errorflg;}
              else if (cc != classcnt) cnt_mismatch++;

            firstflg=1;
            pingon=0;
            qstate=QMON_CHK; 
            continue;
        }

 
        //Initialize or reinitialize the fair linklimit.
        if (resetbw) {
           saved_realtime_limit=saved_active_limit=new_dbw_ul= DBW_UL * .9;
           resetbw=0;
        }

        //Look at an ping response time we got.  If we did not get any then it most likely
        //got dropped so use the maximum value that we have recently seen as we know the downlink
        //queue must be at least this long.
        if (!rawfltime) {
           rawfltime = rawfltime_max/1000;
           nopingresponse=1;
        } else
           nopingresponse=0;

        //Update the filtered ping response time based on what happened.
        //If we are not pinging then no change in the filtered value.
        if (pingon) 
           fil_triptime = ((rawfltime*1000 - fil_triptime)*alpha)/1000 + fil_triptime;

        //Run the state machine
        switch (qstate) {

            // Wait to see if the ping targer will respond at all before doing anything
            case QMON_CHK: 
                pingon=1;

                //If we get two pings go ahead and lower the link speed.
                if (nreceived >= 2) {

                    //If the pinglimit was entered on the command line 
                    //without the add flag then go directly to the 
                    //IDLE state otherwise automatically determine an appropriate 
                    //ping limit.
                    if ((pinglimit) && !(pingflags & ADDENTITLEMENT)) {
                        dbw_ul=0;                  //Forces an update in tc_class_modify()
                        tc_class_modify(new_dbw_ul); 
                        fil_triptime = rawfltime*1000;
                        qstate=QMON_IDLE;
                     } else {
                        tc_class_modify(1000);  //Unload the link for the measurement.
                        nreceived=0;
                        qstate=QMON_INIT;
                     }
                } 
                break; 

            // Take a measurement of the practical ping time we can expect in an unsaturated
            // link.  We do this by making pings and using the filter response after
            // throttling all traffic in the link.
            case QMON_INIT:
                //Filter starts at ten seconds and runs until 15 seconds.
                //For the first ten seconds we initialize the filter to the last ping time we saw.
                //After the seventh second we start filtering.
                if (nreceived < (10000/period)+1) fil_triptime = rawfltime*1000;

                //After 15 seconds we have measured our ping response entitlement.
                //Move on to the active state. 
                if (nreceived > (15000/period)+1) {
                    qstate=QMON_IDLE;
                    tc_class_modify(new_dbw_ul);  //Restore reasonable bandwidth

                    //If the user specified no limit then the RTT ping limit is computed from what was
                    //entered on the command line.
                    if (pingflags & ADDENTITLEMENT) {
                        //Add what the user specified to the 110% of the measure ping time.
                        pinglimit += (fil_triptime*1.1);
                    } else {
                        //Without the '-a' flag we just use 200% of measure ping time.  
                        //This works OK in my system but I have no evidence that it will work in other systems.
                        pinglimit = fil_triptime*2.0;
                    }

                    //Sanity Checks
                    if (pinglimit < 10000) pinglimit=10000;
                    if (pinglimit > 800000) pinglimit=800000;

                    //Reasonable max ping. 
                    rawfltime_max = 2*pinglimit;
                }
                break;

            // In the idle state we have a nearly idle link.
            // In these cases it is not necessary to monitor delay times so the active
            // ping is disabled.
            case QMON_IDLE:
                pingon=0;

            //Add a hysterisis band when going in/out of IDLE mode.
            //to try and prevent getting stuck in IDLE mode at the edge of the dynamic range
            //
            //We exit idle mode when the link gets above 12% of the upper limit.
            //We enter idle mode when we get below 10%. (2% hysterisis band).
            //With this setup at 15% it would be possible to get stuck here since the dynamic limit
            //can fall as low as 15% which would mean we might not be able to get above 15% to restart the ACTIVE mode.
            //Hopefully we will always be able to get above 12% at least.
            if (dbw_fil < 0.12 * DBW_UL) break;

            // In the ACTIVE & REALTIME states we observe ping times as long as the
            // link remains active.  While we are observing we adjust the 
            // link upper limit speed to maintain the specified pinglimit.
            // If the amount of data we are recieving dies down we enter the WAIT state
            case QMON_ACTIVE:
            case QMON_REALTIME:
                pingon=1;

                //Save the bandwidth limit for each mode.
                if (qstate == QMON_REALTIME) saved_realtime_limit = new_dbw_ul;
                if (qstate == QMON_ACTIVE) saved_active_limit = new_dbw_ul;

                //The pinglimit we will use depends on if any realtime classes are active
                //or not.  In realtime mode we only allow 'pinglimit' round trip times which
                //makes our pings low but also lowers our throughput.  The automatic measurement 
                //above set pinglimit to the average RTT of the ping assuming it has to wait on
                //average for 2/3 of an single MTU sized packet to transmit.  The means on 
                //average there is nothing in the buffer but a packet is transmitting.

                //When not in realtime mode the stradegy is that we allow enough packets in the queue
                //to fully utilize the downlink.

                //We are talking about a queue controlled by the ISP so we don't know much about it.
                //We make an assumption that the queue is long enough to allow full utilization of the link.
                //This should be the case and often the queue is much longer than needed (bufferbloat).  
                //When not in realtime mode we can allow this buffer to fill but we don't want it to overflow 
                //because it will then drop packets which will cause our QoS to breakdown.  So we want it to fill
                //just enough to promote full link utilization.

                //The classical optimum queue size would be equal to the bandwidth * RTT and the 
                //additional time it will take our ping to pass through such a queue turns out to be the RTT. 
                //But Barman et all, Globecomm2004 indicates that only 20-30% of this is really needed.  
                //
                //When we measured an RTT above that it was to the ISPs gateway so we do not really know what the average 
                //RTT time to other IPs on the internet.  And since not all hosts respond the same anyway I doubt there
                //is consistant RTT that we could use.
				//	
                //For ACTIVE mode on a 925kbps/450kbps link I measured the following 
                //relationship between ping limit and throughput with large packets downloading.
                //
                //Ping Limit   Throughput   Percent
                // 612ms       918kbps      100 
                // 525ms       915kbps      99.6
                // 437ms       898kbps      97.8
                // 350ms       875kbps      95.3 
                // 262ms       862kbps      93.8 
                //  81ms       870kbps      94.7
                //  60ms       680kbps      69.8
                //  50ms       630kbps      68.6
                //  40ms       490kbps      53.3      
                //
                //The 1500 byte packet time is 1500*10/925kbps download and 1500*10/425kbps upload for a total
                //RTT of around 48ms.  Idle ping times on this link are around 35ms.
                //
                //These results indicate that on my link not much is gained by increasing beyond 81ms.  This is pretty much
                //the MINRTT mode computed with the -a switch.  Still other links may be different so I suspect that
                //switching to active mode will benefit some people.
                //
                //The statedgy I will use for the ACTIVE mode limit will be to add an additional 135% packet delay over
                //what we have in RTT mode.  The packet delay was entered on the command line or zero if nothing was entered.

                //I hope that this will work well for a broad range of users from satellite links with RTTs of 1 second or more
                //to users with hot connections that have small queues upstream of them.

                if ((RTDCA == 0) && (pingflags & ADDENTITLEMENT)) {
                    plimit=135*pinglimit_cl/100+pinglimit;

                    //When switching into active mode for the first time initialize the bandwidth
                    //limit to the last value that was known to work.
                    if (qstate != QMON_ACTIVE) {
                        qstate=QMON_ACTIVE;
                        new_dbw_ul=saved_active_limit;
                        tc_class_modify(new_dbw_ul);
                    }

                } else {
                    plimit = pinglimit;

                    //When switching into realtime mode for the first time initialize the bandwidth
                    //limit to the last value that was known to work.
                    if (qstate != QMON_REALTIME) {
                        qstate=QMON_REALTIME;
                        new_dbw_ul=saved_realtime_limit;
                        tc_class_modify(new_dbw_ul);
                    }

                }

                //When the downlink falls below 10% utilization we turn off the pinger.
                if (dbw_fil < 0.1 * DBW_UL) qstate=QMON_IDLE;

                //Compute the ping error
                err = fil_triptime - plimit;

                //Negative error means we might be able to increase the link limit.
                if (err < 0) {

                   //Do not increase the bandwidth until we reach 85% of the current limit.
                   if  (dbw_fil < dbw_ul * 0.85) break;

                   //Increase slowly (0.4%/sec).  err is negative here.  
                   new_dbw_ul = new_dbw_ul * (1.0 - 0.004*err*(float)period/(float)plimit/1000.0);
                   if (new_dbw_ul > DBW_UL) new_dbw_ul=DBW_UL;

                } else {
                //Positive error means we need to decrease the bandwidth.

                   new_dbw_ul = new_dbw_ul * (1.0 - 0.004*err*(float)period/(float)plimit/1000.0);

                   //Dynamic range is 1/.15 or 6.67 : 1.  
                   if (new_dbw_ul < DBW_UL*.15) new_dbw_ul=DBW_UL*.15;
                }   

                //Modify parent download limit as needed.
                tc_class_modify(new_dbw_ul);

                //Keep downward pressure on rawfltime_max to keep it fresh.
                if (rawfltime_max > plimit) rawfltime_max -= 100;

                break;
                    
                      
        }

        update_status(statusfd);

        //If we get here the first pass is over. 
        firstflg=0;
 
    }  //Next ping


    qstate=QMON_EXIT;

    //We got a signal to terminate so start by restoring the root TC class to
    //the original upper limit.
    tc_class_modify(DBW_UL);
    
    update_status(statusfd);

    //Write a message in the system log
    if (DEAMON) {
      syslog( LOG_NOTICE, "terminated sigterm=%i, sel_err=%i", sigterm, sel_err );
      closelog();
    } 

#ifndef ONLYBG
    else { 
      endwin();
      fflush(stdout);
    }
#endif

}

