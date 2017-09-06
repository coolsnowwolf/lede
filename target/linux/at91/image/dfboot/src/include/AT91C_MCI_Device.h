//*---------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*---------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*---------------------------------------------------------------------------
//* File Name           : AT91C_MCI_Device.h
//* Object              : Data Flash Atmel Description File
//* Translator          :
//*
//* 1.0 26/11/02 FB		: Creation
//*---------------------------------------------------------------------------

#ifndef AT91C_MCI_Device_h
#define AT91C_MCI_Device_h

#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"

typedef unsigned int AT91S_MCIDeviceStatus;

/////////////////////////////////////////////////////////////////////////////////////////////////////

#define AT91C_CARD_REMOVED			0
#define AT91C_MMC_CARD_INSERTED		1
#define AT91C_SD_CARD_INSERTED		2

#define AT91C_NO_ARGUMENT			0x0

#define AT91C_FIRST_RCA				0xCAFE
#define AT91C_MAX_MCI_CARDS			10

#define AT91C_BUS_WIDTH_1BIT		0x00
#define AT91C_BUS_WIDTH_4BITS		0x02

/* Driver State */
#define AT91C_MCI_IDLE       		0x0
#define AT91C_MCI_TIMEOUT_ERROR		0x1
#define AT91C_MCI_RX_SINGLE_BLOCK	0x2
#define AT91C_MCI_RX_MULTIPLE_BLOCK	0x3
#define AT91C_MCI_RX_STREAM			0x4
#define AT91C_MCI_TX_SINGLE_BLOCK	0x5
#define AT91C_MCI_TX_MULTIPLE_BLOCK	0x6
#define AT91C_MCI_TX_STREAM 		0x7

/* TimeOut */
#define AT91C_TIMEOUT_CMDRDY		30

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MMC & SDCard Structures 
/////////////////////////////////////////////////////////////////////////////////////////////////////

/*-----------------------------------------------*/
/* SDCard Device Descriptor Structure Definition */
/*-----------------------------------------------*/
typedef struct	_AT91S_MciDeviceDesc
{
    volatile unsigned char	state;
	unsigned char			SDCard_bus_width;

} AT91S_MciDeviceDesc, *AT91PS_MciDeviceDesc;

/*---------------------------------------------*/
/* MMC & SDCard Structure Device Features	   */
/*---------------------------------------------*/
typedef struct	_AT91S_MciDeviceFeatures
{
    unsigned char	Card_Inserted;				// (0=AT91C_CARD_REMOVED) (1=AT91C_MMC_CARD_INSERTED) (2=AT91C_SD_CARD_INSERTED)
    unsigned int 	Relative_Card_Address;		// RCA
	unsigned int 	Max_Read_DataBlock_Length;	// 2^(READ_BL_LEN) in CSD 
	unsigned int 	Max_Write_DataBlock_Length;	// 2^(WRITE_BL_LEN) in CSD
	unsigned char	Read_Partial;				// READ_BL_PARTIAL
	unsigned char	Write_Partial;				// WRITE_BL_PARTIAL
	unsigned char	Erase_Block_Enable;			// ERASE_BLK_EN
	unsigned char	Read_Block_Misalignment;	// READ_BLK_MISALIGN
	unsigned char	Write_Block_Misalignment;	// WRITE_BLK_MISALIGN
	unsigned char	Sector_Size;				// SECTOR_SIZE
	unsigned int	Memory_Capacity;			// Size in bits of the device
	
}	AT91S_MciDeviceFeatures, *AT91PS_MciDeviceFeatures ;

/*---------------------------------------------*/
/* MCI Device Structure Definition 			   */
/*---------------------------------------------*/
typedef struct _AT91S_MciDevice
{
	AT91PS_MciDeviceDesc		 	pMCI_DeviceDesc;	// MCI device descriptor
	AT91PS_MciDeviceFeatures		pMCI_DeviceFeatures;// Pointer on a MCI device features array  
}AT91S_MciDevice, *AT91PS_MciDevice;

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MCI_CMD Register Value 
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define AT91C_POWER_ON_INIT						(0	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_INIT | AT91C_MCI_OPDCMD)

/////////////////////////////////////////////////////////////////	
// Class 0 & 1 commands: Basic commands and Read Stream commands
/////////////////////////////////////////////////////////////////

