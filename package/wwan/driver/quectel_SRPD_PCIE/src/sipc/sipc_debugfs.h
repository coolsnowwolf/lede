/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __SIPC_DEBUGFS_H
#define __SIPC_DEBUGFS_H

int sbuf_init_debugfs(void *root);
int smsg_init_debugfs(void *root);
int sblock_init_debugfs(void *root);
int smem_init_debugfs(void *root);

#ifdef CONFIG_SPRD_SIPC_ZERO_COPY_SIPX
int sipx_init_debugfs(void *root);
#endif

#ifdef CONFIG_SPRD_SIPC_SWCNBLK
int swcnblk_init_debugfs(void *root);
#endif

#if defined(CONFIG_SPRD_SIPC_SMSGC)
int smsgc_init_debugfs(void *root);
#endif

#ifdef CONFIG_SPRD_MAILBOX
int mbox_init_debugfs(void *root);
#endif
#endif /* !__SIPC_DEBUGFS_H */

