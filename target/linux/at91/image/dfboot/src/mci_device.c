//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : mci_device.c
//* Object              : TEST DataFlash Functions
//* Creation            : FB   26/11/2002
//*
//*----------------------------------------------------------------------------

#include <AT91C_MCI_Device.h>
#include "stdio.h"

#define AT91C_MCI_TIMEOUT			1000000   /* For AT91F_MCIDeviceWaitReady */
#define BUFFER_SIZE_MCI_DEVICE		512
#define MASTER_CLOCK				60000000
#define FALSE						0
#define TRUE						1

//* External Functions
extern void AT91F_ASM_MCI_Handler(void);
//* Global Variables
AT91S_MciDeviceFeatures			MCI_Device_Features;
AT91S_MciDeviceDesc				MCI_Device_Desc;
AT91S_MciDevice					MCI_Device;

#undef ENABLE_WRITE
#undef MMC

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SendCommand
//* \brief Generic function to send a command to the MMC or SDCard
//*----------------------------------------------------------------------------
int AT91F_MCI_SendCommand (
	AT91PS_MciDevice pMCI_Device,
	unsigned int Cmd,
	unsigned int Arg)
{
	unsigned int	error,status;
	//unsigned int	tick=0;

    // Send the command
    AT91C_BASE_MCI->MCI_ARGR = Arg;
    AT91C_BASE_MCI->MCI_CMDR = Cmd;

	// wait for CMDRDY Status flag to read the response
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		//tick++;
	}
	while( !(status & AT91C_MCI_CMDRDY) );//&& (tick<100) );

    // Test error  ==> if crc error and response R3 ==> don't check error
    error = (AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR;
	if(error != 0 )
	{
		// if the command is SEND_OP_COND the CRC error flag is always present (cf : R3 response)
		if ( (Cmd != AT91C_SDCARD_APP_OP_COND_CMD) && (Cmd != AT91C_MMC_SEND_OP_COND_CMD) )
			return ((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR);
		else
		{
			if (error != AT91C_MCI_RCRCE)
				return ((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR);
		}
	}
    return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_SendAppCommand
//* \brief Specific function to send a specific command to the SDCard
//*----------------------------------------------------------------------------
int AT91F_MCI_SDCard_SendAppCommand (
	AT91PS_MciDevice pMCI_Device,
	unsigned int Cmd_App,
	unsigned int Arg	)
{
	unsigned int status;
	//unsigned int	tick=0;

	// Send the CMD55 for application specific command
    AT91C_BASE_MCI->MCI_ARGR = (pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address << 16 );
    AT91C_BASE_MCI->MCI_CMDR = AT91C_APP_CMD;

	// wait for CMDRDY Status flag to read the response
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		//tick++;
	}
	while( !(status & AT91C_MCI_CMDRDY) );//&& (tick<100) );	

    // if an error occurs
    if (((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR) != 0 )
		return ((AT91C_BASE_MCI->MCI_SR) & AT91C_MCI_SR_ERROR);

    // check if it is a specific command and then send the command
	if ( (Cmd_App && AT91C_SDCARD_APP_ALL_CMD) == 0)
		return AT91C_CMD_SEND_ERROR;

   return( AT91F_MCI_SendCommand(pMCI_Device,Cmd_App,Arg) );
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_GetStatus
//* \brief Addressed card sends its status register
//*----------------------------------------------------------------------------
int AT91F_MCI_GetStatus(AT91PS_MciDevice pMCI_Device,unsigned int relative_card_address)
{
	if (AT91F_MCI_SendCommand(pMCI_Device,
								AT91C_SEND_STATUS_CMD,
								relative_card_address <<16) == AT91C_CMD_SEND_OK)
    	return (AT91C_BASE_MCI->MCI_RSPR[0]);

    return AT91C_CMD_SEND_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Device_Handler
//* \brief MCI C interrupt handler
//*----------------------------------------------------------------------------
void AT91F_MCI_Device_Handler(
	AT91PS_MciDevice pMCI_Device,
	unsigned int status)
{
	// If End of Tx Buffer Empty interrupt occurred
	if ( status & AT91C_MCI_TXBUFE )
    {
		AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_TXBUFE;
 		AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_TXTDIS;
        	
		pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_IDLE;
	}	// End of if AT91C_MCI_TXBUFF		
	
    // If End of Rx Buffer Full interrupt occurred
    if ( status & AT91C_MCI_RXBUFF )
    {        
       	AT91C_BASE_MCI->MCI_IDR = AT91C_MCI_RXBUFF;
 		AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_RXTDIS;
	
		pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_IDLE;
	}	// End of if AT91C_MCI_RXBUFF

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Handler
//* \brief MCI Handler
//*----------------------------------------------------------------------------
void AT91F_MCI_Handler(void)
{
	int status;

	status = ( AT91C_BASE_MCI->MCI_SR & AT91C_BASE_MCI->MCI_IMR );

	AT91F_MCI_Device_Handler(&MCI_Device,status);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_ReadBlock
//* \brief Read an ENTIRE block or PARTIAL block
//*----------------------------------------------------------------------------
int AT91F_MCI_ReadBlock(
	AT91PS_MciDevice pMCI_Device,
	int src,
	unsigned int *dataBuffer,
	int sizeToRead )
{
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(pMCI_Device->pMCI_DeviceDesc->state != AT91C_MCI_IDLE)
    	return AT91C_READ_ERROR;
    
    if( (AT91F_MCI_GetStatus(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address) & AT91C_SR_READY_FOR_DATA) != AT91C_SR_READY_FOR_DATA)
    	return AT91C_READ_ERROR;
    	
    if ( (src + sizeToRead) > pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity )
		return AT91C_READ_ERROR;

    // If source does not fit a begin of a block
	if ( (src % pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) != 0 )
		return AT91C_READ_ERROR;
   
     // Test if the MMC supports Partial Read Block
     // ALWAYS SUPPORTED IN SD Memory Card
     if( (sizeToRead < pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) 
    	&& (pMCI_Device->pMCI_DeviceFeatures->Read_Partial == 0x00) )
   		return AT91C_READ_ERROR;
   		
    if( sizeToRead > pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length)
   		return AT91C_READ_ERROR;
    ////////////////////////////////////////////////////////////////////////////////////////////
      
    // Init Mode Register
	AT91C_BASE_MCI->MCI_MR |= ((pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length << 16) | AT91C_MCI_PDCMODE);
	 
    if (sizeToRead %4)
		sizeToRead = (sizeToRead /4)+1;
	else
		sizeToRead = sizeToRead/4;

	AT91C_BASE_PDC_MCI->PDC_PTCR = (AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS);
    AT91C_BASE_PDC_MCI->PDC_RPR  = (unsigned int)dataBuffer;
    AT91C_BASE_PDC_MCI->PDC_RCR  = sizeToRead;

	// Send the Read single block command
    if ( AT91F_MCI_SendCommand(pMCI_Device, AT91C_READ_SINGLE_BLOCK_CMD, src) != AT91C_CMD_SEND_OK )
    	return AT91C_READ_ERROR;

	pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_RX_SINGLE_BLOCK;

	// Enable AT91C_MCI_RXBUFF Interrupt
    AT91C_BASE_MCI->MCI_IER = AT91C_MCI_RXBUFF;

	// (PDC) Receiver Transfer Enable
	AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_RXTEN;
	
	return AT91C_READ_OK;
}


#ifdef ENABLE_WRITE
//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_WriteBlock
//* \brief  Write an ENTIRE block but not always PARTIAL block !!!
//*----------------------------------------------------------------------------
int AT91F_MCI_WriteBlock(
	AT91PS_MciDevice pMCI_Device,
	int dest,
	unsigned int *dataBuffer,
	int sizeToWrite )
{
    ////////////////////////////////////////////////////////////////////////////////////////////
	if( pMCI_Device->pMCI_DeviceDesc->state != AT91C_MCI_IDLE)
    	return AT91C_WRITE_ERROR;
    
    if( (AT91F_MCI_GetStatus(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address) & AT91C_SR_READY_FOR_DATA) != AT91C_SR_READY_FOR_DATA)
    	return AT91C_WRITE_ERROR;
    	
    if ( (dest + sizeToWrite) > pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity )
		return AT91C_WRITE_ERROR;

    // If source does not fit a begin of a block
	if ( (dest % pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) != 0 )
		return AT91C_WRITE_ERROR;
   
    // Test if the MMC supports Partial Write Block 
    if( (sizeToWrite < pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length) 
    	&& (pMCI_Device->pMCI_DeviceFeatures->Write_Partial == 0x00) )
   		return AT91C_WRITE_ERROR;
   		
   	if( sizeToWrite > pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length )
   		return AT91C_WRITE_ERROR;
    ////////////////////////////////////////////////////////////////////////////////////////////
  
    // Init Mode Register
	AT91C_BASE_MCI->MCI_MR |= ((pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length << 16) | AT91C_MCI_PDCMODE);
	
	if (sizeToWrite %4)
		sizeToWrite = (sizeToWrite /4)+1;
	else
		sizeToWrite = sizeToWrite/4;

	// Init PDC for write sequence
    AT91C_BASE_PDC_MCI->PDC_PTCR = (AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS);
    AT91C_BASE_PDC_MCI->PDC_TPR = (unsigned int) dataBuffer;
    AT91C_BASE_PDC_MCI->PDC_TCR = sizeToWrite;

	// Send the write single block command
    if ( AT91F_MCI_SendCommand(pMCI_Device, AT91C_WRITE_BLOCK_CMD, dest) != AT91C_CMD_SEND_OK)
    	return AT91C_WRITE_ERROR;

	pMCI_Device->pMCI_DeviceDesc->state = AT91C_MCI_TX_SINGLE_BLOCK;

	// Enable AT91C_MCI_TXBUFE Interrupt
    AT91C_BASE_MCI->MCI_IER = AT91C_MCI_TXBUFE;
  
  	// Enables TX for PDC transfert requests
    AT91C_BASE_PDC_MCI->PDC_PTCR = AT91C_PDC_TXTEN;
  
	return AT91C_WRITE_OK;
}
#endif

#ifdef MMC
//*------------------------------------------------------------------------------------------------------------
//* \fn    AT91F_MCI_MMC_SelectCard
//* \brief Toggles a card between the Stand_by and Transfer states or between Programming and Disconnect states
//*------------------------------------------------------------------------------------------------------------
int AT91F_MCI_MMC_SelectCard(AT91PS_MciDevice pMCI_Device, unsigned int relative_card_address)
{
    int status;
	
	//* Check if the MMC card chosen is already the selected one
	status = AT91F_MCI_GetStatus(pMCI_Device,relative_card_address);

	if (status < 0)
		return AT91C_CARD_SELECTED_ERROR;

	if ((status & AT91C_SR_CARD_SELECTED) == AT91C_SR_CARD_SELECTED)
		return AT91C_CARD_SELECTED_OK;

	//* Search for the MMC Card to be selected, status = the Corresponding Device Number
	status = 0;
	while( (pMCI_Device->pMCI_DeviceFeatures[status].Relative_Card_Address != relative_card_address)
		   && (status < AT91C_MAX_MCI_CARDS) )
		status++;

	if (status > AT91C_MAX_MCI_CARDS)
    	return AT91C_CARD_SELECTED_ERROR;

    if (AT91F_MCI_SendCommand( pMCI_Device,
    								   AT91C_SEL_DESEL_CARD_CMD,
    								   pMCI_Device->pMCI_DeviceFeatures[status].Relative_Card_Address << 16) == AT91C_CMD_SEND_OK)
    	return AT91C_CARD_SELECTED_OK;
    return AT91C_CARD_SELECTED_ERROR;
}
#endif

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_GetCSD
//* \brief Asks to the specified card to send its CSD
//*----------------------------------------------------------------------------
int AT91F_MCI_GetCSD (AT91PS_MciDevice pMCI_Device, unsigned int relative_card_address , unsigned int * response)
{
 	
 	if(AT91F_MCI_SendCommand(pMCI_Device,
								  AT91C_SEND_CSD_CMD,
								  (relative_card_address << 16)) != AT91C_CMD_SEND_OK)
		return AT91C_CMD_SEND_ERROR;
	
    response[0] = AT91C_BASE_MCI->MCI_RSPR[0];
   	response[1] = AT91C_BASE_MCI->MCI_RSPR[1];
    response[2] = AT91C_BASE_MCI->MCI_RSPR[2];
    response[3] = AT91C_BASE_MCI->MCI_RSPR[3];
    
    return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SetBlocklength
//* \brief Select a block length for all following block commands (R/W)
//*----------------------------------------------------------------------------
int AT91F_MCI_SetBlocklength(AT91PS_MciDevice pMCI_Device,unsigned int length)
{
    return( AT91F_MCI_SendCommand(pMCI_Device, AT91C_SET_BLOCKLEN_CMD, length) );
}

#ifdef MMC
//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_MMC_GetAllOCR
//* \brief Asks to all cards to send their operations conditions
//*----------------------------------------------------------------------------
int AT91F_MCI_MMC_GetAllOCR (AT91PS_MciDevice pMCI_Device)
{
	unsigned int	response =0x0;
 	
 	while(1)
    {
    	response = AT91F_MCI_SendCommand(pMCI_Device,
  										AT91C_MMC_SEND_OP_COND_CMD,
  										AT91C_MMC_HOST_VOLTAGE_RANGE);
		if (response != AT91C_CMD_SEND_OK)
			return AT91C_INIT_ERROR;
		
		response = AT91C_BASE_MCI->MCI_RSPR[0];
		
		if ( (response & AT91C_CARD_POWER_UP_BUSY) == AT91C_CARD_POWER_UP_BUSY)
			return(response);	
	}
}
#endif

#ifdef MMC
//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_MMC_GetAllCID
//* \brief Asks to the MMC on the chosen slot to send its CID
//*----------------------------------------------------------------------------
int AT91F_MCI_MMC_GetAllCID (AT91PS_MciDevice pMCI_Device, unsigned int *response)
{
	int Nb_Cards_Found=-1;
  
	while(1)
	{
	 	if(AT91F_MCI_SendCommand(pMCI_Device,
								AT91C_MMC_ALL_SEND_CID_CMD,
								AT91C_NO_ARGUMENT) != AT91C_CMD_SEND_OK)
			return Nb_Cards_Found;
		else
		{		
			Nb_Cards_Found = 0;
			//* Assignation of the relative address to the MMC CARD
			pMCI_Device->pMCI_DeviceFeatures[Nb_Cards_Found].Relative_Card_Address = Nb_Cards_Found + AT91C_FIRST_RCA;
			//* Set the insert flag
			pMCI_Device->pMCI_DeviceFeatures[Nb_Cards_Found].Card_Inserted = AT91C_MMC_CARD_INSERTED;
	
			if (AT91F_MCI_SendCommand(pMCI_Device,
									 AT91C_MMC_SET_RELATIVE_ADDR_CMD,
									 (Nb_Cards_Found + AT91C_FIRST_RCA) << 16) != AT91C_CMD_SEND_OK)
				return AT91C_CMD_SEND_ERROR;
				 
			//* If no error during assignation address ==> Increment Nb_cards_Found
			Nb_Cards_Found++ ;
		}
	}
}
#endif
#ifdef MMC
//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_MMC_Init
//* \brief Return the MMC initialisation status
//*----------------------------------------------------------------------------
int AT91F_MCI_MMC_Init (AT91PS_MciDevice pMCI_Device)
{
    unsigned int	tab_response[4];
	unsigned int	mult,blocknr;
	unsigned int 	i,Nb_Cards_Found=0;

	//* Resets all MMC Cards in Idle state
	AT91F_MCI_SendCommand(pMCI_Device, AT91C_MMC_GO_IDLE_STATE_CMD, AT91C_NO_ARGUMENT);

    if(AT91F_MCI_MMC_GetAllOCR(pMCI_Device) == AT91C_INIT_ERROR)
    	return AT91C_INIT_ERROR;

	Nb_Cards_Found = AT91F_MCI_MMC_GetAllCID(pMCI_Device,tab_response);
	if (Nb_Cards_Found != AT91C_CMD_SEND_ERROR)
	{
	    //* Set the Mode Register
    	AT91C_BASE_MCI->MCI_MR = AT91C_MCI_MR_PDCMODE;

		for(i = 0; i < Nb_Cards_Found; i++)
		{
			if (AT91F_MCI_GetCSD(pMCI_Device,
									  pMCI_Device->pMCI_DeviceFeatures[i].Relative_Card_Address,
									  tab_response) != AT91C_CMD_SEND_OK)
				pMCI_Device->pMCI_DeviceFeatures[i].Relative_Card_Address = 0;					  
			else
			{
				pMCI_Device->pMCI_DeviceFeatures[i].Max_Read_DataBlock_Length = 1 << ((tab_response[1] >> AT91C_CSD_RD_B_LEN_S) & AT91C_CSD_RD_B_LEN_M );
	 			pMCI_Device->pMCI_DeviceFeatures[i].Max_Write_DataBlock_Length =	1 << ((tab_response[3] >> AT91C_CSD_WBLEN_S) & AT91C_CSD_WBLEN_M );
				pMCI_Device->pMCI_DeviceFeatures[i].Sector_Size = 1 + ((tab_response[2] >> AT91C_CSD_v22_SECT_SIZE_S) & AT91C_CSD_v22_SECT_SIZE_M );
		  		pMCI_Device->pMCI_DeviceFeatures[i].Read_Partial = (tab_response[1] >> AT91C_CSD_RD_B_PAR_S) & AT91C_CSD_RD_B_PAR_M;
				pMCI_Device->pMCI_DeviceFeatures[i].Write_Partial = (tab_response[3] >> AT91C_CSD_WBLOCK_P_S) & AT91C_CSD_WBLOCK_P_M;
				
				// None in MMC specification version 2.2
				pMCI_Device->pMCI_DeviceFeatures[i].Erase_Block_Enable = 0;
				
				pMCI_Device->pMCI_DeviceFeatures[i].Read_Block_Misalignment = (tab_response[1] >> AT91C_CSD_RD_B_MIS_S) & AT91C_CSD_RD_B_MIS_M;
				pMCI_Device->pMCI_DeviceFeatures[i].Write_Block_Misalignment = (tab_response[1] >> AT91C_CSD_WR_B_MIS_S) & AT91C_CSD_WR_B_MIS_M;

				//// Compute Memory Capacity
				// compute MULT
				mult = 1 << ( ((tab_response[2] >> AT91C_CSD_C_SIZE_M_S) & AT91C_CSD_C_SIZE_M_M) + 2 );
				// compute MSB of C_SIZE
				blocknr = ((tab_response[1] >> AT91C_CSD_CSIZE_H_S) & AT91C_CSD_CSIZE_H_M) << 2;
				// compute MULT * (LSB of C-SIZE + MSB already computed + 1) = BLOCKNR
				blocknr = mult * ( ( blocknr + ( (tab_response[2] >> AT91C_CSD_CSIZE_L_S) & AT91C_CSD_CSIZE_L_M) ) + 1 );

				pMCI_Device->pMCI_DeviceFeatures[i].Memory_Capacity =  pMCI_Device->pMCI_DeviceFeatures[i].Max_Read_DataBlock_Length * blocknr;
		  		//// End of Compute Memory Capacity
		  		
			}	// end of else			  
		}	// end of for
		
		return AT91C_INIT_OK;
	}	// end of if

    return AT91C_INIT_ERROR;
}
#endif

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_GetOCR
//* \brief Asks to all cards to send their operations conditions
//*----------------------------------------------------------------------------
int AT91F_MCI_SDCard_GetOCR (AT91PS_MciDevice pMCI_Device)
{
	unsigned int	response =0x0;

	// The RCA to be used for CMD55 in Idle state shall be the card's default RCA=0x0000.
	pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address = 0x0;
 	
 	while( (response & AT91C_CARD_POWER_UP_BUSY) != AT91C_CARD_POWER_UP_BUSY )
    {
    	response = AT91F_MCI_SDCard_SendAppCommand(pMCI_Device,
  										AT91C_SDCARD_APP_OP_COND_CMD,
  										AT91C_MMC_HOST_VOLTAGE_RANGE);
		if (response != AT91C_CMD_SEND_OK)
			return AT91C_INIT_ERROR;
		
		response = AT91C_BASE_MCI->MCI_RSPR[0];
	}
	
	return(AT91C_BASE_MCI->MCI_RSPR[0]);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_GetCID
//* \brief Asks to the SDCard on the chosen slot to send its CID
//*----------------------------------------------------------------------------
int AT91F_MCI_SDCard_GetCID (AT91PS_MciDevice pMCI_Device, unsigned int *response)
{
 	if(AT91F_MCI_SendCommand(pMCI_Device,
							AT91C_ALL_SEND_CID_CMD,
							AT91C_NO_ARGUMENT) != AT91C_CMD_SEND_OK)
		return AT91C_CMD_SEND_ERROR;
	
    response[0] = AT91C_BASE_MCI->MCI_RSPR[0];
   	response[1] = AT91C_BASE_MCI->MCI_RSPR[1];
    response[2] = AT91C_BASE_MCI->MCI_RSPR[2];
    response[3] = AT91C_BASE_MCI->MCI_RSPR[3];
    
    return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_SetBusWidth
//* \brief  Set bus width for SDCard
//*----------------------------------------------------------------------------
int AT91F_MCI_SDCard_SetBusWidth(AT91PS_MciDevice pMCI_Device)
{
	volatile int	ret_value;
	char			bus_width;

	do
	{
		ret_value =AT91F_MCI_GetStatus(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address);
	}
	while((ret_value > 0) && ((ret_value & AT91C_SR_READY_FOR_DATA) == 0));

	// Select Card
    AT91F_MCI_SendCommand(pMCI_Device,
    						AT91C_SEL_DESEL_CARD_CMD,
    						(pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address)<<16);

	// Set bus width for Sdcard
	if(pMCI_Device->pMCI_DeviceDesc->SDCard_bus_width == AT91C_MCI_SCDBUS)
		 	bus_width = AT91C_BUS_WIDTH_4BITS;
	else	bus_width = AT91C_BUS_WIDTH_1BIT;

	if (AT91F_MCI_SDCard_SendAppCommand(pMCI_Device,AT91C_SDCARD_SET_BUS_WIDTH_CMD,bus_width) != AT91C_CMD_SEND_OK)
		return AT91C_CMD_SEND_ERROR;

	return AT91C_CMD_SEND_OK;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_SDCard_Init
//* \brief Return the SDCard initialisation status
//*----------------------------------------------------------------------------
int AT91F_MCI_SDCard_Init (AT91PS_MciDevice pMCI_Device)
{
    unsigned int	tab_response[4];
	unsigned int	mult,blocknr;

	AT91F_MCI_SendCommand(pMCI_Device, AT91C_GO_IDLE_STATE_CMD, AT91C_NO_ARGUMENT);

    if(AT91F_MCI_SDCard_GetOCR(pMCI_Device) == AT91C_INIT_ERROR)
    	return AT91C_INIT_ERROR;

	if (AT91F_MCI_SDCard_GetCID(pMCI_Device,tab_response) == AT91C_CMD_SEND_OK)
	{
	    pMCI_Device->pMCI_DeviceFeatures->Card_Inserted = AT91C_SD_CARD_INSERTED;

	    if (AT91F_MCI_SendCommand(pMCI_Device, AT91C_SET_RELATIVE_ADDR_CMD, 0) == AT91C_CMD_SEND_OK)
		{
			pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address = (AT91C_BASE_MCI->MCI_RSPR[0] >> 16);
			if (AT91F_MCI_GetCSD(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Relative_Card_Address,tab_response) == AT91C_CMD_SEND_OK)
			{
		  		pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length = 1 << ((tab_response[1] >> AT91C_CSD_RD_B_LEN_S) & AT91C_CSD_RD_B_LEN_M );
	 			pMCI_Device->pMCI_DeviceFeatures->Max_Write_DataBlock_Length =	1 << ((tab_response[3] >> AT91C_CSD_WBLEN_S) & AT91C_CSD_WBLEN_M );
				pMCI_Device->pMCI_DeviceFeatures->Sector_Size = 1 + ((tab_response[2] >> AT91C_CSD_v21_SECT_SIZE_S) & AT91C_CSD_v21_SECT_SIZE_M );
		  		pMCI_Device->pMCI_DeviceFeatures->Read_Partial = (tab_response[1] >> AT91C_CSD_RD_B_PAR_S) & AT91C_CSD_RD_B_PAR_M;
				pMCI_Device->pMCI_DeviceFeatures->Write_Partial = (tab_response[3] >> AT91C_CSD_WBLOCK_P_S) & AT91C_CSD_WBLOCK_P_M;
				pMCI_Device->pMCI_DeviceFeatures->Erase_Block_Enable = (tab_response[3] >> AT91C_CSD_v21_ER_BLEN_EN_S) & AT91C_CSD_v21_ER_BLEN_EN_M;
				pMCI_Device->pMCI_DeviceFeatures->Read_Block_Misalignment = (tab_response[1] >> AT91C_CSD_RD_B_MIS_S) & AT91C_CSD_RD_B_MIS_M;
				pMCI_Device->pMCI_DeviceFeatures->Write_Block_Misalignment = (tab_response[1] >> AT91C_CSD_WR_B_MIS_S) & AT91C_CSD_WR_B_MIS_M;

				//// Compute Memory Capacity
					// compute MULT
					mult = 1 << ( ((tab_response[2] >> AT91C_CSD_C_SIZE_M_S) & AT91C_CSD_C_SIZE_M_M) + 2 );
					// compute MSB of C_SIZE
					blocknr = ((tab_response[1] >> AT91C_CSD_CSIZE_H_S) & AT91C_CSD_CSIZE_H_M) << 2;
					// compute MULT * (LSB of C-SIZE + MSB already computed + 1) = BLOCKNR
					blocknr = mult * ( ( blocknr + ( (tab_response[2] >> AT91C_CSD_CSIZE_L_S) & AT91C_CSD_CSIZE_L_M) ) + 1 );

					pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity =  pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length * blocknr;
			  	//// End of Compute Memory Capacity
					printf("SD-Card: %d Bytes\n\r", pMCI_Device->pMCI_DeviceFeatures->Memory_Capacity);

		  		if( AT91F_MCI_SDCard_SetBusWidth(pMCI_Device) == AT91C_CMD_SEND_OK )
				{	
					 if (AT91F_MCI_SetBlocklength(pMCI_Device,pMCI_Device->pMCI_DeviceFeatures->Max_Read_DataBlock_Length) == AT91C_CMD_SEND_OK)
					return AT91C_INIT_OK;
				}
			}
		}
	}
    return AT91C_INIT_ERROR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_CfgDevice
//* \brief This function is used to initialise MMC or SDCard Features
//*----------------------------------------------------------------------------
void AT91F_CfgDevice(void)
{
	// Init Device Structure

	MCI_Device_Features.Relative_Card_Address 		= 0;
	MCI_Device_Features.Card_Inserted 				= AT91C_CARD_REMOVED;
	MCI_Device_Features.Max_Read_DataBlock_Length	= 0;
	MCI_Device_Features.Max_Write_DataBlock_Length 	= 0;
	MCI_Device_Features.Read_Partial 				= 0;
	MCI_Device_Features.Write_Partial 				= 0;
	MCI_Device_Features.Erase_Block_Enable 			= 0;
	MCI_Device_Features.Sector_Size 				= 0;
	MCI_Device_Features.Memory_Capacity 			= 0;
	
	MCI_Device_Desc.state							= AT91C_MCI_IDLE;
	MCI_Device_Desc.SDCard_bus_width				= AT91C_MCI_SCDBUS;
	
	// Init AT91S_DataFlash Global Structure, by default AT45DB choosen !!!
	MCI_Device.pMCI_DeviceDesc 		= &MCI_Device_Desc;
	MCI_Device.pMCI_DeviceFeatures 	= &MCI_Device_Features;

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Init
//* \brief Initialsise Card
//*----------------------------------------------------------------------------
int AT91F_MCI_Init(void)
{

///////////////////////////////////////////////////////////////////////////////////////////
//  MCI Init : common to MMC and SDCard
///////////////////////////////////////////////////////////////////////////////////////////

    // Set up PIO SDC_TYPE to switch on MMC/SDCard and not DataFlash Card
	AT91F_PIO_CfgOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);
	AT91F_PIO_SetOutput(AT91C_BASE_PIOB,AT91C_PIO_PB7);
	
	// Init MCI for MMC and SDCard interface
	AT91F_MCI_CfgPIO();	
	AT91F_MCI_CfgPMC();
	AT91F_PDC_Open(AT91C_BASE_PDC_MCI);

    // Disable all the interrupts
    AT91C_BASE_MCI->MCI_IDR = 0xFFFFFFFF;

	// Init MCI Device Structures
	AT91F_CfgDevice();

	// Configure MCI interrupt 
	AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,
						 AT91C_ID_MCI,
						 AT91C_AIC_PRIOR_HIGHEST,
						 AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE,
						 AT91F_ASM_MCI_Handler);

	// Enable MCI interrupt
	AT91F_AIC_EnableIt(AT91C_BASE_AIC,AT91C_ID_MCI);

	// Enable Receiver
	AT91F_US_EnableRx((AT91PS_USART) AT91C_BASE_DBGU);

	AT91F_MCI_Configure(AT91C_BASE_MCI,
						AT91C_MCI_DTOR_1MEGA_CYCLES,
						AT91C_MCI_MR_PDCMODE,			// 15MHz for MCK = 60MHz (CLKDIV = 1)
						AT91C_MCI_SDCARD_4BITS_SLOTA);
	
	if(AT91F_MCI_SDCard_Init(&MCI_Device) != AT91C_INIT_OK)
		return FALSE;
	else
		return TRUE;

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCIDeviceWaitReady
//* \brief Wait for MCI Device ready
//*----------------------------------------------------------------------------
void AT91F_MCIDeviceWaitReady(unsigned int timeout)
{
	volatile int status;
	
	do
	{
		status = AT91C_BASE_MCI->MCI_SR;
		timeout--;
	}
	while( !(status & AT91C_MCI_NOTBUSY)  && (timeout>0) );	
}

unsigned int swab32(unsigned int data)
{
	unsigned int res = 0;

	res = (data & 0x000000ff) << 24 |
				(data & 0x0000ff00) << 8  |
				(data & 0x00ff0000) >> 8  |
				(data & 0xff000000) >> 24;

	return res;
}

//*--------------------------------------------------------------------
//* \fn    AT91F_MCI_ReadBlockSwab
//* \brief Read Block and swap byte order
//*--------------------------------------------------------------------
int AT91F_MCI_ReadBlockSwab(
	AT91PS_MciDevice pMCI_Device,
	int src,
	unsigned int *databuffer,
	int sizeToRead)
{
	int i;
	unsigned char *buf = (unsigned char *)databuffer;

	//* Read Block 1
	for(i=0;i<BUFFER_SIZE_MCI_DEVICE;i++)
		*buf++ = 0x00;	
	AT91F_MCI_ReadBlock(&MCI_Device,src,databuffer,sizeToRead);

	//* Wait end of Read
	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

	{
		int index;
		unsigned int *uiBuffer = databuffer;

		for(index = 0; index < 512/4; index++)
			uiBuffer[index] = swab32(uiBuffer[index]);
	}
	return(1);
}

