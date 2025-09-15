/* 
 * Taken from fli4l 3.0
 * Make sure you compile it against the same libpcap version used in OpenWrt
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

#include <linux/types.h>
#include <linux/ppp_defs.h>

#include <pcap.h>
#include <pcap-bpf.h>

int main (int argc, char ** argv)
{
    pcap_t  *pc; /* Fake struct pcap so we can compile expr */
    struct  bpf_program filter; /* Filter program for link-active pkts */
    u_int32_t netmask=0;

    int dflag = 3;
    if (argc == 4)
    {
	if (!strcmp (argv[1], "-d"))
	{
	    dflag = atoi (argv[2]);
	    argv += 2;
	    argc -=2;
	}
    }
    if (argc != 2)
    {
	printf ("usage; %s [ -d <debug_level> ] expression\n", argv[0]);
	return 1;
    }

    pc = pcap_open_dead(DLT_PPP_PPPD, PPP_HDRLEN);
    if (pcap_compile(pc, &filter, argv[1], 1, netmask) == 0)
    {
	printf ("#\n# Expression: %s\n#\n", argv[1]);
	bpf_dump (&filter, dflag);
	return 0;
    }
    else
    {
	printf("error in active-filter expression: %s\n", pcap_geterr(pc));
    }
    return 1;
}
