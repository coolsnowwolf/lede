/*
 * luci-bwc - Very simple bandwidth collector cache for LuCI realtime graphs
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <jow@openwrt.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include <dlfcn.h>
#include <iwinfo.h>

#define STEP_COUNT	60
#define STEP_TIME	1
#define TIMEOUT		10

#define PID_PATH	"/var/run/luci-bwc.pid"

#define DB_PATH		"/var/lib/luci-bwc"
#define DB_IF_FILE	DB_PATH "/if/%s"
#define DB_RD_FILE	DB_PATH "/if/%s"
#define DB_CN_FILE	DB_PATH "/connections"
#define DB_LD_FILE	DB_PATH "/load"

#define IF_SCAN_PATTERN \
	" %[^ :]:%u %u" \
	" %*d %*d %*d %*d %*d %*d" \
	" %u %u"

#define LD_SCAN_PATTERN \
	"%f %f %f"


struct file_map {
	int fd;
	int size;
	char *mmap;
};

struct traffic_entry {
	uint32_t time;
	uint32_t rxb;
	uint32_t rxp;
	uint32_t txb;
	uint32_t txp;
};

struct conn_entry {
	uint32_t time;
	uint32_t udp;
	uint32_t tcp;
	uint32_t other;
};

struct load_entry {
	uint32_t time;
	uint16_t load1;
	uint16_t load5;
	uint16_t load15;
};

struct radio_entry {
	uint32_t time;
	uint16_t rate;
	uint8_t  rssi;
	uint8_t  noise;
};

static int readpid(void)
{
	int fd;
	int pid = -1;
	char buf[9] = { 0 };

	if ((fd = open(PID_PATH, O_RDONLY)) > -1)
	{
		if (read(fd, buf, sizeof(buf)))
		{
			buf[8] = 0;
			pid = atoi(buf);
		}

		close(fd);
	}

	return pid;
}

static int writepid(void)
{
	int fd;
	int wlen;
	char buf[9] = { 0 };

	if ((fd = open(PID_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0600)) > -1)
	{
		wlen = snprintf(buf, sizeof(buf), "%i", getpid());
		write(fd, buf, wlen);
		close(fd);

		return 0;
	}

	return -1;
}

static int timeout = TIMEOUT;
static int countdown = -1;

static void reset_countdown(int sig)
{
	countdown = timeout;

}


static char *progname;
static int prognamelen;

static struct iwinfo_ops *backend = NULL;


static int init_directory(char *path)
{
	char *p = path;

	for (p = &path[1]; *p; p++)
	{
		if (*p == '/')
		{
			*p = 0;

			if (mkdir(path, 0700) && (errno != EEXIST))
				return -1;

			*p = '/';
		}
	}

	return 0;
}

static int init_file(char *path, int esize)
{
	int i, file;
	char buf[sizeof(struct traffic_entry)] = { 0 };

	if (init_directory(path))
		return -1;

	if ((file = open(path, O_WRONLY | O_CREAT, 0600)) >= 0)
	{
		for (i = 0; i < STEP_COUNT; i++)
		{
			if (write(file, buf, esize) < 0)
				break;
		}

		close(file);

		return 0;
	}

	return -1;
}

static inline uint32_t timeof(void *entry)
{
	return ntohl(((struct traffic_entry *)entry)->time);
}

static int update_file(const char *path, void *entry, int esize)
{
	int rv = -1;
	int file;
	char *map;

	if ((file = open(path, O_RDWR)) >= 0)
	{
		map = mmap(NULL, esize * STEP_COUNT, PROT_READ | PROT_WRITE,
				   MAP_SHARED | MAP_LOCKED, file, 0);

		if ((map != NULL) && (map != MAP_FAILED))
		{
			if (timeof(entry) > timeof(map + esize * (STEP_COUNT-1)))
			{
				memmove(map, map + esize, esize * (STEP_COUNT-1));
				memcpy(map + esize * (STEP_COUNT-1), entry, esize);
			}

			munmap(map, esize * STEP_COUNT);

			rv = 0;
		}

		close(file);
	}

	return rv;
}

static int mmap_file(const char *path, int esize, struct file_map *m)
{
	m->fd   = -1;
	m->size = -1;
	m->mmap = NULL;

	if ((m->fd = open(path, O_RDONLY)) >= 0)
	{
		m->size = STEP_COUNT * esize;
		m->mmap = mmap(NULL, m->size, PROT_READ,
					   MAP_SHARED | MAP_LOCKED, m->fd, 0);

		if ((m->mmap != NULL) && (m->mmap != MAP_FAILED))
			return 0;
	}

	return -1;
}

static void umap_file(struct file_map *m)
{
	if ((m->mmap != NULL) && (m->mmap != MAP_FAILED))
		munmap(m->mmap, m->size);

	if (m->fd > -1)
		close(m->fd);
}

static void * iw_open(void)
{
	return dlopen("/usr/lib/libiwinfo.so", RTLD_LAZY);
}

static int iw_update(
	void *iw, const char *ifname, uint16_t *rate, uint8_t *rssi, uint8_t *noise
) {
	struct iwinfo_ops *(*probe)(const char *);
	int val;

	if (!backend)
	{
		probe = dlsym(iw, "iwinfo_backend");

		if (!probe)
			return 0;

		backend = probe(ifname);

		if (!backend)
			return 0;
	}

	*rate = (backend->bitrate && !backend->bitrate(ifname, &val)) ? val : 0;
	*rssi = (backend->signal && !backend->signal(ifname, &val)) ? val : 0;
	*noise = (backend->noise && !backend->noise(ifname, &val)) ? val : 0;

	return 1;
}

static void iw_close(void *iw)
{
	void (*finish)(void);

	finish = dlsym(iw, "iwinfo_finish");

	if (finish)
		finish();

	dlclose(iw);
}


static int update_ifstat(
	const char *ifname, uint32_t rxb, uint32_t rxp, uint32_t txb, uint32_t txp
) {
	char path[1024];

	struct stat s;
	struct traffic_entry e;

	snprintf(path, sizeof(path), DB_IF_FILE, ifname);

	if (stat(path, &s))
	{
		if (init_file(path, sizeof(struct traffic_entry)))
		{
			fprintf(stderr, "Failed to init %s: %s\n",
					path, strerror(errno));

			return -1;
		}
	}

	e.time = htonl(time(NULL));
	e.rxb  = htonl(rxb);
	e.rxp  = htonl(rxp);
	e.txb  = htonl(txb);
	e.txp  = htonl(txp);

	return update_file(path, &e, sizeof(struct traffic_entry));
}

static int update_radiostat(
	const char *ifname, uint16_t rate, uint8_t rssi, uint8_t noise
) {
	char path[1024];

	struct stat s;
	struct radio_entry e;

	snprintf(path, sizeof(path), DB_RD_FILE, ifname);

	if (stat(path, &s))
	{
		if (init_file(path, sizeof(struct radio_entry)))
		{
			fprintf(stderr, "Failed to init %s: %s\n",
					path, strerror(errno));

			return -1;
		}
	}

	e.time  = htonl(time(NULL));
	e.rate  = htons(rate);
	e.rssi  = rssi;
	e.noise = noise;

	return update_file(path, &e, sizeof(struct radio_entry));
}

static int update_cnstat(uint32_t udp, uint32_t tcp, uint32_t other)
{
	char path[1024];

	struct stat s;
	struct conn_entry e;

	snprintf(path, sizeof(path), DB_CN_FILE);

	if (stat(path, &s))
	{
		if (init_file(path, sizeof(struct conn_entry)))
		{
			fprintf(stderr, "Failed to init %s: %s\n",
					path, strerror(errno));

			return -1;
		}
	}

	e.time  = htonl(time(NULL));
	e.udp   = htonl(udp);
	e.tcp   = htonl(tcp);
	e.other = htonl(other);

	return update_file(path, &e, sizeof(struct conn_entry));
}

static int update_ldstat(uint16_t load1, uint16_t load5, uint16_t load15)
{
	char path[1024];

	struct stat s;
	struct load_entry e;

	snprintf(path, sizeof(path), DB_LD_FILE);

	if (stat(path, &s))
	{
		if (init_file(path, sizeof(struct load_entry)))
		{
			fprintf(stderr, "Failed to init %s: %s\n",
					path, strerror(errno));

			return -1;
		}
	}

	e.time   = htonl(time(NULL));
	e.load1  = htons(load1);
	e.load5  = htons(load5);
	e.load15 = htons(load15);

	return update_file(path, &e, sizeof(struct load_entry));
}

static int run_daemon(void)
{
	FILE *info;
	uint32_t rxb, txb, rxp, txp;
	uint32_t udp, tcp, other;
	uint16_t rate;
	uint8_t rssi, noise;
	float lf1, lf5, lf15;
	char line[1024];
	char ifname[16];
	int i;
	void *iw;
	struct sigaction sa;

	struct stat s;
	const char *ipc = stat("/proc/net/nf_conntrack", &s)
		? "/proc/net/ip_conntrack" : "/proc/net/nf_conntrack";

	switch (fork())
	{
		case -1:
			perror("fork()");
			return -1;

		case 0:
			if (chdir("/") < 0)
			{
				perror("chdir()");
				exit(1);
			}

			close(0);
			close(1);
			close(2);
			break;

		default:
			return 0;
	}

	/* setup USR1 signal handler to reset timer */
	sa.sa_handler = reset_countdown;
	sa.sa_flags   = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);

	/* write pid */
	if (writepid())
	{
		fprintf(stderr, "Failed to write pid file: %s\n", strerror(errno));
		return 1;
	}

	/* initialize iwinfo */
	iw = iw_open();

	/* go */
	for (reset_countdown(0); countdown >= 0; countdown--)
	{
		/* alter progname for ps, top */
		memset(progname, 0, prognamelen);
		snprintf(progname, prognamelen, "luci-bwc %d", countdown);

		if ((info = fopen("/proc/net/dev", "r")) != NULL)
		{
			while (fgets(line, sizeof(line), info))
			{
				if (strchr(line, '|'))
					continue;

				if (sscanf(line, IF_SCAN_PATTERN, ifname, &rxb, &rxp, &txb, &txp))
				{
					if (strncmp(ifname, "lo", sizeof(ifname)))
						update_ifstat(ifname, rxb, rxp, txb, txp);
				}
			}

			fclose(info);
		}

		if (iw)
		{
			for (i = 0; i < 5; i++)
			{
#define iw_checkif(pattern) \
				do {                                                      \
					snprintf(ifname, sizeof(ifname), pattern, i);         \
					if (iw_update(iw, ifname, &rate, &rssi, &noise))  \
					{                                                     \
						update_radiostat(ifname, rate, rssi, noise);      \
						continue;                                         \
					}                                                     \
				} while(0)

				iw_checkif("wlan%d");
				iw_checkif("ath%d");
				iw_checkif("wl%d");
				iw_checkif("host%d");
				iw_checkif("rausb%d");
				iw_checkif("rai%d");
				iw_checkif("ra%d");
				iw_checkif("apcliusb%d");
				iw_checkif("apclii%d");
				iw_checkif("apcli%d");
			}
		}

		if ((info = fopen(ipc, "r")) != NULL)
		{
			udp   = 0;
			tcp   = 0;
			other = 0;

			while (fgets(line, sizeof(line), info))
			{
				if (strstr(line, "TIME_WAIT"))
					continue;

				if ((strstr(line, "src=127.0.0.1 ") && strstr(line, "dst=127.0.0.1 ")) 
				|| (strstr(line, "src=::1 ") && strstr(line, "dst=::1 ")))
					continue;

				if (sscanf(line, "%*s %*d %s", ifname) || sscanf(line, "%s %*d", ifname))
				{
					if (!strcmp(ifname, "tcp"))
						tcp++;
					else if (!strcmp(ifname, "udp"))
						udp++;
					else
						other++;
				}
			}

			update_cnstat(udp, tcp, other);

			fclose(info);
		}

		if ((info = fopen("/proc/loadavg", "r")) != NULL)
		{
			if (fscanf(info, LD_SCAN_PATTERN, &lf1, &lf5, &lf15))
			{
				update_ldstat((uint16_t)(lf1  * 100),
							  (uint16_t)(lf5  * 100),
							  (uint16_t)(lf15 * 100));
			}

			fclose(info);
		}

		sleep(STEP_TIME);
	}

	unlink(PID_PATH);

	if (iw)
		iw_close(iw);

	return 0;
}

