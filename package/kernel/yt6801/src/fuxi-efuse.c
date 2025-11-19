// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2021 Motor-comm Corporation. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "fuxi-gmac.h"
#include "fuxi-gmac-reg.h"
#include "fuxi-efuse.h"

#ifdef FXGMAC_USE_ADAPTER_HANDLE
#include "fuxi-mp.h"
#endif

bool fxgmac_read_patch_from_efuse_per_index(struct fxgmac_pdata* pdata, u8 index, u32 __far* offset, u32 __far* value) /* read patch per index. */
{
    unsigned int wait, i;
    u32 regval = 0;
    bool succeed = false;

    if (index >= FXGMAC_EFUSE_MAX_ENTRY) {
        FXGMAC_PR("Reading efuse out of range, index %d\n", index);
        return false;
    }

    if (offset) {
        *offset = 0;
    }
    for (i = EFUSE_PATCH_ADDR_START_BYTE; i < EFUSE_PATCH_DATA_START_BYTE; i++) {
        regval = 0;
        regval = FXGMAC_SET_REG_BITS(regval, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_REGION_A_B_LENGTH + index * EFUSE_EACH_PATH_SIZE + i);
        regval = FXGMAC_SET_REG_BITS(regval, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
        regval = FXGMAC_SET_REG_BITS(regval, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_READ);
        writereg(pdata->pAdapter, regval, pdata->base_mem + EFUSE_OP_CTRL_0);
        wait = 1000;
        while (wait--) {
            usleep_range_ex(pdata->pAdapter, 20, 50);
            regval = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
            if (FXGMAC_GET_REG_BITS(regval, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
                succeed = true;
                break;
            }
        }
        if (succeed) {
            if (offset) {
                *offset |= (FXGMAC_GET_REG_BITS(regval, EFUSE_OP_RD_DATA_POS, EFUSE_OP_RD_DATA_LEN) << (i << 3));
            }
        }
        else {
            FXGMAC_PR("Fail to reading efuse Byte%d\n", index * EFUSE_EACH_PATH_SIZE + i);
            return succeed;
        }
    }

    if (value) {
        *value = 0;
    }
    for (i = EFUSE_PATCH_DATA_START_BYTE; i < EFUSE_EACH_PATH_SIZE; i++) {
        regval = 0;
        regval = FXGMAC_SET_REG_BITS(regval, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_REGION_A_B_LENGTH + index * EFUSE_EACH_PATH_SIZE + i);
        regval = FXGMAC_SET_REG_BITS(regval, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
        regval = FXGMAC_SET_REG_BITS(regval, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_READ);
        writereg(pdata->pAdapter, regval, pdata->base_mem + EFUSE_OP_CTRL_0);
        wait = 1000;
        while (wait--) {
            usleep_range_ex(pdata->pAdapter, 20, 50);
            regval = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
            if (FXGMAC_GET_REG_BITS(regval, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
                succeed = true;
                break;
            }
        }
        if (succeed) {
            if (value) {
                *value |= (FXGMAC_GET_REG_BITS(regval, EFUSE_OP_RD_DATA_POS, EFUSE_OP_RD_DATA_LEN) << ((i - 2) << 3));
            }
        }
        else {
            FXGMAC_PR("Fail to reading efuse Byte%d\n", index * EFUSE_EACH_PATH_SIZE + i);
            return succeed;
        }
    }

    return succeed;
}

bool fxgmac_read_mac_subsys_from_efuse(struct fxgmac_pdata* pdata, u8* mac_addr, u32* subsys, u32* revid)
{
    u32 offset = 0, value = 0;
    u32 machr = 0, maclr = 0;
    bool succeed = true;
    u8 index = 0;

    for (index = 0; index < FXGMAC_EFUSE_MAX_ENTRY; index++) {
        if (!fxgmac_read_patch_from_efuse_per_index(pdata, index, &offset, &value)) {
            succeed = false;
            break; // reach the last item.
        }
        if (0x00 == offset) {
            break; // reach the blank.
        }
        if (MACA0LR_FROM_EFUSE == offset) {
            maclr = value;
        }
        if (MACA0HR_FROM_EFUSE == offset) {
            machr = value;
        }

        if ((0x08 == offset) && revid) {
            *revid = value;
        }
        if ((0x2C == offset) && subsys) {
            *subsys = value;
        }
    }
    if (mac_addr) {
        mac_addr[5] = (u8)(maclr & 0xFF);
        mac_addr[4] = (u8)((maclr >> 8) & 0xFF);
        mac_addr[3] = (u8)((maclr >> 16) & 0xFF);
        mac_addr[2] = (u8)((maclr >> 24) & 0xFF);
        mac_addr[1] = (u8)(machr & 0xFF);
        mac_addr[0] = (u8)((machr >> 8) & 0xFF);
    }

    return succeed;
}

bool fxgmac_efuse_read_data(struct fxgmac_pdata* pdata, u32 offset, u32 __far* value)
{
    bool succeed = false;
    unsigned int wait;
    u32 reg_val = 0;

    //if (reg >= EFUSE_REGION_A_B_LENGTH) {
    //    FXGMAC_PR("Read addr out of range %d", reg);
    //    return succeed;
    //}

    if (value) {
        *value = 0;
    }

    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, offset);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_READ);
    writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);
    wait = 1000;
    while (wait--) {
        usleep_range_ex(pdata->pAdapter, 20, 50);
        reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
        if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
            succeed = true;
            break;
        }
    }

    if (succeed) {
        if (value) {
            *value = FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_RD_DATA_POS, EFUSE_OP_RD_DATA_LEN);
        }
    }
    else {
        FXGMAC_PR("Fail to reading efuse Byte%d\n", offset);
    }

    return succeed;
}

