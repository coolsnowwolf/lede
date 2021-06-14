// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2020 Andre Heider <a.heider@gmail.com>
 */

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libubus.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dsl_cpe_control.h"
#include <drv_dsl_cpe_api_ioctl.h>
#ifdef INCLUDE_DSL_CPE_API_VRX
#include <drv_mei_cpe_interface.h>
#endif

#define U16(v1, v2) ( \
	((uint16_t)(v1) << 8) | \
	((uint16_t)(v2)))

#define U32(v1, v2, v3, v4) ( \
	((uint32_t)(v1) << 24) | \
	((uint32_t)(v2) << 16) | \
	((uint32_t)(v3) << 8) | \
	((uint32_t)(v4)))

#define STR_CASE(id, text) \
	case id: \
		str = text; \
		break;

#define STR_CASE_MAP(id, text, number) \
	case id: \
		str = text; \
		map = number; \
		break;

#define IOCTL(type, request) \
	type out; \
	memset(&out, 0, sizeof(type)); \
	if (ioctl(fd, request, &out)) \
		return;

#define IOCTL_DIR(type, request, dir) \
	type out; \
	memset(&out, 0, sizeof(type)); \
	out.nDirection = dir; \
	if (ioctl(fd, request, &out)) \
		return;

#define IOCTL_DIR_DELT(type, request, dir, delt) \
	type out; \
	memset(&out, 0, sizeof(type)); \
	out.nDirection = dir; \
	out.nDeltDataType = delt; \
	if (ioctl(fd, request, &out)) \
		return;

typedef enum {
	ANNEX_UNKNOWN = 0,
	ANNEX_A,
	ANNEX_B,
	ANNEX_C,
	ANNEX_I,
	ANNEX_J,
	ANNEX_L,
	ANNEX_M,
} annex_t;

typedef enum {
	STD_UNKNOWN = 0,
	STD_T1_413,
	STD_TS_101_388,
	STD_G_992_1,
	STD_G_992_2,
	STD_G_992_3,
	STD_G_992_4,
	STD_G_992_5,
	STD_G_993_1,
	STD_G_993_2,
} standard_t;

typedef enum {
	VECTOR_UNKNOWN = 0,
	VECTOR_OFF,
	VECTOR_ON_DS,
	VECTOR_ON_DS_US,
} vector_t;

typedef enum {
	PROFILE_UNKNOWN = 0,
	PROFILE_8A,
	PROFILE_8B,
	PROFILE_8C,
	PROFILE_8D,
	PROFILE_12A,
	PROFILE_12B,
	PROFILE_17A,
	PROFILE_30A,
	PROFILE_35B,
} profile_t;

/* These values are exported via ubus and backwards compability
 * needs to be kept!
 */
enum {
	LSTATE_MAP_UNKNOWN = -1,
	LSTATE_MAP_NOT_INITIALIZED,
	LSTATE_MAP_EXCEPTION,
	LSTATE_MAP_IDLE,
	LSTATE_MAP_SILENT,
	LSTATE_MAP_HANDSHAKE,
	LSTATE_MAP_FULL_INIT,
	LSTATE_MAP_SHOWTIME_NO_SYNC,
	LSTATE_MAP_SHOWTIME_TC_SYNC,
	LSTATE_MAP_RESYNC,
};

/* These values are exported via ubus and backwards compability
 * needs to be kept!
 */
enum {
	PSTATE_MAP_UNKNOWN = -2,
	PSTATE_MAP_NA,
	PSTATE_MAP_L0,
	PSTATE_MAP_L1,
	PSTATE_MAP_L2,
	PSTATE_MAP_L3,
};

static DSL_CPE_ThreadCtrl_t thread;
static struct ubus_context *ctx;
static struct blob_buf b;

static inline void m_double(const char *id, double value) {
	blobmsg_add_double(&b, id, value);
}

static inline void m_bool(const char *id, bool value) {
	blobmsg_add_u8(&b, id, value);
}

static inline void m_u32(const char *id, uint32_t value) {
	blobmsg_add_u32(&b, id, value);
}

static inline void m_str(const char *id, const char *value) {
	blobmsg_add_string(&b, id, value);
}

static inline void m_db(const char *id, int value) {
	m_double(id, (double)value / 10);
}

