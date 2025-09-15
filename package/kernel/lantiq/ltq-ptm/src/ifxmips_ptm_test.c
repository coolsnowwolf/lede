/******************************************************************************
**
** FILE NAME    : ifxmips_ptm_vdsl.c
** PROJECT      : UEIP
** MODULES      : PTM
**
** DATE         : 7 Jul 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM driver common source file (core functions for VR9)
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 07 JUL 2009  Xu Liang        Init Version
*******************************************************************************/



#ifdef CONFIG_IFX_PTM_TEST_PROC

/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/etherdevice.h>

/*
 *  Chip Specific Head File
 */
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include "ifxmips_ptm_common.h"
#include "ifxmips_ptm_ppe_common.h"



/*
 * ####################################
 *              Definition
 * ####################################
 */



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  Proc File Functions
 */
static inline void proc_file_create(void);
static inline void proc_file_delete(void);

/*
 *  Proc Help Functions
 */
static int proc_write_mem(struct file *, const char *, unsigned long, void *);
static int proc_read_pp32(char *, char **, off_t, int, int *, void *);
static int proc_write_pp32(struct file *, const char *, unsigned long, void *);
static int stricmp(const char *, const char *);
static int strincmp(const char *, const char *, int);
static int get_token(char **, char **, int *, int *);
static int get_number(char **, int *, int);
static inline void ignore_space(char **, int *);



/*
 * ####################################
 *            Local Variable
 * ####################################
 */



/*
 * ####################################
 *            Local Function
 * ####################################
 */

static inline void proc_file_create(void)
{
    struct proc_dir_entry *res;

    res = create_proc_entry("driver/ifx_ptm/mem",
                            0,
                            NULL);
    if ( res != NULL )
        res->write_proc = proc_write_mem;
    else
        printk("%s:%s:%d: failed to create proc mem!", __FILE__, __func__, __LINE__);

    res = create_proc_entry("driver/ifx_ptm/pp32",
                            0,
                            NULL);
    if ( res != NULL ) {
        res->read_proc = proc_read_pp32;
        res->write_proc = proc_write_pp32;
    }
    else
        printk("%s:%s:%d: failed to create proc pp32!", __FILE__, __func__, __LINE__);
}

static inline void proc_file_delete(void)
{
    remove_proc_entry("driver/ifx_ptm/pp32", NULL);

    remove_proc_entry("driver/ifx_ptm/mem", NULL);
}

static inline unsigned long sb_addr_to_fpi_addr_convert(unsigned long sb_addr)
{
#define PP32_SB_ADDR_END        0xFFFF

    if ( sb_addr < PP32_SB_ADDR_END) {
        return (unsigned long ) SB_BUFFER(sb_addr);
    }
    else {
        return sb_addr;
    }
}

static int proc_write_mem(struct file *file, const char *buf, unsigned long count, void *data)
{
    char *p1, *p2;
    int len;
    int colon;
    unsigned long *p;
    char local_buf[1024];
    int i, n, l;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    colon = 1;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "w") == 0 || stricmp(p1, "write") == 0 || stricmp(p1, "r") == 0 || stricmp(p1, "read") == 0 )
            break;

        p1 = p2;
        colon = 1;
    }

    if ( *p1 == 'w' )
    {
        ignore_space(&p2, &len);
        p = (unsigned long *)get_number(&p2, &len, 1);
        p = (unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);

        if ( (u32)p >= KSEG0 )
            while ( 1 )
            {
                ignore_space(&p2, &len);
                if ( !len || !((*p2 >= '0' && *p2 <= '9') || (*p2 >= 'a' && *p2 <= 'f') || (*p2 >= 'A' && *p2 <= 'F')) )
                    break;

                *p++ = (u32)get_number(&p2, &len, 1);
            }
    }
    else if ( *p1 == 'r' )
    {
        ignore_space(&p2, &len);
        p = (unsigned long *)get_number(&p2, &len, 1);
        p = (unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);

        if ( (u32)p >= KSEG0 )
        {
            ignore_space(&p2, &len);
            n = (int)get_number(&p2, &len, 0);
            if ( n )
            {
                char str[32] = {0};
                char *pch = str;
                int k;
                u32 data;
                char c;

                n += (l = ((int)p >> 2) & 0x03);
                p = (unsigned long *)((u32)p & ~0x0F);
                for ( i = 0; i < n; i++ )
                {
                    if ( (i & 0x03) == 0 )
                    {
                        printk("%08X:", (u32)p);
                        pch = str;
                    }
                    if ( i < l )
                    {
                        printk("         ");
                        sprintf(pch, "    ");
                    }
                    else
                    {
                        data = (u32)*p;
                        printk(" %08X", data);
                        for ( k = 0; k < 4; k++ )
                        {
                            c = ((char*)&data)[k];
                            pch[k] = c < ' ' ? '.' : c;
                        }
                    }
                    p++;
                    pch += 4;
                    if ( (i & 0x03) == 0x03 )
                    {
                        pch[0] = 0;
                        printk(" ; %s\n", str);
                    }
                }
                if ( (n & 0x03) != 0x00 )
                {
                    for ( k = 4 - (n & 0x03); k > 0; k-- )
                        printk("         ");
                    pch[0] = 0;
                    printk(" ; %s\n", str);
                }
            }
        }
    }

    return count;
}