#ifndef COMMENT_UNUSED_CODE_TO_REDUCE_SIZE
bool fxgmac_read_patch_from_efuse(struct fxgmac_pdata* pdata, u32 offset, u32* value)
{
    u32 reg_offset, reg_val;
    u32 cur_val = 0;
    bool succeed = true;
    u8 index = 0;

    if(offset >> 16){
        FXGMAC_PR("Reading efuse out of range, reg %d. reg must be 2bytes.\n", index);
        return false;
    }

    for (index = 0; index < FXGMAC_EFUSE_MAX_ENTRY; index++) {
        if (!fxgmac_read_patch_from_efuse_per_index(pdata, index, &reg_offset, &reg_val)) {
            succeed = false;
            break;
        } else if (reg_offset == offset) {
            cur_val = reg_val;
        } else if (0 == reg_offset && 0 == reg_val) {
            break; // first blank. We should write here.
        }
    }

    if (value) {
        *value = cur_val;
    }
    
    return succeed;
}

bool fxgmac_write_patch_to_efuse_per_index(struct fxgmac_pdata* pdata, u8 index, u32 offset, u32 value)
{
    unsigned int    wait, i;
    u32             reg_val;
    bool            succeed = false;
    u32             cur_reg, cur_val;
    u8              max_index = FXGMAC_EFUSE_MAX_ENTRY;

    if(offset >> 16){
        FXGMAC_PR("Reading efuse out of range, reg %d. reg must be 2bytes.\n", index);
        return false;
    }

    fxgmac_efuse_read_data(pdata, EFUSE_LED_ADDR, &reg_val);
    if (EFUSE_LED_COMMON_SOLUTION == reg_val) {
        max_index = FXGMAC_EFUSE_MAX_ENTRY_UNDER_LED_COMMON;
    }

    if (index >= max_index) {
        FXGMAC_PR("Writing efuse out of range, index %d max index %d\n", index, max_index);
        return false;
    }

    if (fxgmac_read_patch_from_efuse_per_index(pdata, index, &cur_reg, &cur_val)) {
        if(cur_reg != 0 || cur_val != 0){
            FXGMAC_PR(" The index %d has writed value, cannot rewrite it.\n", index);
            return false;
        }
    }else{
        FXGMAC_PR("Cannot read index %d.\n", index);
        return false;
    }

    for (i = EFUSE_PATCH_ADDR_START_BYTE; i < EFUSE_PATCH_DATA_START_BYTE; i++) {
        reg_val = 0;
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_REGION_A_B_LENGTH + index * EFUSE_EACH_PATH_SIZE + i);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, (offset >> (i << 3)) & 0xFF);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
        writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

        succeed = false;
        wait = 1000;
        while (wait--) {
            usleep_range_ex(pdata->pAdapter, 20, 50);
            reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
            if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
                succeed = true;
                break;
            }
        }
        if (!succeed) {
            FXGMAC_PR("Fail to writing efuse Byte%d\n", index * EFUSE_EACH_PATH_SIZE + i);
            return succeed;
        }
    }

    for (i = 2; i < 6; i++) {
        reg_val = 0;
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, 18 + index * 6 + i);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, (value >> ((i - 2) << 3)) & 0xFF);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
        writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

        succeed = false;
        wait = 1000;
        while (wait--) {
            usleep_range_ex(pdata->pAdapter, 20, 50);
            reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
            if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
                succeed = true;
                break;
            }
        }
        if (!succeed) {
            FXGMAC_PR("Fail to writing efuse Byte%d\n", index * EFUSE_EACH_PATH_SIZE + i);
            return succeed;
        }
    }

    return succeed;
}