static inline void m_array(const char *id, const uint8_t *value, uint8_t len) {
	void *c = blobmsg_open_array(&b, id);

	for (uint8_t i = 0; i < len; ++i)
		blobmsg_add_u16(&b, "", value[i]);

	blobmsg_close_array(&b, c);
}

static void m_vendor(const char *id, const uint8_t *value) {
	// ITU-T T.35: U.S.
	if (U16(value[0], value[1]) != 0xb500)
		return;

	const char *str = NULL;
	switch (U32(value[2], value[3], value[4], value[5])) {
	STR_CASE(0x414C4342, "Alcatel")
	STR_CASE(0x414E4456, "Analog Devices")
	STR_CASE(0x4244434D, "Broadcom")
	STR_CASE(0x43454E54, "Centillium")
	STR_CASE(0x4753504E, "Globespan")
	STR_CASE(0x494B4E53, "Ikanos")
	STR_CASE(0x4946544E, "Infineon")
	STR_CASE(0x54535443, "Texas Instruments")
	STR_CASE(0x544D4D42, "Thomson MultiMedia Broadband")
	STR_CASE(0x5443544E, "Trend Chip Technologies")
	STR_CASE(0x53544D49, "ST Micro")
	};

	if (!str)
		return;

	if ((value[6] == 0) && (value[7] == 0)) {
		m_str(id, str);
		return;
	}

	char buf[64];
	sprintf(buf, "%s %d.%d", str, value[6], value[7]);
	m_str(id, buf);

	return;
}

annex_t get_annex(const uint8_t *xtse) {
	if ((xtse[0] & XTSE_1_01_A_T1_413) ||
	    (xtse[0] & XTSE_1_03_A_1_NO) ||
	    (xtse[0] & XTSE_1_04_A_1_O) ||
	    (xtse[1] & XTSE_2_01_A_2_NO) ||
	    (xtse[2] & XTSE_3_03_A_3_NO) ||
	    (xtse[2] & XTSE_3_04_A_3_O) ||
	    (xtse[3] & XTSE_4_01_A_4_NO) ||
	    (xtse[3] & XTSE_4_02_A_4_O) ||
	    (xtse[5] & XTSE_6_01_A_5_NO) ||
	    (xtse[5] & XTSE_6_02_A_5_O) ||
	    (xtse[7] & XTSE_8_01_A))
		return ANNEX_A;

	if ((xtse[0] & XTSE_1_05_B_1_NO) ||
	    (xtse[0] & XTSE_1_06_B_1_O) ||
	    (xtse[1] & XTSE_2_02_B_2_O) ||
	    (xtse[2] & XTSE_3_05_B_3_NO) ||
	    (xtse[2] & XTSE_3_06_B_3_O) ||
	    (xtse[5] & XTSE_6_03_B_5_NO) ||
	    (xtse[5] & XTSE_6_04_B_5_O) ||
	    (xtse[7] & XTSE_8_02_B))
		return ANNEX_B;

	if ((xtse[0] & XTSE_1_02_C_TS_101388) ||
	    (xtse[0] & XTSE_1_07_C_1_NO) ||
	    (xtse[0] & XTSE_1_08_C_1_O) ||
	    (xtse[1] & XTSE_2_03_C_2_NO) ||
	    (xtse[1] & XTSE_2_04_C_2_O) ||
	    (xtse[7] & XTSE_8_03_C))
		return ANNEX_C;

	if ((xtse[3] & XTSE_4_05_I_3_NO) ||
	    (xtse[3] & XTSE_4_06_I_3_O) ||
	    (xtse[4] & XTSE_5_01_I_4_NO) ||
	    (xtse[4] & XTSE_5_02_I_4_O) ||
	    (xtse[5] & XTSE_6_07_I_5_NO) ||
	    (xtse[5] & XTSE_6_08_I_5_O))
		return ANNEX_I;

	if ((xtse[3] & XTSE_4_07_J_3_NO) ||
	    (xtse[3] & XTSE_4_08_J_3_O) ||
	    (xtse[6] & XTSE_7_01_J_5_NO) ||
	    (xtse[6] & XTSE_7_02_J_5_O))
		return ANNEX_J;

	if ((xtse[4] & XTSE_5_03_L_3_NO) ||
	    (xtse[4] & XTSE_5_04_L_3_NO) ||
	    (xtse[4] & XTSE_5_05_L_3_O) ||
	    (xtse[4] & XTSE_5_06_L_3_O))
		return ANNEX_L;

	if ((xtse[4] & XTSE_5_07_M_3_NO) ||
	    (xtse[4] & XTSE_5_08_M_3_O) ||
	    (xtse[6] & XTSE_7_03_M_5_NO) ||
	    (xtse[6] & XTSE_7_04_M_5_O))
		return ANNEX_M;

	return ANNEX_UNKNOWN;
}

