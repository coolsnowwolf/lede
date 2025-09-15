/*----------------------------------------------------------------------------
 *      ATMEL Microcontroller Software Support  -  ROUSSET  -
 *----------------------------------------------------------------------------
 * The software is delivered "AS IS" without warranty or condition of any
 * kind, either express, implied or statutory. This includes without
 * limitation any warranty or condition with respect to merchantability or
 * fitness for any particular purpose, or against the infringements of
 * intellectual property rights of others.
 *----------------------------------------------------------------------------
 * File Name           : at45c.h
 * Object              : 
 *
 * 1.0  10/12/03 HIi    : Creation.
 * 1.01 03/05/04 HIi    : Bug Fix in AT91F_DataFlashWaitReady() Function.
 *----------------------------------------------------------------------------
 */
#include "config.h"
#include "stdio.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "dataflash.h"
#include "main.h"


/*----------------------------------------------------------------------------*/
/* \fn    AT91F_SpiInit                                                       */
/* \brief SPI Low level Init                                                  */
/*----------------------------------------------------------------------------*/
void AT91F_SpiInit(void) {
	/* Configure PIOs */
	AT91C_BASE_PIOA->PIO_ASR = AT91C_PA3_NPCS0 | AT91C_PA4_NPCS1 | 
	                           AT91C_PA1_MOSI | AT91C_PA5_NPCS2 |
	                           AT91C_PA6_NPCS3 | AT91C_PA0_MISO |
	                           AT91C_PA2_SPCK;
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA3_NPCS0 | AT91C_PA4_NPCS1 |
	                           AT91C_PA1_MOSI | AT91C_PA5_NPCS2 |
	                           AT91C_PA6_NPCS3 | AT91C_PA0_MISO |
	                           AT91C_PA2_SPCK;
	/* Enable CLock */
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI;

	/* Reset the SPI */
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SWRST;

	/* Configure SPI in Master Mode with No CS selected !!! */
	AT91C_BASE_SPI->SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS | AT91C_SPI_PCS;

	/* Configure CS0 and CS3 */
	*(AT91C_SPI_CSR + 0) = AT91C_SPI_CPOL | (AT91C_SPI_DLYBS & DATAFLASH_TCSS) |
	                       (AT91C_SPI_DLYBCT & DATAFLASH_TCHS) |
	                       ((AT91C_MASTER_CLOCK / (2*AT91C_SPI_CLK)) << 8);
	*(AT91C_SPI_CSR + 3) = AT91C_SPI_CPOL | (AT91C_SPI_DLYBS & DATAFLASH_TCSS) |
	                       (AT91C_SPI_DLYBCT & DATAFLASH_TCHS) |
	                       ((AT91C_MASTER_CLOCK / (2*AT91C_SPI_CLK)) << 8);
}


/*----------------------------------------------------------------------------*/
/* \fn    AT91F_SpiEnable                                                     */
/* \brief Enable SPI chip select                                              */
/*----------------------------------------------------------------------------*/
static void AT91F_SpiEnable(int cs) {
	switch(cs) {
	case 0:	/* Configure SPI CS0 for Serial DataFlash AT45DBxx */
		AT91C_BASE_SPI->SPI_MR &= 0xFFF0FFFF;
		AT91C_BASE_SPI->SPI_MR |= ((AT91C_SPI_PCS0_SERIAL_DATAFLASH << 16) & AT91C_SPI_PCS);
		break;
	case 3:	/* Configure SPI CS3 for Serial DataFlash Card */
		/* Set up PIO SDC_TYPE to switch on DataFlash Card and not MMC/SDCard */
		AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB7;	/* Set in PIO mode */
		AT91C_BASE_PIOB->PIO_OER = AT91C_PIO_PB7;	/* Configure in output */
		/* Clear Output */
		AT91C_BASE_PIOB->PIO_CODR = AT91C_PIO_PB7;
		/* Configure PCS */
		AT91C_BASE_SPI->SPI_MR &= 0xFFF0FFFF;
		AT91C_BASE_SPI->SPI_MR |= ((AT91C_SPI_PCS3_DATAFLASH_CARD<<16) & AT91C_SPI_PCS);
		break;
	}

	/* SPI_Enable */
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIEN;
}