bool fxgmac_write_patch_to_efuse(struct fxgmac_pdata* pdata, u32 offset, u32 value)
{
    unsigned int wait, i;
    u32 reg_offset, reg_val;
    u32 cur_offset = 0, cur_val = 0;
    bool succeed = false;
    u8 index = 0;

    if(offset >> 16){
        FXGMAC_PR("Reading efuse out of range, reg %d. reg must be 2bytes.\n", index);
        return false;
    }

    for (index = 0; index < FXGMAC_EFUSE_MAX_ENTRY; index++) {
        if (!fxgmac_read_patch_from_efuse_per_index(pdata, index, &reg_offset, &reg_val)) {
            return false;
        } else if (reg_offset == offset) {
            cur_offset = reg_offset;
            cur_val = reg_val;
        } else if (0 == reg_offset && 0 == reg_val) {
            break; // first blank. We should write here.
        }
    }

    if (cur_offset == offset) {
        if (cur_val == value) {
            FXGMAC_PR("0x%x -> Reg0x%x already exists, ignore.\n", value, offset);
            return true;
        } else {
            FXGMAC_PR("Reg0x%x entry current value 0x%x, reprogram.\n", offset, value);
        }
    }

    for (i = EFUSE_PATCH_ADDR_START_BYTE; i < EFUSE_PATCH_DATA_START_BYTE; i++) {
        reg_val = 0;
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_REGION_A_B_LENGTH + index * EFUSE_EACH_PATH_SIZE + i);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, (offset >> (i << 3)) & 0xFF );
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
        writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

        succeed = false;
        wait = 1000;
        while (wait--) {
            usleep_range_ex(pdata->pAdapter, 20, 50);
            reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
            if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
                succeed = true;
                break;
            }
        }
        if (!succeed) {
            FXGMAC_PR("Fail to writing efuse Byte%d\n", index * EFUSE_EACH_PATH_SIZE + i);
            return succeed;
        }
    }

    for (i = EFUSE_PATCH_DATA_START_BYTE; i < EFUSE_EACH_PATH_SIZE; i++) {
        reg_val = 0;
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_REGION_A_B_LENGTH + index * EFUSE_EACH_PATH_SIZE + i);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, (value >> ((i - 2) << 3)) & 0xFF);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
        reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
        writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

        succeed = false;
        wait = 1000;
        while (wait--) {
            usleep_range_ex(pdata->pAdapter, 20, 50);
            reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
            if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
                succeed = true;
                break;
            }
        }
        if (!succeed) {
            FXGMAC_PR("Fail to writing efuse Byte%d\n", index * EFUSE_EACH_PATH_SIZE + i);
            return succeed;
        }
    }

    return succeed;
}

bool fxgmac_write_mac_subsys_to_efuse(struct fxgmac_pdata* pdata, u8* mac_addr, u32* subsys, u32* revid)
{
#ifdef DBG
    u32 machr = 0, maclr = 0;
#endif
    u32 cur_subsysid = 0;
    u32 pcie_cfg_ctrl= PCIE_CFG_CTRL_DEFAULT_VAL;
    bool succeed = true;
    if (mac_addr) {
#ifdef DBG
        machr = readreg(pdata->pAdapter, pdata->base_mem + MACA0HR_FROM_EFUSE);
        maclr = readreg(pdata->pAdapter, pdata->base_mem + MACA0LR_FROM_EFUSE);
        DPRINTK("Current mac address from efuse is %02x-%02x-%02x-%02x-%02x-%02x.\n", 
            (machr >> 8) & 0xFF, machr & 0xFF, (maclr >> 24) & 0xFF, (maclr >> 16) & 0xFF, (maclr >> 8) & 0xFF, maclr & 0xFF);
#endif
        if(!fxgmac_write_patch_to_efuse(pdata, MACA0HR_FROM_EFUSE, (((u32)mac_addr[0]) << 8) | mac_addr[1])){
            succeed = false;
        }
        if(!fxgmac_write_patch_to_efuse(pdata, MACA0LR_FROM_EFUSE, (((u32)mac_addr[2]) << 24) | (((u32)mac_addr[3]) << 16) | (((u32)mac_addr[4]) << 8) | mac_addr[5])){
            succeed = false;
        }
    }

    if (revid) {
        if(!fxgmac_write_patch_to_efuse(pdata, EFUSE_REVID_REGISTER, *revid)){
            succeed = false;
        }
    }
    if (subsys) {
        if (!fxgmac_read_mac_subsys_from_efuse(pdata, NULL, &cur_subsysid, NULL))
            return false;

        if (cur_subsysid != *subsys)
        {           
            pcie_cfg_ctrl = FXGMAC_SET_REG_BITS(pcie_cfg_ctrl, MGMT_PCIE_CFG_CTRL_CS_EN_POS, MGMT_PCIE_CFG_CTRL_CS_EN_LEN, 1);
            if (!fxgmac_write_patch_to_efuse(pdata, MGMT_PCIE_CFG_CTRL, pcie_cfg_ctrl)) {
                succeed = false;
            }
            if (!fxgmac_write_patch_to_efuse(pdata, EFUSE_SUBSYS_REGISTER, *subsys)) {
                succeed = false;
            }
            pcie_cfg_ctrl = FXGMAC_SET_REG_BITS(pcie_cfg_ctrl, MGMT_PCIE_CFG_CTRL_CS_EN_POS, MGMT_PCIE_CFG_CTRL_CS_EN_LEN, 0);
            if (!fxgmac_write_patch_to_efuse(pdata, MGMT_PCIE_CFG_CTRL, pcie_cfg_ctrl)) {
                succeed = false;
            }
        }
    }
    return succeed;
}