static standard_t get_standard(const uint8_t *xtse) {
	if (xtse[0] & XTSE_1_01_A_T1_413)
		return STD_T1_413;

	if (xtse[0] & XTSE_1_02_C_TS_101388)
		return STD_TS_101_388;

	if ((xtse[0] & XTSE_1_03_A_1_NO) ||
	    (xtse[0] & XTSE_1_04_A_1_O) ||
	    (xtse[0] & XTSE_1_05_B_1_NO) ||
	    (xtse[0] & XTSE_1_06_B_1_O) ||
	    (xtse[0] & XTSE_1_07_C_1_NO) ||
	    (xtse[0] & XTSE_1_08_C_1_O))
		return STD_G_992_1;

	if ((xtse[1] & XTSE_2_01_A_2_NO) ||
	    (xtse[1] & XTSE_2_02_B_2_O) ||
	    (xtse[1] & XTSE_2_03_C_2_NO) ||
	    (xtse[1] & XTSE_2_04_C_2_O))
		return STD_G_992_2;

	if ((xtse[2] & XTSE_3_03_A_3_NO) ||
	    (xtse[2] & XTSE_3_04_A_3_O) ||
	    (xtse[2] & XTSE_3_05_B_3_NO) ||
	    (xtse[2] & XTSE_3_06_B_3_O) ||
	    (xtse[3] & XTSE_4_05_I_3_NO) ||
	    (xtse[3] & XTSE_4_06_I_3_O) ||
	    (xtse[3] & XTSE_4_07_J_3_NO) ||
	    (xtse[3] & XTSE_4_08_J_3_O) ||
	    (xtse[4] & XTSE_5_03_L_3_NO) ||
	    (xtse[4] & XTSE_5_04_L_3_NO) ||
	    (xtse[4] & XTSE_5_05_L_3_O) ||
	    (xtse[4] & XTSE_5_06_L_3_O) ||
	    (xtse[4] & XTSE_5_07_M_3_NO) ||
	    (xtse[4] & XTSE_5_08_M_3_O))
		return STD_G_992_3;

	if ((xtse[3] & XTSE_4_01_A_4_NO) ||
	    (xtse[3] & XTSE_4_02_A_4_O) ||
	    (xtse[4] & XTSE_5_01_I_4_NO) ||
	    (xtse[4] & XTSE_5_02_I_4_O))
		return STD_G_992_4;

	if ((xtse[5] & XTSE_6_01_A_5_NO) ||
	    (xtse[5] & XTSE_6_02_A_5_O) ||
	    (xtse[5] & XTSE_6_03_B_5_NO) ||
	    (xtse[5] & XTSE_6_04_B_5_O) ||
	    (xtse[5] & XTSE_6_07_I_5_NO) ||
	    (xtse[5] & XTSE_6_08_I_5_O) ||
	    (xtse[6] & XTSE_7_01_J_5_NO) ||
	    (xtse[6] & XTSE_7_02_J_5_O) ||
	    (xtse[6] & XTSE_7_03_M_5_NO) ||
	    (xtse[6] & XTSE_7_04_M_5_O))
		return STD_G_992_5;

	if (xtse[7] & XTSE_8_08)
		return STD_G_993_1;

	if ((xtse[7] & XTSE_8_01_A) ||
	    (xtse[7] & XTSE_8_02_B) ||
	    (xtse[7] & XTSE_8_03_C))
		return STD_G_993_2;

	return STD_UNKNOWN;
}

static void version_information(int fd) {
	IOCTL(DSL_VersionInformation_t, DSL_FIO_VERSION_INFORMATION_GET)

	m_str("api_version", out.data.DSL_DriverVersionApi);
	m_str("firmware_version", out.data.DSL_ChipSetFWVersion);
	m_str("chipset", out.data.DSL_ChipSetType);
	m_str("driver_version", out.data.DSL_DriverVersionMeiBsp);
}