/*----------------------------------------------------------------------------*/
/* \fn    AT91F_SpiWrite                                                      */
/* \brief Set the PDC registers for a transfert                               */
/*----------------------------------------------------------------------------*/
static unsigned int AT91F_SpiWrite(AT91PS_DataflashDesc pDesc)
{
   	unsigned int timeout;
	
   	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;

   	/* Initialize the Transmit and Receive Pointer */
    	AT91C_BASE_SPI->SPI_RPR = (unsigned int)pDesc->rx_cmd_pt ;
    	AT91C_BASE_SPI->SPI_TPR = (unsigned int)pDesc->tx_cmd_pt ;

    	/* Intialize the Transmit and Receive Counters */
    	AT91C_BASE_SPI->SPI_RCR = pDesc->rx_cmd_size;
    	AT91C_BASE_SPI->SPI_TCR = pDesc->tx_cmd_size;

	if ( pDesc->tx_data_size != 0 ) {
	   	/* Initialize the Next Transmit and Next Receive Pointer */
	   	AT91C_BASE_SPI->SPI_RNPR = (unsigned int)pDesc->rx_data_pt ;
		AT91C_BASE_SPI->SPI_TNPR = (unsigned int)pDesc->tx_data_pt ;

		/* Intialize the Next Transmit and Next Receive Counters */
		AT91C_BASE_SPI->SPI_RNCR = pDesc->rx_data_size ;
	 	AT91C_BASE_SPI->SPI_TNCR = pDesc->tx_data_size ;
   	}

	/* ARM simple, non interrupt dependent timer */
	timeout = 0;

	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTEN + AT91C_PDC_RXTEN;
	while(!(AT91C_BASE_SPI->SPI_SR & AT91C_SPI_RXBUFF));

   	AT91C_BASE_SPI->SPI_PTCR = AT91C_PDC_TXTDIS + AT91C_PDC_RXTDIS;

	if (timeout >= AT91C_DATAFLASH_TIMEOUT){
		return AT91C_DATAFLASH_ERROR;
	}

	return AT91C_DATAFLASH_OK;
}


/*----------------------------------------------------------------------*/
/* \fn    AT91F_DataFlashSendCommand					*/
/* \brief Generic function to send a command to the dataflash		*/
/*----------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_DataFlashSendCommand(
	AT91PS_DataFlash pDataFlash,
	unsigned char OpCode,
	unsigned int CmdSize,
	unsigned int DataflashAddress)
{
	unsigned int adr;

	/* process the address to obtain page address and byte address */
	adr = ((DataflashAddress / (pDataFlash->pDevice->pages_size)) 
	        << pDataFlash->pDevice->page_offset) +
	        (DataflashAddress % (pDataFlash->pDevice->pages_size));

	/* fill the  command  buffer */
	pDataFlash->pDataFlashDesc->command[0] = OpCode;
	if (pDataFlash->pDevice->pages_number >= 16384)
	{
		pDataFlash->pDataFlashDesc->command[1] = (unsigned char)((adr & 0x0F000000) >> 24);
		pDataFlash->pDataFlashDesc->command[2] = (unsigned char)((adr & 0x00FF0000) >> 16);
		pDataFlash->pDataFlashDesc->command[3] = (unsigned char)((adr & 0x0000FF00) >> 8);
		pDataFlash->pDataFlashDesc->command[4] = (unsigned char)(adr & 0x000000FF);
	}
	else
	{	
		pDataFlash->pDataFlashDesc->command[1] = (unsigned char)((adr & 0x00FF0000) >> 16);
		pDataFlash->pDataFlashDesc->command[2] = (unsigned char)((adr & 0x0000FF00) >> 8);
		pDataFlash->pDataFlashDesc->command[3] = (unsigned char)(adr & 0x000000FF) ;
		pDataFlash->pDataFlashDesc->command[4] = 0;
	}
	pDataFlash->pDataFlashDesc->command[5] = 0;
	pDataFlash->pDataFlashDesc->command[6] = 0;
	pDataFlash->pDataFlashDesc->command[7] = 0;

	/* Initialize the SpiData structure for the spi write fuction */
	pDataFlash->pDataFlashDesc->tx_cmd_pt   =  pDataFlash->pDataFlashDesc->command ;
	pDataFlash->pDataFlashDesc->tx_cmd_size =  CmdSize ;
	pDataFlash->pDataFlashDesc->rx_cmd_pt   =  pDataFlash->pDataFlashDesc->command ;
	pDataFlash->pDataFlashDesc->rx_cmd_size =  CmdSize ;

	return AT91F_SpiWrite(pDataFlash->pDataFlashDesc);			
}


