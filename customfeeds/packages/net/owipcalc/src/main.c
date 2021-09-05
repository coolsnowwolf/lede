#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>

#include "owipcalc.h"

struct op {
	const char *name;
	const char *desc;
	struct {
		bool (*a1)(struct cidr *a);
		bool (*a2)(struct cidr *a, struct cidr *b);
	} f4;
	struct {
		bool (*a1)(struct cidr *a);
		bool (*a2)(struct cidr *a, struct cidr *b);
	} f6;
};

struct op ops[] = {
	{ .name = "add",
	  .desc = "Add argument to base address",
	  .f4.a2 = cidr_add4,
	  .f6.a2 = cidr_add6 },

	{ .name = "sub",
	  .desc = "Substract argument from base address",
	  .f4.a2 = cidr_sub4,
	  .f6.a2 = cidr_sub6 },

	{ .name = "next",
	  .desc = "Advance base address to next prefix of given size",
	  .f4.a2 = cidr_next4,
	  .f6.a2 = cidr_next6 },

	{ .name = "prev",
	  .desc = "Lower base address to previous prefix of give size",
	  .f4.a2 = cidr_prev4,
	  .f6.a2 = cidr_prev6 },

	{ .name = "network",
	  .desc = "Turn base address into network address",
	  .f4.a1 = cidr_network4,
	  .f6.a1 = cidr_network6 },

	{ .name = "broadcast",
	  .desc = "Turn base address into broadcast address",
	  .f4.a1 = cidr_broadcast4 },

	{ .name = "prefix",
	  .desc = "Set the prefix of base address to argument",
	  .f4.a2 = cidr_prefix,
	  .f6.a2 = cidr_prefix },

	{ .name = "netmask",
	  .desc = "Calculate netmask of base address",
	  .f4.a1 = cidr_netmask4 },

	{ .name = "6to4",
	  .desc = "Calculate 6to4 prefix of given ipv4-address",
	  .f4.a1 = cidr_6to4 },

	{ .name = "howmany",
	  .desc = "Print amount of righ-hand prefixes that fit into base address",
	  .f4.a2 = cidr_howmany,
	  .f6.a2 = cidr_howmany },

	{ .name = "contains",
	  .desc = "Print '1' if argument fits into base address or '0' if not",
	  .f4.a2 = cidr_contains4,
	  .f6.a2 = cidr_contains6 },

	{ .name = "private",
	  .desc = "Print '1' if base address is in RFC1918 private space or '0' "
	          "if not",
	  .f4.a1 = cidr_private4 },

	{ .name = "linklocal",
	  .desc = "Print '1' if base address is in 169.254.0.0/16 or FE80::/10 "
	          "link local space or '0' if not",
	  .f4.a1 = cidr_linklocal4,
	  .f6.a1 = cidr_linklocal6 },

	{ .name = "ula",
	  .desc = "Print '1' if base address is in FC00::/7 unique local address "
	          "(ULA) space or '0' if not",
	  .f6.a1 = cidr_ula6 },

	{ .name = "quiet",
	  .desc = "Suppress output, useful for test operation where the result can "
	          "be inferred from the exit code",
	  .f4.a1 = cidr_quiet,
	  .f6.a1 = cidr_quiet },

	{ .name = "pop",
	  .desc = "Pop intermediate result from stack",
	  .f4.a1 = cidr_pop,
	  .f6.a1 = cidr_pop },

	{ .name = "print",
	  .desc = "Print intermediate result and pop it from stack, invoked "
	          "implicitely at the end of calculation if no intermediate prints "
	          "happened",
	  .f4.a1 = cidr_print4,
	  .f6.a1 = cidr_print6 },
};

