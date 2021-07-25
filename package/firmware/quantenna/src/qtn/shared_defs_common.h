/*SH1
*******************************************************************************
**                                                                           **
**         Copyright (c) 2014 Quantenna Communications Inc                   **
**                            All Rights Reserved                            **
**                                                                           **
**  Author      : Quantenna Communications Inc                               **
**  File        : shared_defs.h                                              **
**  Description :                                                            **
**                                                                           **
*******************************************************************************
**                                                                           **
**  Redistribution and use in source and binary forms, with or without       **
**  modification, are permitted provided that the following conditions       **
**  are met:                                                                 **
**  1. Redistributions of source code must retain the above copyright        **
**     notice, this list of conditions and the following disclaimer.         **
**  2. Redistributions in binary form must reproduce the above copyright     **
**     notice, this list of conditions and the following disclaimer in the   **
**     documentation and/or other materials provided with the distribution.  **
**  3. The name of the author may not be used to endorse or promote products **
**     derived from this software without specific prior written permission. **
**                                                                           **
**  Alternatively, this software may be distributed under the terms of the   **
**  GNU General Public License ("GPL") version 2, or (at your option) any    **
**  later version as published by the Free Software Foundation.              **
**                                                                           **
**  In the case this software is distributed under the GPL license,          **
**  you should have received a copy of the GNU General Public License        **
**  along with this software; if not, write to the Free Software             **
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  **
**                                                                           **
**  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR       **
**  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES**
**  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  **
**  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,         **
**  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT **
**  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,**
**  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    **
**  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT      **
**  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF **
**  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.        **
**                                                                           **
*******************************************************************************
EH1*/

#ifndef _SHARED_DEFS_COMMON_H_
#define _SHARED_DEFS_COMMON_H_

/*
 * Default board type is 0 to match the default (fallback) from get_bootval.
 * Script returns 0 if the parameter is not defined.
 */
#define  QTN_RUBY_BOARD_TYPE_DEFAULT			0

#define  QTN_RUBY_BRINGUP_BOARD				0
#define  QTN_RUBY_BRINGUP_BOARD_32_320			1
#define  QTN_RUBY_BRINGUP_BOARD_16_320			2
#define  QTN_RUBY_BRINGUP_BOARD_16_160			3
#define  QTN_RUBY_BRINGUP_BOARD_ETRON			4
#define  QTN_RUBY_BRINGUP_BOARD_ETRON_320		5
#define  QTN_RUBY_BRINGUP_BOARD_ETRON_160		6
#define  QTN_RUBY_BRINGUP_BOARD_16_200			7
#define  QTN_RUBY_BRINGUP_BOARD_32_200			8
#define  QTN_RUBY_BRINGUP_BOARD_PCIE			9
/* diag board ids */
#define  QTN_RUBY_BRINGUP_BOARD_32_160_ARB		10
#define  QTN_RUBY_BRINGUP_BOARD_32_160_ARB_1		11
#define  QTN_RUBY_BRINGUP_BOARD_16_160_ARB_1		12
#define  QTN_RUBY_BRINGUP_BOARD_32_160_ARB_0		13
#define  QTN_RUBY_BRINGUP_BOARD_ETRON_160_EMAC1		14
#define  QTN_RUBY_BRINGUP_BOARD_ETRON_250_EMAC1		15
#define  QTN_RUBY_BRINGUP_BOARD_ETRON_32_320_EMAC1	16
#define  QTN_RUBY_BRINGUP_ETRON32_160			17
#define  QTN_RUBY_BRINGUP_ETRON32_320			18
#define  QTN_RUBY_BRINGUP_BOARD_MICRON_DUALEMAC		19
#define  QTN_RUBY_BRINGUP_BOARD_MICRON_DUALEMAC_MII	20
#define  QTN_RUBY_BRINGUP_BOARD_MICRON_DUALEMAC_LOOPBACK 21
#define  QTN_RUBY_BRINGUP_BOARD_16_160_DUALEMAC		22


