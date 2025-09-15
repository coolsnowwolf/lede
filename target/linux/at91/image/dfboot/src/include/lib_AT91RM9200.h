//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : lib_AT91RM9200.h
//* Object              : AT91RM9200 inlined functions
//* Generated           : AT91 SW Application Group  11/19/2003 (17:20:51)
//*
//* CVS Reference       : /lib_pdc.h/1.2/Tue Jul 02 12:29:40 2002//
//* CVS Reference       : /lib_dbgu.h/1.1/Fri Jan 31 12:18:40 2003//
//* CVS Reference       : /lib_rtc_1245d.h/1.1/Fri Jan 31 12:19:12 2003//
//* CVS Reference       : /lib_ssc.h/1.4/Fri Jan 31 12:19:20 2003//
//* CVS Reference       : /lib_spi_AT91RMxxxx.h/1.2/Fri Jan 31 12:19:31 2003//
//* CVS Reference       : /lib_tc_1753b.h/1.1/Fri Jan 31 12:20:02 2003//
//* CVS Reference       : /lib_pmc.h/1.3/Thu Nov 14 07:40:45 2002//
//* CVS Reference       : /lib_pio.h/1.3/Fri Jan 31 12:18:56 2003//
//* CVS Reference       : /lib_twi.h/1.2/Fri Jan 31 12:19:38 2003//
//* CVS Reference       : /lib_usart.h/1.5/Thu Nov 21 16:01:53 2002//
//* CVS Reference       : /lib_mci.h/1.2/Wed Nov 20 14:18:55 2002//
//* CVS Reference       : /lib_aic.h/1.3/Fri Jul 12 07:46:11 2002//
//* CVS Reference       : /lib_udp.h/1.3/Fri Jan 31 12:19:48 2003//
//* CVS Reference       : /lib_st.h/1.4/Fri Jan 31 12:20:13 2003//
//*----------------------------------------------------------------------------

#ifndef lib_AT91RM9200_H
#define lib_AT91RM9200_H

/* *****************************************************************************
                SOFTWARE API FOR PDC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetNextRx
//* \brief Set the next receive transfer descriptor
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_SetNextRx (
	AT91PS_PDC pPDC,     // \arg pointer to a PDC controller
	char *address,       // \arg address to the next bloc to be received
	unsigned int bytes)  // \arg number of bytes to be received
{
	pPDC->PDC_RNPR = (unsigned int) address;
	pPDC->PDC_RNCR = bytes;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetNextTx
//* \brief Set the next transmit transfer descriptor
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_SetNextTx (
	AT91PS_PDC pPDC,       // \arg pointer to a PDC controller
	char *address,         // \arg address to the next bloc to be transmitted
	unsigned int bytes)    // \arg number of bytes to be transmitted
{
	pPDC->PDC_TNPR = (unsigned int) address;
	pPDC->PDC_TNCR = bytes;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetRx
//* \brief Set the receive transfer descriptor
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_SetRx (
	AT91PS_PDC pPDC,       // \arg pointer to a PDC controller
	char *address,         // \arg address to the next bloc to be received
	unsigned int bytes)    // \arg number of bytes to be received
{
	pPDC->PDC_RPR = (unsigned int) address;
	pPDC->PDC_RCR = bytes;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SetTx
//* \brief Set the transmit transfer descriptor
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_SetTx (
	AT91PS_PDC pPDC,       // \arg pointer to a PDC controller
	char *address,         // \arg address to the next bloc to be transmitted
	unsigned int bytes)    // \arg number of bytes to be transmitted
{
	pPDC->PDC_TPR = (unsigned int) address;
	pPDC->PDC_TCR = bytes;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_EnableTx
//* \brief Enable transmit
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_EnableTx (
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	pPDC->PDC_PTCR = AT91C_PDC_TXTEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_EnableRx
//* \brief Enable receive
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_EnableRx (
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	pPDC->PDC_PTCR = AT91C_PDC_RXTEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_DisableTx
//* \brief Disable transmit
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_DisableTx (
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	pPDC->PDC_PTCR = AT91C_PDC_TXTDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_DisableRx
//* \brief Disable receive
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_DisableRx (
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	pPDC->PDC_PTCR = AT91C_PDC_RXTDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsTxEmpty
//* \brief Test if the current transfer descriptor has been sent
//*----------------------------------------------------------------------------
static inline int AT91F_PDC_IsTxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	return !(pPDC->PDC_TCR);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsNextTxEmpty
//* \brief Test if the next transfer descriptor has been moved to the current td
//*----------------------------------------------------------------------------
static inline int AT91F_PDC_IsNextTxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	return !(pPDC->PDC_TNCR);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsRxEmpty
//* \brief Test if the current transfer descriptor has been filled
//*----------------------------------------------------------------------------
static inline int AT91F_PDC_IsRxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	return !(pPDC->PDC_RCR);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_IsNextRxEmpty
//* \brief Test if the next transfer descriptor has been moved to the current td
//*----------------------------------------------------------------------------
static inline int AT91F_PDC_IsNextRxEmpty ( // \return return 1 if transfer is complete
	AT91PS_PDC pPDC )       // \arg pointer to a PDC controller
{
	return !(pPDC->PDC_RNCR);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_Open
//* \brief Open PDC: disable TX and RX reset transfer descriptors, re-enable RX and TX
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_Open (
	AT91PS_PDC pPDC)       // \arg pointer to a PDC controller
{
    //* Disable the RX and TX PDC transfer requests
	AT91F_PDC_DisableRx(pPDC);
	AT91F_PDC_DisableTx(pPDC);

	//* Reset all Counter register Next buffer first
	AT91F_PDC_SetNextTx(pPDC, (char *) 0, 0);
	AT91F_PDC_SetNextRx(pPDC, (char *) 0, 0);
	AT91F_PDC_SetTx(pPDC, (char *) 0, 0);
	AT91F_PDC_SetRx(pPDC, (char *) 0, 0);

    //* Enable the RX and TX PDC transfer requests
	AT91F_PDC_EnableRx(pPDC);
	AT91F_PDC_EnableTx(pPDC);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_Close
//* \brief Close PDC: disable TX and RX reset transfer descriptors
//*----------------------------------------------------------------------------
static inline void AT91F_PDC_Close (
	AT91PS_PDC pPDC)       // \arg pointer to a PDC controller
{
    //* Disable the RX and TX PDC transfer requests
	AT91F_PDC_DisableRx(pPDC);
	AT91F_PDC_DisableTx(pPDC);

	//* Reset all Counter register Next buffer first
	AT91F_PDC_SetNextTx(pPDC, (char *) 0, 0);
	AT91F_PDC_SetNextRx(pPDC, (char *) 0, 0);
	AT91F_PDC_SetTx(pPDC, (char *) 0, 0);
	AT91F_PDC_SetRx(pPDC, (char *) 0, 0);

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_SendFrame
//* \brief Close PDC: disable TX and RX reset transfer descriptors
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PDC_SendFrame(
	AT91PS_PDC pPDC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	if (AT91F_PDC_IsTxEmpty(pPDC)) {
		//* Buffer and next buffer can be initialized
		AT91F_PDC_SetTx(pPDC, pBuffer, szBuffer);
		AT91F_PDC_SetNextTx(pPDC, pNextBuffer, szNextBuffer);
		return 2;
	}
	else if (AT91F_PDC_IsNextTxEmpty(pPDC)) {
		//* Only one buffer can be initialized
		AT91F_PDC_SetNextTx(pPDC, pBuffer, szBuffer);
		return 1;
	}
	else {
		//* All buffer are in use...
		return 0;
	}
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PDC_ReceiveFrame
//* \brief Close PDC: disable TX and RX reset transfer descriptors
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PDC_ReceiveFrame (
	AT91PS_PDC pPDC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	if (AT91F_PDC_IsRxEmpty(pPDC)) {
		//* Buffer and next buffer can be initialized
		AT91F_PDC_SetRx(pPDC, pBuffer, szBuffer);
		AT91F_PDC_SetNextRx(pPDC, pNextBuffer, szNextBuffer);
		return 2;
	}
	else if (AT91F_PDC_IsNextRxEmpty(pPDC)) {
		//* Only one buffer can be initialized
		AT91F_PDC_SetNextRx(pPDC, pBuffer, szBuffer);
		return 1;
	}
	else {
		//* All buffer are in use...
		return 0;
	}
}
/* *****************************************************************************
                SOFTWARE API FOR DBGU
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_InterruptEnable
//* \brief Enable DBGU Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_DBGU_InterruptEnable(
        AT91PS_DBGU pDbgu,   // \arg  pointer to a DBGU controller
        unsigned int flag) // \arg  dbgu interrupt to be enabled
{
        pDbgu->DBGU_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_InterruptDisable
//* \brief Disable DBGU Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_DBGU_InterruptDisable(
        AT91PS_DBGU pDbgu,   // \arg  pointer to a DBGU controller
        unsigned int flag) // \arg  dbgu interrupt to be disabled
{
        pDbgu->DBGU_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_GetInterruptMaskStatus
//* \brief Return DBGU Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_DBGU_GetInterruptMaskStatus( // \return DBGU Interrupt Mask Status
        AT91PS_DBGU pDbgu) // \arg  pointer to a DBGU controller
{
        return pDbgu->DBGU_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_IsInterruptMasked
//* \brief Test if DBGU Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_DBGU_IsInterruptMasked(
        AT91PS_DBGU pDbgu,   // \arg  pointer to a DBGU controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_DBGU_GetInterruptMaskStatus(pDbgu) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR RTC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_RTC_InterruptEnable
//* \brief Enable RTC Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_RTC_InterruptEnable(
        AT91PS_RTC pRtc,   // \arg  pointer to a RTC controller
        unsigned int flag) // \arg  RTC interrupt to be enabled
{
        pRtc->RTC_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_RTC_InterruptDisable
//* \brief Disable RTC Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_RTC_InterruptDisable(
        AT91PS_RTC pRtc,   // \arg  pointer to a RTC controller
        unsigned int flag) // \arg  RTC interrupt to be disabled
{
        pRtc->RTC_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_RTC_GetInterruptMaskStatus
//* \brief Return RTC Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_RTC_GetInterruptMaskStatus( // \return RTC Interrupt Mask Status
        AT91PS_RTC pRtc) // \arg  pointer to a RTC controller
{
        return pRtc->RTC_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_RTC_IsInterruptMasked
//* \brief Test if RTC Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_RTC_IsInterruptMasked(
        AT91PS_RTC pRtc,   // \arg  pointer to a RTC controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_RTC_GetInterruptMaskStatus(pRtc) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR SSC
   ***************************************************************************** */