static void check_daemon(void)
{
	int pid;

	if ((pid = readpid()) < 0 || kill(pid, 0) < 0)
	{
		/* daemon ping failed, try to start it up */
		if (run_daemon())
		{
			fprintf(stderr,
				"Failed to ping daemon and unable to start it up: %s\n",
				strerror(errno));

			exit(1);
		}
	}
	else if (kill(pid, SIGUSR1))
	{
		fprintf(stderr, "Failed to send signal: %s\n", strerror(errno));
		exit(2);
	}
}

static int run_dump_ifname(const char *ifname)
{
	int i;
	char path[1024];
	struct file_map m;
	struct traffic_entry *e;

	check_daemon();
	snprintf(path, sizeof(path), DB_IF_FILE, ifname);

	if (mmap_file(path, sizeof(struct traffic_entry), &m))
	{
		fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
		return 1;
	}

	for (i = 0; i < m.size; i += sizeof(struct traffic_entry))
	{
		e = (struct traffic_entry *) &m.mmap[i];

		if (!e->time)
			continue;

		printf("[ %u, %u, %" PRIu32
			   ", %u, %u ]%s\n",
			ntohl(e->time),
			ntohl(e->rxb), ntohl(e->rxp),
			ntohl(e->txb), ntohl(e->txp),
			((i + sizeof(struct traffic_entry)) < m.size) ? "," : "");
	}

	umap_file(&m);

	return 0;
}