bool fxgmac_write_mac_addr_to_efuse(struct fxgmac_pdata* pdata, u8* mac_addr)
{
#ifdef DBG
    u32 machr = 0, maclr = 0;
#endif
    bool succeed = true;

    if (mac_addr) {
#ifdef DBG
        machr = readreg(pdata->pAdapter, pdata->base_mem + MACA0HR_FROM_EFUSE);
        maclr = readreg(pdata->pAdapter, pdata->base_mem + MACA0LR_FROM_EFUSE);
        DPRINTK("Current mac address from efuse is %02x-%02x-%02x-%02x-%02x-%02x.\n", 
            (machr >> 8) & 0xFF, machr & 0xFF, (maclr >> 24) & 0xFF, (maclr >> 16) & 0xFF, (maclr >> 8) & 0xFF, maclr & 0xFF);
#endif
        if(!fxgmac_write_patch_to_efuse(pdata, MACA0HR_FROM_EFUSE, (((u32)mac_addr[0]) << 8) | mac_addr[1])){
            succeed = false;
        }
        if(!fxgmac_write_patch_to_efuse(pdata, MACA0LR_FROM_EFUSE, (((u32)mac_addr[2]) << 24) | (((u32)mac_addr[3]) << 16) | (((u32)mac_addr[4]) << 8) | mac_addr[5])){
            succeed = false;
        }
    }

    return succeed;
}

bool fxgmac_read_subsys_from_efuse(struct fxgmac_pdata* pdata, u32* subsys, u32* revid)
{
    u32 offset = 0, value = 0;
    u8 index;
    bool succeed = true;

    for (index = 0; index < FXGMAC_EFUSE_MAX_ENTRY; index++) {
        if (!fxgmac_read_patch_from_efuse_per_index(pdata, index, &offset, &value)) {
            succeed = false;
            break; // reach the last item.
        }
        if (0x00 == offset) {
            break; // reach the blank.
        }

        if ((EFUSE_REVID_REGISTER == offset) && revid) {
            *revid = value;
        }else{
            succeed = false;
        }
        if ((EFUSE_SUBSYS_REGISTER == offset) && subsys) {
            *subsys = value;
        }else{
            succeed = false;
        }
    }
    
    return succeed;
}

bool fxgmac_write_subsys_to_efuse(struct fxgmac_pdata* pdata, u32* subsys, u32* revid)
{
    bool succeed = true;

    /* write subsys info */
    if (revid) {
        if(!fxgmac_write_patch_to_efuse(pdata, EFUSE_REVID_REGISTER, *revid)){
            succeed = false;
        }
    }
    if (subsys) {
        if(!fxgmac_write_patch_to_efuse(pdata, EFUSE_SUBSYS_REGISTER, *subsys)){
            succeed = false;
        }
    }
    return succeed;
}

bool  fxgmac_efuse_load(struct fxgmac_pdata* pdata)
{
    bool succeed = false;
    unsigned int wait;
    u32 reg_val = 0;
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_AUTO_LOAD);
    writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

    wait = 1000;
    while (wait--) {
        usleep_range_ex(pdata->pAdapter, 20, 50);
        reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
        if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
            succeed = true;
            break;
        }
    }
    if (!succeed) {
        FXGMAC_PR("Fail to loading efuse, ctrl_1 0x%08x\n", reg_val);
    }
    return succeed;
}