//* Define the standard I2S mode configuration

//* Configuration to set in the SSC Transmit Clock Mode Register
//* Parameters :  nb_bit_by_slot : 8, 16 or 32 bits
//* 			  nb_slot_by_frame : number of channels
#define AT91C_I2S_ASY_MASTER_TX_SETTING(nb_bit_by_slot, nb_slot_by_frame)( +\
									   AT91C_SSC_CKS_DIV   +\
                            		   AT91C_SSC_CKO_CONTINOUS      +\
                            		   AT91C_SSC_CKG_NONE    +\
                                       AT91C_SSC_START_FALL_RF +\
                           			   AT91C_SSC_STTOUT  +\
                            		   ((1<<16) & AT91C_SSC_STTDLY) +\
                            		   ((((nb_bit_by_slot*nb_slot_by_frame)/2)-1) <<24))


//* Configuration to set in the SSC Transmit Frame Mode Register
//* Parameters : nb_bit_by_slot : 8, 16 or 32 bits
//* 			 nb_slot_by_frame : number of channels
#define AT91C_I2S_ASY_TX_FRAME_SETTING(nb_bit_by_slot, nb_slot_by_frame)( +\
									(nb_bit_by_slot-1)  +\
                            		AT91C_SSC_MSBF   +\
                            		(((nb_slot_by_frame-1)<<8) & AT91C_SSC_DATNB)  +\
                            		(((nb_bit_by_slot-1)<<16) & AT91C_SSC_FSLEN) +\
                            		AT91C_SSC_FSOS_NEGATIVE)


