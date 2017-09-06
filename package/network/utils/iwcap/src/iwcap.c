/*
 * iwcap.c - A simply radiotap capture utility outputting pcap dumps
 *
 *    Copyright 2012 Jo-Philipp Wich <jo@mein.io>
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <byteswap.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_packet.h>

#define ARPHRD_IEEE80211_RADIOTAP	803

#define DLT_IEEE802_11_RADIO		127
#define LEN_IEEE802_11_HDR			32

#define FRAMETYPE_MASK				0xFC
#define FRAMETYPE_BEACON			0x80
#define FRAMETYPE_DATA				0x08

#if __BYTE_ORDER == __BIG_ENDIAN
#define le16(x) __bswap_16(x)
#else
#define le16(x) (x)
#endif

uint8_t run_dump   = 0;
uint8_t run_stop   = 0;
uint8_t run_daemon = 0;

uint32_t frames_captured = 0;
uint32_t frames_filtered = 0;

int capture_sock = -1;
const char *ifname = NULL;


struct ringbuf {
	uint32_t len;            /* number of slots */
	uint32_t fill;           /* last used slot */
	uint32_t slen;           /* slot size */
	void *buf;               /* ring memory */
};

struct ringbuf_entry {
	uint32_t len;            /* used slot memory */
	uint32_t olen;           /* original data size */
	uint32_t sec;            /* epoch of slot creation */
	uint32_t usec;			 /* epoch microseconds */
};

typedef struct pcap_hdr_s {
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	int32_t  thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

typedef struct ieee80211_radiotap_header {
	u_int8_t  it_version;    /* set to 0 */
	u_int8_t  it_pad;
	u_int16_t it_len;        /* entire length */
	u_int32_t it_present;    /* fields present */
} __attribute__((__packed__)) radiotap_hdr_t;


int check_type(void)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(capture_sock, SIOCGIFHWADDR, &ifr) < 0)
		return -1;

	return (ifr.ifr_hwaddr.sa_family == ARPHRD_IEEE80211_RADIOTAP);
}

int set_promisc(int on)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	if (ioctl(capture_sock, SIOCGIFFLAGS, &ifr) < 0)
		return -1;

	if (on && !(ifr.ifr_flags & IFF_PROMISC))
	{
		ifr.ifr_flags |= IFF_PROMISC;

		if (ioctl(capture_sock, SIOCSIFFLAGS, &ifr))
			return -1;

		return 1;
	}
	else if (!on && (ifr.ifr_flags & IFF_PROMISC))
	{
		ifr.ifr_flags &= ~IFF_PROMISC;

		if (ioctl(capture_sock, SIOCSIFFLAGS, &ifr))
			return -1;

		return 1;
	}

	return 0;
}


void sig_dump(int sig)
{
	run_dump = 1;
}

void sig_teardown(int sig)
{
	run_stop = 1;
}


void write_pcap_header(FILE *o)
{
	pcap_hdr_t ghdr = {
		.magic_number  = 0xa1b2c3d4,
		.version_major = 2,
		.version_minor = 4,
		.thiszone      = 0,
		.sigfigs       = 0,
		.snaplen       = 0xFFFF,
		.network       = DLT_IEEE802_11_RADIO
	};

	fwrite(&ghdr, 1, sizeof(ghdr), o);
}

void write_pcap_frame(FILE *o, uint32_t *sec, uint32_t *usec,
					  uint16_t len, uint16_t olen)
{
	struct timeval tv;
	pcaprec_hdr_t fhdr;

	if (!sec || !usec)
	{
		gettimeofday(&tv, NULL);
	}
	else
	{
		tv.tv_sec  = *sec;
		tv.tv_usec = *usec;
	}

	fhdr.ts_sec   = tv.tv_sec;
	fhdr.ts_usec  = tv.tv_usec;
	fhdr.incl_len = len;
	fhdr.orig_len = olen;

	fwrite(&fhdr, 1, sizeof(fhdr), o);
}


struct ringbuf * ringbuf_init(uint32_t num_item, uint16_t len_item)
{
	static struct ringbuf r;

	if (len_item <= 0)
		return NULL;

	r.buf = malloc(num_item * (len_item + sizeof(struct ringbuf_entry)));

	if (r.buf)
	{
		r.len = num_item;
		r.fill = 0;
		r.slen = (len_item + sizeof(struct ringbuf_entry));

		memset(r.buf, 0, num_item * len_item);

		return &r;
	}

	return NULL;
}

struct ringbuf_entry * ringbuf_add(struct ringbuf *r)
{
	struct timeval t;
	struct ringbuf_entry *e;