/*----------------------------------------------------------------------*/
/* \fn    AT91F_DataFlashGetStatus					*/
/* \brief Read the status register of the dataflash			*/
/*----------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_DataFlashGetStatus(AT91PS_DataflashDesc pDesc)
{
	AT91S_DataFlashStatus status;

	/* first send the read status command (D7H) */
	pDesc->command[0] = DB_STATUS;
	pDesc->command[1] = 0;

	pDesc->DataFlash_state = GET_STATUS;
    	pDesc->tx_data_size    = 0 ; /* Transmit the command and receive response */
    	pDesc->tx_cmd_pt       = pDesc->command ;
    	pDesc->rx_cmd_pt       = pDesc->command ;
    	pDesc->rx_cmd_size     = 2 ;
    	pDesc->tx_cmd_size     = 2 ;
    	status = AT91F_SpiWrite (pDesc);

	pDesc->DataFlash_state = *( (unsigned char *) (pDesc->rx_cmd_pt) +1);
	return status;
}

/*-----------------------------------------------------------------------------
 * Function Name       : AT91F_DataFlashWaitReady
 * Object              : wait for dataflash ready (bit7 of the status register == 1)
 * Input Parameters    : DataFlash Service and timeout
 * Return value        : DataFlash status "ready or not"
 *-----------------------------------------------------------------------------
 */
static AT91S_DataFlashStatus AT91F_DataFlashWaitReady(
	AT91PS_DataflashDesc pDataFlashDesc,
	unsigned int timeout)
{
	pDataFlashDesc->DataFlash_state = IDLE;
        do {
                AT91F_DataFlashGetStatus(pDataFlashDesc);
                timeout--;
        }
        while(((pDataFlashDesc->DataFlash_state & 0x80) != 0x80) && (timeout > 0));

        if((pDataFlashDesc->DataFlash_state & 0x80) != 0x80)
                return AT91C_DATAFLASH_ERROR;

        return AT91C_DATAFLASH_OK;
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataFlashContinuousRead                          */
/* Object              : Continuous stream Read                                 */
/* Input Parameters    : DataFlash Service                                      */
/*                     : <src> = dataflash address                              */
/*                     : <*dataBuffer> = data buffer pointer                    */
/*                     : <sizeToRead> = data buffer size                        */
/* Return value        : State of the dataflash                                 */
/*------------------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_DataFlashContinuousRead(
 	AT91PS_DataFlash pDataFlash,
	int src,
	unsigned char *dataBuffer,
	int sizeToRead )
{
	AT91S_DataFlashStatus status;
	/* Test the size to read in the device */
	if ( (src + sizeToRead) > (pDataFlash->pDevice->pages_size * (pDataFlash->pDevice->pages_number)))
		return AT91C_DATAFLASH_MEMORY_OVERFLOW;

	pDataFlash->pDataFlashDesc->rx_data_pt = dataBuffer;
	pDataFlash->pDataFlashDesc->rx_data_size = sizeToRead;
	pDataFlash->pDataFlashDesc->tx_data_pt = dataBuffer;
	pDataFlash->pDataFlashDesc->tx_data_size = sizeToRead;
	
	status = AT91F_DataFlashSendCommand(pDataFlash, DB_CONTINUOUS_ARRAY_READ, 8, src);
	/* Send the command to the dataflash */
	return(status);
}



/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_MainMemoryToBufferTransfer                       */
/* Object              : Read a page in the SRAM Buffer 1 or 2                  */
/* Input Parameters    : DataFlash Service                                      */
/*                     : Page concerned                                         */
/*                     :                                                        */
/* Return value        : State of the dataflash                                 */
/*------------------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_MainMemoryToBufferTransfer(
	AT91PS_DataFlash pDataFlash,
	unsigned char BufferCommand,
	unsigned int page)
{
	int cmdsize;
	/* Test if the buffer command is legal */
	if ((BufferCommand != DB_PAGE_2_BUF1_TRF) && (BufferCommand != DB_PAGE_2_BUF2_TRF))
		return AT91C_DATAFLASH_BAD_COMMAND;

	/* no data to transmit or receive */
    	pDataFlash->pDataFlashDesc->tx_data_size = 0;
	cmdsize = 4;
	if (pDataFlash->pDevice->pages_number >= 16384)
		cmdsize = 5;
	return(AT91F_DataFlashSendCommand(pDataFlash, BufferCommand, cmdsize,
	                                  page*pDataFlash->pDevice->pages_size));
}