bool fxgmac_efuse_write_oob(struct fxgmac_pdata* pdata)
{
    bool succeed = false;
    unsigned int wait;
    u32 reg_val, value;

    if (!fxgmac_efuse_read_data(pdata, EFUSE_OOB_ADDR, &reg_val)) {
        return succeed;
    }

    if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OOB_POS, EFUSE_OOB_LEN)) {
        FXGMAC_PR("OOB Ctrl bit already exists");
        return true;
    }

    value = 0;
    value = FXGMAC_SET_REG_BITS(value, EFUSE_OOB_POS, EFUSE_OOB_LEN, 1);

    reg_val = 0;
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_OOB_ADDR);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, value & 0xFF);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
    writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

    wait = 1000;
    while (wait--) {
        usleep_range_ex(pdata->pAdapter, 20, 50);
        reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
        if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
            succeed = true;
            break;
        }
    }

    if (!succeed) {
        FXGMAC_PR("Fail to writing efuse Byte OOB");
    }

    return succeed;
}

bool fxgmac_efuse_write_led(struct fxgmac_pdata* pdata, u32 value)
{
    bool succeed = false;
    unsigned int wait;
    u32 reg_val;

    if (!fxgmac_efuse_read_data(pdata, EFUSE_LED_ADDR, &reg_val)) {
        return succeed;
    }

    if (reg_val == value) {
        FXGMAC_PR("Led Ctrl option already exists");
        return true;
    }

    reg_val = 0;
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, EFUSE_LED_ADDR);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, value & 0xFF);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
    writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

    wait = 1000;
    while (wait--) {
        usleep_range_ex(pdata->pAdapter, 20, 50);
        reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
        if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
            succeed = true;
            break;
        }
    }

    if (!succeed) {
        FXGMAC_PR("Fail to writing efuse Byte LED");
    }

    return succeed;
}

bool fxgmac_efuse_write_data(struct fxgmac_pdata* pdata, u32 offset, u32 value)
{
    bool succeed = false;
    unsigned int wait;
    u32 reg_val;

    if (!fxgmac_efuse_read_data(pdata, offset, &reg_val)) {
        return succeed;
    }

    if (reg_val == value) {
        FXGMAC_PR("offset 0x%x already exists", offset);
        return true;
    }

    reg_val = 0;
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_ADDR_POS, EFUSE_OP_ADDR_LEN, offset & 0xFF);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_WR_DATA_POS, EFUSE_OP_WR_DATA_LEN, value & 0xFF);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_START_POS, EFUSE_OP_START_LEN, 1);
    reg_val = FXGMAC_SET_REG_BITS(reg_val, EFUSE_OP_MODE_POS, EFUSE_OP_MODE_LEN, EFUSE_OP_MODE_ROW_WRITE);
    writereg(pdata->pAdapter, reg_val, pdata->base_mem + EFUSE_OP_CTRL_0);

    wait = 1000;
    while (wait--) {
        usleep_range_ex(pdata->pAdapter, 20, 50);
        reg_val = readreg(pdata->pAdapter, pdata->base_mem + EFUSE_OP_CTRL_1);
        if (FXGMAC_GET_REG_BITS(reg_val, EFUSE_OP_DONE_POS, EFUSE_OP_DONE_LEN)) {
            succeed = true;
            break;
        }
    }

    if (!succeed) {
        FXGMAC_PR("Fail to writing efuse 0x%x Byte LED", offset);
    }

    return succeed;
}

static void fxgmac_read_led_efuse_config(struct fxgmac_pdata* pdata, struct led_setting* pfirst, struct led_setting* psecond)
{
    u32 val_high = 0, val_low = 0;

    //read first area
    fxgmac_efuse_read_data(pdata, EFUSE_FISRT_UPDATE_ADDR, &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 1), &val_low);
    pfirst->disable_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 2), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 3), &val_low);
    pfirst->disable_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 4), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 5), &val_low);
    pfirst->disable_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 6), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 7), &val_low);
    pfirst->disable_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 8), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 9), &val_low);
    pfirst->disable_led_setting[0] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 10), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 11), &val_low);
    pfirst->s5_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 12), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 13), &val_low);
    pfirst->s5_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 14), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 15), &val_low);
    pfirst->s5_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 16), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 17), &val_low);
    pfirst->s5_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 18), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 19), &val_low);
    pfirst->s5_led_setting[0] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 20), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 21), &val_low);
    pfirst->s3_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 22), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 23), &val_low);
    pfirst->s3_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 24), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 25), &val_low);
    pfirst->s3_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 26), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 27), &val_low);
    pfirst->s3_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 28), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 29), &val_low);
    pfirst->s3_led_setting[0] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 30), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 31), &val_low);
    pfirst->s0_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 32), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 33), &val_low);
    pfirst->s0_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 34), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 35), &val_low);
    pfirst->s0_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 36), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 37), &val_low);
    pfirst->s0_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 38), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 39), &val_low);
    pfirst->s0_led_setting[0] = ((val_high << 8) + val_low);

    //read second area
    fxgmac_efuse_read_data(pdata, EFUSE_SECOND_UPDATE_ADDR, &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 1), &val_low);
    psecond->disable_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 2), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 3), &val_low);
    psecond->disable_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 4), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 5), &val_low);
    psecond->disable_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 6), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 7), &val_low);
    psecond->disable_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 8), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 9), &val_low);
    psecond->disable_led_setting[0] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 10), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 11), &val_low);
    psecond->s5_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 12), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 13), &val_low);
    psecond->s5_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 14), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 15), &val_low);
    psecond->s5_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 16), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 17), &val_low);
    psecond->s5_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 18), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 19), &val_low);
    psecond->s5_led_setting[0] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 20), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 21), &val_low);
    psecond->s3_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 22), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 23), &val_low);
    psecond->s3_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 24), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 25), &val_low);
    psecond->s3_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 26), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 27), &val_low);
    psecond->s3_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 28), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 29), &val_low);
    psecond->s3_led_setting[0] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 30), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 31), &val_low);
    psecond->s0_led_setting[4] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 32), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 33), &val_low);
    psecond->s0_led_setting[3] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 34), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 35), &val_low);
    psecond->s0_led_setting[2] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 36), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 37), &val_low);
    psecond->s0_led_setting[1] = ((val_high << 8) + val_low);

    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 38), &val_high);
    fxgmac_efuse_read_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 39), &val_low);
    psecond->s0_led_setting[0] = ((val_high << 8) + val_low);
}