static void line_state(int fd) {
	IOCTL(DSL_LineState_t, DSL_FIO_LINE_STATE_GET)

	int map = LSTATE_MAP_UNKNOWN;
	const char *str;
	switch (out.data.nLineState) {
	STR_CASE_MAP(DSL_LINESTATE_NOT_INITIALIZED, "Not initialized", LSTATE_MAP_NOT_INITIALIZED)
	STR_CASE_MAP(DSL_LINESTATE_EXCEPTION, "Exception", LSTATE_MAP_EXCEPTION)
	STR_CASE(DSL_LINESTATE_NOT_UPDATED, "Not updated")
	STR_CASE(DSL_LINESTATE_IDLE_REQUEST, "Idle request")
	STR_CASE_MAP(DSL_LINESTATE_IDLE, "Idle", LSTATE_MAP_IDLE)
	STR_CASE(DSL_LINESTATE_SILENT_REQUEST, "Silent request")
	STR_CASE_MAP(DSL_LINESTATE_SILENT, "Silent", LSTATE_MAP_SILENT)
	STR_CASE_MAP(DSL_LINESTATE_HANDSHAKE, "Handshake", LSTATE_MAP_HANDSHAKE)
	STR_CASE(DSL_LINESTATE_BONDING_CLR, "Bonding CLR")
	STR_CASE_MAP(DSL_LINESTATE_FULL_INIT, "Full init", LSTATE_MAP_FULL_INIT)
	STR_CASE(DSL_LINESTATE_SHORT_INIT_ENTRY, "Short init entry")
	STR_CASE(DSL_LINESTATE_DISCOVERY, "Discovery")
	STR_CASE(DSL_LINESTATE_TRAINING, "Training")
	STR_CASE(DSL_LINESTATE_ANALYSIS, "Analysis")
	STR_CASE(DSL_LINESTATE_EXCHANGE, "Exchange")
	STR_CASE_MAP(DSL_LINESTATE_SHOWTIME_NO_SYNC, "Showtime without TC-Layer sync", LSTATE_MAP_SHOWTIME_NO_SYNC)
	STR_CASE_MAP(DSL_LINESTATE_SHOWTIME_TC_SYNC, "Showtime with TC-Layer sync", LSTATE_MAP_SHOWTIME_TC_SYNC)
	STR_CASE(DSL_LINESTATE_FASTRETRAIN, "Fastretrain")
	STR_CASE(DSL_LINESTATE_LOWPOWER_L2, "Lowpower L2")
	STR_CASE(DSL_LINESTATE_LOOPDIAGNOSTIC_ACTIVE, "Loopdiagnostic active")
	STR_CASE(DSL_LINESTATE_LOOPDIAGNOSTIC_DATA_EXCHANGE, "Loopdiagnostic data exchange")
	STR_CASE(DSL_LINESTATE_LOOPDIAGNOSTIC_DATA_REQUEST, "Loopdiagnostic data request")
	STR_CASE(DSL_LINESTATE_LOOPDIAGNOSTIC_COMPLETE, "Loopdiagnostic complete")
	STR_CASE_MAP(DSL_LINESTATE_RESYNC, "Resync", LSTATE_MAP_RESYNC)
	STR_CASE(DSL_LINESTATE_TEST, "Test")
	STR_CASE(DSL_LINESTATE_TEST_LOOP, "Test loop")
	STR_CASE(DSL_LINESTATE_TEST_REVERB, "Test reverb")
	STR_CASE(DSL_LINESTATE_TEST_MEDLEY, "Test medley")
	STR_CASE(DSL_LINESTATE_TEST_SHOWTIME_LOCK, "Showtime lock")
	STR_CASE(DSL_LINESTATE_TEST_QUIET, "Quiet")
	STR_CASE(DSL_LINESTATE_LOWPOWER_L3, "Lowpower L3")
#ifndef INCLUDE_DSL_CPE_API_DANUBE
	STR_CASE(DSL_LINESTATE_DISABLED, "Disabled")
	STR_CASE(DSL_LINESTATE_T1413, "T1413")
	STR_CASE(DSL_LINESTATE_ORDERLY_SHUTDOWN_REQUEST, "Orderly shutdown request")
	STR_CASE(DSL_LINESTATE_ORDERLY_SHUTDOWN, "Orderly shutdown")
	STR_CASE(DSL_LINESTATE_TEST_FILTERDETECTION_ACTIVE, "Test filterdetection active")
	STR_CASE(DSL_LINESTATE_TEST_FILTERDETECTION_COMPLETE, "Test filterdetection complete")
#endif
	default:
		str = NULL;
		break;
	};

	if (str)
		m_str("state", str);

	if (map != LSTATE_MAP_UNKNOWN )
		m_u32("state_num", map);

	m_bool("up", out.data.nLineState == DSL_LINESTATE_SHOWTIME_TC_SYNC);
}