/*----------------------------------------------------------------------------- */
/* Function Name       : AT91F_DataFlashWriteBuffer                             */
/* Object              : Write data to the internal sram buffer 1 or 2          */
/* Input Parameters    : DataFlash Service                                      */
/*                     : <BufferCommand> = command to write buffer1 or buffer2  */
/*                     : <*dataBuffer> = data buffer to write                   */
/*                     : <bufferAddress> = address in the internal buffer       */
/*                     : <SizeToWrite> = data buffer size                       */
/* Return value        : State of the dataflash                                 */
/*------------------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_DataFlashWriteBuffer(
	AT91PS_DataFlash pDataFlash,
	unsigned char BufferCommand,
	unsigned char *dataBuffer,
	unsigned int bufferAddress,
	int SizeToWrite )
{
	int cmdsize;
	/* Test if the buffer command is legal */
	if ((BufferCommand != DB_BUF1_WRITE) && (BufferCommand != DB_BUF2_WRITE))
		return AT91C_DATAFLASH_BAD_COMMAND;

	/* buffer address must be lower than page size */
	if (bufferAddress > pDataFlash->pDevice->pages_size)
		return AT91C_DATAFLASH_BAD_ADDRESS;

    	/* Send first Write Command */
	pDataFlash->pDataFlashDesc->command[0] = BufferCommand;
	pDataFlash->pDataFlashDesc->command[1] = 0;
	if (pDataFlash->pDevice->pages_number >= 16384)
	{
	   	pDataFlash->pDataFlashDesc->command[2] = 0;
	   	pDataFlash->pDataFlashDesc->command[3] = (unsigned char)(((unsigned int)(bufferAddress &  pDataFlash->pDevice->byte_mask)) >> 8) ;
	   	pDataFlash->pDataFlashDesc->command[4] = (unsigned char)((unsigned int)bufferAddress  & 0x00FF) ;
		cmdsize = 5;
	}
	else
	{
	   	pDataFlash->pDataFlashDesc->command[2] = (unsigned char)(((unsigned int)(bufferAddress &  pDataFlash->pDevice->byte_mask)) >> 8) ;
	   	pDataFlash->pDataFlashDesc->command[3] = (unsigned char)((unsigned int)bufferAddress  & 0x00FF) ;
	   	pDataFlash->pDataFlashDesc->command[4] = 0;
		cmdsize = 4;
	}
		
	pDataFlash->pDataFlashDesc->tx_cmd_pt 	 = pDataFlash->pDataFlashDesc->command ;
	pDataFlash->pDataFlashDesc->tx_cmd_size  = cmdsize ;
	pDataFlash->pDataFlashDesc->rx_cmd_pt 	 = pDataFlash->pDataFlashDesc->command ;
	pDataFlash->pDataFlashDesc->rx_cmd_size  = cmdsize ;

	pDataFlash->pDataFlashDesc->rx_data_pt 	 = dataBuffer ;
	pDataFlash->pDataFlashDesc->tx_data_pt 	 = dataBuffer ;
	pDataFlash->pDataFlashDesc->rx_data_size = SizeToWrite ;
	pDataFlash->pDataFlashDesc->tx_data_size = SizeToWrite ;

	return AT91F_SpiWrite(pDataFlash->pDataFlashDesc);
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_PageErase                                        */             
/* Object              : Read a page in the SRAM Buffer 1 or 2                  */
/* Input Parameters    : DataFlash Service                                      */
/*                     : Page concerned                                         */
/*                     :                                                        */
/* Return value        : State of the dataflash                                 */
/*------------------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_PageErase(
	AT91PS_DataFlash pDataFlash,
	unsigned int page)
{
	int cmdsize;
	/* Test if the buffer command is legal */	
	/* no data to transmit or receive */
    	pDataFlash->pDataFlashDesc->tx_data_size = 0;
	
	cmdsize = 4;
	if (pDataFlash->pDevice->pages_number >= 16384)
		cmdsize = 5;
	return(AT91F_DataFlashSendCommand(pDataFlash, DB_PAGE_ERASE, cmdsize,
	                                  page*pDataFlash->pDevice->pages_size));
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_WriteBufferToMain                                */
/* Object              : Write buffer to the main memory                        */
/* Input Parameters    : DataFlash Service                                      */
/*                     : <BufferCommand> = command to send to buf1 or buf2      */
/*                     : <dest> = main memory address                           */
/* Return value        : State of the dataflash                                 */
/*------------------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_WriteBufferToMain (
	AT91PS_DataFlash pDataFlash,
	unsigned char BufferCommand,
	unsigned int dest )
{
	int cmdsize;
	/* Test if the buffer command is correct */
	if ((BufferCommand != DB_BUF1_PAGE_PGM) &&
	    (BufferCommand != DB_BUF1_PAGE_ERASE_PGM) &&
	    (BufferCommand != DB_BUF2_PAGE_PGM) &&
	    (BufferCommand != DB_BUF2_PAGE_ERASE_PGM) )
		return AT91C_DATAFLASH_BAD_COMMAND;

	/* no data to transmit or receive */
	pDataFlash->pDataFlashDesc->tx_data_size = 0;

	cmdsize = 4;
	if (pDataFlash->pDevice->pages_number >= 16384)
		cmdsize = 5;
	/* Send the command to the dataflash */
	return(AT91F_DataFlashSendCommand (pDataFlash, BufferCommand, cmdsize, dest));
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_PartialPageWrite                                 */
/* Object              : Erase partially a page                                 */
/* Input Parameters    : <page> = page number                                   */
/*                     : <AdrInpage> = adr to begin the fading                  */
/*                     : <length> = Number of bytes to erase                    */
/*------------------------------------------------------------------------------*/
static AT91S_DataFlashStatus AT91F_PartialPageWrite (
	AT91PS_DataFlash pDataFlash,
	unsigned char *src,
	unsigned int dest,
	unsigned int size)
{
	unsigned int page;
	unsigned int AdrInPage;

	page = dest / (pDataFlash->pDevice->pages_size);
	AdrInPage = dest % (pDataFlash->pDevice->pages_size);

	/* Read the contents of the page in the Sram Buffer */
	AT91F_MainMemoryToBufferTransfer(pDataFlash, DB_PAGE_2_BUF1_TRF, page);
	AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
	
	/*Update the SRAM buffer */
	AT91F_DataFlashWriteBuffer(pDataFlash, DB_BUF1_WRITE, src, AdrInPage, size);
	AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
	
	/* Erase page if a 128 Mbits device */
	if (pDataFlash->pDevice->pages_number >= 16384)
	{
		AT91F_PageErase(pDataFlash, page);
		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
	}

	/* Rewrite the modified Sram Buffer in the main memory */
	return(AT91F_WriteBufferToMain(pDataFlash, DB_BUF1_PAGE_ERASE_PGM,
	                               (page*pDataFlash->pDevice->pages_size)));
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataFlashWrite                                   */
/* Object              :                                                        */
/* Input Parameters    : <*src> = Source buffer                                 */
/*                     : <dest> = dataflash adress                              */
/*                     : <size> = data buffer size                              */
/*------------------------------------------------------------------------------*/
AT91S_DataFlashStatus AT91F_DataFlashWrite(
	AT91PS_DataFlash pDataFlash,
	unsigned char *src,
	int dest,
	int size )
{
	unsigned int length;
	unsigned int page;
	unsigned int status;

	AT91F_SpiEnable(pDataFlash->pDevice->cs);

	if ( (dest + size) > (pDataFlash->pDevice->pages_size * (pDataFlash->pDevice->pages_number)))
		return AT91C_DATAFLASH_MEMORY_OVERFLOW;

    	/* If destination does not fit a page start address */
	if ((dest % ((unsigned int)(pDataFlash->pDevice->pages_size)))  != 0 ) {
		length = pDataFlash->pDevice->pages_size - (dest % ((unsigned int)(pDataFlash->pDevice->pages_size)));

		if (size < length)
			length = size;

		if(!AT91F_PartialPageWrite(pDataFlash,src, dest, length))
			return AT91C_DATAFLASH_ERROR;

		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);

		/* Update size, source and destination pointers */
        	size -= length;
        	dest += length;
        	src += length;
	}

	while (( size - pDataFlash->pDevice->pages_size ) >= 0 ) 
	{
		/* program dataflash page */		
		page = (unsigned int)dest / (pDataFlash->pDevice->pages_size);

		status = AT91F_DataFlashWriteBuffer(pDataFlash, DB_BUF1_WRITE, src,
		                                    0, pDataFlash->pDevice->pages_size);
		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
	
		status = AT91F_PageErase(pDataFlash, page);
		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
		if (!status)
			return AT91C_DATAFLASH_ERROR;
		
		status = AT91F_WriteBufferToMain (pDataFlash, DB_BUF1_PAGE_PGM, dest);
		if(!status)
			return AT91C_DATAFLASH_ERROR;

		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
	
		/* Update size, source and destination pointers */
	   	size -= pDataFlash->pDevice->pages_size ;
	   	dest += pDataFlash->pDevice->pages_size ;
	   	src  += pDataFlash->pDevice->pages_size ;
	}

	/* If still some bytes to read */
	if ( size > 0 ) {
		/* program dataflash page */
		if(!AT91F_PartialPageWrite(pDataFlash, src, dest, size) )
			return AT91C_DATAFLASH_ERROR;
		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
	}
	return AT91C_DATAFLASH_OK;
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataFlashRead                                    */
/* Object              : Read a block in dataflash                              */
/* Input Parameters    :                                                        */
/* Return value        :                                                        */
/*------------------------------------------------------------------------------*/
int AT91F_DataFlashRead(
	AT91PS_DataFlash pDataFlash,
	unsigned long addr,
	unsigned long size,
	char *buffer)
{
	unsigned long SizeToRead;

	AT91F_SpiEnable(pDataFlash->pDevice->cs);

	if(AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT) != AT91C_DATAFLASH_OK)
		return -1;

	while (size)
	{
		SizeToRead = (size < 0x8000)? size:0x8000;

		if (AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT)
		    != AT91C_DATAFLASH_OK)
			return -1;

		if (AT91F_DataFlashContinuousRead (pDataFlash, addr, (unsigned char *)buffer,
		                                   SizeToRead) != AT91C_DATAFLASH_OK)
			return -1;

		size -= SizeToRead;
		addr += SizeToRead;
		buffer += SizeToRead;
	}

   	return AT91C_DATAFLASH_OK;
}