bool fxgmac_write_led_setting_to_efuse(struct fxgmac_pdata* pdata)
{
    struct led_setting led_config_first;
    struct led_setting led_config_second;
    bool bfirstflag = false, bsecondflag = false;
    bool bsucceed = false;

    fxgmac_read_led_efuse_config(pdata, &led_config_first, &led_config_second);

    if (0x00 == led_config_first.s0_led_setting[0] && 0x00 == led_config_first.s0_led_setting[1] && 0x00 == led_config_first.s0_led_setting[2] && 0x00 == led_config_first.s0_led_setting[3] && 0x00 == led_config_first.s0_led_setting[4]
        && 0x00 == led_config_first.s3_led_setting[0] && 0x00 == led_config_first.s3_led_setting[1] && 0x00 == led_config_first.s3_led_setting[2] && 0x00 == led_config_first.s3_led_setting[3] && 0x00 == led_config_first.s3_led_setting[4]
        && 0x00 == led_config_first.s5_led_setting[0] && 0x00 == led_config_first.s5_led_setting[1] && 0x00 == led_config_first.s5_led_setting[2] && 0x00 == led_config_first.s5_led_setting[3] && 0x00 == led_config_first.s5_led_setting[4]
        && 0x00 == led_config_first.disable_led_setting[0] && 0x00 == led_config_first.disable_led_setting[1] && 0x00 == led_config_first.disable_led_setting[2] && 0x00 == led_config_first.disable_led_setting[3] && 0x00 == led_config_first.disable_led_setting[4]
        ) {
        bfirstflag = true;
    }

    if (0x00 == led_config_second.s0_led_setting[0] && 0x00 == led_config_second.s0_led_setting[1] && 0x00 == led_config_second.s0_led_setting[2] && 0x00 == led_config_second.s0_led_setting[3] && 0x00 == led_config_second.s0_led_setting[4]
        && 0x00 == led_config_second.s3_led_setting[0] && 0x00 == led_config_second.s3_led_setting[1] && 0x00 == led_config_second.s3_led_setting[2] && 0x00 == led_config_second.s3_led_setting[3] && 0x00 == led_config_second.s3_led_setting[4]
        && 0x00 == led_config_second.s5_led_setting[0] && 0x00 == led_config_second.s5_led_setting[1] && 0x00 == led_config_second.s5_led_setting[2] && 0x00 == led_config_second.s5_led_setting[3] && 0x00 == led_config_second.s5_led_setting[4]
        && 0x00 == led_config_second.disable_led_setting[0] && 0x00 == led_config_second.disable_led_setting[1] && 0x00 == led_config_second.disable_led_setting[2] && 0x00 == led_config_second.disable_led_setting[3] && 0x00 == led_config_second.disable_led_setting[4]
        ) {
        bsecondflag = true;
    }

#ifndef LINUX
    DbgPrintF(MP_TRACE, "%s s0 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->ledconfig.s0_led_setting[0], pdata->ledconfig.s0_led_setting[1], pdata->ledconfig.s0_led_setting[2], pdata->ledconfig.s0_led_setting[3], pdata->ledconfig.s0_led_setting[4]);
    DbgPrintF(MP_TRACE, "%s s3 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->ledconfig.s3_led_setting[0], pdata->ledconfig.s3_led_setting[1], pdata->ledconfig.s3_led_setting[2], pdata->ledconfig.s3_led_setting[3], pdata->ledconfig.s3_led_setting[4]);
    DbgPrintF(MP_TRACE, "%s s5 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->ledconfig.s5_led_setting[0], pdata->ledconfig.s5_led_setting[1], pdata->ledconfig.s5_led_setting[2], pdata->ledconfig.s5_led_setting[3], pdata->ledconfig.s5_led_setting[4]);
    DbgPrintF(MP_TRACE, "%s disable 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->ledconfig.disable_led_setting[0], pdata->ledconfig.disable_led_setting[1], pdata->ledconfig.disable_led_setting[2], pdata->ledconfig.disable_led_setting[3], pdata->ledconfig.disable_led_setting[4]);
#endif

    if (bfirstflag && bsecondflag) {
        //update first area
        fxgmac_efuse_write_data(pdata, EFUSE_FISRT_UPDATE_ADDR, (pdata->ledconfig.disable_led_setting[4]>>8)&0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 1), pdata->ledconfig.disable_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 2), (pdata->ledconfig.disable_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 3), pdata->ledconfig.disable_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 4), (pdata->ledconfig.disable_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 5), pdata->ledconfig.disable_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 6), (pdata->ledconfig.disable_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 7), pdata->ledconfig.disable_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 8), (pdata->ledconfig.disable_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 9), pdata->ledconfig.disable_led_setting[0]);

        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 10), (pdata->ledconfig.s5_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 11), pdata->ledconfig.s5_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 12), (pdata->ledconfig.s5_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 13), pdata->ledconfig.s5_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 14), (pdata->ledconfig.s5_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 15), pdata->ledconfig.s5_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 16), (pdata->ledconfig.s5_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 17), pdata->ledconfig.s5_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 18), (pdata->ledconfig.s5_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 19), pdata->ledconfig.s5_led_setting[0]);

        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 20), (pdata->ledconfig.s3_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 21), pdata->ledconfig.s3_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 22), (pdata->ledconfig.s3_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 23), pdata->ledconfig.s3_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 24), (pdata->ledconfig.s3_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 25), pdata->ledconfig.s3_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 26), (pdata->ledconfig.s3_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 27), pdata->ledconfig.s3_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 28), (pdata->ledconfig.s3_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 29), pdata->ledconfig.s3_led_setting[0]);

        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 30), (pdata->ledconfig.s0_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 31), pdata->ledconfig.s0_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 32), (pdata->ledconfig.s0_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 33), pdata->ledconfig.s0_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 34), (pdata->ledconfig.s0_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 35), pdata->ledconfig.s0_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 36), (pdata->ledconfig.s0_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 37), pdata->ledconfig.s0_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 38), (pdata->ledconfig.s0_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_FISRT_UPDATE_ADDR - 39), pdata->ledconfig.s0_led_setting[0]);

        bsucceed = true;
    }
    else if (!bfirstflag && bsecondflag) {
        //update second area
        fxgmac_efuse_write_data(pdata, EFUSE_SECOND_UPDATE_ADDR, (pdata->ledconfig.disable_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 1), pdata->ledconfig.disable_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 2), (pdata->ledconfig.disable_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 3), pdata->ledconfig.disable_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 4), (pdata->ledconfig.disable_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 5), pdata->ledconfig.disable_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 6), (pdata->ledconfig.disable_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 7), pdata->ledconfig.disable_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 8), (pdata->ledconfig.disable_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 9), pdata->ledconfig.disable_led_setting[0]);

        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 10), (pdata->ledconfig.s5_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 11), pdata->ledconfig.s5_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 12), (pdata->ledconfig.s5_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 13), pdata->ledconfig.s5_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 14), (pdata->ledconfig.s5_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 15), pdata->ledconfig.s5_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 16), (pdata->ledconfig.s5_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 17), pdata->ledconfig.s5_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 18), (pdata->ledconfig.s5_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 19), pdata->ledconfig.s5_led_setting[0]);

        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 20), (pdata->ledconfig.s3_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 21), pdata->ledconfig.s3_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 22), (pdata->ledconfig.s3_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 23), pdata->ledconfig.s3_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 24), (pdata->ledconfig.s3_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 25), pdata->ledconfig.s3_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 26), (pdata->ledconfig.s3_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 27), pdata->ledconfig.s3_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 28), (pdata->ledconfig.s3_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 29), pdata->ledconfig.s3_led_setting[0]);

        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 30), (pdata->ledconfig.s0_led_setting[4] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 31), pdata->ledconfig.s0_led_setting[4]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 32), (pdata->ledconfig.s0_led_setting[3] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 33), pdata->ledconfig.s0_led_setting[3]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 34), (pdata->ledconfig.s0_led_setting[2] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 35), pdata->ledconfig.s0_led_setting[2]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 36), (pdata->ledconfig.s0_led_setting[1] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 37), pdata->ledconfig.s0_led_setting[1]);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 38), (pdata->ledconfig.s0_led_setting[0] >> 8) & 0xFF);
        fxgmac_efuse_write_data(pdata, (EFUSE_SECOND_UPDATE_ADDR - 39), pdata->ledconfig.s0_led_setting[0]);

        bsucceed = true;
    }

    return bsucceed;
}

