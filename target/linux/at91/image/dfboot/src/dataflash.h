//*---------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*---------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*---------------------------------------------------------------------------
//* File Name           : AT91_SpiDataFlash.h
//* Object              : Data Flash Atmel Description File
//* Translator          :
//*
//* 1.0 03/04/01 HI	: Creation
//*
//*---------------------------------------------------------------------------

#ifndef _DataFlash_h
#define _DataFlash_h

/* Max value = 15Mhz to be compliant with the Continuous array read function */
#ifdef	SPI_LOW_SPEED
#define AT91C_SPI_CLK 14976000/4
#else
#define AT91C_SPI_CLK 14976000 
#endif

/* AC characteristics */
/* DLYBS = tCSS= 250ns min and DLYBCT = tCSH = 250ns */

#define DATAFLASH_TCSS (0xf << 16)	/* 250ns 15/60000000 */
#define DATAFLASH_TCHS (0x1 << 24)	/* 250ns 32*1/60000000 */


#define AT91C_SPI_PCS0_SERIAL_DATAFLASH		0xE     /* Chip Select 0 : NPCS0 %1110 */
#define AT91C_SPI_PCS3_DATAFLASH_CARD		0x7     /* Chip Select 3 : NPCS3 %0111 */

#define CFG_MAX_DATAFLASH_BANKS 	    2
#define CFG_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000
#define CFG_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000

typedef struct {
	unsigned long base;		/* logical base address for a bank */
	unsigned long size;		/* total bank size */
	unsigned long page_count;
	unsigned long page_size;
	unsigned long id;		/* device id */
} dataflash_info_t;

typedef unsigned int AT91S_DataFlashStatus;

/*----------------------------------------------------------------------*/
/* DataFlash Structures							*/
/*----------------------------------------------------------------------*/

/*---------------------------------------------*/
/* DataFlash Descriptor Structure Definition   */
/*---------------------------------------------*/
typedef struct _AT91S_DataflashDesc {
	unsigned char *tx_cmd_pt;
	unsigned int tx_cmd_size;
	unsigned char *rx_cmd_pt;
	unsigned int rx_cmd_size;
	unsigned char *tx_data_pt;
	unsigned int tx_data_size;
	unsigned char *rx_data_pt;
	unsigned int rx_data_size;
	volatile unsigned char DataFlash_state;
	unsigned char command[8];
} AT91S_DataflashDesc, *AT91PS_DataflashDesc;

/*---------------------------------------------*/
/* DataFlash device definition structure       */
/*---------------------------------------------*/
typedef struct _AT91S_Dataflash {
	int pages_number;			/* dataflash page number */
	int pages_size;				/* dataflash page size */
	int page_offset;			/* page offset in command */
	int byte_mask;				/* byte mask in command */
	int cs;
} AT91S_DataflashFeatures, *AT91PS_DataflashFeatures;


/*---------------------------------------------*/
/* DataFlash Structure Definition	       */
/*---------------------------------------------*/
typedef struct _AT91S_DataFlash {
	AT91PS_DataflashDesc pDataFlashDesc;	/* dataflash descriptor */
	AT91PS_DataflashFeatures pDevice;	/* Pointer on a dataflash features array */
} AT91S_DataFlash, *AT91PS_DataFlash;


typedef struct _AT91S_DATAFLASH_INFO {

	AT91S_DataflashDesc 	Desc;
	AT91S_DataflashFeatures Device; /* Pointer on a dataflash features array */
	unsigned long 			logical_address;
	unsigned int 			id;			/* device id */
} AT91S_DATAFLASH_INFO, *AT91PS_DATAFLASH_INFO;


/*-------------------------------------------------------------------------------------------------*/

#define AT45DB161		0x2c
#define AT45DB321		0x34
#define AT45DB642		0x3c
#define AT45DB128		0x10

#define AT91C_DATAFLASH_TIMEOUT			20000	/* For AT91F_DataFlashWaitReady */

/* DataFlash return value */
#define AT91C_DATAFLASH_BUSY			0x00
#define AT91C_DATAFLASH_OK			0x01
#define AT91C_DATAFLASH_ERROR			0x02
#define AT91C_DATAFLASH_MEMORY_OVERFLOW		0x03
#define AT91C_DATAFLASH_BAD_COMMAND		0x04
#define AT91C_DATAFLASH_BAD_ADDRESS		0x05


