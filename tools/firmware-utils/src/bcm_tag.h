#ifndef __BCM63XX_TAG_H
#define __BCM63XX_TAG_H

#define TAGVER_LEN 4                   /* Length of Tag Version */
#define TAGLAYOUT_LEN 4                /* Length of FlashLayoutVer */
#define SIG1_LEN 20		       /* Company Signature 1 Length */
#define SIG2_LEN 14                    /* Company Signature 2 Lenght */
#define BOARDID_LEN 16		       /* Length of BoardId */
#define ENDIANFLAG_LEN 2               /* Endian Flag Length */
#define CHIPID_LEN 6		       /* Chip Id Length */
#define IMAGE_LEN 10                   /* Length of Length Field */
#define ADDRESS_LEN 12                 /* Length of Address field */
#define DUALFLAG_LEN 2		       /* Dual Image flag Length */
#define INACTIVEFLAG_LEN 2	       /* Inactie Flag Length */
#define RSASIG_LEN 20   	       /* Length of RSA Signature in tag */
#define TAGINFO1_LEN 30                /* Length of vendor information field1 in tag */
#define FLASHLAYOUTVER_LEN 4	       /* Length of Flash Layout Version String tag */
#define TAGINFO2_LEN 16                /* Length of vendor information field2 in tag */
#define CRC_LEN 4                      /* Length of CRC in bytes */
#define ALTTAGINFO_LEN 54              /* Alternate length for vendor information; Pirelli */

#define NUM_PIRELLI 2
#define IMAGETAG_CRC_START		0xFFFFFFFF

#define PIRELLI_BOARDS { \
  "AGPF-S0", \
  "DWV-S0", \
}

/*
 * The broadcom firmware assumes the rootfs starts the image,
 * therefore uses the rootfs start (flashImageAddress)
 * to determine where to flash the image.  Since we have the kernel first
 * we have to give it the kernel address, but the crc uses the length
 * associated with this address (rootLength), which is added to the kernel
 * length (kernelLength) to determine the length of image to flash and thus
 * needs to be rootfs + deadcode (jffs2 EOF marker)
*/

struct bcm_tag {
	char tagVersion[TAGVER_LEN];           // 0-3: Version of the image tag
	char sig_1[SIG1_LEN];                  // 4-23: Company Line 1
	char sig_2[SIG2_LEN];                  // 24-37: Company Line 2
	char chipid[CHIPID_LEN];               // 38-43: Chip this image is for
	char boardid[BOARDID_LEN];             // 44-59: Board name
	char big_endian[ENDIANFLAG_LEN];       // 60-61: Map endianness -- 1 BE 0 LE
	char totalLength[IMAGE_LEN];           // 62-71: Total length of image
	char cfeAddress[ADDRESS_LEN];          // 72-83: Address in memory of CFE
	char cfeLength[IMAGE_LEN];             // 84-93: Size of CFE
	char flashImageStart[ADDRESS_LEN];     // 94-105: Address in memory of image start (kernel for OpenWRT, rootfs for stock firmware)
	char flashRootLength[IMAGE_LEN];            // 106-115: Size of rootfs for flashing
	char kernelAddress[ADDRESS_LEN];       // 116-127: Address in memory of kernel
	char kernelLength[IMAGE_LEN];          // 128-137: Size of kernel
	char dualImage[DUALFLAG_LEN];          // 138-139: Unused at present
	char inactiveFlag[INACTIVEFLAG_LEN];   // 140-141: Unused at present
        char rsa_signature[RSASIG_LEN];        // 142-161: RSA Signature (unused at present; some vendors may use this)
	char information1[TAGINFO1_LEN];       // 162-191: Compilation and related information (not generated/used by OpenWRT)
        char flashLayoutVer[FLASHLAYOUTVER_LEN];// 192-195: Version flash layout
        char fskernelCRC[CRC_LEN];             // 196-199: kernel+rootfs CRC32
	char information2[TAGINFO2_LEN];       // 200-215: Unused at present except Alice Gate where is is information
	char imageCRC[CRC_LEN];                // 216-219: CRC32 of image less imagetag (kernel for Alice Gate)
        char rootfsCRC[CRC_LEN];               // 220-223: CRC32 of rootfs partition
        char kernelCRC[CRC_LEN];               // 224-227: CRC32 of kernel partition
        char imageSequence[4];		       // 228-231: Image sequence number
        char rootLength[4];                    // 232-235: steal from reserved1 to keep the real root length so we can use in the flash map even after we have change the rootLength to 0 to satisfy devices that check CRC on every boot
        char headerCRC[CRC_LEN];               // 236-239: CRC32 of header excluding tagVersion
        char reserved2[16];                    // 240-255: Unused at present
};

#endif /* __BCM63XX_TAG_H */
