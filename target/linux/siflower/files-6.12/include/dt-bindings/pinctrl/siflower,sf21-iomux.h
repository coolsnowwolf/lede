/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __DT_BINDINGS_SF21_IOMUX_H__
#define __DT_BINDINGS_SF21_IOMUX_H__

#define SW_DS		0xf		/* Drive strength */
#define SW_ST		(1 << 4)	/* Schmitt enable */
#define SW_PD		(1 << 5)	/* Pull-down enable */
#define SW_PU		(1 << 6)	/* Pull-up enable */
#define SW_OEN		(1 << 7)	/* Output disable [sic] */
#define SW_IE		(1 << 8)	/* Input enable */
#define MODE_BIT0	(1 << 9)	/* Function mode LSB */
#define MODE_BIT1	(1 << 10)	/* Function mode MSB */
#define FMUX_SEL	(1 << 11)	/* GPIO mode enable */
#define FUNC_SW_SEL	(1 << 12)	/* Function mode enable */

#define FUNC_MODE_MASK	0x1f80
#define FUNC_MODE0	(FUNC_SW_SEL | SW_IE)
#define FUNC_MODE1	(FUNC_MODE0 | MODE_BIT0)
#define FUNC_MODE2	(FUNC_MODE0 | MODE_BIT1)
#define FUNC_MODE3	(FUNC_MODE0 | MODE_BIT0 | MODE_BIT1)
#define GPIO_MODE	(FUNC_MODE0 | FMUX_SEL)

#define EXT_CLK_IN	0x00
#define CLK_OUT		0x04
#define SPI0_TXD	0x08
#define SPI0_RXD	0x0c
#define SPI0_CLK	0x10
#define SPI0_CSN	0x14
#define SPI0_HOLD	0x18
#define SPI0_WP		0x1c
#define JTAG_TDO	0x20
#define JTAG_TDI	0x24
#define JTAG_TMS	0x28
#define JTAG_TCK	0x2c
#define JTAG_RST	0x30
#define UART1_TX	0x34
#define UART1_RX	0x38
#define I2C0_DAT	0x3c
#define I2C0_CLK	0x40
#define I2C1_DAT	0x44
#define I2C1_CLK	0x48
#define PWM0		0x4c
#define PWM1		0x50
#define RGMII_GTX_CLK	0x54
#define RGMII_TXCLK	0x58
#define RGMII_TXD0	0x5c
#define RGMII_TXD1	0x60
#define RGMII_TXD2	0x64
#define RGMII_TXD3	0x68
#define RGMII_TXCTL	0x6c
#define RGMII_RXCLK	0x70
#define RGMII_RXD0	0x74
#define RGMII_RXD1	0x78
#define RGMII_RXD2	0x7c
#define RGMII_RXD3	0x80
#define RGMII_RXCTL	0x84
#define QSGMII_MDIO	0x88
#define QSGMII_MDC	0x8c
#define SXGMII_MDIO	0x90
#define SXGMII_MDC	0x94
#define DGS_INT		0x98
#define PHY_RSTN	0x9c
#define PHY_INT		0xa0
#endif
