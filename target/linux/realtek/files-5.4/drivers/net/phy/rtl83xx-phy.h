// SPDX-License-Identifier: GPL-2.0-only

// TODO: not really used
struct rtl838x_phy_priv {
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

// TODO: fixed path?
#define FIRMWARE_838X_8380_1	"rtl838x_phy/rtl838x_8380.fw"
#define FIRMWARE_838X_8214FC_1	"rtl838x_phy/rtl838x_8214fc.fw"
#define FIRMWARE_838X_8218b_1	"rtl838x_phy/rtl838x_8218b.fw"

/* External RTL8218B and RTL8214FC IDs are identical */
#define PHY_ID_RTL8214C		0x001cc942
#define PHY_ID_RTL8214FC	0x001cc981
#define PHY_ID_RTL8218B_E	0x001cc981
#define PHY_ID_RTL8218D		0x001cc983
#define PHY_ID_RTL8218B_I	0x001cca40
#define PHY_ID_RTL8226		0x001cc838
#define PHY_ID_RTL8390_GENERIC	0x001ccab0
#define PHY_ID_RTL8393_I	0x001c8393
#define PHY_ID_RTL9300_I	0x70d03106

// PHY MMD devices
#define MMD_AN		7
#define MMD_VEND2	31

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