static void pm_channel_counters_showtime(int fd) {
	IOCTL_DIR(DSL_PM_ChannelCounters_t, DSL_FIO_PM_CHANNEL_COUNTERS_SHOWTIME_GET, DSL_NEAR_END);

	m_u32("uptime", out.interval.nElapsedTime);
}

static void g997_line_inventory(int fd) {
	IOCTL_DIR(DSL_G997_LineInventory_t, DSL_FIO_G997_LINE_INVENTORY_GET, DSL_DOWNSTREAM)

	m_array("vendor_id", out.data.G994VendorID, DSL_G997_LI_MAXLEN_VENDOR_ID);
	m_vendor("vendor", out.data.G994VendorID);
	m_array("system_vendor_id", out.data.SystemVendorID, DSL_G997_LI_MAXLEN_VENDOR_ID);
	m_vendor("system_vendor", out.data.SystemVendorID);
	m_array("version", out.data.VersionNumber, DSL_G997_LI_MAXLEN_VERSION);
	m_array("serial", out.data.SerialNumber, DSL_G997_LI_MAXLEN_SERIAL);
}

static void g997_power_management_status(int fd) {
	IOCTL(DSL_G997_PowerManagementStatus_t, DSL_FIO_G997_POWER_MANAGEMENT_STATUS_GET)

	int map = PSTATE_MAP_UNKNOWN;
	const char *str;
	switch (out.data.nPowerManagementStatus) {
	STR_CASE_MAP(DSL_G997_PMS_NA, "Power management state is not available", PSTATE_MAP_NA)
	STR_CASE_MAP(DSL_G997_PMS_L0, "L0 - Synchronized", PSTATE_MAP_L0)
	STR_CASE_MAP(DSL_G997_PMS_L1, "L1 - Power Down Data transmission (G.992.2)", PSTATE_MAP_L1)
	STR_CASE_MAP(DSL_G997_PMS_L2, "L2 - Power Down Data transmission (G.992.3 and G.992.4)", PSTATE_MAP_L2)
	STR_CASE_MAP(DSL_G997_PMS_L3, "L3 - No power", PSTATE_MAP_L3)
	default:
		str = NULL;
		break;
	};

	if (str)
		m_str("power_state", str);

	if (map != PSTATE_MAP_UNKNOWN)
		m_u32("power_state_num", map);
}

static void g997_xtu_system_enabling(int fd, standard_t *standard) {
	IOCTL(DSL_G997_XTUSystemEnabling_t, DSL_FIO_G997_XTU_SYSTEM_ENABLING_STATUS_GET)

	m_array("xtse", out.data.XTSE, DSL_G997_NUM_XTSE_OCTETS);

	const char *str;
	switch (get_annex(out.data.XTSE)) {
	STR_CASE(ANNEX_A, "A")
	STR_CASE(ANNEX_B, "B")
	STR_CASE(ANNEX_C, "C")
	STR_CASE(ANNEX_I, "I")
	STR_CASE(ANNEX_J, "J")
	STR_CASE(ANNEX_L, "L")
	STR_CASE(ANNEX_M, "M")
	default:
		str = NULL;
		break;
	};
	if (str)
		m_str("annex", str);

	*standard = get_standard(out.data.XTSE);

	switch (*standard) {
	STR_CASE(STD_T1_413, "T1.413")
	STR_CASE(STD_TS_101_388, "TS 101 388")
	STR_CASE(STD_G_992_1, "G.992.1")
	STR_CASE(STD_G_992_2, "G.992.2")
	STR_CASE(STD_G_992_3, "G.992.3")
	STR_CASE(STD_G_992_4, "G.992.4")
	STR_CASE(STD_G_992_5, "G.992.5")
	STR_CASE(STD_G_993_1, "G.993.1")
	STR_CASE(STD_G_993_2, "G.993.2")
	default:
		str = NULL;
		break;
	}
	if (str)
		m_str("standard", str);
}

