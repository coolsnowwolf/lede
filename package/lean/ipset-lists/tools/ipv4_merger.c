#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef u_int32_t u32;
typedef int bool;
#define true  1
#define false 0
typedef unsigned gfp_t;

static inline char *ipv4_hltos(u32 u, char *s)
{
	static char ss[20];
	if (!s)
		s = ss;
	sprintf(s, "%d.%d.%d.%d",
		(int)(u >> 24) & 0xff, (int)(u >> 16) & 0xff,
		(int)(u >> 8) & 0xff, (int)u & 0xff );
	return s;
}

static inline u32 ipv4_stohl(const char *s)
{
	int u[4];
	if (sscanf(s, "%d.%d.%d.%d", &u[0], &u[1], &u[2], &u[3]) == 4) {
		return  (((u32)u[0] & 0xff) << 24) |
				(((u32)u[1] & 0xff) << 16) |
				(((u32)u[2] & 0xff) << 8) |
				(((u32)u[3] & 0xff));
	} else
		return 0xffffffff;
}

static inline bool is_ipv4_addr(const char *s)
{
	int u[4];
	if (sscanf(s, "%d.%d.%d.%d", &u[0], &u[1], &u[2], &u[3]) == 4)
		return true;
	else
		return false;
}


struct ipv4_range {
	u32 start;
	u32 end;
};

struct sa_open_data {
	struct ipv4_range *tmp_base;
	size_t tmp_size;
	size_t tmp_length;
	int    errors;
};

static int __touch_tmp_base(struct sa_open_data *od, gfp_t gfp)
{
	if (!od->tmp_base) {
		/**
		 * Allocate a temporary table with twice the size of the previous
		 *  table or at least 100, on which new entries can be inserted.
		 */
		if (od->tmp_size < 100)
			od->tmp_size = 100;
		od->tmp_base = (struct ipv4_range *)malloc(
			sizeof(struct ipv4_range) * od->tmp_size /*, gfp*/ );
		if (!od->tmp_base) {
			fprintf(stderr,
				"salist: cannot allocate the temporary list for enlarging it.\n");
			return -ENOMEM;
		}
		od->tmp_length = 0;
	}
	return 0;
}

static int ipv4_list_add_range(struct sa_open_data *od, u32 start,
		u32 end, gfp_t gfp)
{
	struct ipv4_range *cur;
	int ret;

	/* Ignore a new range if it or a larger range already exists */
	//if (salist_check_ipv4(od->table, start, end))
	//	return 0;

	if ((ret = __touch_tmp_base(od, gfp)) < 0)
		return ret;

	/* Check if the size is efficient. Enlarge it if needed. */
	if (od->tmp_length + 1 >= od->tmp_size) {
		size_t old_size = od->tmp_size;
		struct ipv4_range *old_base = od->tmp_base;

		od->tmp_size *= 2;
		od->tmp_base = (struct ipv4_range *)realloc(od->tmp_base,
				sizeof(struct ipv4_range) * od->tmp_size);
		if (!od->tmp_base) {
			od->tmp_size = old_size;
			od->tmp_base = old_base;
			return -ENOMEM;
		}
	}

	cur = &od->tmp_base[od->tmp_length++];
	cur->start = start;
	cur->end = end;

	return 0;
}

static inline int ipv4_list_add_netmask(struct sa_open_data *od,
		u32 net, u32 net_mask, gfp_t gfp)
{
	u32 start = net & net_mask;
	u32 end = net | ~net_mask;

	return ipv4_list_add_range(od, start, end, gfp);
}

static int ipv4_list_add_net(struct sa_open_data *od, u32 net,
		int net_bits, gfp_t gfp)
{
	u32 net_mask;

	if(net_bits == 0)
		net_mask = 0x00000000;
	else
		net_mask = ~(((u32)1 << (32 - net_bits)) - 1);
	//printf("%d: %08x, %08x\n", net_bits, net_mask, net_size);

	return ipv4_list_add_netmask(od, net, net_mask, gfp);
}

static int salist_cmd_parse(struct sa_open_data *od, char *cmd, gfp_t gfp)
{
	char *a1 = NULL, *a2 = NULL;
	char *sep;
	char sc;
	int n = 32;

	/* Case 3: Append an item */

	/* Check IP description part: network segment or range? */
	if ((sep = strchr(cmd, '/'))) { }
	else if ((sep = strchr(cmd, '-'))) { }
	else if ((sep = strchr(cmd, ':'))) { }

	if (sep) {
		/* Describes a subnet or range. */
		sc = *sep;
		*sep = '\0';

		a1 = cmd;
		a2 = sep + 1;

		if (*a2 == '\0') {
			fprintf(stderr, "Nothing after '%c'.\n", sc);
			return -EINVAL;
		}
	} else {
		/* Describes a single IP. */
		sc = '\0';
		a1 = cmd;
	}

	switch (sc) {
	case '/':
		/* 10.10.20.0/24 */
		/* ------------------------------------ */
		if (is_ipv4_addr(a2)) {
			ipv4_list_add_netmask(od, ipv4_stohl(a1), ipv4_stohl(a2), gfp);
		} else {
			sscanf(a2, "%d", &n);
			ipv4_list_add_net(od, ipv4_stohl(a1), n, gfp);
		}
		/* ------------------------------------ */
		break;
	case ':':
	case '-':
		/* 10.10.20.0-10.20.0.255 */
		/* ------------------------------------ */
		ipv4_list_add_range(od, ipv4_stohl(a1), ipv4_stohl(a2), gfp);
		/* ------------------------------------ */
		break;
	default:
		if (is_ipv4_addr(a1)) {
			/* Single IP address. */
			u32 ip = ipv4_stohl(a1);
			/* ------------------------------------ */
			ipv4_list_add_range(od, ip, ip, gfp);
			/* ------------------------------------ */
		} else {
			fprintf(stderr, "Invalid IP address '%s'.\n", a1);
			return -EINVAL;
		}
		break;
	}
	return 0;
}