static int run_dump_radio(const char *ifname)
{
	int i;
	char path[1024];
	struct file_map m;
	struct radio_entry *e;

	check_daemon();
	snprintf(path, sizeof(path), DB_RD_FILE, ifname);

	if (mmap_file(path, sizeof(struct radio_entry), &m))
	{
		fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
		return 1;
	}

	for (i = 0; i < m.size; i += sizeof(struct radio_entry))
	{
		e = (struct radio_entry *) &m.mmap[i];

		if (!e->time)
			continue;

		printf("[ %u, %d, %d, %d ]%s\n",
			ntohl(e->time),
			e->rate, e->rssi, e->noise,
			((i + sizeof(struct radio_entry)) < m.size) ? "," : "");
	}

	umap_file(&m);

	return 0;
}

static int run_dump_conns(void)
{
	int i;
	char path[1024];
	struct file_map m;
	struct conn_entry *e;

	check_daemon();
	snprintf(path, sizeof(path), DB_CN_FILE);

	if (mmap_file(path, sizeof(struct conn_entry), &m))
	{
		fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
		return 1;
	}

	for (i = 0; i < m.size; i += sizeof(struct conn_entry))
	{
		e = (struct conn_entry *) &m.mmap[i];

		if (!e->time)
			continue;

		printf("[ %u, %u, %u, %u ]%s\n",
			ntohl(e->time), ntohl(e->udp),
			ntohl(e->tcp), ntohl(e->other),
			((i + sizeof(struct conn_entry)) < m.size) ? "," : "");
	}

	umap_file(&m);

	return 0;
}