static vector_t get_vector_status() {
#ifdef INCLUDE_DSL_CPE_API_VRX
	int fd = open(DSL_CPE_DSL_LOW_DEV "/0", O_RDWR, 0644);
	if (fd < 0)
		return VECTOR_UNKNOWN;

	IOCTL_MEI_dsmStatus_t out;
	memset(&out, 0, sizeof(IOCTL_MEI_dsmStatus_t));
	int ret = ioctl(fd, FIO_MEI_DSM_STATUS_GET, &out);
	close(fd);

	if (ret)
		return VECTOR_UNKNOWN;

	switch (out.eVectorStatus) {
	case e_MEI_VECTOR_STAT_OFF:
		return VECTOR_OFF;
	case e_MEI_VECTOR_STAT_ON_DS:
		return VECTOR_ON_DS;
	case e_MEI_VECTOR_STAT_ON_DS_US:
		return VECTOR_ON_DS_US;
	default:
		return VECTOR_UNKNOWN;
	};
#else
	return VECTOR_UNKNOWN;
#endif
}

static void band_plan_status(int fd, profile_t *profile) {
#if (INCLUDE_DSL_CPE_API_VDSL_SUPPORT == 1)
	IOCTL(DSL_BandPlanStatus_t, DSL_FIO_BAND_PLAN_STATUS_GET)

	switch (out.data.nProfile) {
	case DSL_PROFILE_8A:
		*profile = PROFILE_8A;
		break;
	case DSL_PROFILE_8B:
		*profile = PROFILE_8B;
		break;
	case DSL_PROFILE_8C:
		*profile = PROFILE_8C;
		break;
	case DSL_PROFILE_8D:
		*profile = PROFILE_8D;
		break;
	case DSL_PROFILE_12A:
		*profile = PROFILE_12A;
		break;
	case DSL_PROFILE_12B:
		*profile = PROFILE_12B;
		break;
	case DSL_PROFILE_17A:
		*profile = PROFILE_17A;
		break;
	case DSL_PROFILE_30A:
		*profile = PROFILE_30A;
		break;
	case DSL_PROFILE_35B:
		*profile = PROFILE_35B;
		break;
	default:
		*profile = PROFILE_UNKNOWN;
		break;
	};

	const char *str;
	switch (*profile) {
	STR_CASE(PROFILE_8A, "8a")
	STR_CASE(PROFILE_8B, "8b")
	STR_CASE(PROFILE_8C, "8c")
	STR_CASE(PROFILE_8D, "8d")
	STR_CASE(PROFILE_12A, "12a")
	STR_CASE(PROFILE_12B, "12b")
	STR_CASE(PROFILE_17A, "17a")
	STR_CASE(PROFILE_30A, "30a")
	STR_CASE(PROFILE_35B, "35b")
	default:
		str = NULL;
		break;
	};
	if (str)
		m_str("profile", str);
#endif
}

static void line_feature_config(int fd, DSL_AccessDir_t direction) {
	IOCTL_DIR(DSL_LineFeature_t, DSL_FIO_LINE_FEATURE_STATUS_GET, direction)

	m_bool("trellis", out.data.bTrellisEnable);
	m_bool("bitswap", out.data.bBitswapEnable);
	m_bool("retx", out.data.bReTxEnable);
	m_bool("virtual_noise", out.data.bVirtualNoiseSupport);
}

static void g997_channel_status(int fd, DSL_AccessDir_t direction) {
	IOCTL_DIR(DSL_G997_ChannelStatus_t, DSL_FIO_G997_CHANNEL_STATUS_GET, direction);

	m_u32("interleave_delay", out.data.ActualInterleaveDelay * 10);
#ifndef INCLUDE_DSL_CPE_API_DANUBE
	// prefer ACTNDR, see comments in drv_dsl_cpe_api_g997.h
	m_u32("data_rate", out.data.ActualNetDataRate);
#else
	m_u32("data_rate", out.data.ActualDataRate);
#endif
}

static void g997_line_status(int fd, DSL_AccessDir_t direction) {
	IOCTL_DIR_DELT(DSL_G997_LineStatus_t, DSL_FIO_G997_LINE_STATUS_GET, direction, DSL_DELT_DATA_SHOWTIME);

	m_db("latn", out.data.LATN);
	m_db("satn", out.data.SATN);
	m_db("snr", out.data.SNR);
	m_db("actps", out.data.ACTPS);
	m_db("actatp", out.data.ACTATP);
	m_u32("attndr", out.data.ATTNDR);
}