#ifdef CONFIG_DANUBE

static int proc_read_pp32(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    static const char *halt_stat[] = {
        "reset",
        "break in line",
        "stop",
        "step",
        "code",
        "data0",
        "data1"
    };
    static const char *brk_src_data[] = {
        "off",
        "read",
        "write",
        "read/write",
        "write_equal",
        "N/A",
        "N/A",
        "N/A"
    };
    static const char *brk_src_code[] = {
        "off",
        "on"
    };

    int len = 0;
    int cur_task;
    int i, j;
    int k;
    unsigned long bit;

    len += sprintf(page + off + len, "Task No %d, PC %04x\n", *PP32_DBG_TASK_NO & 0x03, *PP32_DBG_CUR_PC & 0xFFFF);

    if ( !(*PP32_HALT_STAT & 0x01) )
        len += sprintf(page + off + len, "  Halt State: Running\n");
    else
    {
        len += sprintf(page + off + len, "  Halt State: Stopped");
        k = 0;
        for ( bit = 2, i = 0; bit <= (1 << 7); bit <<= 1, i++ )
            if ( (*PP32_HALT_STAT & bit) )
            {
                if ( !k )
                {
                    len += sprintf(page + off + len, ", ");
                    k++;
                }
                else
                    len += sprintf(page + off + len, " | ");
                len += sprintf(page + off + len, halt_stat[i]);
            }

        len += sprintf(page + off + len, "\n");

        cur_task = *PP32_DBG_TASK_NO & 0x03;
        len += sprintf(page + off + len, "General Purpose Register (Task %d):\n", cur_task);
        for ( i = 0; i < 4; i++ )
        {
            for ( j = 0; j < 4; j++ )
                len += sprintf(page + off + len, "   %2d: %08x", i + j * 4, *PP32_DBG_TASK_GPR(cur_task, i + j * 4));
            len += sprintf(page + off + len, "\n");
        }
    }

    len += sprintf(page + off + len, "  Break Src:  data1 - %s, data0 - %s, pc3 - %s, pc2 - %s, pc1 - %s, pc0 - %s\n",
                                                    brk_src_data[(*PP32_BRK_SRC >> 11) & 0x07], brk_src_data[(*PP32_BRK_SRC >> 8) & 0x07], brk_src_code[(*PP32_BRK_SRC >> 3) & 0x01], brk_src_code[(*PP32_BRK_SRC >> 2) & 0x01], brk_src_code[(*PP32_BRK_SRC >> 1) & 0x01], brk_src_code[*PP32_BRK_SRC & 0x01]);

    for ( i = 0; i < 4; i++ )
        len += sprintf(page + off + len, "    pc%d:      %04x - %04x\n", i, *PP32_DBG_PC_MIN(i), *PP32_DBG_PC_MAX(i));

    for ( i = 0; i < 2; i++ )
        len += sprintf(page + off + len, "    data%d:    %04x - %04x (%08x)\n", i, *PP32_DBG_DATA_MIN(i), *PP32_DBG_DATA_MAX(i), *PP32_DBG_DATA_VAL(i));

    *eof = 1;

    return len;
}

