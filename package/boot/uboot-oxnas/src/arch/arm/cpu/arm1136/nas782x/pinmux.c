#include <common.h>
#include <asm/arch/pinmux.h>

void pinmux_set(int bank, int pin, int func)
{
	u32 reg;
	u32 base;
	/* TODO: check parameters */

	if (bank == PINMUX_BANK_MFA)
		base = SYS_CONTROL_BASE;
	else
		base = SEC_CONTROL_BASE;

	clrbits_le32(base + PINMUX_SECONDARY_SEL, BIT(pin));
	clrbits_le32(base + PINMUX_TERTIARY_SEL, BIT(pin));
	clrbits_le32(base + PINMUX_QUATERNARY_SEL, BIT(pin));
	clrbits_le32(base + PINMUX_DEBUG_SEL, BIT(pin));
	clrbits_le32(base + PINMUX_ALTERNATIVE_SEL, BIT(pin));

	switch (func) {
	case PINMUX_GPIO:
	default:
		return;
		break;
	case PINMUX_2:
		reg = base + PINMUX_SECONDARY_SEL;
		break;
	case PINMUX_3:
		reg = base + PINMUX_TERTIARY_SEL;
		break;
	case PINMUX_4:
		reg = base + PINMUX_QUATERNARY_SEL;
		break;
	case PINMUX_DEBUG:
		reg = base + PINMUX_DEBUG_SEL;
		break;
	case PINMUX_ALT:
		reg = base + PINMUX_ALTERNATIVE_SEL;
		break;
	}
	setbits_le32(reg, BIT(pin));
}