//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_SetBaudrate
//* \brief Set the baudrate according to the CPU clock
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_SetBaudrate (
        AT91PS_SSC pSSC,        // \arg pointer to a SSC controller
        unsigned int mainClock, // \arg peripheral clock
        unsigned int speed)     // \arg SSC baudrate
{
        unsigned int baud_value;
        //* Define the baud rate divisor register
        if (speed == 0)
           baud_value = 0;
        else
        {
           baud_value = (unsigned int) (mainClock * 10)/(2*speed);
           if ((baud_value % 10) >= 5)
                  baud_value = (baud_value / 10) + 1;
           else
                  baud_value /= 10;
        }

        pSSC->SSC_CMR = baud_value;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_Configure
//* \brief Configure SSC
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_Configure (
             AT91PS_SSC pSSC,          // \arg pointer to a SSC controller
             unsigned int syst_clock,  // \arg System Clock Frequency
             unsigned int baud_rate,   // \arg Expected Baud Rate Frequency
             unsigned int clock_rx,    // \arg Receiver Clock Parameters
             unsigned int mode_rx,     // \arg mode Register to be programmed
             unsigned int clock_tx,    // \arg Transmitter Clock Parameters
             unsigned int mode_tx)     // \arg mode Register to be programmed
{
    //* Disable interrupts
	pSSC->SSC_IDR = (unsigned int) -1;

    //* Reset receiver and transmitter
	pSSC->SSC_CR = AT91C_SSC_SWRST | AT91C_SSC_RXDIS | AT91C_SSC_TXDIS ;

    //* Define the Clock Mode Register
	AT91F_SSC_SetBaudrate(pSSC, syst_clock, baud_rate);

     //* Write the Receive Clock Mode Register
	pSSC->SSC_RCMR =  clock_rx;

     //* Write the Transmit Clock Mode Register
	pSSC->SSC_TCMR =  clock_tx;

     //* Write the Receive Frame Mode Register
	pSSC->SSC_RFMR =  mode_rx;

     //* Write the Transmit Frame Mode Register
	pSSC->SSC_TFMR =  mode_tx;

    //* Clear Transmit and Receive Counters
	AT91F_PDC_Open((AT91PS_PDC) &(pSSC->SSC_RPR));


}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_EnableRx
//* \brief Enable receiving datas
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_EnableRx (
	AT91PS_SSC pSSC)     // \arg pointer to a SSC controller
{
    //* Enable receiver
    pSSC->SSC_CR = AT91C_SSC_RXEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_DisableRx
//* \brief Disable receiving datas
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_DisableRx (
	AT91PS_SSC pSSC)     // \arg pointer to a SSC controller
{
    //* Disable receiver
    pSSC->SSC_CR = AT91C_SSC_RXDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_EnableTx
//* \brief Enable sending datas
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_EnableTx (
	AT91PS_SSC pSSC)     // \arg pointer to a SSC controller
{
    //* Enable  transmitter
    pSSC->SSC_CR = AT91C_SSC_TXEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_DisableTx
//* \brief Disable sending datas
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_DisableTx (
	AT91PS_SSC pSSC)     // \arg pointer to a SSC controller
{
    //* Disable  transmitter
    pSSC->SSC_CR = AT91C_SSC_TXDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_EnableIt
//* \brief Enable SSC IT
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_EnableIt (
	AT91PS_SSC pSSC, // \arg pointer to a SSC controller
	unsigned int flag)   // \arg IT to be enabled
{
	//* Write to the IER register
	pSSC->SSC_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_DisableIt
//* \brief Disable SSC IT
//*----------------------------------------------------------------------------
static inline void AT91F_SSC_DisableIt (
	AT91PS_SSC pSSC, // \arg pointer to a SSC controller
	unsigned int flag)   // \arg IT to be disabled
{
	//* Write to the IDR register
	pSSC->SSC_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_ReceiveFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initialized with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SSC_ReceiveFrame (
	AT91PS_SSC pSSC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	return AT91F_PDC_ReceiveFrame(
		(AT91PS_PDC) &(pSSC->SSC_RPR),
		pBuffer,
		szBuffer,
		pNextBuffer,
		szNextBuffer);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_SendFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initialized with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SSC_SendFrame(
	AT91PS_SSC pSSC,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	return AT91F_PDC_SendFrame(
		(AT91PS_PDC) &(pSSC->SSC_RPR),
		pBuffer,
		szBuffer,
		pNextBuffer,
		szNextBuffer);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_GetInterruptMaskStatus
//* \brief Return SSC Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SSC_GetInterruptMaskStatus( // \return SSC Interrupt Mask Status
        AT91PS_SSC pSsc) // \arg  pointer to a SSC controller
{
        return pSsc->SSC_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC_IsInterruptMasked
//* \brief Test if SSC Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_SSC_IsInterruptMasked(
        AT91PS_SSC pSsc,   // \arg  pointer to a SSC controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_SSC_GetInterruptMaskStatus(pSsc) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR SPI
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Open
//* \brief Open a SPI Port
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SPI_Open (
        const unsigned int null)  // \arg
{
        /* NOT DEFINED AT THIS MOMENT */
        return ( 0 );
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgCs
//* \brief Configure SPI chip select register
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_CfgCs (
	int cs,     // SPI cs number (0 to 3)
 	int val)   //  chip select register
{
	//* Write to the CSR register
	*(AT91C_SPI_CSR + cs) = val;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_EnableIt
//* \brief Enable SPI interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_EnableIt (
	AT91PS_SPI pSPI,     // pointer to a SPI controller
	unsigned int flag)   // IT to be enabled
{
	//* Write to the IER register
	pSPI->SPI_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_DisableIt
//* \brief Disable SPI interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_DisableIt (
	AT91PS_SPI pSPI, // pointer to a SPI controller
	unsigned int flag) // IT to be disabled
{
	//* Write to the IDR register
	pSPI->SPI_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Reset
//* \brief Reset the SPI controller
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_Reset (
	AT91PS_SPI pSPI // pointer to a SPI controller
	)
{
	//* Write to the CR register
	pSPI->SPI_CR = AT91C_SPI_SWRST;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Enable
//* \brief Enable the SPI controller
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_Enable (
	AT91PS_SPI pSPI // pointer to a SPI controller
	)
{
	//* Write to the CR register
	pSPI->SPI_CR = AT91C_SPI_SPIEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Disable
//* \brief Disable the SPI controller
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_Disable (
	AT91PS_SPI pSPI // pointer to a SPI controller
	)
{
	//* Write to the CR register
	pSPI->SPI_CR = AT91C_SPI_SPIDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgMode
//* \brief Enable the SPI controller
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_CfgMode (
	AT91PS_SPI pSPI, // pointer to a SPI controller
	int mode)        // mode register 
{
	//* Write to the MR register
	pSPI->SPI_MR = mode;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgPCS
//* \brief Switch to the correct PCS of SPI Mode Register : Fixed Peripheral Selected
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_CfgPCS (
	AT91PS_SPI pSPI, // pointer to a SPI controller
	char PCS_Device) // PCS of the Device
{	
 	//* Write to the MR register
	pSPI->SPI_MR &= 0xFFF0FFFF;
	pSPI->SPI_MR |= ( (PCS_Device<<16) & AT91C_SPI_PCS );
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_ReceiveFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SPI_ReceiveFrame (
	AT91PS_SPI pSPI,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	return AT91F_PDC_ReceiveFrame(
		(AT91PS_PDC) &(pSPI->SPI_RPR),
		pBuffer,
		szBuffer,
		pNextBuffer,
		szNextBuffer);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_SendFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is bSPIy
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SPI_SendFrame(
	AT91PS_SPI pSPI,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	return AT91F_PDC_SendFrame(
		(AT91PS_PDC) &(pSPI->SPI_RPR),
		pBuffer,
		szBuffer,
		pNextBuffer,
		szNextBuffer);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_Close
//* \brief Close SPI: disable IT disable transfert, close PDC
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_Close (
	AT91PS_SPI pSPI)     // \arg pointer to a SPI controller
{
    //* Reset all the Chip Select register
    pSPI->SPI_CSR[0] = 0 ;
    pSPI->SPI_CSR[1] = 0 ;
    pSPI->SPI_CSR[2] = 0 ;
    pSPI->SPI_CSR[3] = 0 ;

    //* Reset the SPI mode
    pSPI->SPI_MR = 0  ;

    //* Disable all interrupts
    pSPI->SPI_IDR = 0xFFFFFFFF ;

    //* Abort the Peripheral Data Transfers
    AT91F_PDC_Close((AT91PS_PDC) &(pSPI->SPI_RPR));

    //* Disable receiver and transmitter and stop any activity immediately
    pSPI->SPI_CR = AT91C_SPI_SPIDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_PutChar
//* \brief Send a character,does not check if ready to send
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_PutChar (
	AT91PS_SPI pSPI,
	unsigned int character,
             unsigned int cs_number )
{
    unsigned int value_for_cs;
    value_for_cs = (~(1 << cs_number)) & 0xF;  //Place a zero among a 4 ONEs number
    pSPI->SPI_TDR = (character & 0xFFFF) | (value_for_cs << 16);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_GetChar
//* \brief Receive a character,does not check if a character is available
//*----------------------------------------------------------------------------
static inline int AT91F_SPI_GetChar (
	const AT91PS_SPI pSPI)
{
    return((pSPI->SPI_RDR) & 0xFFFF);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_GetInterruptMaskStatus
//* \brief Return SPI Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_SPI_GetInterruptMaskStatus( // \return SPI Interrupt Mask Status
        AT91PS_SPI pSpi) // \arg  pointer to a SPI controller
{
        return pSpi->SPI_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_IsInterruptMasked
//* \brief Test if SPI Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_SPI_IsInterruptMasked(
        AT91PS_SPI pSpi,   // \arg  pointer to a SPI controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_SPI_GetInterruptMaskStatus(pSpi) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR TC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_InterruptEnable
//* \brief Enable TC Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_TC_InterruptEnable(
        AT91PS_TC pTc,   // \arg  pointer to a TC controller
        unsigned int flag) // \arg  TC interrupt to be enabled
{
        pTc->TC_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_InterruptDisable
//* \brief Disable TC Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_TC_InterruptDisable(
        AT91PS_TC pTc,   // \arg  pointer to a TC controller
        unsigned int flag) // \arg  TC interrupt to be disabled
{
        pTc->TC_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_GetInterruptMaskStatus
//* \brief Return TC Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_TC_GetInterruptMaskStatus( // \return TC Interrupt Mask Status
        AT91PS_TC pTc) // \arg  pointer to a TC controller
{
        return pTc->TC_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC_IsInterruptMasked
//* \brief Test if TC Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_TC_IsInterruptMasked(
        AT91PS_TC pTc,   // \arg  pointer to a TC controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_TC_GetInterruptMaskStatus(pTc) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR PMC
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_CKGR_GetMainClock
//* \brief Return Main clock in Hz
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_CKGR_GetMainClock (
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock)  // \arg slowClock in Hz
{
	return ((pCKGR->CKGR_MCFR  & AT91C_CKGR_MAINF) * slowClock) >> 4;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetProcessorClock
//* \brief Return processor clock in Hz (for AT91RM3400 and AT91RM9200)
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PMC_GetProcessorClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock)  // \arg slowClock in Hz
{
	unsigned int reg = pPMC->PMC_MCKR;
	unsigned int prescaler = (1 << ((reg & AT91C_PMC_PRES) >> 2));
	unsigned int pllDivider, pllMultiplier;

	switch (reg & AT91C_PMC_CSS) {
		case AT91C_PMC_CSS_SLOW_CLK: // Slow clock selected
			return slowClock / prescaler;
		case AT91C_PMC_CSS_MAIN_CLK: // Main clock is selected
			return AT91F_CKGR_GetMainClock(pCKGR, slowClock) / prescaler;
		case AT91C_PMC_CSS_PLLA_CLK: // PLLA clock is selected
			reg = pCKGR->CKGR_PLLAR;
			pllDivider    = (reg  & AT91C_CKGR_DIVA);
			pllMultiplier = ((reg  & AT91C_CKGR_MULA) >> 16) + 1;
			if (reg & AT91C_CKGR_SRCA) // Source is Main clock
				return AT91F_CKGR_GetMainClock(pCKGR, slowClock) / pllDivider * pllMultiplier / prescaler;
			else                       // Source is Slow clock
				return slowClock / pllDivider * pllMultiplier / prescaler;
		case AT91C_PMC_CSS_PLLB_CLK: // PLLB clock is selected
			reg = pCKGR->CKGR_PLLBR;
			pllDivider    = (reg  & AT91C_CKGR_DIVB);
			pllMultiplier = ((reg  & AT91C_CKGR_MULB) >> 16) + 1;
			return AT91F_CKGR_GetMainClock(pCKGR, slowClock) / pllDivider * pllMultiplier / prescaler;
	}
	return 0;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_GetMasterClock
//* \brief Return master clock in Hz (just for AT91RM9200)
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PMC_GetMasterClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	AT91PS_CKGR pCKGR, // \arg pointer to CKGR controller
	unsigned int slowClock)  // \arg slowClock in Hz
{
	return AT91F_PMC_GetProcessorClock(pPMC, pCKGR, slowClock) /
		(((pPMC->PMC_MCKR & AT91C_PMC_MDIV) >> 8)+1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_EnablePeriphClock
//* \brief Enable peripheral clock
//*----------------------------------------------------------------------------
static inline void AT91F_PMC_EnablePeriphClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int periphIds)  // \arg IDs of peripherals to enable
{
	pPMC->PMC_PCER = periphIds;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_DisablePeriphClock
//* \brief Enable peripheral clock
//*----------------------------------------------------------------------------
static inline void AT91F_PMC_DisablePeriphClock (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int periphIds)  // \arg IDs of peripherals to enable
{
	pPMC->PMC_PCDR = periphIds;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_EnablePCK
//* \brief Enable peripheral clock
//*----------------------------------------------------------------------------
static inline void AT91F_PMC_EnablePCK (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int pck,  // \arg Peripheral clock identifier 0 .. 7
	unsigned int ccs,  // \arg clock selection: AT91C_PMC_CSS_SLOW_CLK, AT91C_PMC_CSS_MAIN_CLK, AT91C_PMC_CSS_PLLA_CLK, AT91C_PMC_CSS_PLLB_CLK
	unsigned int pres) // \arg Programmable clock prescalar AT91C_PMC_PRES_CLK, AT91C_PMC_PRES_CLK_2, ..., AT91C_PMC_PRES_CLK_64
{
	pPMC->PMC_PCKR[pck] = ccs | pres;
	pPMC->PMC_SCER = (1 << pck) << 8;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_DisablePCK
//* \brief Enable peripheral clock
//*----------------------------------------------------------------------------
static inline void AT91F_PMC_DisablePCK (
	AT91PS_PMC pPMC, // \arg pointer to PMC controller
	unsigned int pck)  // \arg Peripheral clock identifier 0 .. 7
{
	pPMC->PMC_SCDR = (1 << pck) << 8;
}

/* *****************************************************************************
                SOFTWARE API FOR PIO
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgPeriph
//* \brief Enable pins to be drived by peripheral
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgPeriph(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int periphAEnable,  // \arg PERIPH A to enable
	unsigned int periphBEnable)  // \arg PERIPH B to enable

{
	pPio->PIO_ASR = periphAEnable;
	pPio->PIO_BSR = periphBEnable;
	pPio->PIO_PDR = (periphAEnable | periphBEnable); // Set in Periph mode
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgOutput
//* \brief Enable PIO in output mode
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgOutput(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int pioEnable)      // \arg PIO to be enabled
{
	pPio->PIO_PER = pioEnable; // Set in PIO mode
	pPio->PIO_OER = pioEnable; // Configure in Output
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgInput
//* \brief Enable PIO in input mode
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgInput(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int inputEnable)      // \arg PIO to be enabled
{
	// Disable output
	pPio->PIO_ODR  = inputEnable;
	pPio->PIO_PER  = inputEnable;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgOpendrain
//* \brief Configure PIO in open drain
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgOpendrain(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int multiDrvEnable) // \arg pio to be configured in open drain
{
	// Configure the multi-drive option
	pPio->PIO_MDDR = ~multiDrvEnable;
	pPio->PIO_MDER = multiDrvEnable;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgPullup
//* \brief Enable pullup on PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgPullup(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int pullupEnable)   // \arg enable pullup on PIO
{
		// Connect or not Pullup
	pPio->PIO_PPUDR = ~pullupEnable;
	pPio->PIO_PPUER = pullupEnable;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgDirectDrive
//* \brief Enable direct drive on PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgDirectDrive(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int directDrive)    // \arg PIO to be configured with direct drive

{
	// Configure the Direct Drive
	pPio->PIO_OWDR  = ~directDrive;
	pPio->PIO_OWER  = directDrive;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_CfgInputFilter
//* \brief Enable input filter on input PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_CfgInputFilter(
	AT91PS_PIO pPio,             // \arg pointer to a PIO controller
	unsigned int inputFilter)    // \arg PIO to be configured with input filter

{
	// Configure the Direct Drive
	pPio->PIO_IFDR  = ~inputFilter;
	pPio->PIO_IFER  = inputFilter;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInput
//* \brief Return PIO input value
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetInput( // \return PIO input
	AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
	return pPio->PIO_PDSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInputSet
//* \brief Test if PIO is input flag is active
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsInputSet(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag) // \arg  flag to be tested
{
	return (AT91F_PIO_GetInput(pPio) & flag);
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_SetOutput
//* \brief Set to 1 output PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_SetOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag) // \arg  output to be set
{
	pPio->PIO_SODR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_ClearOutput
//* \brief Set to 0 output PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_ClearOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag) // \arg  output to be cleared
{
	pPio->PIO_CODR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_ForceOutput
//* \brief Force output when Direct drive option is enabled
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_ForceOutput(
	AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
	unsigned int flag) // \arg  output to be forced
{
	pPio->PIO_ODSR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_Enable
//* \brief Enable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_Enable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio to be enabled 
{
        pPio->PIO_PER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_Disable
//* \brief Disable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_Disable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio to be disabled 
{
        pPio->PIO_PDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetStatus
//* \brief Return PIO Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetStatus( // \return PIO Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_PSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsSet
//* \brief Test if PIO is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputEnable
//* \brief Output Enable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_OutputEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio output to be enabled
{
        pPio->PIO_OER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputDisable
//* \brief Output Enable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_OutputDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio output to be disabled
{
        pPio->PIO_ODR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetOutputStatus
//* \brief Return PIO Output Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetOutputStatus( // \return PIO Output Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_OSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsOuputSet
//* \brief Test if PIO Output is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsOutputSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetOutputStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InputFilterEnable
//* \brief Input Filter Enable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_InputFilterEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio input filter to be enabled
{
        pPio->PIO_IFER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InputFilterDisable
//* \brief Input Filter Disable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_InputFilterDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio input filter to be disabled
{
        pPio->PIO_IFDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInputFilterStatus
//* \brief Return PIO Input Filter Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetInputFilterStatus( // \return PIO Input Filter Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_IFSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInputFilterSet
//* \brief Test if PIO Input filter is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsInputFilterSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetInputFilterStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetOutputDataStatus
//* \brief Return PIO Output Data Status 
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetOutputDataStatus( // \return PIO Output Data Status 
	AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_ODSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InterruptEnable
//* \brief Enable PIO Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_InterruptEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio interrupt to be enabled
{
        pPio->PIO_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_InterruptDisable
//* \brief Disable PIO Interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_InterruptDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio interrupt to be disabled
{
        pPio->PIO_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInterruptMaskStatus
//* \brief Return PIO Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetInterruptMaskStatus( // \return PIO Interrupt Mask Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetInterruptStatus
//* \brief Return PIO Interrupt Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetInterruptStatus( // \return PIO Interrupt Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_ISR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInterruptMasked
//* \brief Test if PIO Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsInterruptMasked(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetInterruptMaskStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsInterruptSet
//* \brief Test if PIO Interrupt is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsInterruptSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetInterruptStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_MultiDriverEnable
//* \brief Multi Driver Enable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_MultiDriverEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio to be enabled
{
        pPio->PIO_MDER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_MultiDriverDisable
//* \brief Multi Driver Disable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_MultiDriverDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio to be disabled
{
        pPio->PIO_MDDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetMultiDriverStatus
//* \brief Return PIO Multi Driver Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetMultiDriverStatus( // \return PIO Multi Driver Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_MDSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsMultiDriverSet
//* \brief Test if PIO MultiDriver is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsMultiDriverSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetMultiDriverStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_A_RegisterSelection
//* \brief PIO A Register Selection 
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_A_RegisterSelection(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio A register selection
{
        pPio->PIO_ASR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_B_RegisterSelection
//* \brief PIO B Register Selection 
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_B_RegisterSelection(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio B register selection 
{
        pPio->PIO_BSR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_Get_AB_RegisterStatus
//* \brief Return PIO Interrupt Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_Get_AB_RegisterStatus( // \return PIO AB Register Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_ABSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsAB_RegisterSet
//* \brief Test if PIO AB Register is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsAB_RegisterSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_Get_AB_RegisterStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputWriteEnable
//* \brief Output Write Enable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_OutputWriteEnable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio output write to be enabled
{
        pPio->PIO_OWER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_OutputWriteDisable
//* \brief Output Write Disable PIO
//*----------------------------------------------------------------------------
static inline void AT91F_PIO_OutputWriteDisable(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  pio output write to be disabled
{
        pPio->PIO_OWDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetOutputWriteStatus
//* \brief Return PIO Output Write Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetOutputWriteStatus( // \return PIO Output Write Status
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_OWSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsOutputWriteSet
//* \brief Test if PIO OutputWrite is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsOutputWriteSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetOutputWriteStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_GetCfgPullup
//* \brief Return PIO Configuration Pullup
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_PIO_GetCfgPullup( // \return PIO Configuration Pullup 
        AT91PS_PIO pPio) // \arg  pointer to a PIO controller
{
        return pPio->PIO_PPUSR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsOutputDataStatusSet
//* \brief Test if PIO Output Data Status is Set 
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsOutputDataStatusSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_PIO_GetOutputDataStatus(pPio) & flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIO_IsCfgPullupStatusSet
//* \brief Test if PIO Configuration Pullup Status is Set
//*----------------------------------------------------------------------------
static inline int AT91F_PIO_IsCfgPullupStatusSet(
        AT91PS_PIO pPio,   // \arg  pointer to a PIO controller
        unsigned int flag) // \arg  flag to be tested
{
        return (~AT91F_PIO_GetCfgPullup(pPio) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR TWI
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_EnableIt
//* \brief Enable TWI IT
//*----------------------------------------------------------------------------
static inline void AT91F_TWI_EnableIt (
	AT91PS_TWI pTWI, // \arg pointer to a TWI controller
	unsigned int flag)   // \arg IT to be enabled
{
	//* Write to the IER register
	pTWI->TWI_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_DisableIt
//* \brief Disable TWI IT
//*----------------------------------------------------------------------------
static inline void AT91F_TWI_DisableIt (
	AT91PS_TWI pTWI, // \arg pointer to a TWI controller
	unsigned int flag)   // \arg IT to be disabled
{
	//* Write to the IDR register
	pTWI->TWI_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_Configure
//* \brief Configure TWI in master mode
//*----------------------------------------------------------------------------
static inline void AT91F_TWI_Configure ( AT91PS_TWI pTWI )          // \arg pointer to a TWI controller
{
    //* Disable interrupts
	pTWI->TWI_IDR = (unsigned int) -1;

    //* Reset peripheral
	pTWI->TWI_CR = AT91C_TWI_SWRST;

	//* Set Master mode
	pTWI->TWI_CR = AT91C_TWI_MSEN | AT91C_TWI_SVDIS;

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_GetInterruptMaskStatus
//* \brief Return TWI Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_TWI_GetInterruptMaskStatus( // \return TWI Interrupt Mask Status
        AT91PS_TWI pTwi) // \arg  pointer to a TWI controller
{
        return pTwi->TWI_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_IsInterruptMasked
//* \brief Test if TWI Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_TWI_IsInterruptMasked(
        AT91PS_TWI pTwi,   // \arg  pointer to a TWI controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_TWI_GetInterruptMaskStatus(pTwi) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR USART
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Baudrate
//* \brief Calculate the baudrate
//* Standard Asynchronous Mode : 8 bits , 1 stop , no parity
#define AT91C_US_ASYNC_MODE ( AT91C_US_USMODE_NORMAL + \
                        AT91C_US_NBSTOP_1_BIT + \
                        AT91C_US_PAR_NONE + \
                        AT91C_US_CHRL_8_BITS + \
                        AT91C_US_CLKS_CLOCK )

//* Standard External Asynchronous Mode : 8 bits , 1 stop , no parity
#define AT91C_US_ASYNC_SCK_MODE ( AT91C_US_USMODE_NORMAL + \
                            AT91C_US_NBSTOP_1_BIT + \
                            AT91C_US_PAR_NONE + \
                            AT91C_US_CHRL_8_BITS + \
                            AT91C_US_CLKS_EXT )

//* Standard Synchronous Mode : 8 bits , 1 stop , no parity
#define AT91C_US_SYNC_MODE ( AT91C_US_SYNC + \
                       AT91C_US_USMODE_NORMAL + \
                       AT91C_US_NBSTOP_1_BIT + \
                       AT91C_US_PAR_NONE + \
                       AT91C_US_CHRL_8_BITS + \
                       AT91C_US_CLKS_CLOCK )

//* SCK used Label
#define AT91C_US_SCK_USED (AT91C_US_CKLO | AT91C_US_CLKS_EXT)

//* Standard ISO T=0 Mode : 8 bits , 1 stop , parity
#define AT91C_US_ISO_READER_MODE ( AT91C_US_USMODE_ISO7816_0 + \
					   		 AT91C_US_CLKS_CLOCK +\
                       		 AT91C_US_NBSTOP_1_BIT + \
                       		 AT91C_US_PAR_EVEN + \
                       		 AT91C_US_CHRL_8_BITS + \
                       		 AT91C_US_CKLO +\
                       		 AT91C_US_OVER)

//* Standard IRDA mode
#define AT91C_US_ASYNC_IRDA_MODE (  AT91C_US_USMODE_IRDA + \
                            AT91C_US_NBSTOP_1_BIT + \
                            AT91C_US_PAR_NONE + \
                            AT91C_US_CHRL_8_BITS + \
                            AT91C_US_CLKS_CLOCK )

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Baudrate
//* \brief Caluculate baud_value according to the main clock and the baud rate
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_US_Baudrate (
	const unsigned int main_clock, // \arg peripheral clock
	const unsigned int baud_rate)  // \arg UART baudrate
{
	unsigned int baud_value = ((main_clock*10)/(baud_rate * 16));
	if ((baud_value % 10) >= 5)
		baud_value = (baud_value / 10) + 1;
	else
		baud_value /= 10;
	return baud_value;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SetBaudrate
//* \brief Set the baudrate according to the CPU clock
//*----------------------------------------------------------------------------
static inline void AT91F_US_SetBaudrate (
	AT91PS_USART pUSART,    // \arg pointer to a USART controller
	unsigned int mainClock, // \arg peripheral clock
	unsigned int speed)     // \arg UART baudrate
{
	//* Define the baud rate divisor register
	pUSART->US_BRGR = AT91F_US_Baudrate(mainClock, speed);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SetTimeguard
//* \brief Set USART timeguard
//*----------------------------------------------------------------------------
static inline void AT91F_US_SetTimeguard (
	AT91PS_USART pUSART,    // \arg pointer to a USART controller
	unsigned int timeguard) // \arg timeguard value
{
	//* Write the Timeguard Register
	pUSART->US_TTGR = timeguard ;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_EnableIt
//* \brief Enable USART IT
//*----------------------------------------------------------------------------
static inline void AT91F_US_EnableIt (
	AT91PS_USART pUSART, // \arg pointer to a USART controller
	unsigned int flag)   // \arg IT to be enabled
{
	//* Write to the IER register
	pUSART->US_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_DisableIt
//* \brief Disable USART IT
//*----------------------------------------------------------------------------
static inline void AT91F_US_DisableIt (
	AT91PS_USART pUSART, // \arg pointer to a USART controller
	unsigned int flag)   // \arg IT to be disabled
{
	//* Write to the IER register
	pUSART->US_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Configure
//* \brief Configure USART
//*----------------------------------------------------------------------------
static inline void AT91F_US_Configure (
	AT91PS_USART pUSART,     // \arg pointer to a USART controller
	unsigned int mainClock,  // \arg peripheral clock
	unsigned int mode ,      // \arg mode Register to be programmed
	unsigned int baudRate ,  // \arg baudrate to be programmed
	unsigned int timeguard ) // \arg timeguard to be programmed
{
    //* Disable interrupts
    pUSART->US_IDR = (unsigned int) -1;

    //* Reset receiver and transmitter
    pUSART->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS ;

	//* Define the baud rate divisor register
	AT91F_US_SetBaudrate(pUSART, mainClock, baudRate);

	//* Write the Timeguard Register
	AT91F_US_SetTimeguard(pUSART, timeguard);

    //* Clear Transmit and Receive Counters
    AT91F_PDC_Open((AT91PS_PDC) &(pUSART->US_RPR));

    //* Define the USART mode
    pUSART->US_MR = mode  ;

}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_EnableRx
//* \brief Enable receiving characters
//*----------------------------------------------------------------------------
static inline void AT91F_US_EnableRx (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
    //* Enable receiver
    pUSART->US_CR = AT91C_US_RXEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_EnableTx
//* \brief Enable sending characters
//*----------------------------------------------------------------------------
static inline void AT91F_US_EnableTx (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
    //* Enable  transmitter
    pUSART->US_CR = AT91C_US_TXEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_ResetRx
//* \brief Reset Receiver and re-enable it
//*----------------------------------------------------------------------------
static inline void AT91F_US_ResetRx (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
	//* Reset receiver
	pUSART->US_CR = AT91C_US_RSTRX;
    //* Re-Enable receiver
    pUSART->US_CR = AT91C_US_RXEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_ResetTx
//* \brief Reset Transmitter and re-enable it
//*----------------------------------------------------------------------------
static inline void AT91F_US_ResetTx (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
	//* Reset transmitter
	pUSART->US_CR = AT91C_US_RSTTX;
    //* Enable transmitter
    pUSART->US_CR = AT91C_US_TXEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_DisableRx
//* \brief Disable Receiver
//*----------------------------------------------------------------------------
static inline void AT91F_US_DisableRx (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
    //* Disable receiver
    pUSART->US_CR = AT91C_US_RXDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_DisableTx
//* \brief Disable Transmitter
//*----------------------------------------------------------------------------
static inline void AT91F_US_DisableTx (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
    //* Disable transmitter
    pUSART->US_CR = AT91C_US_TXDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Close
//* \brief Close USART: disable IT disable receiver and transmitter, close PDC
//*----------------------------------------------------------------------------
static inline void AT91F_US_Close (
	AT91PS_USART pUSART)     // \arg pointer to a USART controller
{
    //* Reset the baud rate divisor register
    pUSART->US_BRGR = 0 ;

    //* Reset the USART mode
    pUSART->US_MR = 0  ;

    //* Reset the Timeguard Register
    pUSART->US_TTGR = 0;

    //* Disable all interrupts
    pUSART->US_IDR = 0xFFFFFFFF ;

    //* Abort the Peripheral Data Transfers
    AT91F_PDC_Close((AT91PS_PDC) &(pUSART->US_RPR));

    //* Disable receiver and transmitter and stop any activity immediately
    pUSART->US_CR = AT91C_US_TXDIS | AT91C_US_RXDIS | AT91C_US_RSTTX | AT91C_US_RSTRX ;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_TxReady
//* \brief Return 1 if a character can be written in US_THR
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_US_TxReady (
	AT91PS_USART pUSART )     // \arg pointer to a USART controller
{
    return (pUSART->US_CSR & AT91C_US_TXRDY);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_RxReady
//* \brief Return 1 if a character can be read in US_RHR
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_US_RxReady (
	AT91PS_USART pUSART )     // \arg pointer to a USART controller
{
    return (pUSART->US_CSR & AT91C_US_RXRDY);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Error
//* \brief Return the error flag
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_US_Error (
	AT91PS_USART pUSART )     // \arg pointer to a USART controller
{
    return (pUSART->US_CSR &
    	(AT91C_US_OVRE |  // Overrun error
    	 AT91C_US_FRAME | // Framing error
    	 AT91C_US_PARE));  // Parity error
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_PutChar
//* \brief Send a character,does not check if ready to send
//*----------------------------------------------------------------------------
static inline void AT91F_US_PutChar (
	AT91PS_USART pUSART,
	int character )
{
    pUSART->US_THR = (character & 0x1FF);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_GetChar
//* \brief Receive a character,does not check if a character is available
//*----------------------------------------------------------------------------
static inline int AT91F_US_GetChar (
	const AT91PS_USART pUSART)
{
    return((pUSART->US_RHR) & 0x1FF);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SendFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_US_SendFrame(
	AT91PS_USART pUSART,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	return AT91F_PDC_SendFrame(
		(AT91PS_PDC) &(pUSART->US_RPR),
		pBuffer,
		szBuffer,
		pNextBuffer,
		szNextBuffer);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_ReceiveFrame
//* \brief Return 2 if PDC has been initialized with Buffer and Next Buffer, 1 if PDC has been initializaed with Next Buffer, 0 if PDC is busy
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_US_ReceiveFrame (
	AT91PS_USART pUSART,
	char *pBuffer,
	unsigned int szBuffer,
	char *pNextBuffer,
	unsigned int szNextBuffer )
{
	return AT91F_PDC_ReceiveFrame(
		(AT91PS_PDC) &(pUSART->US_RPR),
		pBuffer,
		szBuffer,
		pNextBuffer,
		szNextBuffer);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_SetIrdaFilter
//* \brief Set the value of IrDa filter tregister
//*----------------------------------------------------------------------------
static inline void AT91F_US_SetIrdaFilter (
	AT91PS_USART pUSART,
	unsigned char value
)
{
	pUSART->US_IF = value;
}

/* *****************************************************************************
                SOFTWARE API FOR MCI
   ***************************************************************************** */
//* Classic MCI Mode Register Configuration with PDC mode enabled and MCK = MCI Clock
#define AT91C_MCI_MR_PDCMODE	(AT91C_MCI_CLKDIV |\
                                AT91C_MCI_PWSDIV |\
                                (AT91C_MCI_PWSDIV<<1) |\
                                AT91C_MCI_PDCMODE)

//* Classic MCI Data Timeout Register Configuration with 1048576 MCK cycles between 2 data transfer
#define AT91C_MCI_DTOR_1MEGA_CYCLES	(AT91C_MCI_DTOCYC | AT91C_MCI_DTOMUL)

//* Classic MCI SDCard Register Configuration with 1-bit data bus on slot A
#define AT91C_MCI_MMC_SLOTA	(AT91C_MCI_SCDSEL & 0x0)

//* Classic MCI SDCard Register Configuration with 1-bit data bus on slot B
#define AT91C_MCI_MMC_SLOTB	(AT91C_MCI_SCDSEL)

//* Classic MCI SDCard Register Configuration with 4-bit data bus on slot A
#define AT91C_MCI_SDCARD_4BITS_SLOTA	( (AT91C_MCI_SCDSEL & 0x0) | AT91C_MCI_SCDBUS )

//* Classic MCI SDCard Register Configuration with 4-bit data bus on slot B
#define AT91C_MCI_SDCARD_4BITS_SLOTB	(AT91C_MCI_SCDSEL | AT91C_MCI_SCDBUS)



//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Configure
//* \brief Configure the MCI
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_Configure (
        AT91PS_MCI pMCI,  			 // \arg pointer to a MCI controller
        unsigned int DTOR_register,  // \arg Data Timeout Register to be programmed
        unsigned int MR_register,  	 // \arg Mode Register to be programmed
        unsigned int SDCR_register)  // \arg SDCard Register to be programmed
{
    //* Reset the MCI
    pMCI->MCI_CR = AT91C_MCI_MCIEN | AT91C_MCI_PWSEN;

    //* Disable all the interrupts
    pMCI->MCI_IDR = 0xFFFFFFFF;

    //* Set the Data Timeout Register
    pMCI->MCI_DTOR = DTOR_register;

    //* Set the Mode Register
    pMCI->MCI_MR = MR_register;

    //* Set the SDCard Register
    pMCI->MCI_SDCR = SDCR_register;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_EnableIt
//* \brief Enable MCI IT
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_EnableIt (
        AT91PS_MCI pMCI, // \arg pointer to a MCI controller
        unsigned int flag)   // \arg IT to be enabled
{
    //* Write to the IER register
    pMCI->MCI_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_DisableIt
//* \brief Disable MCI IT
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_DisableIt (
        AT91PS_MCI pMCI, // \arg pointer to a MCI controller
        unsigned int flag)   // \arg IT to be disabled
{
    //* Write to the IDR register
    pMCI->MCI_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Enable_Interface
//* \brief Enable the MCI Interface
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_Enable_Interface (
        AT91PS_MCI pMCI)     // \arg pointer to a MCI controller
{
    //* Enable the MCI
    pMCI->MCI_CR = AT91C_MCI_MCIEN;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Disable_Interface
//* \brief Disable the MCI Interface
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_Disable_Interface (
        AT91PS_MCI pMCI)     // \arg pointer to a MCI controller
{
    //* Disable the MCI
    pMCI->MCI_CR = AT91C_MCI_MCIDIS;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_Cfg_ModeRegister
//* \brief Configure the MCI Mode Register
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_Cfg_ModeRegister (
        AT91PS_MCI pMCI, // \arg pointer to a MCI controller
        unsigned int mode_register)   // \arg value to set in the mode register
{
    //* Configure the MCI MR
    pMCI->MCI_MR = mode_register;
}
/* *****************************************************************************
                SOFTWARE API FOR AIC
   ***************************************************************************** */
#define AT91C_AIC_BRANCH_OPCODE ((void (*) ()) 0xE51FFF20) // ldr, pc, [pc, #-&F20]

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_ConfigureIt
//* \brief Interrupt Handler Initialization
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_AIC_ConfigureIt (
	AT91PS_AIC pAic,  // \arg pointer to the AIC registers
	unsigned int irq_id,     // \arg interrupt number to initialize
	unsigned int priority,   // \arg priority to give to the interrupt
	unsigned int src_type,   // \arg activation and sense of activation
	void (*newHandler) (void) ) // \arg address of the interrupt handler
{
	unsigned int oldHandler;
    unsigned int mask ;

    oldHandler = pAic->AIC_SVR[irq_id];

    mask = 0x1 << irq_id ;
    //* Disable the interrupt on the interrupt controller
    pAic->AIC_IDCR = mask ;
    //* Save the interrupt handler routine pointer and the interrupt priority
    pAic->AIC_SVR[irq_id] = (unsigned int) newHandler ;
    //* Store the Source Mode Register
    pAic->AIC_SMR[irq_id] = src_type | priority  ;
    //* Clear the interrupt on the interrupt controller
    pAic->AIC_ICCR = mask ;

	return oldHandler;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_EnableIt
//* \brief Enable corresponding IT number
//*----------------------------------------------------------------------------
static inline void AT91F_AIC_EnableIt (
	AT91PS_AIC pAic,      // \arg pointer to the AIC registers
	unsigned int irq_id ) // \arg interrupt number to initialize
{
    //* Enable the interrupt on the interrupt controller
    pAic->AIC_IECR = 0x1 << irq_id ;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_DisableIt
//* \brief Disable corresponding IT number
//*----------------------------------------------------------------------------
static inline void AT91F_AIC_DisableIt (
	AT91PS_AIC pAic,      // \arg pointer to the AIC registers
	unsigned int irq_id ) // \arg interrupt number to initialize
{
    unsigned int mask = 0x1 << irq_id;
    //* Disable the interrupt on the interrupt controller
    pAic->AIC_IDCR = mask ;
    //* Clear the interrupt on the Interrupt Controller ( if one is pending )
    pAic->AIC_ICCR = mask ;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_ClearIt
//* \brief Clear corresponding IT number
//*----------------------------------------------------------------------------
static inline void AT91F_AIC_ClearIt (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id) // \arg interrupt number to initialize
{
    //* Clear the interrupt on the Interrupt Controller ( if one is pending )
    pAic->AIC_ICCR = (0x1 << irq_id);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_AcknowledgeIt
//* \brief Acknowledge corresponding IT number
//*----------------------------------------------------------------------------
static inline void AT91F_AIC_AcknowledgeIt (
	AT91PS_AIC pAic)     // \arg pointer to the AIC registers
{
    pAic->AIC_EOICR = pAic->AIC_EOICR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_SetExceptionVector
//* \brief Configure vector handler
//*----------------------------------------------------------------------------
static inline unsigned int  AT91F_AIC_SetExceptionVector (
	unsigned int *pVector, // \arg pointer to the AIC registers
	void (*Handler) () )   // \arg Interrupt Handler
{
	unsigned int oldVector = *pVector;

	if ((unsigned int) Handler == (unsigned int) AT91C_AIC_BRANCH_OPCODE)
		*pVector = (unsigned int) AT91C_AIC_BRANCH_OPCODE;
	else
		*pVector = (((((unsigned int) Handler) - ((unsigned int) pVector) - 0x8) >> 2) & 0x00FFFFFF) | 0xEA000000;

	return oldVector;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_Trig
//* \brief Trig an IT
//*----------------------------------------------------------------------------
static inline void  AT91F_AIC_Trig (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id) // \arg interrupt number
{
	pAic->AIC_ISCR = (0x1 << irq_id) ;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_IsActive
//* \brief Test if an IT is active
//*----------------------------------------------------------------------------
static inline unsigned int  AT91F_AIC_IsActive (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id) // \arg Interrupt Number
{
	return (pAic->AIC_ISR & (0x1 << irq_id));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_IsPending
//* \brief Test if an IT is pending
//*----------------------------------------------------------------------------
static inline unsigned int  AT91F_AIC_IsPending (
	AT91PS_AIC pAic,     // \arg pointer to the AIC registers
	unsigned int irq_id) // \arg Interrupt Number
{
	return (pAic->AIC_IPR & (0x1 << irq_id));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_Open
//* \brief Set exception vectors and AIC registers to default values
//*----------------------------------------------------------------------------
static inline void AT91F_AIC_Open(
	AT91PS_AIC pAic,        // \arg pointer to the AIC registers
	void (*IrqHandler) (),  // \arg Default IRQ vector exception
	void (*FiqHandler) (),  // \arg Default FIQ vector exception
	void (*DefaultHandler)  (), // \arg Default Handler set in ISR
	void (*SpuriousHandler) (), // \arg Default Spurious Handler
	unsigned int protectMode)   // \arg Debug Control Register
{
	int i;

	// Disable all interrupts and set IVR to the default handler
	for (i = 0; i < 32; ++i) {
		AT91F_AIC_DisableIt(pAic, i);
		AT91F_AIC_ConfigureIt(pAic, i, AT91C_AIC_PRIOR_LOWEST, AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, DefaultHandler);
	}

	// Set the IRQ exception vector
	AT91F_AIC_SetExceptionVector((unsigned int *) 0x18, IrqHandler);
	// Set the Fast Interrupt exception vector
	AT91F_AIC_SetExceptionVector((unsigned int *) 0x1C, FiqHandler);

	pAic->AIC_SPU = (unsigned int) SpuriousHandler;
	pAic->AIC_DCR = protectMode;
}
/* *****************************************************************************
                SOFTWARE API FOR UDP
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EnableIt
//* \brief Enable UDP IT
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EnableIt (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag)   // \arg IT to be enabled
{
	//* Write to the IER register
	pUDP->UDP_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_DisableIt
//* \brief Disable UDP IT
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_DisableIt (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag)   // \arg IT to be disabled
{
	//* Write to the IDR register
	pUDP->UDP_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_SetAddress
//* \brief Set UDP functional address
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_SetAddress (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned char address)   // \arg new UDP address
{
	pUDP->UDP_FADDR = (AT91C_UDP_FEN | address);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EnableEp
//* \brief Enable Endpoint
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EnableEp (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag)   // \arg endpoints to be enabled
{
	pUDP->UDP_GLBSTATE  |= flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_DisableEp
//* \brief Enable Endpoint
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_DisableEp (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag)   // \arg endpoints to be enabled
{
	pUDP->UDP_GLBSTATE  &= ~(flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_SetState
//* \brief Set UDP Device state
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_SetState (
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag)   // \arg new UDP address
{
	pUDP->UDP_GLBSTATE  &= ~(AT91C_UDP_FADDEN | AT91C_UDP_CONFG);
	pUDP->UDP_GLBSTATE  |= flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_GetState
//* \brief return UDP Device state
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_UDP_GetState ( // \return the UDP device state
	AT91PS_UDP pUDP)     // \arg pointer to a UDP controller
{
	return (pUDP->UDP_GLBSTATE  & (AT91C_UDP_FADDEN | AT91C_UDP_CONFG));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_ResetEp
//* \brief Reset UDP endpoint
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_ResetEp ( // \return the UDP device state
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned int flag)   // \arg Endpoints to be reset
{
	pUDP->UDP_RSTEP = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpStall
//* \brief Endpoint will STALL requests
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EpStall(
	AT91PS_UDP pUDP,     // \arg pointer to a UDP controller
	unsigned char endpoint)   // \arg endpoint number
{
	pUDP->UDP_CSR[endpoint] |= AT91C_UDP_FORCESTALL;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpWrite
//* \brief Write value in the DPR
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EpWrite(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint,  // \arg endpoint number
	unsigned char value)     // \arg value to be written in the DPR
{
	pUDP->UDP_FDR[endpoint] = value;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpRead
//* \brief Return value from the DPR
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_UDP_EpRead(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint)  // \arg endpoint number
{
	return pUDP->UDP_FDR[endpoint];
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpEndOfWr
//* \brief Notify the UDP that values in DPR are ready to be sent
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EpEndOfWr(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint)  // \arg endpoint number
{
	pUDP->UDP_CSR[endpoint] |= AT91C_UDP_TXPKTRDY;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpClear
//* \brief Clear flag in the endpoint CSR register
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EpClear(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint,  // \arg endpoint number
	unsigned int flag)       // \arg flag to be cleared
{
	pUDP->UDP_CSR[endpoint] &= ~(flag);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpSet
//* \brief Set flag in the endpoint CSR register
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_EpSet(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint,  // \arg endpoint number
	unsigned int flag)       // \arg flag to be cleared
{
	pUDP->UDP_CSR[endpoint] |= flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_EpStatus
//* \brief Return the endpoint CSR register
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_UDP_EpStatus(
	AT91PS_UDP pUDP,         // \arg pointer to a UDP controller
	unsigned char endpoint)  // \arg endpoint number
{
	return pUDP->UDP_CSR[endpoint];
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_GetInterruptMaskStatus
//* \brief Return UDP Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_UDP_GetInterruptMaskStatus( // \return UDP Interrupt Mask Status
        AT91PS_UDP pUdp) // \arg  pointer to a UDP controller
{
        return pUdp->UDP_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_IsInterruptMasked
//* \brief Test if UDP Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_UDP_IsInterruptMasked(
        AT91PS_UDP pUdp,   // \arg  pointer to a UDP controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_UDP_GetInterruptMaskStatus(pUdp) & flag);
}

/* *****************************************************************************
                SOFTWARE API FOR ST
   ***************************************************************************** */
//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_SetPeriodInterval
//* \brief Set Periodic Interval Interrupt (period in ms)
//*----------------------------------------------------------------------------
static inline void AT91F_ST_SetPeriodInterval(
	AT91PS_ST pSt,
	unsigned int period)
{
	volatile int status;
	pSt->ST_IDR = AT91C_ST_PITS;			/* Interrupt disable Register */

	status = pSt->ST_SR;
    pSt->ST_PIMR = period << 5;   			/* Period Interval Mode Register == timer interval = 1ms*/
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_EnableIt
//* \brief Enable system timer interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_ST_EnableIt(
	AT91PS_ST pSt,
	unsigned int flag)
{
	pSt->ST_IER = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_DisableIt
//* \brief Disable system timer interrupt
//*----------------------------------------------------------------------------
static inline void AT91F_ST_DisableIt(
	AT91PS_ST pSt,
	unsigned int flag)
{
	pSt->ST_IDR = flag;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_GetInterruptMaskStatus
//* \brief Return ST Interrupt Mask Status
//*----------------------------------------------------------------------------
static inline unsigned int AT91F_ST_GetInterruptMaskStatus( // \return ST Interrupt Mask Status
        AT91PS_ST pSt) // \arg  pointer to a ST controller
{
        return pSt->ST_IMR;
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_IsInterruptMasked
//* \brief Test if ST Interrupt is Masked 
//*----------------------------------------------------------------------------
static inline int AT91F_ST_IsInterruptMasked(
        AT91PS_ST pSt,   // \arg  pointer to a ST controller
        unsigned int flag) // \arg  flag to be tested
{
        return (AT91F_ST_GetInterruptMaskStatus(pSt) & flag);
}
//*----------------------------------------------------------------------------
//* \fn    AT91F_EBI_CfgPIO
//* \brief Configure PIO controllers to drive EBI signals
//*----------------------------------------------------------------------------
static inline void AT91F_EBI_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOC, // PIO controller base address
		((unsigned int) AT91C_PC8_A24     ) |
		((unsigned int) AT91C_PC7_A23     ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_CfgPMC
//* \brief Enable Peripheral clock in PMC for  DBGU
//*----------------------------------------------------------------------------
static inline void AT91F_DBGU_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SYS));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_DBGU_CfgPIO
//* \brief Configure PIO controllers to drive DBGU signals
//*----------------------------------------------------------------------------
static inline void AT91F_DBGU_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA31_DTXD    ) |
		((unsigned int) AT91C_PA30_DRXD    ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SYS_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SYS
//*----------------------------------------------------------------------------
static inline void AT91F_SYS_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SYS));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UHP_CfgPMC
//* \brief Enable Peripheral clock in PMC for  UHP
//*----------------------------------------------------------------------------
static inline void AT91F_UHP_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_UHP));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SDRC_CfgPIO
//* \brief Configure PIO controllers to drive SDRC signals
//*----------------------------------------------------------------------------
static inline void AT91F_SDRC_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOC, // PIO controller base address
		((unsigned int) AT91C_PC20_D20     ) |
		((unsigned int) AT91C_PC21_D21     ) |
		((unsigned int) AT91C_PC30_D30     ) |
		((unsigned int) AT91C_PC22_D22     ) |
		((unsigned int) AT91C_PC31_D31     ) |
		((unsigned int) AT91C_PC23_D23     ) |
		((unsigned int) AT91C_PC16_D16     ) |
		((unsigned int) AT91C_PC24_D24     ) |
		((unsigned int) AT91C_PC17_D17     ) |
		((unsigned int) AT91C_PC25_D25     ) |
		((unsigned int) AT91C_PC18_D18     ) |
		((unsigned int) AT91C_PC26_D26     ) |
		((unsigned int) AT91C_PC19_D19     ) |
		((unsigned int) AT91C_PC27_D27     ) |
		((unsigned int) AT91C_PC28_D28     ) |
		((unsigned int) AT91C_PC29_D29     ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_EMAC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  EMAC
//*----------------------------------------------------------------------------
static inline void AT91F_EMAC_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_EMAC));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_EMAC_CfgPIO
//* \brief Configure PIO controllers to drive EMAC signals
//*----------------------------------------------------------------------------
static inline void AT91F_EMAC_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA14_ERXER   ) |
		((unsigned int) AT91C_PA12_ERX0    ) |
		((unsigned int) AT91C_PA13_ERX1    ) |
		((unsigned int) AT91C_PA8_ETXEN   ) |
		((unsigned int) AT91C_PA16_EMDIO   ) |
		((unsigned int) AT91C_PA9_ETX0    ) |
		((unsigned int) AT91C_PA10_ETX1    ) |
		((unsigned int) AT91C_PA11_ECRS_ECRSDV) |
		((unsigned int) AT91C_PA15_EMDC    ) |
		((unsigned int) AT91C_PA7_ETXCK_EREFCK), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_RTC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  RTC
//*----------------------------------------------------------------------------
static inline void AT91F_RTC_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SYS));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC2_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SSC2
//*----------------------------------------------------------------------------
static inline void AT91F_SSC2_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SSC2));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC2_CfgPIO
//* \brief Configure PIO controllers to drive SSC2 signals
//*----------------------------------------------------------------------------
static inline void AT91F_SSC2_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		((unsigned int) AT91C_PB12_TF2     ) |
		((unsigned int) AT91C_PB17_RF2     ) |
		((unsigned int) AT91C_PB13_TK2     ) |
		((unsigned int) AT91C_PB16_RK2     ) |
		((unsigned int) AT91C_PB14_TD2     ) |
		((unsigned int) AT91C_PB15_RD2     ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC1_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SSC1
//*----------------------------------------------------------------------------
static inline void AT91F_SSC1_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SSC1));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC1_CfgPIO
//* \brief Configure PIO controllers to drive SSC1 signals
//*----------------------------------------------------------------------------
static inline void AT91F_SSC1_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		((unsigned int) AT91C_PB11_RF1     ) |
		((unsigned int) AT91C_PB10_RK1     ) |
		((unsigned int) AT91C_PB8_TD1     ) |
		((unsigned int) AT91C_PB9_RD1     ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SSC0_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SSC0
//*----------------------------------------------------------------------------
static inline void AT91F_SSC0_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SSC0));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgPMC
//* \brief Enable Peripheral clock in PMC for  SPI
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SPI));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SPI_CfgPIO
//* \brief Configure PIO controllers to drive SPI signals
//*----------------------------------------------------------------------------
static inline void AT91F_SPI_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA3_NPCS0   ) |
		((unsigned int) AT91C_PA4_NPCS1   ) |
		((unsigned int) AT91C_PA1_MOSI    ) |
		((unsigned int) AT91C_PA5_NPCS2   ) |
		((unsigned int) AT91C_PA6_NPCS3   ) |
		((unsigned int) AT91C_PA0_MISO    ) |
		((unsigned int) AT91C_PA2_SPCK    ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC5_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC5
//*----------------------------------------------------------------------------
static inline void AT91F_TC5_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TC5));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC4_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC4
//*----------------------------------------------------------------------------
static inline void AT91F_TC4_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TC4));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC3_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC3
//*----------------------------------------------------------------------------
static inline void AT91F_TC3_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TC3));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC2_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC2
//*----------------------------------------------------------------------------
static inline void AT91F_TC2_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TC2));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC1_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC1
//*----------------------------------------------------------------------------
static inline void AT91F_TC1_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TC1));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TC0_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TC0
//*----------------------------------------------------------------------------
static inline void AT91F_TC0_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TC0));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_SMC2_CfgPIO
//* \brief Configure PIO controllers to drive SMC2 signals
//*----------------------------------------------------------------------------
static inline void AT91F_SMC2_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOC, // PIO controller base address
		((unsigned int) AT91C_PC10_NCS4_CFCS) |
		((unsigned int) AT91C_PC9_A25_CFRNW) |
		((unsigned int) AT91C_PC12_NCS6_CFCE2) |
		((unsigned int) AT91C_PC11_NCS5_CFCE1), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PMC
//*----------------------------------------------------------------------------
static inline void AT91F_PMC_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SYS));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PMC_CfgPIO
//* \brief Configure PIO controllers to drive PMC signals
//*----------------------------------------------------------------------------
static inline void AT91F_PMC_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		0, // Peripheral A
		((unsigned int) AT91C_PA24_PCK1    )); // Peripheral B
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		((unsigned int) AT91C_PB27_PCK0    ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIOD_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PIOD
//*----------------------------------------------------------------------------
static inline void AT91F_PIOD_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_PIOD));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIOC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PIOC
//*----------------------------------------------------------------------------
static inline void AT91F_PIOC_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_PIOC));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIOB_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PIOB
//*----------------------------------------------------------------------------
static inline void AT91F_PIOB_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_PIOB));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_PIOA_CfgPMC
//* \brief Enable Peripheral clock in PMC for  PIOA
//*----------------------------------------------------------------------------
static inline void AT91F_PIOA_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_PIOA));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_CfgPMC
//* \brief Enable Peripheral clock in PMC for  TWI
//*----------------------------------------------------------------------------
static inline void AT91F_TWI_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_TWI));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_TWI_CfgPIO
//* \brief Configure PIO controllers to drive TWI signals
//*----------------------------------------------------------------------------
static inline void AT91F_TWI_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA25_TWD     ) |
		((unsigned int) AT91C_PA26_TWCK    ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US3_CfgPMC
//* \brief Enable Peripheral clock in PMC for  US3
//*----------------------------------------------------------------------------
static inline void AT91F_US3_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_US3));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US2_CfgPMC
//* \brief Enable Peripheral clock in PMC for  US2
//*----------------------------------------------------------------------------
static inline void AT91F_US2_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_US2));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US2_CfgPIO
//* \brief Configure PIO controllers to drive US2 signals
//*----------------------------------------------------------------------------
static inline void AT91F_US2_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA23_TXD2    ) |
		((unsigned int) AT91C_PA22_RXD2    ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US1_CfgPMC
//* \brief Enable Peripheral clock in PMC for  US1
//*----------------------------------------------------------------------------
static inline void AT91F_US1_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_US1));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US1_CfgPIO
//* \brief Configure PIO controllers to drive US1 signals
//*----------------------------------------------------------------------------
static inline void AT91F_US1_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		((unsigned int) AT91C_PB21_RXD1    ) |
		((unsigned int) AT91C_PB26_RTS1    ) |
		((unsigned int) AT91C_PB25_DSR1    ) |
		((unsigned int) AT91C_PB24_CTS1    ) |
		((unsigned int) AT91C_PB19_DTR1    ) |
		((unsigned int) AT91C_PB23_DCD1    ) |
		((unsigned int) AT91C_PB20_TXD1    ) |
		((unsigned int) AT91C_PB18_RI1     ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US0_CfgPMC
//* \brief Enable Peripheral clock in PMC for  US0
//*----------------------------------------------------------------------------
static inline void AT91F_US0_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_US0));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_US0_CfgPIO
//* \brief Configure PIO controllers to drive US0 signals
//*----------------------------------------------------------------------------
static inline void AT91F_US0_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA17_TXD0    ) |
		((unsigned int) AT91C_PA21_RTS0    ) |
		((unsigned int) AT91C_PA19_SCK0    ) |
		((unsigned int) AT91C_PA20_CTS0    ), // Peripheral A
		0); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_CfgPMC
