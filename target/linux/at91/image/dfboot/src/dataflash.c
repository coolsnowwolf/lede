/*----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 *----------------------------------------------------------------------------
 * The software is delivered "AS IS" without warranty or condition of any
 * kind, either express, implied or statutory. This includes without
 * limitation any warranty or condition with respect to merchantability or
 * fitness for any particular purpose, or against the infringements of
 * intellectual property rights of others.
 *----------------------------------------------------------------------------
 * File Name           : dataflash.c
 * Object              : High level functions for the dataflash
 * Creation            : HIi   10/10/2003
 *----------------------------------------------------------------------------
 */
#include "config.h"
#include "stdio.h"
#include "dataflash.h"


AT91S_DATAFLASH_INFO dataflash_info[CFG_MAX_DATAFLASH_BANKS];
static AT91S_DataFlash DataFlashInst;

int cs[][CFG_MAX_DATAFLASH_BANKS] = {
	{CFG_DATAFLASH_LOGIC_ADDR_CS0, 0},	/* Logical adress, CS */
	{CFG_DATAFLASH_LOGIC_ADDR_CS3, 3}
};

int AT91F_DataflashInit(void)
{
	int i;
	int dfcode;
	int Nb_device = 0;
		
	AT91F_SpiInit();

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {
		dataflash_info[i].id = 0;
		dataflash_info[i].Device.pages_number = 0;
		dfcode = AT91F_DataflashProbe (cs[i][1], &dataflash_info[i].Desc);

		switch (dfcode) {
		case AT45DB161:
			dataflash_info[i].Device.pages_number = 4096;
			dataflash_info[i].Device.pages_size = 528;
			dataflash_info[i].Device.page_offset = 10;
			dataflash_info[i].Device.byte_mask = 0x300;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			Nb_device++;
			break;

		case AT45DB321:
			dataflash_info[i].Device.pages_number = 8192;
			dataflash_info[i].Device.pages_size = 528;
			dataflash_info[i].Device.page_offset = 10;
			dataflash_info[i].Device.byte_mask = 0x300;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			Nb_device++;
			break;

		case AT45DB642:
			dataflash_info[i].Device.pages_number = 8192;
			dataflash_info[i].Device.pages_size = 1056;
			dataflash_info[i].Device.page_offset = 11;
			dataflash_info[i].Device.byte_mask = 0x700;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			Nb_device++;
			break;
		case AT45DB128:
			dataflash_info[i].Device.pages_number = 16384;
			dataflash_info[i].Device.pages_size = 1056;
			dataflash_info[i].Device.page_offset = 11;
			dataflash_info[i].Device.byte_mask = 0x700;
			dataflash_info[i].Device.cs = cs[i][1];
			dataflash_info[i].Desc.DataFlash_state = IDLE;
			dataflash_info[i].logical_address = cs[i][0];
			dataflash_info[i].id = dfcode;
			Nb_device++;
			break;
		default:
			break;
		}
	}			
	return (Nb_device);
}


void AT91F_DataflashPrintInfo(void)
{
	int i;
	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++) {
		if (dataflash_info[i].id != 0) {
			printf ("DF:AT45DB");
			switch (dataflash_info[i].id) {
			case AT45DB161:
				printf ("161");
				break;

			case AT45DB321:
				printf ("321");
				break;

			case AT45DB642:
				printf ("642");
				break;
			case AT45DB128:				
				printf ("128");
				break;
			}

			printf ("\n# PG: %6d\n"
				"PG SZ: %6d\n"
				"SZ=%8d bytes\n"
				"ADDR: %08X\n",
				(unsigned int) dataflash_info[i].Device.pages_number,
				(unsigned int) dataflash_info[i].Device.pages_size,
				(unsigned int) dataflash_info[i].Device.pages_number *
				dataflash_info[i].Device.pages_size,
				(unsigned int) dataflash_info[i].logical_address);
		}
	}
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataflashSelect                                  */
/* Object              : Select the correct device                              */
/*------------------------------------------------------------------------------*/
static AT91PS_DataFlash AT91F_DataflashSelect(AT91PS_DataFlash pFlash,
                                              unsigned int *addr)
{
	char addr_valid = 0;
	int i;

	for (i = 0; i < CFG_MAX_DATAFLASH_BANKS; i++)
		if ((*addr & 0xFF000000) == dataflash_info[i].logical_address) {
			addr_valid = 1;
			break;
		}
	if (!addr_valid) {
		pFlash = (AT91PS_DataFlash) 0;
		return pFlash;
	}
	pFlash->pDataFlashDesc = &(dataflash_info[i].Desc);
	pFlash->pDevice = &(dataflash_info[i].Device);
	*addr -= dataflash_info[i].logical_address;
	return (pFlash);
}


/*------------------------------------------------------------------------------*/
/* Function Name       : read_dataflash                                         */
/* Object              : dataflash memory read                                  */
/*------------------------------------------------------------------------------*/
int read_dataflash(unsigned long addr, unsigned long size, char *result)
{
	unsigned int AddrToRead = addr;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect (pFlash, &AddrToRead);
	if (pFlash == 0)
		return -1;

	return (AT91F_DataFlashRead(pFlash, AddrToRead, size, result));
}


/*-----------------------------------------------------------------------------*/
/* Function Name       : write_dataflash                                       */
/* Object              : write a block in dataflash                            */
/*-----------------------------------------------------------------------------*/
int write_dataflash(unsigned long addr_dest, unsigned int addr_src,
                    unsigned int size)
{
	unsigned int AddrToWrite = addr_dest;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect(pFlash, &AddrToWrite);
	if (AddrToWrite == -1)
		return -1;

	return AT91F_DataFlashWrite(pFlash, (unsigned char *) addr_src, AddrToWrite, size);
}

/*-----------------------------------------------------------------------------*/
/* Function Name       : erase_dataflash                                       */
/* Object              : Erase entire dataflash                                */
/*-----------------------------------------------------------------------------*/
int erase_dataflash(unsigned long addr_dest)
{
	unsigned int AddrToWrite = addr_dest;
	AT91PS_DataFlash pFlash = &DataFlashInst;

	pFlash = AT91F_DataflashSelect (pFlash, &AddrToWrite);
	if (AddrToWrite == -1)
		return -1;

	return AT91F_DataFlashErase(pFlash);
}