	gettimeofday(&t, NULL);

	e = r->buf + (r->fill++ * r->slen);
	r->fill %= r->len;

	memset(e, 0, r->slen);

	e->sec = t.tv_sec;
	e->usec = t.tv_usec;

	return e;
}

struct ringbuf_entry * ringbuf_get(struct ringbuf *r, int i)
{
	struct ringbuf_entry *e = r->buf + (((r->fill + i) % r->len) * r->slen);

	if (e->len > 0)
		return e;

	return NULL;
}

void ringbuf_free(struct ringbuf *r)
{
	free(r->buf);
	memset(r, 0, sizeof(*r));
}


void msg(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	if (run_daemon)
		vsyslog(LOG_INFO | LOG_USER, fmt, ap);
	else
		vfprintf(stderr, fmt, ap);

	va_end(ap);
}


int main(int argc, char **argv)
{
	int i, n;
	struct ringbuf *ring;
	struct ringbuf_entry *e;
	struct sockaddr_ll local = {
		.sll_family   = AF_PACKET,
		.sll_protocol = htons(ETH_P_ALL)
	};

	radiotap_hdr_t *rhdr;

	uint8_t frametype;
	uint8_t pktbuf[0xFFFF];
	ssize_t pktlen;

	FILE *o;

	int opt;

	uint8_t promisc        = 0;
	uint8_t streaming      = 0;
	uint8_t foreground     = 0;
	uint8_t filter_data    = 0;
	uint8_t filter_beacon  = 0;
	uint8_t header_written = 0;

	uint32_t ringsz   = 1024 * 1024; /* 1 Mbyte ring buffer */
	uint16_t pktcap   = 256;		 /* truncate frames after 265KB */

	const char *output = NULL;


	while ((opt = getopt(argc, argv, "i:r:c:o:sfhBD")) != -1)
	{
		switch (opt)
		{
		case 'i':
			ifname = optarg;
			if (!(local.sll_ifindex = if_nametoindex(ifname)))
			{
				msg("Unknown interface '%s'\n", ifname);
				return 2;
			}
			break;

		case 'r':
			ringsz = atoi(optarg);
			if (ringsz < (3 * pktcap))
			{
				msg("Ring size of %d bytes is too short, "
					"must be at least %d bytes\n", ringsz, 3 * pktcap);
				return 3;
			}
			break;

		case 'c':
			pktcap = atoi(optarg);
			if (pktcap <= (sizeof(radiotap_hdr_t) + LEN_IEEE802_11_HDR))
			{
				msg("Packet truncate after %d bytes is too short, "
					"must be at least %d bytes\n",
					pktcap, sizeof(radiotap_hdr_t) + LEN_IEEE802_11_HDR);
				return 4;
			}
			break;

		case 's':
			streaming = 1;
			break;

		case 'o':
			output = optarg;
			break;

		case 'B':
			filter_beacon = 1;
			break;

		case 'D':
			filter_data = 1;
			break;

		case 'f':
			foreground = 1;
			break;

		case 'h':
			msg(
				"Usage:\n"
				"  %s -i {iface} -s [-b] [-d]\n"
				"  %s -i {iface} -o {file} [-r len] [-c len] [-B] [-D] [-f]\n"
				"\n"
				"  -i iface\n"
				"    Specify interface to use, must be in monitor mode and\n"
				"    produce IEEE 802.11 Radiotap headers.\n\n"
				"  -s\n"
				"    Stream to stdout instead of Dumping to file on USR1.\n\n"
				"  -o file\n"
				"    Write current ringbuffer contents to given output file\n"
				"    on receipt of SIGUSR1.\n\n"
				"  -r len\n"
				"    Specify the amount of bytes to use for the ringbuffer.\n"
				"    The default length is %d bytes.\n\n"
				"  -c len\n"
				"    Truncate captured packets after given amount of bytes.\n"
				"    The default size limit is %d bytes.\n\n"
				"  -B\n"
				"    Don't store beacon frames in ring, default is keep.\n\n"
				"  -D\n"
				"    Don't store data frames in ring, default is keep.\n\n"
				"  -f\n"
				"    Do not daemonize but keep running in foreground.\n\n"
				"  -h\n"
				"    Display this help.\n\n",
				argv[0], argv[0], ringsz, pktcap);

			return 1;
		}
	}

	if (!streaming && !output)
	{
		msg("No output file specified\n");
		return 1;
	}

	if (streaming && output)
	{
		msg("The -s and -o options are exclusive\n");
		return 1;
	}

	if (streaming && isatty(1))
	{
		msg("Refusing to stream into a terminal\n");
		return 1;
	}

	if (!local.sll_ifindex)
	{
		msg("No interface specified\n");
		return 2;
	}

	if (!check_type())
	{
		msg("Bad interface: not ARPHRD_IEEE80211_RADIOTAP\n");
		return 2;
	}

	if ((capture_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		msg("Unable to create raw socket: %s\n",
				strerror(errno));
		return 6;
	}

	if (bind(capture_sock, (struct sockaddr *)&local, sizeof(local)) == -1)
	{
		msg("Unable to bind to interface: %s\n",
			strerror(errno));
		return 7;
	}

	if (!streaming)
	{
		if (!foreground)
		{
			switch (fork())
			{
				case -1:
					msg("Unable to fork: %s\n", strerror(errno));
					return 8;

				case 0:
					umask(0077);
					chdir("/");
					freopen("/dev/null", "r", stdin);
					freopen("/dev/null", "w", stdout);
					freopen("/dev/null", "w", stderr);
					run_daemon = 1;
					break;

				default:
					msg("Daemon launched ...\n");
					return 0;
			}
		}

		msg("Monitoring interface %s ...\n", ifname);

		if (!(ring = ringbuf_init(ringsz / pktcap, pktcap)))
		{
			msg("Unable to allocate ring buffer: %s\n",
				strerror(errno));
			return 5;
		}

		msg(" * Using %d bytes ringbuffer with %d slots\n", ringsz, ring->len);
		msg(" * Truncating frames at %d bytes\n", pktcap);
		msg(" * Dumping data to file %s\n", output);

		signal(SIGUSR1, sig_dump);
	}
	else
	{
		msg("Monitoring interface %s ...\n", ifname);
		msg(" * Streaming data to stdout\n");
	}

	msg(" * Beacon frames are %sfiltered\n", filter_beacon ? "" : "not ");
	msg(" * Data frames are %sfiltered\n", filter_data ? "" : "not ");

	signal(SIGINT, sig_teardown);
	signal(SIGTERM, sig_teardown);

	promisc = set_promisc(1);

	/* capture loop */
	while (1)
	{
		if (run_dump)
		{
			msg("Dumping ring to %s ...\n", output);

			if (!(o = fopen(output, "w")))
			{
				msg("Unable to open %s: %s\n",
					output, strerror(errno));
			}
			else
			{
				write_pcap_header(o);

				/* sig_dump packet buffer */
				for (i = 0, n = 0; i < ring->len; i++)
				{
					if (!(e = ringbuf_get(ring, i)))
						continue;

					write_pcap_frame(o, &(e->sec), &(e->usec), e->len, e->olen);
					fwrite((void *)e + sizeof(*e), 1, e->len, o);
					n++;
				}

				fclose(o);

				msg(" * %d frames captured\n", frames_captured);
				msg(" * %d frames filtered\n", frames_filtered);
				msg(" * %d frames dumped\n", n);
			}

			run_dump = 0;
		}
		if (run_stop)
		{
			msg("Shutting down ...\n");

			if (promisc)
				set_promisc(0);

			if (ring)
				ringbuf_free(ring);

			return 0;
		}

		pktlen = recvfrom(capture_sock, pktbuf, sizeof(pktbuf), 0, NULL, 0);
		frames_captured++;

		/* check received frametype, if we should filter it, rewind the ring */
		rhdr = (radiotap_hdr_t *)pktbuf;

		if (pktlen <= sizeof(radiotap_hdr_t) || le16(rhdr->it_len) >= pktlen)
		{
			frames_filtered++;
			continue;
		}

		frametype = *(uint8_t *)(pktbuf + le16(rhdr->it_len));

		if ((filter_data   && (frametype & FRAMETYPE_MASK) == FRAMETYPE_DATA) ||
		    (filter_beacon && (frametype & FRAMETYPE_MASK) == FRAMETYPE_BEACON))
		{
			frames_filtered++;
			continue;
		}

		if (streaming)
		{
			if (!header_written)
			{
				write_pcap_header(stdout);
				header_written = 1;
			}

			write_pcap_frame(stdout, NULL, NULL, pktlen, pktlen);
			fwrite(pktbuf, 1, pktlen, stdout);
			fflush(stdout);
		}
		else
		{
			e = ringbuf_add(ring);
			e->olen = pktlen;
			e->len = (pktlen > pktcap) ? pktcap : pktlen;

			memcpy((void *)e + sizeof(*e), pktbuf, e->len);
		}
	}

	return 0;
}