/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataflashProbe                                   */
/* Object              :                                                        */
/* Input Parameters    :                                                        */
/* Return value	       : Dataflash status register                              */
/*------------------------------------------------------------------------------*/
int AT91F_DataflashProbe(int cs, AT91PS_DataflashDesc pDesc)
{
	AT91F_SpiEnable(cs);
   	AT91F_DataFlashGetStatus(pDesc);
   	return ((pDesc->command[1] == 0xFF)? 0: (pDesc->command[1] & 0x3C));
}

/*------------------------------------------------------------------------------*/
/* Function Name       : AT91F_DataFlashErase                                   */
/* Object              :                                                        */
/* Input Parameters    : <*pDataFlash> = Device info                            */
/*------------------------------------------------------------------------------*/
AT91S_DataFlashStatus AT91F_DataFlashErase(AT91PS_DataFlash pDataFlash)
{
	unsigned int page;
	unsigned int status;

	AT91F_SpiEnable(pDataFlash->pDevice->cs);
 
 	for(page=0; page < pDataFlash->pDevice->pages_number; page++)
	    {
		/* Erase dataflash page */
		if ((page & 0x00FF) == 0)
			printf("\rERA %d/%d", page, pDataFlash->pDevice->pages_number);
		status = AT91F_PageErase(pDataFlash, page);
		AT91F_DataFlashWaitReady(pDataFlash->pDataFlashDesc, AT91C_DATAFLASH_TIMEOUT);
		if (!status)
			return AT91C_DATAFLASH_ERROR;
   	}

	return AT91C_DATAFLASH_OK;
}