#define  QTN_RUBY_REFERENCE_DESIGN_BOARD		1000
#define  QTN_RUBY_REFERENCE_DESIGN_BOARD_250		1001
#define  QTN_RUBY_REF_BOARD_DUAL_CON			1002
#define  QTN_RUBY_REFERENCE_DESIGN_BOARD_320		1003
#define  QTN_RUBY_ETRON_32_320_EMAC1			1004
#define  QTN_RUBY_ETRON_32_250_EMAC1			1005
#define  QTN_RUBY_REFERENCE_DESIGN_BOARD_RGMII_DLL	1006
#define  QTN_RUBY_QHS710_5S5_SIGE_DDR250		1007
#define  QTN_RUBY_QHS710_5S5_SIGE_DDR320		1008
#define  QTN_RUBY_OHS711_PCIE_320DDR			1009
/* pcie reference ids */
#define  QTN_RUBY_QHS713_5S1_PCIERC_DDR160		1170
#define  QTN_RUBY_OHS711_5S13_PCIE_DDR320		1171 /* duplicate of 1009 */
#define  QTN_RUBY_QHS713_5S1_PCIERC_DDR320		1172

#define  QTN_RUBY_ODM_BOARD_0				1200
#define  QTN_RUBY_ODM_BOARD_1				1201
#define  QTN_RUBY_ODM_BOARD_2				1202
#define  QTN_RUBY_ODM_BOARD_3				1203
#define  QTN_RUBY_ODM_BOARD_4				1204
#define  QTN_RUBY_ODM_BOARD_5				1205
#define  QTN_RUBY_ODM_BOARD_6				1206
#define  QTN_RUBY_ODM_BOARD_7				1207
#define  QTN_RUBY_ODM_BOARD_8				1208
#define  QTN_RUBY_ODM_BOARD_9				1209
#define  QTN_RUBY_ODM_BOARD_10				1210
#define  QTN_RUBY_ODM_BOARD_11				1211
#define  QTN_RUBY_ODM_BOARD_12				1212
#define  QTN_RUBY_ODM_BOARD_13				1213
#define  QTN_RUBY_ODM_BOARD_14				1214
#define  QTN_RUBY_ODM_BOARD_15				1215
#define  QTN_RUBY_ODM_BOARD_16				1216
#define  QTN_RUBY_ODM_BOARD_17				1217
#define  QTN_RUBY_ODM_BOARD_18				1218
#define  QTN_RUBY_ODM_BOARD_19				1219
#define  QTN_RUBY_ODM_BOARD_20				1220
#define  QTN_RUBY_ODM_BOARD_21				1221
#define  QTN_RUBY_ODM_BOARD_22				1222
#define  QTN_TOPAZ_FPGAA_BOARD				1223
#define  QTN_TOPAZ_FPGAB_BOARD				1224
#define  QTN_TOPAZ_DUAL_EMAC_FPGAA_BOARD		1225
#define  QTN_TOPAZ_DUAL_EMAC_FPGAB_BOARD		1226
#define  QTN_TOPAZ_RC_BOARD				1227
#define  QTN_TOPAZ_EP_BOARD				1228
#define  QTN_TOPAZ_BB_BOARD				1229
#define  QTN_TOPAZ_RF_BOARD				1230
#define  QTN_TOPAZ_QHS840_5S1				1231

#define		QTN_RUBY_AUTOCONFIG_ID				32768
#define		QTN_RUBY_UNIVERSAL_BOARD_ID			65535

#define  QTN_RUBY_NOSUCH_BOARD_TYPE			-1

#define  QTN_RUBY_BRINGUP_RWPA				0
#define  QTN_RUBY_REF_RWPA				1
#define  QTN_RUBY_SIGE					2
#define  QTN_RUBY_UNDEFINED				3
#define  QTN_RUBY_WIFI_NONE				4
#define	 QTN_TPZ_SE5003L1				5
#define	 QTN_TPZ_SE5003L1_INV				6
#define  QTN_TPZ_SKY85703				7
#define  QTN_TPZ_SKY85405_BPF840			8
#define  QTN_TPZ_DBS					9	/* BBIC4 + RFIC6 */
#define  QTN_TPZ_SE5502L				10	/* BBIC4 + RFIC5 */
#define  QTN_TPZ_SKY85710_NG				11

#endif /* _SHARED_DEFS_COMMON_H_ */