#define AT91C_GO_IDLE_STATE_CMD					(0 	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE )
#define AT91C_MMC_GO_IDLE_STATE_CMD				(0 	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE  | AT91C_MCI_OPDCMD)
#define AT91C_MMC_SEND_OP_COND_CMD				(1	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 | AT91C_MCI_OPDCMD)
#define AT91C_ALL_SEND_CID_CMD					(2	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 )
#define AT91C_MMC_ALL_SEND_CID_CMD				(2	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 | AT91C_MCI_OPDCMD)
#define AT91C_SET_RELATIVE_ADDR_CMD				(3	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48		| AT91C_MCI_MAXLAT )
#define AT91C_MMC_SET_RELATIVE_ADDR_CMD			(3	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48		| AT91C_MCI_MAXLAT | AT91C_MCI_OPDCMD)

#define AT91C_SET_DSR_CMD						(4	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_NO		| AT91C_MCI_MAXLAT )	// no tested

#define AT91C_SEL_DESEL_CARD_CMD				(7	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48  		| AT91C_MCI_MAXLAT )
#define AT91C_SEND_CSD_CMD						(9	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 		| AT91C_MCI_MAXLAT )
#define AT91C_SEND_CID_CMD						(10	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_136 		| AT91C_MCI_MAXLAT )
#define AT91C_MMC_READ_DAT_UNTIL_STOP_CMD		(11	| AT91C_MCI_TRTYP_STREAM| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRDIR	| AT91C_MCI_TRCMD_START | AT91C_MCI_MAXLAT )

#define AT91C_STOP_TRANSMISSION_CMD				(12	| AT91C_MCI_TRCMD_STOP 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 		| AT91C_MCI_MAXLAT )
#define AT91C_STOP_TRANSMISSION_SYNC_CMD		(12	| AT91C_MCI_TRCMD_STOP 	| AT91C_MCI_SPCMD_SYNC	| AT91C_MCI_RSPTYP_48 		| AT91C_MCI_MAXLAT )
#define AT91C_SEND_STATUS_CMD					(13	| AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 		| AT91C_MCI_MAXLAT )
#define AT91C_GO_INACTIVE_STATE_CMD				(15	| AT91C_MCI_RSPTYP_NO )

//*------------------------------------------------
//* Class 2 commands: Block oriented Read commands
//*------------------------------------------------

#define AT91C_SET_BLOCKLEN_CMD					(16 | AT91C_MCI_TRCMD_NO 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48		| AT91C_MCI_MAXLAT )
#define AT91C_READ_SINGLE_BLOCK_CMD				(17 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 	| AT91C_MCI_TRCMD_START	| AT91C_MCI_TRTYP_BLOCK	| AT91C_MCI_TRDIR	| AT91C_MCI_MAXLAT)
#define AT91C_READ_MULTIPLE_BLOCK_CMD			(18 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 	| AT91C_MCI_TRCMD_START	| AT91C_MCI_TRTYP_MULTIPLE	| AT91C_MCI_TRDIR	| AT91C_MCI_MAXLAT)

//*--------------------------------------------
//* Class 3 commands: Sequential write commands
//*--------------------------------------------

#define AT91C_MMC_WRITE_DAT_UNTIL_STOP_CMD		(20 | AT91C_MCI_TRTYP_STREAM| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48 & ~(AT91C_MCI_TRDIR) | AT91C_MCI_TRCMD_START | AT91C_MCI_MAXLAT )	// MMC

//*------------------------------------------------
//* Class 4 commands: Block oriented write commands
//*------------------------------------------------
	
#define AT91C_WRITE_BLOCK_CMD					(24 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_START	| (AT91C_MCI_TRTYP_BLOCK 	&  ~(AT91C_MCI_TRDIR))	| AT91C_MCI_MAXLAT)
#define AT91C_WRITE_MULTIPLE_BLOCK_CMD			(25 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_START	| (AT91C_MCI_TRTYP_MULTIPLE	&  ~(AT91C_MCI_TRDIR)) 	| AT91C_MCI_MAXLAT)
#define AT91C_PROGRAM_CSD_CMD					(27 | AT91C_MCI_RSPTYP_48 )


//*----------------------------------------
//* Class 6 commands: Group Write protect
//*----------------------------------------

#define AT91C_SET_WRITE_PROT_CMD				(28	| AT91C_MCI_RSPTYP_48 )
#define AT91C_CLR_WRITE_PROT_CMD				(29	| AT91C_MCI_RSPTYP_48 )
#define AT91C_SEND_WRITE_PROT_CMD				(30	| AT91C_MCI_RSPTYP_48 )


//*----------------------------------------
//* Class 5 commands: Erase commands
//*----------------------------------------