static int ipv4_range_sort_cmp(const void *a, const void *b)
{
	struct ipv4_range *ra = (struct ipv4_range *)a;
	struct ipv4_range *rb = (struct ipv4_range *)b;

	if (ra->start > rb->start) {
		return 1;
	} else if (ra->start < rb->start) {
		return -1;
	} else if (ra->end > rb->end) {
		return 1;
	} else if (ra->end < rb->end) {
		return -1;
	} else {
		return 0;
	}
}

static void ipv4_range_swap(void *a, void *b, int size)
{
	struct ipv4_range *ra = (struct ipv4_range *)a;
	struct ipv4_range *rb = (struct ipv4_range *)b;
	struct ipv4_range tmp;
	tmp = *ra;
	*ra = *rb;
	*rb = tmp;
}

static struct sa_open_data *salist_open(void)
{
	struct sa_open_data *od = NULL;

	od = (struct sa_open_data *)malloc(sizeof(*od));
	if (!od) {
		fprintf(stderr, "salist: cannot allocate sa_open_data.\n");
		return NULL;
	}
	memset(od, 0, sizeof(*od));
	od->errors = 0;

	return od;
}

static int salist_close(struct sa_open_data *od)
{
	size_t ri, wi;
	struct ipv4_range *old_base;

	/* Flush the table if any modification has been done */
	if (od->tmp_base) {
		/* Sort the table and merge entries as many as possible. */
		if (od->tmp_length >= 2) {
			qsort(od->tmp_base, od->tmp_length, sizeof(struct ipv4_range),
				ipv4_range_sort_cmp);
			
			for (wi = 0, ri = 1; ri < od->tmp_length; ri++) {
				/* NOTICE: 0xffffffff + 1 ? */
				if (od->tmp_base[wi].end == (u32)(-1)) {
					/* Nothing */
				} else if (od->tmp_base[ri].start <= od->tmp_base[wi].end + 1) {
					/* The two ranges overlap, so merge the 2nd to the 1st one */
					if (od->tmp_base[ri].end > od->tmp_base[wi].end)
						od->tmp_base[wi].end = od->tmp_base[ri].end;
				} else {
					wi++;
					if (wi < ri)
						od->tmp_base[wi] = od->tmp_base[ri];
				}
			}

			od->tmp_length = wi + 1;
		}

		/* Reduce the size */
		if (od->tmp_length < od->tmp_size) {
			struct ipv4_range *__tmp = od->tmp_base;
			od->tmp_base = (struct ipv4_range *)malloc(
				sizeof(struct ipv4_range) * (od->tmp_length ? od->tmp_length : 1));
			if (od->tmp_base) {
				memcpy(od->tmp_base, __tmp,
					sizeof(struct ipv4_range) * od->tmp_length);
				free(__tmp);
			} else {
				fprintf(stderr, "[%s:%d] Failed to allocate temporary table.\n",
					__FUNCTION__, __LINE__);
				/* If failed to allocate new memory, do not reduce it. */
				od->tmp_base = __tmp;
			}
		}

		/* Dump the table instead */
	}

	if (od->errors) {
		fprintf(stderr, "[%s] %d errors detected during table operation.\n",
				__FUNCTION__, od->errors);
	}

	return 0;
}

static void sa_open_data_dump(struct sa_open_data *od)
{
	size_t i;
	char s1[20], s2[20];

	for (i = 0; i < od->tmp_length; i++) {
		printf("%s-%s\n", ipv4_hltos(od->tmp_base[i].start, s1),
			ipv4_hltos(od->tmp_base[i].end, s2));
	}
}

int main(int argc, char *argv[])
{
	struct sa_open_data *od;
	char lbuf[128];

	od = salist_open();

	while (fgets(lbuf, sizeof(lbuf), stdin)) {
		size_t llen = strlen(lbuf);
		if (llen > 0 && lbuf[llen - 1] == '\n')
			lbuf[--llen] = '\0';
		if (llen > 0 && lbuf[llen - 1] == '\r')
			lbuf[--llen] = '\0';
		if (llen == 0)
			continue;
		salist_cmd_parse(od, lbuf, 0);
	}

	salist_close(od);

	sa_open_data_dump(od);

	return 0;
}
