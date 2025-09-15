	// Arm assembly to copy the Gemini kernel on Storlink reference
	// designs and derived devices with the same flash layout and
	// boot loader.
	//
	// This will execute at 0x01600000
	//
	// Copies the kernel from two fragments (originally zImage
	// and initramdisk) to 0x00400000 making space for a kernel
	// image of up to 8 MB except for these 512 bytes used for
	// this bootstrap.
	//
	// 0x01600200 .. 0x017fffff -> 0x00400000 .. 0x005ffdff
	// 0x00800000 .. 0x00dfffff -> 0x005ffe00 .. 0x00bffdff

	// Memory used for this bootstrap
	.equ BOOT_HEADROOM,	0x200

	.global _start // Stand-alone assembly code
_start:
	mov r1, #0x01600000
	mov r2, #0x00400000
	mov r3, #0x00200000
	add r1, r1, #BOOT_HEADROOM
	sub r3, r3, #BOOT_HEADROOM
copyloop1:
	ldr r0, [r1]
	str r0, [r2]
	add r1, r1, #4
	add r2, r2, #4
	sub r3, r3, #4
	cmp r3, #0
	bne copyloop1
	mov r1, #0x00800000
	mov r3, #0x00600000
copyloop2:
	ldr r0, [r1]
	str r0, [r2]
	add r1, r1, #4
	add r2, r2, #4
	sub r3, r3, #4
	cmp r3, #0
	bne copyloop2
	mov r0, #0x00400000
	// Let's go
	mov pc, r0
