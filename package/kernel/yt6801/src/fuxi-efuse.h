/* SPDX-License-Identifier: GPL-2.0+ */
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

#ifndef __FXGMAC_EFUSE_H__
#define __FXGMAC_EFUSE_H__

bool fxgmac_read_patch_from_efuse_per_index(struct fxgmac_pdata* pdata, u8 index, u32 __far* offset, u32 __far* value); /* read patch per 0-based index. */
bool fxgmac_read_mac_subsys_from_efuse(struct fxgmac_pdata* pdata, u8* mac_addr, u32* subsys, u32* revid);
bool fxgmac_efuse_read_data(struct fxgmac_pdata* pdata, u32 offset, u32 __far* value);

#ifndef COMMENT_UNUSED_CODE_TO_REDUCE_SIZE
bool fxgmac_read_patch_from_efuse(struct fxgmac_pdata* pdata, u32 offset, u32* value); /* read patch per register offset. */
bool fxgmac_write_patch_to_efuse(struct fxgmac_pdata* pdata, u32 offset, u32 value);
bool fxgmac_write_patch_to_efuse_per_index(struct fxgmac_pdata* pdata, u8 index, u32 offset, u32 value);
bool fxgmac_write_mac_subsys_to_efuse(struct fxgmac_pdata* pdata, u8* mac_addr, u32* subsys, u32* revid);
bool fxgmac_write_mac_addr_to_efuse(struct fxgmac_pdata* pdata, u8* mac_addr);
bool fxgmac_read_subsys_from_efuse(struct fxgmac_pdata* pdata, u32* subsys, u32* revid);
bool fxgmac_write_subsys_to_efuse(struct fxgmac_pdata* pdata, u32* subsys, u32* revid);
bool fxgmac_efuse_load(struct fxgmac_pdata* pdata);
bool fxgmac_efuse_write_data(struct fxgmac_pdata* pdata, u32 offset, u32 value);
bool fxgmac_efuse_write_oob(struct fxgmac_pdata* pdata);
bool fxgmac_efuse_write_led(struct fxgmac_pdata* pdata, u32 value);
bool fxgmac_read_led_setting_from_efuse(struct fxgmac_pdata* pdata);
bool fxgmac_write_led_setting_to_efuse(struct fxgmac_pdata* pdata);
#endif

#endif // __FXGMAC_EFUSE_H__
