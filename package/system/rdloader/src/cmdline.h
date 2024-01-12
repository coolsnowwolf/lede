//
// Created by juno on 2022/1/11.
//

#ifndef INIT_CMDLINE_H
#define INIT_CMDLINE_H

struct fstype
{
	char *name;
	struct fstype *next;
};

struct bootargs_t
{
	char *root;
	struct fstype *fstype;
};

int parse_cmdline(struct bootargs_t **ret);

void free_bootargs(struct bootargs_t **args);

#endif //INIT_CMDLINE_H
