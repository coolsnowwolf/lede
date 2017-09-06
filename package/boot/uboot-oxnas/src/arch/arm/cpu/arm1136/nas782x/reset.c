#include <common.h>
#include <asm/arch/sysctl.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/clock.h>

void reset_cpu(ulong addr)
{
	u32 value;

	// Assert reset to cores as per power on defaults
	// Don't touch the DDR interface as things will come to an impromptu stop
	// NB Possibly should be asserting reset for PLLB, but there are timing
	//    concerns here according to the docs

	value =
		BIT(SYS_CTRL_RST_COPRO     ) |
		BIT(SYS_CTRL_RST_USBHS     ) |
		BIT(SYS_CTRL_RST_USBHSPHYA ) |
		BIT(SYS_CTRL_RST_MACA      ) |
		BIT(SYS_CTRL_RST_PCIEA     ) |
		BIT(SYS_CTRL_RST_SGDMA     ) |
		BIT(SYS_CTRL_RST_CIPHER    ) |
		BIT(SYS_CTRL_RST_SATA      ) |
		BIT(SYS_CTRL_RST_SATA_LINK ) |
		BIT(SYS_CTRL_RST_SATA_PHY  ) |
		BIT(SYS_CTRL_RST_PCIEPHY   ) |
		BIT(SYS_CTRL_RST_STATIC    ) |
		BIT(SYS_CTRL_RST_UART1     ) |
		BIT(SYS_CTRL_RST_UART2     ) |
		BIT(SYS_CTRL_RST_MISC      ) |
		BIT(SYS_CTRL_RST_I2S       ) |
		BIT(SYS_CTRL_RST_SD        ) |
		BIT(SYS_CTRL_RST_MACB      ) |
		BIT(SYS_CTRL_RST_PCIEB     ) |
		BIT(SYS_CTRL_RST_VIDEO     ) |
		BIT(SYS_CTRL_RST_USBHSPHYB ) |
		BIT(SYS_CTRL_RST_USBDEV    );

	writel(value, SYS_CTRL_RST_SET_CTRL);

	// Release reset to cores as per power on defaults
	writel(BIT(SYS_CTRL_RST_GPIO), SYS_CTRL_RST_CLR_CTRL);

	// Disable clocks to cores as per power-on defaults - must leave DDR
	// related clocks enabled otherwise we'll stop rather abruptly.
	value =
		BIT(SYS_CTRL_CLK_COPRO) 	|
		BIT(SYS_CTRL_CLK_DMA)   	|
		BIT(SYS_CTRL_CLK_CIPHER)	|
		BIT(SYS_CTRL_CLK_SD)  		|
		BIT(SYS_CTRL_CLK_SATA)  	|
		BIT(SYS_CTRL_CLK_I2S)   	|
		BIT(SYS_CTRL_CLK_USBHS) 	|
		BIT(SYS_CTRL_CLK_MAC)   	|
		BIT(SYS_CTRL_CLK_PCIEA)   	|
		BIT(SYS_CTRL_CLK_STATIC)	|
		BIT(SYS_CTRL_CLK_MACB)		|
		BIT(SYS_CTRL_CLK_PCIEB)		|
		BIT(SYS_CTRL_CLK_REF600)	|
		BIT(SYS_CTRL_CLK_USBDEV);

	writel(value, SYS_CTRL_CLK_CLR_CTRL);

	// Enable clocks to cores as per power-on defaults

	// Set sys-control pin mux'ing as per power-on defaults

	writel(0, SYS_CONTROL_BASE + PINMUX_SECONDARY_SEL);
	writel(0, SYS_CONTROL_BASE + PINMUX_TERTIARY_SEL);
	writel(0, SYS_CONTROL_BASE + PINMUX_QUATERNARY_SEL);
	writel(0, SYS_CONTROL_BASE + PINMUX_DEBUG_SEL);
	writel(0, SYS_CONTROL_BASE + PINMUX_ALTERNATIVE_SEL);
	writel(0, SYS_CONTROL_BASE + PINMUX_PULLUP_SEL);

	writel(0, SEC_CONTROL_BASE + PINMUX_SECONDARY_SEL);
	writel(0, SEC_CONTROL_BASE + PINMUX_TERTIARY_SEL);
	writel(0, SEC_CONTROL_BASE + PINMUX_QUATERNARY_SEL);
	writel(0, SEC_CONTROL_BASE + PINMUX_DEBUG_SEL);
	writel(0, SEC_CONTROL_BASE + PINMUX_ALTERNATIVE_SEL);
	writel(0, SEC_CONTROL_BASE + PINMUX_PULLUP_SEL);

	// No need to save any state, as the ROM loader can determine whether reset
	// is due to power cycling or programatic action, just hit the (self-
	// clearing) CPU reset bit of the block reset register
	value =
		BIT(SYS_CTRL_RST_SCU) |
		BIT(SYS_CTRL_RST_ARM0) |
		BIT(SYS_CTRL_RST_ARM1);

	writel(value, SYS_CTRL_RST_SET_CTRL);
}
