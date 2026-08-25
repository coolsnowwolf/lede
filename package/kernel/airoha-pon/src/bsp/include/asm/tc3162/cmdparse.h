#ifndef _CMDPARSE_H_
#define _CMDPARSE_H_
/************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/

typedef	struct {
	char *name;		
	int	(*func)(int argc,char *argv[],void *p);
	int	flags;	
	int	argcmin;
	char *argc_errmsg;	
} cmds_t;

extern int cmd_reg_add(char *cmd_name, cmds_t *cmds_p);
extern int cmd_register(cmds_t *cmds_p);
extern int cmd_unregister(char *name);
extern int subcmd(const cmds_t tab[], int argc, char *argv[], void *p);

#endif /* _CMDPARSE_ */