//* \brief Enable Peripheral clock in PMC for  MCI
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_MCI));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_MCI_CfgPIO
//* \brief Configure PIO controllers to drive MCI signals
//*----------------------------------------------------------------------------
static inline void AT91F_MCI_CfgPIO (void)
{
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA28_MCCDA   ) |
		((unsigned int) AT91C_PA29_MCDA0   ) |
		((unsigned int) AT91C_PA27_MCCK    ), // Peripheral A
		0); // Peripheral B
	// Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOB, // PIO controller base address
		0, // Peripheral A
		((unsigned int) AT91C_PB5_MCDA3   ) |
		((unsigned int) AT91C_PB3_MCDA1   ) |
		((unsigned int) AT91C_PB4_MCDA2   )); // Peripheral B
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_AIC_CfgPMC
//* \brief Enable Peripheral clock in PMC for  AIC
//*----------------------------------------------------------------------------
static inline void AT91F_AIC_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_IRQ4) |
		((unsigned int) 1 << AT91C_ID_FIQ) |
		((unsigned int) 1 << AT91C_ID_IRQ5) |
		((unsigned int) 1 << AT91C_ID_IRQ6) |
		((unsigned int) 1 << AT91C_ID_IRQ0) |
		((unsigned int) 1 << AT91C_ID_IRQ1) |
		((unsigned int) 1 << AT91C_ID_IRQ2) |
		((unsigned int) 1 << AT91C_ID_IRQ3));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UDP_CfgPMC
//* \brief Enable Peripheral clock in PMC for  UDP
//*----------------------------------------------------------------------------
static inline void AT91F_UDP_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_UDP));
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_ST_CfgPMC
//* \brief Enable Peripheral clock in PMC for  ST
//*----------------------------------------------------------------------------
static inline void AT91F_ST_CfgPMC (void)
{
	AT91F_PMC_EnablePeriphClock(
		AT91C_BASE_PMC, // PIO controller base address
		((unsigned int) 1 << AT91C_ID_SYS));
}

#endif // lib_AT91RM9200_H