static void pm_line_sec_counters_total(int fd, DSL_XTUDir_t direction) {
	IOCTL_DIR(DSL_PM_LineSecCountersTotal_t, DSL_FIO_PM_LINE_SEC_COUNTERS_TOTAL_GET, direction)

	m_u32("es", out.data.nES);
	m_u32("ses", out.data.nSES);
	m_u32("loss", out.data.nLOSS);
	m_u32("uas", out.data.nUAS);
	m_u32("lofs", out.data.nLOFS);
#ifndef INCLUDE_DSL_CPE_API_DANUBE
	m_u32("fecs", out.data.nFECS);
#endif
}

static void pm_data_path_counters_total(int fd, DSL_XTUDir_t direction) {
	IOCTL_DIR(DSL_PM_DataPathCountersTotal_t, DSL_FIO_PM_DATA_PATH_COUNTERS_TOTAL_GET, direction);

	m_u32("hec", out.data.nHEC);
	m_u32("ibe", out.data.nIBE);
	m_u32("crc_p", out.data.nCRC_P);
	m_u32("crcp_p", out.data.nCRCP_P);
	m_u32("cv_p", out.data.nCV_P);
	m_u32("cvp_p", out.data.nCVP_P);
}

static void retx_statistics(int fd, DSL_XTUDir_t direction) {
#ifdef INCLUDE_DSL_CPE_PM_RETX_COUNTERS
#ifdef INCLUDE_DSL_CPE_PM_RETX_THRESHOLDS
	IOCTL_DIR(DSL_ReTxStatistics_t, DSL_FIO_RETX_STATISTICS_GET, direction);

	m_u32("rx_corrupted", out.data.nRxCorruptedTotal);
	m_u32("rx_uncorrected_protected", out.data.nRxUncorrectedProtected);
	m_u32("rx_retransmitted", out.data.nRxRetransmitted);
	m_u32("rx_corrected", out.data.nRxCorrected);
	m_u32("tx_retransmitted", out.data.nTxRetransmitted);
#endif
#endif
}

static void describe_mode(standard_t standard, profile_t profile, vector_t vector) {
	char buf[128];

	switch (standard) {
	case STD_T1_413:
		strcpy(buf, "T1.413");
		break;
	case STD_TS_101_388:
		strcpy(buf, "TS 101 388");
		break;
	case STD_G_992_1:
		strcpy(buf, "G.992.1 (ADSL)");
		break;
	case STD_G_992_2:
		strcpy(buf, "G.992.2 (ADSL lite)");
		break;
	case STD_G_992_3:
		strcpy(buf, "G.992.3 (ADSL2)");
		break;
	case STD_G_992_4:
		strcpy(buf, "G.992.4 (ADSL2 lite)");
		break;
	case STD_G_992_5:
		strcpy(buf, "G.992.5 (ADSL2+)");
		break;
	case STD_G_993_1:
		strcpy(buf, "G.993.1 (VDSL)");
		break;
	case STD_G_993_2:
		strcpy(buf, "G.993.2 (VDSL2");

		switch (profile) {
		case PROFILE_8A:
			strcat(buf, ", Profile 8a");
			break;
		case PROFILE_8B:
			strcat(buf, ", Profile 8b");
			break;
		case PROFILE_8C:
			strcat(buf, ", Profile 8c");
			break;
		case PROFILE_8D:
			strcat(buf, ", Profile 8d");
			break;
		case PROFILE_12A:
			strcat(buf, ", Profile 12a");
			break;
		case PROFILE_12B:
			strcat(buf, ", Profile 12b");
			break;
		case PROFILE_17A:
			strcat(buf, ", Profile 17a");
			break;
		case PROFILE_30A:
			strcat(buf, ", Profile 30a");
			break;
		case PROFILE_35B:
			strcat(buf, ", Profile 35b");
			break;
		default:
			break;
		};

		switch (vector) {
		case VECTOR_ON_DS:
			strcat(buf, ", with downstream vectoring");
			break;
		case VECTOR_ON_DS_US:
			strcat(buf, ", with down- and upstream vectoring");
			break;
		default:
			break;
		};

		strcat(buf, ")");
		break;
	default:
		return;
	};

	m_str("mode", buf);
}