#define AT91C_TAG_SECTOR_START_CMD				(32 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_TAG_SECTOR_END_CMD  				(33 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_MMC_UNTAG_SECTOR_CMD				(34 | AT91C_MCI_RSPTYP_48 )
#define AT91C_MMC_TAG_ERASE_GROUP_START_CMD		(35 | AT91C_MCI_RSPTYP_48 )
#define AT91C_MMC_TAG_ERASE_GROUP_END_CMD		(36 | AT91C_MCI_RSPTYP_48 )
#define AT91C_MMC_UNTAG_ERASE_GROUP_CMD			(37 | AT91C_MCI_RSPTYP_48 )
#define AT91C_ERASE_CMD							(38 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT )

//*----------------------------------------
//* Class 7 commands: Lock commands
//*----------------------------------------

#define AT91C_LOCK_UNLOCK						(42 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)	// no tested

//*-----------------------------------------------
// Class 8 commands: Application specific commands
//*-----------------------------------------------

#define AT91C_APP_CMD							(55 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO | AT91C_MCI_MAXLAT)
#define AT91C_GEN_CMD							(56 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO | AT91C_MCI_MAXLAT)	// no tested

#define AT91C_SDCARD_SET_BUS_WIDTH_CMD			(6 	| AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_SDCARD_STATUS_CMD					(13 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_SDCARD_SEND_NUM_WR_BLOCKS_CMD		(22 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_SDCARD_SET_WR_BLK_ERASE_COUNT_CMD	(23 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_SDCARD_APP_OP_COND_CMD			(41 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO )
#define AT91C_SDCARD_SET_CLR_CARD_DETECT_CMD	(42 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)
#define AT91C_SDCARD_SEND_SCR_CMD				(51 | AT91C_MCI_SPCMD_NONE	| AT91C_MCI_RSPTYP_48	| AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)

#define AT91C_SDCARD_APP_ALL_CMD				(AT91C_SDCARD_SET_BUS_WIDTH_CMD +\
												AT91C_SDCARD_STATUS_CMD +\
												AT91C_SDCARD_SEND_NUM_WR_BLOCKS_CMD +\
												AT91C_SDCARD_SET_WR_BLK_ERASE_COUNT_CMD +\
												AT91C_SDCARD_APP_OP_COND_CMD +\
												AT91C_SDCARD_SET_CLR_CARD_DETECT_CMD +\
												AT91C_SDCARD_SEND_SCR_CMD)

//*----------------------------------------
//* Class 9 commands: IO Mode commands
//*----------------------------------------

#define AT91C_MMC_FAST_IO_CMD					(39 | AT91C_MCI_SPCMD_NONE | AT91C_MCI_RSPTYP_48 | AT91C_MCI_MAXLAT)
#define AT91C_MMC_GO_IRQ_STATE_CMD				(40 | AT91C_MCI_SPCMD_NONE | AT91C_MCI_RSPTYP_48 | AT91C_MCI_TRCMD_NO	| AT91C_MCI_MAXLAT)

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions returnals
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define AT91C_CMD_SEND_OK					0		// Command ok
#define AT91C_CMD_SEND_ERROR				-1		// Command failed
#define AT91C_INIT_OK						2		// Init Successfull
#define AT91C_INIT_ERROR					3		// Init Failed
#define AT91C_READ_OK						4		// Read Successfull
#define AT91C_READ_ERROR					5		// Read Failed
#define AT91C_WRITE_OK						6		// Write Successfull
#define AT91C_WRITE_ERROR					7		// Write Failed
#define AT91C_ERASE_OK						8		// Erase Successfull
#define AT91C_ERASE_ERROR					9		// Erase Failed
#define AT91C_CARD_SELECTED_OK				10		// Card Selection Successfull
#define AT91C_CARD_SELECTED_ERROR			11		// Card Selection Failed

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MCI_SR Errors
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define 	AT91C_MCI_SR_ERROR		(AT91C_MCI_UNRE |\
									 AT91C_MCI_OVRE |\
									 AT91C_MCI_DTOE |\
									 AT91C_MCI_DCRCE |\
									 AT91C_MCI_RTOE |\
									 AT91C_MCI_RENDE |\
									 AT91C_MCI_RCRCE |\
									 AT91C_MCI_RDIRE |\
									 AT91C_MCI_RINDE)

