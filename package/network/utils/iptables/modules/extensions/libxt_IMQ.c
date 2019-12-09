/* Shared library add-on to iptables to add IMQ target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_IMQ.h>

/* Function which prints out usage message. */
static void IMQ_help(void)
{
	printf(
"IMQ target options:\n"
"  --todev <N>		enqueue to imq<N>, defaults to 0\n");

}

static struct option IMQ_opts[] = {
	{ "todev", 1, 0, '1' },
	{ 0 }
};

/* Initialize the target. */
static void IMQ_init(struct xt_entry_target *t)
{
	struct xt_imq_info *mr = (struct xt_imq_info*)t->data;

	mr->todev = 0;
}

/* Function which parses command options; returns true if it
   ate an option */
static int IMQ_parse(int c, char **argv, int invert, unsigned int *flags,
      const void *entry, struct xt_entry_target **target)
{
	struct xt_imq_info *mr = (struct xt_imq_info*)(*target)->data;
	
	switch(c) {
	case '1':
/*		if (xtables_check_inverse(optarg, &invert, NULL, 0, argv))
			xtables_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --todev");
*/
		mr->todev=atoi(optarg);
		break;

	default:
		return 0;
	}
	return 1;
}

/* Prints out the targinfo. */
static void IMQ_print(const void *ip,
      const struct xt_entry_target *target,
      int numeric)
{
	struct xt_imq_info *mr = (struct xt_imq_info*)target->data;

	printf("IMQ: todev %u ", mr->todev);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void IMQ_save(const void *ip, const struct xt_entry_target *target)
{
	struct xt_imq_info *mr = (struct xt_imq_info*)target->data;

	printf(" --todev %u", mr->todev);
}

static struct xtables_target imq_target = {
	.name		= "IMQ",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV4,
	.size		= XT_ALIGN(sizeof(struct xt_imq_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_imq_info)),
	.help		= IMQ_help,
	.init		= IMQ_init,
	.parse		= IMQ_parse,
	.print		= IMQ_print,
	.save		= IMQ_save,
	.extra_opts	= IMQ_opts,
};

static struct xtables_target imq_target6 = {
	.name		= "IMQ",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_IPV6,
	.size		= XT_ALIGN(sizeof(struct xt_imq_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct xt_imq_info)),
	.help		= IMQ_help,
	.init		= IMQ_init,
	.parse		= IMQ_parse,
	.print		= IMQ_print,
	.save		= IMQ_save,
	.extra_opts	= IMQ_opts,
};

// void __attribute((constructor)) nf_ext_init(void){
void _init(void){
	xtables_register_target(&imq_target);
	xtables_register_target(&imq_target6);
}