static int metrics(struct ubus_context *ctx, struct ubus_object *obj,
		   struct ubus_request_data *req, const char *method,
		   struct blob_attr *msg)
{
	int fd;
	void *c, *c2;
	standard_t standard = STD_UNKNOWN;
	profile_t profile = PROFILE_UNKNOWN;
	vector_t vector = VECTOR_UNKNOWN;

#ifndef INCLUDE_DSL_CPE_API_DANUBE
	fd = open(DSL_CPE_DEVICE_NAME "0", O_RDWR, 0644);
#else
	fd = open(DSL_CPE_DEVICE_NAME, O_RDWR, 0644);
#endif
	if (fd < 0)
		return UBUS_STATUS_UNKNOWN_ERROR;

	blob_buf_init(&b, 0);

	version_information(fd);
	line_state(fd);
	pm_channel_counters_showtime(fd);

	c = blobmsg_open_table(&b, "atu_c");
	g997_line_inventory(fd);
	blobmsg_close_table(&b, c);

	g997_power_management_status(fd);
	g997_xtu_system_enabling(fd, &standard);

	if (standard == STD_G_993_2) {
		band_plan_status(fd, &profile);
		vector = get_vector_status();
	}

	describe_mode(standard, profile, vector);

	c = blobmsg_open_table(&b, "upstream");
	switch (vector) {
	case VECTOR_OFF:
		m_bool("vector", false);
		break;
	case VECTOR_ON_DS_US:
		m_bool("vector", true);
		break;
	default:
		break;
	};
	line_feature_config(fd, DSL_UPSTREAM);
	g997_channel_status(fd, DSL_UPSTREAM);
	g997_line_status(fd, DSL_UPSTREAM);
	blobmsg_close_table(&b, c);

	c = blobmsg_open_table(&b, "downstream");
	switch (vector) {
	case VECTOR_OFF:
		m_bool("vector", false);
		break;
	case VECTOR_ON_DS:
	case VECTOR_ON_DS_US:
		m_bool("vector", true);
		break;
	default:
		break;
	};
	line_feature_config(fd, DSL_DOWNSTREAM);
	g997_channel_status(fd, DSL_DOWNSTREAM);
	g997_line_status(fd, DSL_DOWNSTREAM);
	blobmsg_close_table(&b, c);

	c = blobmsg_open_table(&b, "errors");
	c2 = blobmsg_open_table(&b, "near");
	pm_line_sec_counters_total(fd, DSL_NEAR_END);
	pm_data_path_counters_total(fd, DSL_NEAR_END);
	retx_statistics(fd, DSL_NEAR_END);
	blobmsg_close_table(&b, c2);

	c2 = blobmsg_open_table(&b, "far");
	pm_line_sec_counters_total(fd, DSL_FAR_END);
	pm_data_path_counters_total(fd, DSL_FAR_END);
	retx_statistics(fd, DSL_FAR_END);
	blobmsg_close_table(&b, c2);
	blobmsg_close_table(&b, c);

	ubus_send_reply(ctx, req, b.head);

	close(fd);

	return 0;
}

static const struct ubus_method dsl_methods[] = {
	UBUS_METHOD_NOARG("metrics", metrics),
};

static struct ubus_object_type dsl_object_type =
	UBUS_OBJECT_TYPE("dsl", dsl_methods);

static struct ubus_object dsl_object = {
	.name = "dsl",
	.type = &dsl_object_type,
	.methods = dsl_methods,
	.n_methods = ARRAY_SIZE(dsl_methods),
};

static DSL_int_t ubus_main(DSL_CPE_Thread_Params_t *params) {
	uloop_run();
	return 0;
}

void ubus_init() {
	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx)
		return;

	if (ubus_add_object(ctx, &dsl_object)) {
		ubus_free(ctx);
		ctx = NULL;
		return;
	}

	ubus_add_uloop(ctx);

	DSL_CPE_ThreadInit(&thread, "ubus", ubus_main, DSL_CPE_PIPE_STACK_SIZE, DSL_CPE_PIPE_PRIORITY, 0, 0);
}

void ubus_deinit() {
	if (!ctx)
		return;

	ubus_free(ctx);
	uloop_done();

	DSL_CPE_ThreadShutdown(&thread, 1000);
}