////////////////////////////////////////////////////////////////////////////////////////////////////
// OCR Register
////////////////////////////////////////////////////////////////////////////////////////////////////
#define AT91C_VDD_16_17					(1 << 4)
#define AT91C_VDD_17_18					(1 << 5)
#define AT91C_VDD_18_19					(1 << 6)
#define AT91C_VDD_19_20					(1 << 7)
#define AT91C_VDD_20_21					(1 << 8)
#define AT91C_VDD_21_22					(1 << 9)
#define AT91C_VDD_22_23					(1 << 10)
#define AT91C_VDD_23_24					(1 << 11)
#define AT91C_VDD_24_25					(1 << 12)
#define AT91C_VDD_25_26					(1 << 13)
#define AT91C_VDD_26_27					(1 << 14)
#define AT91C_VDD_27_28					(1 << 15)
#define AT91C_VDD_28_29					(1 << 16)
#define AT91C_VDD_29_30					(1 << 17)
#define AT91C_VDD_30_31					(1 << 18)
#define AT91C_VDD_31_32					(1 << 19)
#define AT91C_VDD_32_33					(1 << 20)
#define AT91C_VDD_33_34					(1 << 21)
#define AT91C_VDD_34_35					(1 << 22)
#define AT91C_VDD_35_36					(1 << 23)
#define AT91C_CARD_POWER_UP_BUSY		(1 << 31)

#define AT91C_MMC_HOST_VOLTAGE_RANGE	(AT91C_VDD_27_28 +\
										AT91C_VDD_28_29 +\
										AT91C_VDD_29_30 +\
										AT91C_VDD_30_31 +\
										AT91C_VDD_31_32 +\
										AT91C_VDD_32_33)

////////////////////////////////////////////////////////////////////////////////////////////////////
// CURRENT_STATE & READY_FOR_DATA in SDCard Status Register definition (response type R1)
////////////////////////////////////////////////////////////////////////////////////////////////////
#define AT91C_SR_READY_FOR_DATA				(1 << 8)	// corresponds to buffer empty signalling on the bus
#define AT91C_SR_IDLE						(0 << 9)
#define AT91C_SR_READY						(1 << 9)
#define AT91C_SR_IDENT						(2 << 9)
#define AT91C_SR_STBY						(3 << 9)
#define AT91C_SR_TRAN						(4 << 9)
#define AT91C_SR_DATA						(5 << 9)
#define AT91C_SR_RCV						(6 << 9)
#define AT91C_SR_PRG						(7 << 9)
#define AT91C_SR_DIS						(8 << 9)

#define AT91C_SR_CARD_SELECTED				(AT91C_SR_READY_FOR_DATA + AT91C_SR_TRAN)

/////////////////////////////////////////////////////////////////////////////////////////////////////
// MMC CSD register header File					
// AT91C_CSD_xxx_S	for shift value
// AT91C_CSD_xxx_M	for mask  value
/////////////////////////////////////////////////////////////////////////////////////////////////////

// First Response INT <=> CSD[3] : bits 0 to 31
#define	AT91C_CSD_BIT0_S			0		// [0:0]			
#define	AT91C_CSD_BIT0_M			0x01				
#define	AT91C_CSD_CRC_S				1		// [7:1]
#define	AT91C_CSD_CRC_M				0x7F
#define	AT91C_CSD_MMC_ECC_S			8		// [9:8]		reserved for MMC compatibility
#define	AT91C_CSD_MMC_ECC_M			0x03
#define	AT91C_CSD_FILE_FMT_S		10		// [11:10]
#define	AT91C_CSD_FILE_FMT_M		0x03
#define	AT91C_CSD_TMP_WP_S			12		// [12:12]
#define	AT91C_CSD_TMP_WP_M			0x01
#define	AT91C_CSD_PERM_WP_S 		13		// [13:13]
#define	AT91C_CSD_PERM_WP_M 		0x01
#define	AT91C_CSD_COPY_S	 		14		// [14:14]
#define	AT91C_CSD_COPY_M 			0x01
#define	AT91C_CSD_FILE_FMT_GRP_S	15		// [15:15]
#define	AT91C_CSD_FILE_FMT_GRP_M	0x01
//	reserved						16		// [20:16]
//	reserved						0x1F
#define	AT91C_CSD_WBLOCK_P_S 		21		// [21:21]
#define	AT91C_CSD_WBLOCK_P_M 		0x01
#define	AT91C_CSD_WBLEN_S 			22		// [25:22]
#define	AT91C_CSD_WBLEN_M 			0x0F
#define	AT91C_CSD_R2W_F_S 			26		// [28:26]
#define	AT91C_CSD_R2W_F_M 			0x07
#define	AT91C_CSD_MMC_DEF_ECC_S		29		// [30:29]		reserved for MMC compatibility
#define	AT91C_CSD_MMC_DEF_ECC_M		0x03
#define	AT91C_CSD_WP_GRP_EN_S		31		// [31:31]
#define	AT91C_CSD_WP_GRP_EN_M 		0x01