bool fxgmac_read_led_setting_from_efuse(struct fxgmac_pdata* pdata)
{
    struct led_setting led_config_first;
    struct led_setting led_config_second;
    bool bfirstflag = false, bsecondflag = false;
    bool bsucceed = false;

    fxgmac_read_led_efuse_config(pdata, &led_config_first, &led_config_second);

    if (0x00 == led_config_first.s0_led_setting[0] && 0x00 == led_config_first.s0_led_setting[1] && 0x00 == led_config_first.s0_led_setting[2] && 0x00 == led_config_first.s0_led_setting[3] && 0x00 == led_config_first.s0_led_setting[4]
        && 0x00 == led_config_first.s3_led_setting[0] && 0x00 == led_config_first.s3_led_setting[1] && 0x00 == led_config_first.s3_led_setting[2] && 0x00 == led_config_first.s3_led_setting[3] && 0x00 == led_config_first.s3_led_setting[4]
        && 0x00 == led_config_first.s5_led_setting[0] && 0x00 == led_config_first.s5_led_setting[1] && 0x00 == led_config_first.s5_led_setting[2] && 0x00 == led_config_first.s5_led_setting[3] && 0x00 == led_config_first.s5_led_setting[4]
        && 0x00 == led_config_first.disable_led_setting[0] && 0x00 == led_config_first.disable_led_setting[1] && 0x00 == led_config_first.disable_led_setting[2] && 0x00 == led_config_first.disable_led_setting[3] && 0x00 == led_config_first.disable_led_setting[4]
        ) {
        bfirstflag = true;
    }

    if (0x00 == led_config_second.s0_led_setting[0] && 0x00 == led_config_second.s0_led_setting[1] && 0x00 == led_config_second.s0_led_setting[2] && 0x00 == led_config_second.s0_led_setting[3] && 0x00 == led_config_second.s0_led_setting[4]
        && 0x00 == led_config_second.s3_led_setting[0] && 0x00 == led_config_second.s3_led_setting[1] && 0x00 == led_config_second.s3_led_setting[2] && 0x00 == led_config_second.s3_led_setting[3] && 0x00 == led_config_second.s3_led_setting[4]
        && 0x00 == led_config_second.s5_led_setting[0] && 0x00 == led_config_second.s5_led_setting[1] && 0x00 == led_config_second.s5_led_setting[2] && 0x00 == led_config_second.s5_led_setting[3] && 0x00 == led_config_second.s5_led_setting[4]
        && 0x00 == led_config_second.disable_led_setting[0] && 0x00 == led_config_second.disable_led_setting[1] && 0x00 == led_config_second.disable_led_setting[2] && 0x00 == led_config_second.disable_led_setting[3] && 0x00 == led_config_second.disable_led_setting[4]
        ) {
        bsecondflag = true;
    }

    if (!bfirstflag && bsecondflag) {
        //read first area
        memcpy(&pdata->led, &led_config_first, sizeof(struct led_setting));
        bsucceed = true;
    }
    else if (!bfirstflag && !bsecondflag) {
        //read second area
        memcpy(&pdata->led, &led_config_second, sizeof(struct led_setting));
        bsucceed = true;
    }

#ifndef LINUX
    DbgPrintF(MP_TRACE, "%s s0 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->led.s0_led_setting[0], pdata->led.s0_led_setting[1], pdata->led.s0_led_setting[2], pdata->led.s0_led_setting[3], pdata->led.s0_led_setting[4]);
    DbgPrintF(MP_TRACE, "%s s3 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->led.s3_led_setting[0], pdata->led.s3_led_setting[1], pdata->led.s3_led_setting[2], pdata->led.s3_led_setting[3], pdata->led.s3_led_setting[4]);
    DbgPrintF(MP_TRACE, "%s s5 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->led.s5_led_setting[0], pdata->led.s5_led_setting[1], pdata->led.s5_led_setting[2], pdata->led.s5_led_setting[3], pdata->led.s5_led_setting[4]);
    DbgPrintF(MP_TRACE, "%s disable 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x", __FUNCTION__, pdata->led.disable_led_setting[0], pdata->led.disable_led_setting[1], pdata->led.disable_led_setting[2], pdata->led.disable_led_setting[3], pdata->led.disable_led_setting[4]);
#endif

    return bsucceed;
}
#endif
