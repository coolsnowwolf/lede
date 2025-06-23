// SPDX-License-Identifier: GPL-2.0-only

struct rtl83xx_shared_private {
	char *name;
};

struct __attribute__ ((__packed__)) part {
	uint16_t start;
	uint8_t wordsize;
	uint8_t words;
};

struct __attribute__ ((__packed__)) fw_header {
	uint32_t magic;
	uint32_t phy;
	uint32_t checksum;
	uint32_t version;
	struct part parts[10];
};

/* TODO: fixed path? */
#define FIRMWARE_838X_8380_1			"rtl838x_phy/rtl838x_8380.fw"
#define FIRMWARE_838X_8214FC_1			"rtl838x_phy/rtl838x_8214fc.fw"
#define FIRMWARE_838X_8218b_1			"rtl838x_phy/rtl838x_8218b.fw"

#define PHY_ID_RTL8214C				0x001cc942
#define PHY_ID_RTL8218B_E			0x001cc980
#define PHY_ID_RTL8214_OR_8218			0x001cc981
#define PHY_ID_RTL8218D				0x001cc983
#define PHY_ID_RTL8218B_I			0x001cca40
#define PHY_ID_RTL8221B				0x001cc849
#define PHY_ID_RTL8226				0x001cc838
#define PHY_ID_RTL8390_GENERIC			0x001ccab0
#define PHY_ID_RTL8393_I			0x001c8393
#define PHY_ID_RTL9300_I			0x70d03106

/* These PHYs share the same id (0x001cc981) */
#define PHY_IS_NOT_RTL821X			0
#define PHY_IS_RTL8214FC			1
#define PHY_IS_RTL8214FB			2
#define PHY_IS_RTL8218B_E			3

/* Registers of the internal Serdes of the 8380 */
#define RTL838X_SDS_MODE_SEL			(0x0028)
#define RTL838X_SDS_CFG_REG			(0x0034)
#define RTL838X_INT_MODE_CTRL			(0x005c)
#define RTL838X_DMY_REG31			(0x3b28)

#define RTL8380_SDS4_FIB_REG0			(0xF800)
#define RTL838X_SDS4_REG28			(0xef80)
#define RTL838X_SDS4_DUMMY0			(0xef8c)
#define RTL838X_SDS5_EXT_REG6			(0xf18c)
#define RTL838X_SDS4_FIB_REG0			(RTL838X_SDS4_REG28 + 0x880)
#define RTL838X_SDS5_FIB_REG0			(RTL838X_SDS4_REG28 + 0x980)

/* Registers of the internal SerDes of the RTL8390 */
#define RTL839X_SDS12_13_XSG0			(0xB800)

/* Registers of the internal Serdes of the 9300 */
#define RTL930X_SDS_INDACS_CMD			(0x03B0)
#define RTL930X_SDS_INDACS_DATA			(0x03B4)
#define RTL930X_MAC_FORCE_MODE_CTRL		(0xCA1C)

/* Registers of the internal SerDes of the 9310 */
#define RTL931X_SERDES_INDRT_ACCESS_CTRL	(0x5638)
#define RTL931X_SERDES_INDRT_DATA_CTRL		(0x563C)
#define RTL931X_SERDES_MODE_CTRL		(0x13cc)
#define RTL931X_PS_SERDES_OFF_MODE_CTRL_ADDR	(0x13F4)
#define RTL931X_MAC_SERDES_MODE_CTRL(sds)	(0x136C + (((sds) << 2)))

int rtl839x_read_sds_phy(int phy_addr, int phy_reg);
int rtl839x_write_sds_phy(int phy_addr, int phy_reg, u16 v);

int rtl9300_serdes_setup(int port, int sds_num, phy_interface_t phy_mode);
int rtl930x_read_sds_phy(int phy_addr, int page, int phy_reg);
int rtl930x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v);

int rtl931x_read_sds_phy(int phy_addr, int page, int phy_reg);
int rtl931x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v);
int rtl931x_sds_cmu_band_get(int sds, phy_interface_t mode);
void rtl931x_sds_init(u32 sds, phy_interface_t mode);

/*
 * TODO: The following functions are currently not in use. So compiler will complain if
 * they are static and not made available externally. Collect them in this section to
 * preserve for future use.
 */

void rtl9300_do_rx_calibration_3(int sds_num, phy_interface_t phy_mode);
int rtl9300_sds_clock_wait(int timeout);
int rtl9300_sds_cmu_band_get(int sds);
void rtl9300_sds_rxcal_dcvs_get(u32 sds_num, u32 dcvs_id, u32 dcvs_list[]);
void rtl9300_sds_rxcal_dcvs_manual(u32 sds_num, u32 dcvs_id, bool manual, u32 dvcs_list[]);
void rtl9300_sds_set(int sds_num, u32 mode);

int rtl931x_link_sts_get(u32 sds);
void rtl931x_sds_fiber_disable(u32 sds);
int rtl931x_sds_cmu_band_set(int sds, bool enable, u32 band, phy_interface_t mode);