// Seconde Response INT <=> CSD[2] : bits 32 to 63
#define	AT91C_CSD_v21_WP_GRP_SIZE_S	0		// [38:32]				
#define	AT91C_CSD_v21_WP_GRP_SIZE_M	0x7F				
#define	AT91C_CSD_v21_SECT_SIZE_S	7		// [45:39]
#define	AT91C_CSD_v21_SECT_SIZE_M	0x7F
#define	AT91C_CSD_v21_ER_BLEN_EN_S	14		// [46:46]
#define	AT91C_CSD_v21_ER_BLEN_EN_M	0x01

#define	AT91C_CSD_v22_WP_GRP_SIZE_S	0		// [36:32]				
#define	AT91C_CSD_v22_WP_GRP_SIZE_M	0x1F				
#define	AT91C_CSD_v22_ER_GRP_SIZE_S	5		// [41:37]
#define	AT91C_CSD_v22_ER_GRP_SIZE_M	0x1F
#define	AT91C_CSD_v22_SECT_SIZE_S	10		// [46:42]
#define	AT91C_CSD_v22_SECT_SIZE_M	0x1F

#define	AT91C_CSD_C_SIZE_M_S		15		// [49:47]
#define	AT91C_CSD_C_SIZE_M_M		0x07
#define	AT91C_CSD_VDD_WMAX_S 		18		// [52:50]
#define	AT91C_CSD_VDD_WMAX_M 		0x07
#define	AT91C_CSD_VDD_WMIN_S	 	21		// [55:53]
#define	AT91C_CSD_VDD_WMIN_M 		0x07
#define	AT91C_CSD_RCUR_MAX_S 		24		// [58:56]
#define	AT91C_CSD_RCUR_MAX_M 		0x07
#define	AT91C_CSD_RCUR_MIN_S 		27		// [61:59]
#define	AT91C_CSD_RCUR_MIN_M 		0x07
#define	AT91C_CSD_CSIZE_L_S 		30		// [63:62] <=> 2 LSB of CSIZE
#define	AT91C_CSD_CSIZE_L_M 		0x03

// Third Response INT <=> CSD[1] : bits 64 to 95
#define	AT91C_CSD_CSIZE_H_S 		0		// [73:64]	<=> 10 MSB of CSIZE
#define	AT91C_CSD_CSIZE_H_M 		0x03FF
// reserved							10		// [75:74]
// reserved							0x03		
#define	AT91C_CSD_DSR_I_S 			12		// [76:76]
#define	AT91C_CSD_DSR_I_M 			0x01
#define	AT91C_CSD_RD_B_MIS_S 		13		// [77:77]
#define	AT91C_CSD_RD_B_MIS_M 		0x01
#define	AT91C_CSD_WR_B_MIS_S 		14		// [78:78]
#define	AT91C_CSD_WR_B_MIS_M 		0x01
#define	AT91C_CSD_RD_B_PAR_S 		15		// [79:79]
#define	AT91C_CSD_RD_B_PAR_M 		0x01
#define	AT91C_CSD_RD_B_LEN_S 		16		// [83:80]
#define	AT91C_CSD_RD_B_LEN_M 		0x0F
#define	AT91C_CSD_CCC_S	 			20		// [95:84]
#define	AT91C_CSD_CCC_M 			0x0FFF

// Fourth Response INT <=> CSD[0] : bits 96 to 127
#define	AT91C_CSD_TRANS_SPEED_S 	0		// [103:96]
#define	AT91C_CSD_TRANS_SPEED_M 	0xFF
#define	AT91C_CSD_NSAC_S 			8		// [111:104]
#define	AT91C_CSD_NSAC_M 			0xFF
#define	AT91C_CSD_TAAC_S 			16		// [119:112]
#define	AT91C_CSD_TAAC_M 			0xFF
//	reserved						24		// [121:120]
//	reserved						0x03
#define	AT91C_CSD_MMC_SPEC_VERS_S	26		// [125:122]	reserved for MMC compatibility
#define	AT91C_CSD_MMC_SPEC_VERS_M	0x0F
#define	AT91C_CSD_STRUCT_S			30		// [127:126]
#define	AT91C_CSD_STRUCT_M 			0x03

/////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

