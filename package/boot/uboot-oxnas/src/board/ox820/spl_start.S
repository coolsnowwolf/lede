.section .init
.globl _spl_start
_spl_start:
	b	_start
	b	_start+0x4
	b	_start+0x8
	b	_start+0xc
	b	_start+0x10
	b	_start+0x14
	b	_start+0x18
	b	_start+0x1c
	.space	0x30 - (. - _spl_start)
	.ascii	"BOOT"		/* 0x30 signature*/
	.word	0x50		/* 0x34 header size itself */
	.word	0		/* 0x38 */
	.word	0x5000f000	/* boot report location */
	.word	_start		/* 0x40 */

main_crc_size:	.word	code_size	/* 0x44 filled by linker */
main_crc:	.word	0		/* 0x48 fill later */
header_crc:	.word	0		/* 0x4C header crc*/
