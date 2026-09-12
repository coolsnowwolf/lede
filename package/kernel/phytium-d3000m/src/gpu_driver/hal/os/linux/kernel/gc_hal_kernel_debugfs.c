/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    This program is free software; you can redistribute it and/or
*    modify it under the terms of the GNU General Public License
*    as published by the Free Software Foundation; either version 2
*    of the License, or (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#ifdef MODULE
# include <linux/module.h>
#endif
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#ifdef MODVERSIONS
# include <linux/modversions.h>
#endif
#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/uaccess.h>
#include <linux/completion.h>
#include <linux/seq_file.h>
#include "gc_hal_kernel_linux.h"
#include "gc_hal_kernel.h"
#include "gc_hal_kernel_debug.h"

#define _GC_OBJ_ZONE gcvZONE_KERNEL

static int
gc_debugfs_open(struct inode *inode, struct file *file)
{
    gcsINFO_NODE *node = inode->i_private;

    return single_open(file, node->info->show, node);
}

static ssize_t
gc_debugfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    struct seq_file *s = file->private_data;
    gcsINFO_NODE *node = s->private;
    gcsINFO *info = node->info;

    if (info->write)
        info->write(buf, count, node);

    return count;
}

static const struct file_operations gc_debugfs_operations = {
    .owner   = THIS_MODULE,
    .open    = gc_debugfs_open,
    .write   = gc_debugfs_write,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

gceSTATUS
gckDEBUGFS_DIR_Init(gckDEBUGFS_DIR Dir, struct dentry *root, gctCONST_STRING Name)
{
    Dir->root = debugfs_create_dir(Name, root);

    if (!Dir->root)
        return gcvSTATUS_NOT_SUPPORTED;

    INIT_LIST_HEAD(&Dir->nodeList);

    return gcvSTATUS_OK;
}

gceSTATUS
gckDEBUGFS_DIR_CreateFiles(gckDEBUGFS_DIR Dir, gcsINFO *List, int count, gctPOINTER Data)
{
    int i;
    gcsINFO_NODE *node;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Dir=%p List=%p count=%d Data=%p", Dir, List, count, Data);

    for (i = 0; i < count; i++) {
        umode_t mode = 0;

        /* Create a node. */
        node = kzalloc(sizeof(gcsINFO_NODE), GFP_KERNEL);

        if (!node)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

        node->info   = &List[i];
        node->device = Data;

        mode |= List[i].show ? 0444 : 0;
        mode |= List[i].write ? 0200 : 0;

        node->entry = debugfs_create_file(List[i].name, mode,
                                          Dir->root, node, &gc_debugfs_operations);

        if (!node->entry)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

        list_add(&node->head, &Dir->nodeList);
    }

OnError:
    if (gcmIS_ERROR(status))
        gcmkVERIFY_OK(gckDEBUGFS_DIR_RemoveFiles(Dir, List, count));

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckDEBUGFS_DIR_RemoveFiles(gckDEBUGFS_DIR Dir, gcsINFO *List, int count)
{
    int i;
    gcsINFO_NODE *node;
    gcsINFO_NODE *temp;

    gcmkHEADER_ARG("Dir=%p List=%p count=%d", Dir, List, count);

    for (i = 0; i < count; i++) {
        list_for_each_entry_safe(node, temp, &Dir->nodeList, head) {
            if (node->info == &List[i]) {
                debugfs_remove(node->entry);
                list_del(&node->head);
                kfree(node);
            }
        }
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

void
gckDEBUGFS_DIR_Deinit(gckDEBUGFS_DIR Dir)
{
    debugfs_remove(Dir->root);
    Dir->root = NULL;
}