static int proc_write_pp32(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int id;
    u32 addr;
    u32 cmd;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(str, "start") == 0 )
        *PP32_DBG_CTRL = DBG_CTRL_START_SET(1);
    else if ( stricmp(str, "stop") == 0 )
        *PP32_DBG_CTRL = DBG_CTRL_STOP_SET(1);
    else if ( stricmp(str, "step") == 0 )
        *PP32_DBG_CTRL = DBG_CTRL_STEP_SET(1);
    else if ( strincmp(p, "pc", 2) == 0 && p[2] >= '0' && p[2] <= '3' && p[3] == ' ' )
    {
        id = (int)(p[2] - '0');
        p += 4;
        rlen -= 4;
        *PP32_BRK_SRC &= ~PP32_BRK_SRC_PC(id);
        if ( stricmp(p, "off") != 0 )
        {
            ignore_space(&p, &rlen);
            *PP32_DBG_PC_MIN(id) = *PP32_DBG_PC_MAX(id) = get_number(&p, &rlen, 1);
            ignore_space(&p, &rlen);
            if ( rlen > 0 )
            {
                addr = get_number(&p, &rlen, 1);
                if ( addr >= *PP32_DBG_PC_MIN(id) )
                    *PP32_DBG_PC_MAX(id) = addr;
                else
                    *PP32_DBG_PC_MIN(id) = addr;
            }
            *PP32_BRK_SRC |= PP32_BRK_SRC_PC(id);
        }
    }
    else if ( strincmp(p, "daddr", 5) == 0 && p[5] >= '0' && p[5] <= '1' && p[6] == ' ' )
    {
        id = (int)(p[5] - '0');
        p += 7;
        rlen -= 7;
        *PP32_BRK_SRC &= ~PP32_BRK_SRC_DATA(id, 7);
        if ( stricmp(p, "off") != 0 )
        {
            ignore_space(&p, &rlen);
            *PP32_DBG_DATA_MIN(id) = *PP32_DBG_DATA_MAX(id) = get_number(&p, &rlen, 1);
            cmd = 1;
            ignore_space(&p, &rlen);
            if ( rlen > 0 && ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) )
            {
                addr = get_number(&p, &rlen, 1);
                if ( addr >= *PP32_DBG_PC_MIN(id) )
                    *PP32_DBG_DATA_MAX(id) = addr;
                else
                    *PP32_DBG_DATA_MIN(id) = addr;
                ignore_space(&p, &rlen);
            }
            if ( *p == 'w' )
                cmd = 2;
            else if ( *p == 'r' && p[1] == 'w' )
            {
                cmd = 3;
                p++;
                rlen--;
            }
            p++;
            rlen--;
            if ( rlen > 0 )
            {
                ignore_space(&p, &rlen);
                if ( (*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F'))
                {
                    *PP32_DBG_DATA_VAL(id) = get_number(&p, &rlen, 1);
                    cmd = 4;
                }
            }
            *PP32_BRK_SRC |= PP32_BRK_SRC_DATA(id, cmd);
        }
    }
    else
    {
        printk("echo \"<command>\" > /proc/driver/ifx_ptm/pp32\n");
        printk("  command:\n");
        printk("    start - run pp32\n");
        printk("    stop  - stop pp32\n");
        printk("    step  - run pp32 with one step only\n");
        printk("    pc0    - pc0 <addr_min [addr_max]>/off, set break point PC0\n");
        printk("    pc1    - pc1 <addr_min [addr_max]>/off, set break point PC1\n");
        printk("    pc2    - pc2 <addr_min [addr_max]>/off, set break point PC2\n");
        printk("    pc3    - pc3 <addr_min [addr_max]>/off, set break point PC3\n");
        printk("    daddr0 - daddr0 <addr_min [addr_max] r/w/rw [value]>/off, set break point data address 0\n");
        printk("    daddr1 - daddr1 <addr_min [addr_max] r/w/rw [value]>/off, set break point data address 1\n");
        printk("    help  - print this screen\n");
    }

    return count;
}

#else