/* Driver State */
#define IDLE		0x0
#define BUSY		0x1
#define ERROR		0x2

/* DataFlash Driver State */
#define GET_STATUS	0x0F

/*-------------------------------------------------------------------------------------------------*/
/* Command Definition										   */
/*-------------------------------------------------------------------------------------------------*/

/* READ COMMANDS */
#define DB_CONTINUOUS_ARRAY_READ	0xE8	/* Continuous array read */
#define DB_BURST_ARRAY_READ		0xE8	/* Burst array read */
#define DB_PAGE_READ			0xD2	/* Main memory page read */
#define DB_BUF1_READ			0xD4	/* Buffer 1 read */
#define DB_BUF2_READ			0xD6	/* Buffer 2 read */
#define DB_STATUS			0xD7	/* Status Register */

/* PROGRAM and ERASE COMMANDS */
#define DB_BUF1_WRITE			0x84	/* Buffer 1 write */
#define DB_BUF2_WRITE			0x87	/* Buffer 2 write */
#define DB_BUF1_PAGE_ERASE_PGM		0x83	/* Buffer 1 to main memory page program with built-In erase */
#define DB_BUF1_PAGE_ERASE_FASTPGM	0x93	/* Buffer 1 to main memory page program with built-In erase, Fast program */
#define DB_BUF2_PAGE_ERASE_PGM		0x86	/* Buffer 2 to main memory page program with built-In erase */
#define DB_BUF2_PAGE_ERASE_FASTPGM	0x96	/* Buffer 1 to main memory page program with built-In erase, Fast program */
#define DB_BUF1_PAGE_PGM		0x88	/* Buffer 1 to main memory page program without built-In erase */
#define DB_BUF1_PAGE_FASTPGM		0x98	/* Buffer 1 to main memory page program without built-In erase, Fast program */
#define DB_BUF2_PAGE_PGM		0x89	/* Buffer 2 to main memory page program without built-In erase */
#define DB_BUF2_PAGE_FASTPGM		0x99	/* Buffer 1 to main memory page program without built-In erase, Fast program */
#define DB_PAGE_ERASE			0x81	/* Page Erase */
#define DB_BLOCK_ERASE			0x50	/* Block Erase */
#define DB_PAGE_PGM_BUF1		0x82	/* Main memory page through buffer 1 */
#define DB_PAGE_FASTPGM_BUF1		0x92	/* Main memory page through buffer 1, Fast program */
#define DB_PAGE_PGM_BUF2		0x85	/* Main memory page through buffer 2 */
#define DB_PAGE_FastPGM_BUF2		0x95	/* Main memory page through buffer 2, Fast program */

/* ADDITIONAL COMMANDS */
#define DB_PAGE_2_BUF1_TRF		0x53	/* Main memory page to buffer 1 transfert */
#define DB_PAGE_2_BUF2_TRF		0x55	/* Main memory page to buffer 2 transfert */
#define DB_PAGE_2_BUF1_CMP		0x60	/* Main memory page to buffer 1 compare */
#define DB_PAGE_2_BUF2_CMP		0x61	/* Main memory page to buffer 2 compare */
#define DB_AUTO_PAGE_PGM_BUF1		0x58	/* Auto page rewrite throught buffer 1 */
#define DB_AUTO_PAGE_PGM_BUF2		0x59	/* Auto page rewrite throught buffer 2 */

/*-------------------------------------------------------------------------------------------------*/

extern AT91S_DATAFLASH_INFO dataflash_info[CFG_MAX_DATAFLASH_BANKS];

extern void AT91F_SpiInit(void);
extern int AT91F_DataflashProbe(int i, AT91PS_DataflashDesc pDesc);
extern int AT91F_DataFlashRead(AT91PS_DataFlash, unsigned long , unsigned long, char *);
extern AT91S_DataFlashStatus AT91F_DataFlashWrite(AT91PS_DataFlash ,unsigned char *, int, int);
extern AT91S_DataFlashStatus AT91F_DataFlashErase(AT91PS_DataFlash pDataFlash);
extern int AT91F_DataflashInit(void);
extern void AT91F_DataflashPrintInfo(void);
extern int read_dataflash(unsigned long addr, unsigned long size, char *result);
extern int write_dataflash(unsigned long addr_dest, unsigned int addr_src, unsigned int size);
extern int erase_dataflash(unsigned long addr_dest);

#endif