static void usage(const char *prog)
{
	int i;

	fprintf(stderr,
	        "\n"
	        "Usage:\n\n"
	        "  %s {base address} operation [argument] "
	        "[operation [argument] ...]\n\n"
	        "Operations:\n\n",
	        prog);

	for (i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
	{
		if (ops[i].f4.a2 || ops[i].f6.a2)
		{
			fprintf(stderr, "  %s %s\n",
			        ops[i].name,
			        (ops[i].f4.a2 && ops[i].f6.a2) ? "{ipv4/ipv6/amount}" :
			         (ops[i].f6.a2 ? "{ipv6/amount}" : "{ipv4/amount}"));
		}
		else
		{
			fprintf(stderr, "  %s\n", ops[i].name);
		}

		fprintf(stderr, "    %s.\n", ops[i].desc);

		if ((ops[i].f4.a1 && ops[i].f6.a1) || (ops[i].f4.a2 && ops[i].f6.a2))
			fprintf(stderr, "    Applicable to ipv4- and ipv6-addresses.\n\n");
		else if (ops[i].f6.a2 || ops[i].f6.a1)
			fprintf(stderr, "    Only applicable to ipv6-addresses.\n\n");
		else
			fprintf(stderr, "    Only applicable to ipv4-addresses.\n\n");
	}

	fprintf(stderr,
	        "Examples:\n\n"
	        " Calculate a DHCP range:\n\n"
	        "  $ %s 192.168.1.1/255.255.255.0 network add 100 print add 150 print\n"
			"  192.168.1.100\n"
			"  192.168.1.250\n\n"
			" Count number of prefixes:\n\n"
			"  $ %s 2001:0DB8:FDEF::/48 howmany ::/64\n"
			"  65536\n\n",
	        prog, prog);

	exit(1);
}

static bool runop(char ***arg, int *status)
{
	int i;
	char *arg1 = **arg;
	char *arg2 = *(*arg+1);
	struct cidr *a = stack;
	struct cidr *b = NULL;

	if (!arg1)
		return false;

	for (i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
	{
		if (!strcmp(ops[i].name, arg1))
		{
			if (ops[i].f4.a2 || ops[i].f6.a2)
			{
				if (!arg2)
				{
					fprintf(stderr, "'%s' requires an argument\n",
							ops[i].name);

					*status = 2;
					return false;
				}

				b = cidr_parse(ops[i].name, arg2, a->family);

				if (!b)
				{
					fprintf(stderr, "invalid address argument for '%s'\n",
							ops[i].name);

					*status = 3;
					return false;
				}

				*arg += 2;

				if (((a->family == AF_INET)  && !ops[i].f4.a2) ||
					((a->family == AF_INET6) && !ops[i].f6.a2))
				{
					fprintf(stderr, "'%s' not supported for %s addresses\n",
					        ops[i].name,
							(a->family == AF_INET) ? "ipv4" : "ipv6");

					*status = 5;
					return false;
				}

				*status = !((a->family == AF_INET) ? ops[i].f4.a2(a, b)
				                                   : ops[i].f6.a2(a, b));

				return true;
			}
			else
			{
				*arg += 1;

				if (((a->family == AF_INET)  && !ops[i].f4.a1) ||
					((a->family == AF_INET6) && !ops[i].f6.a1))
				{
					fprintf(stderr, "'%s' not supported for %s addresses\n",
					        ops[i].name,
							(a->family == AF_INET) ? "ipv4" : "ipv6");

					*status = 5;
					return false;
				}

				*status = !((a->family == AF_INET) ? ops[i].f4.a1(a)
				                                   : ops[i].f6.a1(a));

				return true;
			}
		}
	}

	return false;
}

int main(int argc, char **argv)
{
	int status = 0;
	char **arg = argv+2;
	struct cidr *a;

	// enable printing
	quiet = false;

	if (argc < 3)
		usage(argv[0]);

	a = strchr(argv[1], ':') ? cidr_parse6(argv[1]) : cidr_parse4(argv[1]);

	if (!a)
		usage(argv[0]);

	cidr_push(a);

	while (runop(&arg, &status));

	if (*arg)
	{
		fprintf(stderr, "unknown operation '%s'\n", *arg);
		exit(6);
	}

	if (!printed && (status < 2))
	{
		if (stack->family == AF_INET)
			cidr_print4(stack);
		else
			cidr_print6(stack);
	}

	qprintf("\n");

	exit(status);
}