static int proc_read_pp32(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    static const char *stron = " on";
    static const char *stroff = "off";

    int len = 0;
    int cur_context;
    int f_stopped;
    char str[256];
    char strlength;
    int i, j;

    int pp32;

    for ( pp32 = 0; pp32 < NUM_OF_PP32; pp32++ )
    {
        f_stopped = 0;

        len += sprintf(page + off + len, "===== pp32 core %d =====\n", pp32);

  #ifdef CONFIG_VR9
        if ( (*PP32_FREEZE & (1 << (pp32 << 4))) != 0 )
        {
            sprintf(str, "freezed");
            f_stopped = 1;
        }
  #else
        if ( 0 )
        {
        }
  #endif
        else if ( PP32_CPU_USER_STOPPED(pp32) || PP32_CPU_USER_BREAKIN_RCV(pp32) || PP32_CPU_USER_BREAKPOINT_MET(pp32) )
        {
            strlength = 0;
            if ( PP32_CPU_USER_STOPPED(pp32) )
                strlength += sprintf(str + strlength, "stopped");
            if ( PP32_CPU_USER_BREAKPOINT_MET(pp32) )
                strlength += sprintf(str + strlength, strlength ? " | breakpoint" : "breakpoint");
            if ( PP32_CPU_USER_BREAKIN_RCV(pp32) )
                strlength += sprintf(str + strlength, strlength ? " | breakin" : "breakin");
            f_stopped = 1;
        }
        else if ( PP32_CPU_CUR_PC(pp32) == PP32_CPU_CUR_PC(pp32) )
        {
            unsigned int pc_value[64] = {0};

            f_stopped = 1;
            for ( i = 0; f_stopped && i < NUM_ENTITY(pc_value); i++ )
            {
                pc_value[i] = PP32_CPU_CUR_PC(pp32);
                for ( j = 0; j < i; j++ )
                    if ( pc_value[j] != pc_value[i] )
                    {
                        f_stopped = 0;
                        break;
                    }
            }
            if ( f_stopped )
                sprintf(str, "hang");
        }
        if ( !f_stopped )
            sprintf(str, "running");
        cur_context = PP32_BRK_CUR_CONTEXT(pp32);
        len += sprintf(page + off + len, "Context: %d, PC: 0x%04x, %s\n", cur_context, PP32_CPU_CUR_PC(pp32), str);

        if ( PP32_CPU_USER_BREAKPOINT_MET(pp32) )
        {
            strlength = 0;
            if ( PP32_BRK_PC_MET(pp32, 0) )
                strlength += sprintf(str + strlength, "pc0");
            if ( PP32_BRK_PC_MET(pp32, 1) )
                strlength += sprintf(str + strlength, strlength ? " | pc1" : "pc1");
            if ( PP32_BRK_DATA_ADDR_MET(pp32, 0) )
                strlength += sprintf(str + strlength, strlength ? " | daddr0" : "daddr0");
            if ( PP32_BRK_DATA_ADDR_MET(pp32, 1) )
                strlength += sprintf(str + strlength, strlength ? " | daddr1" : "daddr1");
            if ( PP32_BRK_DATA_VALUE_RD_MET(pp32, 0) )
            {
                strlength += sprintf(str + strlength, strlength ? " | rdval0" : "rdval0");
                if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(pp32, 0) )
                {
                    if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(pp32, 0) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(pp32, 0) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_RD_MET(pp32, 1) )
            {
                strlength += sprintf(str + strlength, strlength ? " | rdval1" : "rdval1");
                if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(pp32, 1) )
                {
                    if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(pp32, 1) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(pp32, 1) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_WR_MET(pp32, 0) )
            {
                strlength += sprintf(str + strlength, strlength ? " | wtval0" : "wtval0");
                if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(pp32, 0) )
                {
                    if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(pp32, 0) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(pp32, 0) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_WR_MET(pp32, 1) )
            {
                strlength += sprintf(str + strlength, strlength ? " | wtval1" : "wtval1");
                if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(pp32, 1) )
                {
                    if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(pp32, 1) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(pp32, 1) )
                    strlength += sprintf(str + strlength, " >=");
            }
            len += sprintf(page + off + len, "break reason: %s\n", str);
        }

        if ( f_stopped )
        {
            len += sprintf(page + off + len, "General Purpose Register (Context %d):\n", cur_context);
            for ( i = 0; i < 4; i++ )
            {
                for ( j = 0; j < 4; j++ )
                    len += sprintf(page + off + len, "   %2d: %08x", i + j * 4, *PP32_GP_CONTEXTi_REGn(pp32, cur_context, i + j * 4));
                len += sprintf(page + off + len, "\n");
            }
        }

        len += sprintf(page + off + len, "break out on: break in - %s, stop - %s\n",
                                            PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN(pp32) ? stron : stroff,
                                            PP32_CTRL_OPT_BREAKOUT_ON_STOP(pp32) ? stron : stroff);
        len += sprintf(page + off + len, "     stop on: break in - %s, break point - %s\n",
                                            PP32_CTRL_OPT_STOP_ON_BREAKIN(pp32) ? stron : stroff,
                                            PP32_CTRL_OPT_STOP_ON_BREAKPOINT(pp32) ? stron : stroff);
        len += sprintf(page + off + len, "breakpoint:\n");
        len += sprintf(page + off + len, "     pc0: 0x%08x, %s\n", *PP32_BRK_PC(pp32, 0), PP32_BRK_GRPi_PCn(pp32, 0, 0) ? "group 0" : "off");
        len += sprintf(page + off + len, "     pc1: 0x%08x, %s\n", *PP32_BRK_PC(pp32, 1), PP32_BRK_GRPi_PCn(pp32, 1, 1) ? "group 1" : "off");
        len += sprintf(page + off + len, "  daddr0: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(pp32, 0), PP32_BRK_GRPi_DATA_ADDRn(pp32, 0, 0) ? "group 0" : "off");
        len += sprintf(page + off + len, "  daddr1: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(pp32, 1), PP32_BRK_GRPi_DATA_ADDRn(pp32, 1, 1) ? "group 1" : "off");
        len += sprintf(page + off + len, "  rdval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(pp32, 0));
        len += sprintf(page + off + len, "  rdval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(pp32, 1));
        len += sprintf(page + off + len, "  wrval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(pp32, 0));
        len += sprintf(page + off + len, "  wrval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(pp32, 1));
    }

    *eof = 1;

    return len;
}

static int proc_write_pp32(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int pp32 = 0;
    u32 addr;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    if ( strincmp(p, "pp32 ", 5) == 0 )
    {
        p += 5;
        rlen -= 5;

        while ( rlen > 0 && *p >= '0' && *p <= '9' )
        {
            pp32 += *p - '0';
            p++;
            rlen--;
        }
        while ( rlen > 0 && *p && *p <= ' ' )
        {
            p++;
            rlen--;
        }

        if ( pp32 >= NUM_OF_PP32 )
        {
            printk(KERN_ERR __FILE__ ":%d:%s: incorrect pp32 index - %d\n", __LINE__, __FUNCTION__, pp32);
            return count;
        }
    }

    if ( stricmp(p, "start") == 0 )
    {
  #ifdef CONFIG_AMAZON_SE
        *PP32_CTRL_CMD(pp32) = 0;
  #endif
        *PP32_CTRL_CMD(pp32) = PP32_CTRL_CMD_RESTART;
    }
    else if ( stricmp(p, "stop") == 0 )
    {
  #ifdef CONFIG_AMAZON_SE
        *PP32_CTRL_CMD(pp32) = 0;
  #endif
        *PP32_CTRL_CMD(pp32) = PP32_CTRL_CMD_STOP;
    }
    else if ( stricmp(p, "step") == 0 )
    {
  #ifdef CONFIG_AMAZON_SE
        *PP32_CTRL_CMD(pp32) = 0;
  #endif
        *PP32_CTRL_CMD(pp32) = PP32_CTRL_CMD_STEP;
    }
  #ifdef CONFIG_VR9
    else if ( stricmp(p, "unfreeze") == 0 )
        *PP32_FREEZE &= ~(1 << (pp32 << 4));
    else if ( stricmp(p, "freeze") == 0 )
        *PP32_FREEZE |= 1 << (pp32 << 4);
  #else
    else if ( stricmp(p, "unfreeze") == 0 )
        *PP32_DBG_CTRL(pp32) = DBG_CTRL_RESTART;
    else if ( stricmp(p, "freeze") == 0 )
        *PP32_DBG_CTRL(pp32) = DBG_CTRL_STOP;
  #endif
    else if ( strincmp(p, "pc0 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_PCn_OFF(0, 0);
            *PP32_BRK_PC_MASK(pp32, 0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(pp32, 0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(pp32, 0) = addr;
            *PP32_BRK_PC_MASK(pp32, 0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_PCn_ON(0, 0);
        }
    }
    else if ( strincmp(p, "pc1 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_PCn_OFF(1, 1);
            *PP32_BRK_PC_MASK(pp32, 1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(pp32, 1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(pp32, 1) = addr;
            *PP32_BRK_PC_MASK(pp32, 1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_PCn_ON(1, 1);
        }
    }
    else if ( strincmp(p, "daddr0 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_DATA_ADDRn_OFF(0, 0);
            *PP32_BRK_DATA_ADDR_MASK(pp32, 0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(pp32, 0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(pp32, 0) = addr;
            *PP32_BRK_DATA_ADDR_MASK(pp32, 0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_DATA_ADDRn_ON(0, 0);
        }
    }
    else if ( strincmp(p, "daddr1 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_DATA_ADDRn_OFF(1, 1);
            *PP32_BRK_DATA_ADDR_MASK(pp32, 1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(pp32, 1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(pp32, 1) = addr;
            *PP32_BRK_DATA_ADDR_MASK(pp32, 1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(pp32) = PP32_BRK_GRPi_DATA_ADDRn_ON(1, 1);
        }
    }
    else
    {

        printk("echo \"<command>\" > /proc/driver/ifx_ptm/pp32\n");
        printk("  command:\n");
        printk("    unfreeze - unfreeze pp32\n");
        printk("    freeze   - freeze pp32\n");
        printk("    start    - run pp32\n");
        printk("    stop     - stop pp32\n");
        printk("    step     - run pp32 with one step only\n");
        printk("    pc0      - pc0 <addr>/off, set break point PC0\n");
        printk("    pc1      - pc1 <addr>/off, set break point PC1\n");
        printk("    daddr0   - daddr0 <addr>/off, set break point data address 0\n");
        printk("    daddr1   - daddr1 <addr>/off, set break point data address 1\n");
        printk("    help     - print this screen\n");
    }

    if ( *PP32_BRK_TRIG(pp32) )
        *PP32_CTRL_OPT(pp32) = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON;
    else
        *PP32_CTRL_OPT(pp32) = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF;

    return count;
}

#endif

static int stricmp(const char *p1, const char *p2)
{
    int c1, c2;

    while ( *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

static int strincmp(const char *p1, const char *p2, int n)
{
    int c1 = 0, c2;

    while ( n && *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
        n--;
    }

    return n ? *p1 - *p2 : c1;
}

static int get_token(char **p1, char **p2, int *len, int *colon)
{
    int tlen = 0;

    while ( *len && !((**p1 >= 'A' && **p1 <= 'Z') || (**p1 >= 'a' && **p1<= 'z')) )
    {
        (*p1)++;
        (*len)--;
    }
    if ( !*len )
        return 0;

    if ( *colon )
    {
        *colon = 0;
        *p2 = *p1;
        while ( *len && **p2 > ' ' && **p2 != ',' )
        {
            if ( **p2 == ':' )
            {
                *colon = 1;
                break;
            }
            (*p2)++;
            (*len)--;
            tlen++;
        }
        **p2 = 0;
    }
    else
    {
        *p2 = *p1;
        while ( *len && **p2 > ' ' && **p2 != ',' )
        {
            (*p2)++;
            (*len)--;
            tlen++;
        }
        **p2 = 0;
    }

    return tlen;
}

static int get_number(char **p, int *len, int is_hex)
{
    int ret = 0;
    int n = 0;

    if ( (*p)[0] == '0' && (*p)[1] == 'x' )
    {
        is_hex = 1;
        (*p) += 2;
        (*len) -= 2;
    }

    if ( is_hex )
    {
        while ( *len && ((**p >= '0' && **p <= '9') || (**p >= 'a' && **p <= 'f') || (**p >= 'A' && **p <= 'F')) )
        {
            if ( **p >= '0' && **p <= '9' )
                n = **p - '0';
            else if ( **p >= 'a' && **p <= 'f' )
               n = **p - 'a' + 10;
            else if ( **p >= 'A' && **p <= 'F' )
                n = **p - 'A' + 10;
            ret = (ret << 4) | n;
            (*p)++;
            (*len)--;
        }
    }
    else
    {
        while ( *len && **p >= '0' && **p <= '9' )
        {
            n = **p - '0';
            ret = ret * 10 + n;
            (*p)++;
            (*len)--;
        }
    }

    return ret;
}

static inline void ignore_space(char **p, int *len)
{
    while ( *len && (**p <= ' ' || **p == ':' || **p == '.' || **p == ',') )
    {
        (*p)++;
        (*len)--;
    }
}



/*
 * ####################################
 *           Global Function
 * ####################################
 */



/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

static int __init ifx_ptm_test_init(void)
{
    proc_file_create();

    return 0;
}

static void __exit ifx_ptm_test_exit(void)
{
    proc_file_delete();
}

module_init(ifx_ptm_test_init);
module_exit(ifx_ptm_test_exit);

#endif