static int run_dump_load(void)
{
	int i;
	char path[1024];
	struct file_map m;
	struct load_entry *e;

	check_daemon();
	snprintf(path, sizeof(path), DB_LD_FILE);

	if (mmap_file(path, sizeof(struct load_entry), &m))
	{
		fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
		return 1;
	}

	for (i = 0; i < m.size; i += sizeof(struct load_entry))
	{
		e = (struct load_entry *) &m.mmap[i];

		if (!e->time)
			continue;

		printf("[ %u, %u, %u, %u ]%s\n",
			ntohl(e->time),
			ntohs(e->load1), ntohs(e->load5), ntohs(e->load15),
			((i + sizeof(struct load_entry)) < m.size) ? "," : "");
	}

	umap_file(&m);

	return 0;
}


int main(int argc, char *argv[])
{
	int opt;

	progname = argv[0];
	prognamelen = -1;

	for (opt = 0; opt < argc; opt++)
		prognamelen += 1 + strlen(argv[opt]);

	while ((opt = getopt(argc, argv, "t:i:r:cl")) > -1)
	{
		switch (opt)
		{
			case 't':
				timeout = atoi(optarg);
				break;

			case 'i':
				if (optarg)
					return run_dump_ifname(optarg);
				break;

			case 'r':
				if (optarg)
					return run_dump_radio(optarg);
				break;

			case 'c':
				return run_dump_conns();

			case 'l':
				return run_dump_load();

			default:
				break;
		}
	}

	fprintf(stderr,
		"Usage:\n"
		"	%s [-t timeout] -i ifname\n"
		"	%s [-t timeout] -r radiodev\n"
		"	%s [-t timeout] -c\n"
		"	%s [-t timeout] -l\n",
			argv[0], argv[0], argv[0], argv[0]
	);

	return 1;
}
