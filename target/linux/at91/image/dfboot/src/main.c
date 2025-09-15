/*----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support  -  ROUSSET  -
 *----------------------------------------------------------------------------
 * The software is delivered "AS IS" without warranty or condition of any
 * kind, either express, implied or statutory. This includes without
 * limitation any warranty or condition with respect to merchantability or
 * fitness for any particular purpose, or against the infringements of
 * intellectual property rights of others.
 *----------------------------------------------------------------------------
 * File Name		: main.c
 * Object		: 
 * Creation		: HIi	10/10/2003
 * Modif		: HIi	15/06/2004 :	add crc32 to verify the download
 *                                          	from dataflash
 *			: HIi	21/09/2004 :	Set first PLLA to 180Mhz and MCK to
 *						60Mhz to speed up dataflash boot (15Mhz)
 *			: MLC	12/04/2005 :	Modify SetPLL() to avoid errata
 *			: USA	30/12/2005 :	Change to page Size 1056
 *						Change startaddress to C0008400
 *						Change SPI Speed to ~4 Mhz
 *						Add retry on CRC Error
 *----------------------------------------------------------------------------
 */
#include "config.h"
#include "stdio.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "com.h"
#include "main.h"
#include "dataflash.h"
#include "AT91C_MCI_Device.h"

#define	DEBUGOUT
#define XMODEM
#define MEMDISP

#ifdef	PAGESZ_1056
#define	PAGESIZE	1056
#else
#define	PAGESIZE	1024
#endif

#define AT91C_SDRAM_START 0x20000000
#define AT91C_BOOT_ADDR 0x21F00000
#define AT91C_BOOT_SIZE 128*PAGESIZE
#ifdef	PAGESZ_1056
#define AT91C_BOOT_DATAFLASH_ADDR 0xC0008400
#else
#define AT91C_BOOT_DATAFLASH_ADDR 0xC0008000
#endif
#define AT91C_PLLA_VALUE 0x237A3E5A  // crystal= 18.432MHz - fixes BRG error at 115kbps
//#define AT91C_PLLA_VALUE 0x2026BE04	// crystal= 18.432MHz
//#define AT91C_PLLA_VALUE 0x202CBE01	// crystal= 4MHz



#define DISP_LINE_LEN 16

// Reason for boot failure
#define	IMAGE_BAD_SIZE			0
#define	IMAGE_READ_FAILURE	1
#define	IMAGE_CRC_ERROR		2
#define	IMAGE_ERROR			3
#define	SUCCESS				-1

/* prototypes*/
extern void AT91F_ST_ASM_HANDLER(void);
extern void Jump(unsigned int addr);

const char *menu_dataflash[] = {
#ifdef XMODEM
	"1: P DFboot\n",
	"2: P U-Boot\n",
#endif
	"3: P SDCard\n",
#ifdef	PAGESZ_1056
	"4: R UBOOT\n",
#else
	"4: R UBOOT\n",
#endif
#ifdef XMODEM
	"5: P DF [addr]\n",
#endif
	"6: RD DF [addr]\n",
	"7: E DF\n"
};
#ifdef XMODEM
#define	MAXMENU 7
#else
#define MAXMENU 4
#endif

char message[20];
#ifdef XMODEM
volatile char XmodemComplete = 0;
#endif
unsigned int StTick = 0;

AT91S_RomBoot const *pAT91;
#ifdef XMODEM
AT91S_SBuffer sXmBuffer;
AT91S_SvcXmodem svcXmodem;
AT91S_Pipe xmodemPipe;
#endif
AT91S_CtlTempo ctlTempo;


//*--------------------------------------------------------------------------------------
//* Function Name       : GetTickCount()
//* Object              : Return the number of systimer tick 
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
unsigned int GetTickCount(void)
{
	return StTick;
}

#ifdef XMODEM
//*--------------------------------------------------------------------------------------
//* Function Name       : AT91_XmodemComplete()
//* Object              : Perform the remap and jump to appli in RAM
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
static void AT91_XmodemComplete(AT91S_PipeStatus status, void *pVoid)
{
	/* stop the Xmodem tempo */
	svcXmodem.tempo.Stop(&(svcXmodem.tempo));
	XmodemComplete = 1;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_XmodemProtocol(AT91S_PipeStatus status, void *pVoid)
//* Object              : Xmodem dispatcher
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
static void XmodemProtocol(AT91S_PipeStatus status, void *pVoid)
{
	AT91PS_SBuffer pSBuffer = (AT91PS_SBuffer) xmodemPipe.pBuffer->pChild;
	AT91PS_USART   pUsart     = svcXmodem.pUsart;
			
	if (pSBuffer->szRdBuffer == 0) {
		/* Start a tempo to wait the Xmodem protocol complete */
		svcXmodem.tempo.Start(&(svcXmodem.tempo), 10, 0, AT91_XmodemComplete, pUsart);								
	}
}
#endif

//*--------------------------------------------------------------------------------------
//* Function Name       : irq1_c_handler()
//* Object              : C Interrupt handler for Interrutp source 1
//* Input Parameters    : none
//* Output Parameters   : none
//*--------------------------------------------------------------------------------------
void AT91F_ST_HANDLER(void)
{
	volatile unsigned int csr = *AT91C_DBGU_CSR;
#ifdef XMODEM
	unsigned int error;
#endif
	
	if (AT91C_BASE_ST->ST_SR & 0x01) {
		StTick++;
		ctlTempo.CtlTempoTick(&ctlTempo);
		return;
	}

#ifdef XMODEM
	error = AT91F_US_Error((AT91PS_USART)AT91C_BASE_DBGU);
	if (csr & error) {
		/* Stop previous Xmodem transmition*/
		*(AT91C_DBGU_CR) = AT91C_US_RSTSTA;
		AT91F_US_DisableIt((AT91PS_USART)AT91C_BASE_DBGU, AT91C_US_ENDRX);
		AT91F_US_EnableIt((AT91PS_USART)AT91C_BASE_DBGU, AT91C_US_RXRDY);

	}
	
	else if (csr & (AT91C_US_TXRDY | AT91C_US_ENDTX | AT91C_US_TXEMPTY | 
	                AT91C_US_RXRDY | AT91C_US_ENDRX | AT91C_US_TIMEOUT | 
	                AT91C_US_RXBUFF)) {
		if ( !(svcXmodem.eot) )
			svcXmodem.Handler(&svcXmodem, csr);
	}
#endif
}


//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_DisplayMenu()
//* Object              : 
//* Input Parameters    : 
//* Return value		: 
//*-----------------------------------------------------------------------------
static int AT91F_DisplayMenu(void)
{
	int i, mci_present = 0;
	printf("\nDF LOADER %s %s %s\n",AT91C_VERSION,__DATE__,__TIME__);
	AT91F_DataflashPrintInfo();
	mci_present = AT91F_MCI_Init();
	for(i = 0; i < MAXMENU; i++) {
		puts(menu_dataflash[i]);
	}
	return mci_present;
}	


//*-----------------------------------------------------------------------------
//* Function Name       : AsciiToHex()
//* Object              : ascii to hexa conversion
//* Input Parameters    : 
//* Return value		: 
//*-----------------------------------------------------------------------------
static unsigned int AsciiToHex(char *s, unsigned int *val)
{
	int n;

	*val=0;
	
	if(s[0] == '0' && ((s[1] == 'x') || (s[1] == 'X')))
		s+=2;
	n = 0;	
	while((n < 8) && (s[n] !=0))
	{
		*val <<= 4;
		if ( (s[n] >= '0') && (s[n] <='9'))
			*val += (s[n] - '0');
		else	
			if ((s[n] >= 'a') && (s[n] <='f'))
				*val += (s[n] - 0x57);
			else
				if ((s[n] >= 'A') && (s[n] <='F'))
					*val += (s[n] - 0x37);
			else
				return 0;
		n++;
	}

	return 1;				
}


#ifdef MEMDISP
//*-----------------------------------------------------------------------------
//* Function Name       : AT91F_MemoryDisplay()
//* Object              : Display the content of the dataflash
//* Input Parameters    : 
//* Return value		: 
//*-----------------------------------------------------------------------------
static int AT91F_MemoryDisplay(unsigned int addr, unsigned int length)
{
	unsigned long	i, nbytes, linebytes;
	char	*cp;
//	unsigned int 	*uip;
//	unsigned short 	*usp;
	unsigned char 	*ucp;
	char linebuf[DISP_LINE_LEN];

//	nbytes = length * size;
	nbytes = length;
   	do
   	{
//   		uip = (unsigned int *)linebuf;
//   		usp = (unsigned short *)linebuf;
		ucp = (unsigned char *)linebuf;
		
		printf("%08x:", addr);
		linebytes = (nbytes > DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;
                if((addr & 0xF0000000) == 0x20000000) {
			for(i = 0; i < linebytes; i ++) {
				linebuf[i] =  *(char *)(addr+i);
			}
		} else {
			read_dataflash(addr, linebytes, linebuf);
		}
		for (i=0; i<linebytes; i++)
		{
/*			if (size == 4) 
				printf(" %08x", *uip++);
			else if (size == 2)
				printf(" %04x", *usp++);
			else
*/
				printf(" %02x", *ucp++);
//			addr += size;
			addr++;
		}
		printf("    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				printf(".");
			else
				printf("%c", *cp);
			cp++;
		}
		printf("\n");
		nbytes -= linebytes;
	} while (nbytes > 0);
	return 0;
}
#endif

//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_SetPLL
//* Object              : Set the PLLA to 180Mhz and Master clock to 60 Mhz
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
static unsigned int AT91F_SetPLL(void)
{
	AT91_REG tmp;
	AT91PS_PMC pPmc = AT91C_BASE_PMC;
	AT91PS_CKGR pCkgr = AT91C_BASE_CKGR;

	pPmc->PMC_IDR = 0xFFFFFFFF;

	/* -Setup the PLL A */
	pCkgr->CKGR_PLLAR = AT91C_PLLA_VALUE;

	while (!(*AT91C_PMC_SR & AT91C_PMC_LOCKA));
	
	/* - Switch Master Clock from PLLB to PLLA/3 */
	tmp = pPmc->PMC_MCKR;
	/* See Atmel Errata #27 and #28 */
	if (tmp & 0x0000001C) {
		tmp = (tmp & ~0x0000001C);
		pPmc->PMC_MCKR = tmp;
		while (!(*AT91C_PMC_SR & AT91C_PMC_MCKRDY));
	}
	if (tmp != 0x00000202) {
		pPmc->PMC_MCKR = 0x00000202;
		if ((tmp & 0x00000003) != 0x00000002)
			while (!(*AT91C_PMC_SR & AT91C_PMC_MCKRDY));
	}

	return 1;	
}


//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_ResetRegisters
//* Object              : Restore the initial state to registers
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
static unsigned int AT91F_ResetRegisters(void)
{
	volatile int i = 0;

	/* set the PIOs in input*/
	/* This disables the UART output, so dont execute for now*/

#ifndef	DEBUGOUT
	*AT91C_PIOA_ODR = 0xFFFFFFFF;	/* Disables all the output pins */
	*AT91C_PIOA_PER = 0xFFFFFFFF;	/* Enables the PIO to control all the pins */
#endif

	AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
	/* close all peripheral clocks */

#ifndef	DEBUGOUT
	AT91C_BASE_PMC->PMC_PCDR = 0xFFFFFFFC;
#endif
	/* Disable core interrupts and set supervisor mode */
	__asm__ ("msr CPSR_c, #0xDF"); //* ARM_MODE_SYS(0x1F) | I_BIT(0x80) | F_BIT(0x40)
	/* Clear all the interrupts */
	*AT91C_AIC_ICCR = 0xffffffff;

	/* read the AIC_IVR and AIC_FVR */
	i = *AT91C_AIC_IVR;
	i = *AT91C_AIC_FVR;

	/* write the end of interrupt control register */
	*AT91C_AIC_EOICR	= 0;

	return 1;
}


static int AT91F_LoadBoot(void)
{
//	volatile unsigned int crc1 = 0, crc2 = 0;
	volatile unsigned int SizeToDownload = 0x21400;
	volatile unsigned int AddressToDownload = AT91C_BOOT_ADDR;

#if 0
	/* Read vector 6 to extract size to load */	
	if (read_dataflash(AT91C_BOOT_DATAFLASH_ADDR, 32,
	                   (char *)AddressToDownload) != AT91C_DATAFLASH_OK)
	{
		printf("Bad Code Size\n");
		return IMAGE_BAD_SIZE;
	}
	/* calculate the size to download */
	SizeToDownload = *(int *)(AddressToDownload + AT91C_OFFSET_VECT6);
#endif
	
//	printf("\nLoad UBOOT from dataflash[%x] to SDRAM[%x]\n",
//	       AT91C_BOOT_DATAFLASH_ADDR, AT91C_BOOT_ADDR);
	if (read_dataflash(AT91C_BOOT_DATAFLASH_ADDR, SizeToDownload + 8,
	                   (char *)AddressToDownload) != AT91C_DATAFLASH_OK)
	{
		printf("F DF RD\n");
		return IMAGE_READ_FAILURE;
	}
#if 0
	pAT91->CRC32((const unsigned char *)AT91C_BOOT_ADDR,
	             (unsigned int)SizeToDownload , (unsigned int *)&crc2);
	crc1 = (int)(*(char *)(AddressToDownload + SizeToDownload)) +
	       (int)(*(char *)(AddressToDownload + SizeToDownload + 1) << 8) +
	       (int)(*(char *)(AddressToDownload + SizeToDownload + 2) << 16) +
	       (int)(*(char *)(AddressToDownload + SizeToDownload + 3) << 24);

	/* Restore the value of Vector 6 */
	*(int *)(AddressToDownload + AT91C_OFFSET_VECT6) =
		*(int *)(AddressToDownload + SizeToDownload + 4);
	
	if (crc1 != crc2) {
		printf("DF CRC bad %x != %x\n",crc1,crc2);
	 	return	IMAGE_CRC_ERROR;
	}
#endif
	return SUCCESS;
}

static int AT91F_StartBoot(void)
{
	int	sts;
	if((sts = AT91F_LoadBoot()) != SUCCESS) return sts;
//	printf("\n");
//	printf("PLLA[180MHz], MCK[60Mhz] ==> Start UBOOT\n");
	if (AT91F_ResetRegisters())
	{
		printf("Jump");
		Jump(AT91C_BOOT_ADDR);
//		LED_blink(0);
	}
	return	IMAGE_ERROR;
}

#if 0
static void	AT91F_RepeatedStartBoot(void)
{
	int	i;
	for(i = 0; i < CRC_RETRIES; i++) {
		if(AT91F_StartBoot() != IMAGE_CRC_ERROR){
//			LED_blink(1);
			return;
		}
	}
	return;
}
#endif

#define TRUE 1
#define FALSE 0
#define TRX_MAGIC 0x30524448  /* "HDR0" */
#define TRX_VERSION 1

struct trx_header {
	unsigned int magic;
	unsigned int len;
	unsigned int crc32;
	unsigned int flag_version;
	unsigned int offsets[3];
};

#define AT91C_MCI_TIMEOUT 1000000

extern AT91S_MciDevice MCI_Device;
extern void AT91F_MCIDeviceWaitReady(unsigned int);
extern int AT91F_MCI_ReadBlockSwab(AT91PS_MciDevice, int, unsigned int *, int);

int Program_From_MCI(void)
{
  int i;
  unsigned int Max_Read_DataBlock_Length;
  int block = 0;
  int buffer = AT91C_DOWNLOAD_BASE_ADDRESS;
  int bufpos = AT91C_DOWNLOAD_BASE_ADDRESS;
  int NbPage = 0;
  struct trx_header *p;

	p = (struct trx_header *)bufpos;

	Max_Read_DataBlock_Length = MCI_Device.pMCI_DeviceFeatures->Max_Read_DataBlock_Length;

	AT91F_MCIDeviceWaitReady(AT91C_MCI_TIMEOUT);

  AT91F_MCI_ReadBlockSwab(&MCI_Device, block*Max_Read_DataBlock_Length, (unsigned int *)bufpos, Max_Read_DataBlock_Length);

  if (p->magic != TRX_MAGIC) {
		printf("Inv IMG 0x%08x\n", p->magic);
		return FALSE;
		}

	printf("RDSD");
	AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC7 | AT91C_PIO_PC15 | AT91C_PIO_PC8 | AT91C_PIO_PC14;
	for (i=0; i<(p->len/512); i++) {
		AT91F_MCI_ReadBlockSwab(&MCI_Device, block*Max_Read_DataBlock_Length, (unsigned int *)bufpos, Max_Read_DataBlock_Length);
		block++;
		bufpos += Max_Read_DataBlock_Length;
		}

	NbPage = 0;
	i = dataflash_info[0].Device.pages_number;
	while(i >>= 1)
		NbPage++;
	i = ((p->offsets[1] - p->offsets[0])/ 512) + 1 + (NbPage << 13) + (dataflash_info[0].Device.pages_size << 17);
	*(int *)(buffer + p->offsets[0] + AT91C_OFFSET_VECT6) = i;

	printf(" WDFB");
	AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC7 | AT91C_PIO_PC15 | AT91C_PIO_PC14;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC8;
	write_dataflash(0xc0000000, buffer + p->offsets[0], p->offsets[1] - p->offsets[0]);
	printf(" WUB");
	AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC7 | AT91C_PIO_PC15;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC8 | AT91C_PIO_PC14;
	write_dataflash(0xc0008000, buffer + p->offsets[1], p->offsets[2] - p->offsets[1]);
	printf(" WKRFS");
	AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC8 | AT91C_PIO_PC15;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC7 | AT91C_PIO_PC14;
	write_dataflash(0xc0042000, buffer + p->offsets[2], p->len - p->offsets[2]);
	AT91C_BASE_PIOC->PIO_CODR = AT91C_PIO_PC8 | AT91C_PIO_PC14;
	AT91C_BASE_PIOC->PIO_SODR = AT91C_PIO_PC7 | AT91C_PIO_PC15;
	return TRUE;
}

//*----------------------------------------------------------------------------
//* Function Name       : main
//* Object              : Main function
//* Input Parameters    : none
//* Output Parameters   : True
//*----------------------------------------------------------------------------
int main(void)
{
#ifdef XMODEM
	AT91PS_Buffer  		pXmBuffer;
	AT91PS_SvcComm 		pSvcXmodem;
#endif
	AT91S_SvcTempo 		svcBootTempo; 	 // Link to a AT91S_Tempo object
	unsigned int		ix;
	volatile unsigned int AddressToDownload, SizeToDownload;	
 	unsigned int DeviceAddress = 0;
	char command = 0;
#ifdef XMODEM
	volatile int i = 0;	
	unsigned int crc1 = 0, crc2 = 0;
	volatile int device;
	int NbPage;
#endif
	volatile int Nb_Device = 0;
	int mci_present = 0;

	pAT91 = AT91C_ROM_BOOT_ADDRESS;

	if (!AT91F_SetPLL())
	{
		printf("F SetPLL");
		while(1);
	}

	at91_init_uarts();

	/* Tempo Initialisation */
	pAT91->OpenCtlTempo(&ctlTempo, (void *) &(pAT91->SYSTIMER_DESC));
	ctlTempo.CtlTempoStart((void *) &(pAT91->SYSTIMER_DESC));
	
	// Attach the tempo to a tempo controler
	ctlTempo.CtlTempoCreate(&ctlTempo, &svcBootTempo);
//	LED_init();
//	LED_blink(2);

#ifdef XMODEM
	/* Xmodem Initialisation */
	pXmBuffer = pAT91->OpenSBuffer(&sXmBuffer);
	pSvcXmodem = pAT91->OpenSvcXmodem(&svcXmodem,
	             (AT91PS_USART)AT91C_BASE_DBGU, &ctlTempo);
	pAT91->OpenPipe(&xmodemPipe, pSvcXmodem, pXmBuffer);
#endif

	/* System Timer initialization */
	AT91F_AIC_ConfigureIt(
		AT91C_BASE_AIC,                        // AIC base address
		AT91C_ID_SYS,                          // System peripheral ID
		AT91C_AIC_PRIOR_HIGHEST,               // Max priority
		AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, // Level sensitive
		AT91F_ST_ASM_HANDLER
	);
	/* Enable ST interrupt */
	AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_SYS);

#ifndef PRODTEST
	/* Start tempo to start Boot in a delay of
	 * AT91C_DELAY_TO_BOOT sec if no key pressed */
	svcBootTempo.Start(&svcBootTempo, AT91C_DELAY_TO_BOOT,
	                   0, AT91F_StartBoot, NULL);
#endif

	while(1)
	{
		while(command == 0)
		{
			AddressToDownload = AT91C_DOWNLOAD_BASE_ADDRESS;
			SizeToDownload = AT91C_DOWNLOAD_MAX_SIZE;
			DeviceAddress = 0;
			
			/* try to detect Dataflash */
			if (!Nb_Device)
				Nb_Device = AT91F_DataflashInit();				
							
			mci_present = AT91F_DisplayMenu();

#ifdef PRODTEST
			if (mci_present) {
				if (Program_From_MCI())
					AT91F_StartBoot();
			}
#endif

			message[0] = 0;
			AT91F_ReadLine ("Enter: ", message);

#ifndef PRODTEST
			/* stop tempo ==> stop autoboot */
			svcBootTempo.Stop(&svcBootTempo);
#endif

			command = message[0];
			for(ix = 1; (message[ix] == ' ') && (ix < 12); ix++);	// Skip some whitespace
				
			if(!AsciiToHex(&message[ix], &DeviceAddress) )
				DeviceAddress = 0;			// Illegal DeviceAddress
				
			switch(command)
			{
#ifdef XMODEM
				case '1':
				case '2':
				case '5':
					if(command == '1') {
						DeviceAddress = 0xC0000000;
//						printf("Download DataflashBoot.bin to [0x%x]\n", DeviceAddress);
					} else if(command == '2') {
						DeviceAddress = AT91C_BOOT_DATAFLASH_ADDR;
//						printf("Download u-boot.bin to [0x%x]\n", DeviceAddress);
					} else {
//						printf("Download Dataflash to [0x%x]\n", DeviceAddress);
					}
					switch(DeviceAddress & 0xFF000000)
					{
						case CFG_DATAFLASH_LOGIC_ADDR_CS0:
							if (dataflash_info[0].id == 0){
								printf("No DF");
								AT91F_WaitKeyPressed();
								command = 0;
							}

							device = 0;
						break;
					
						case CFG_DATAFLASH_LOGIC_ADDR_CS3:
							if (dataflash_info[1].id == 0){
								printf("No DF");
								AT91F_WaitKeyPressed();
								command = 0;
							}
							device = 1;
						break;
					
						default:
							command = 0;
						break;
					}
				break;
#endif

				case '3':
					if (mci_present)
						Program_From_MCI();
					command = 0;
					break;

				case '4':
					AT91F_StartBoot();
					command = 0;
				break;

#ifdef MEMDISP
				case '6':
					do 
					{
						AT91F_MemoryDisplay(DeviceAddress, 256);
						AT91F_ReadLine (NULL, message);
						DeviceAddress += 0x100;
					}
					while(message[0] == '\0');
					command = 0;
				break;
#endif

				case '7':
					switch(DeviceAddress & 0xFF000000)
					{
						case CFG_DATAFLASH_LOGIC_ADDR_CS0:
							break;
						case CFG_DATAFLASH_LOGIC_ADDR_CS3:
							break;
						default:
							command = 0;
							break;
					}

					if (command != 0) {
						AT91F_ReadLine ("RDY ERA\nSure?",
								message);
						if(message[0] == 'Y' || message[0] == 'y') {
							erase_dataflash(DeviceAddress & 0xFF000000);
//							printf("Erase complete\n\n");
						}
//						else
//							printf("Erase aborted\n");
					}
					command = 0;

				break;

				default:
					command = 0;
				break;
			}
		}
#ifdef XMODEM
		for(i = 0; i <= AT91C_DOWNLOAD_MAX_SIZE; i++)
			*(unsigned char *)(AddressToDownload + i) = 0;
	
		xmodemPipe.Read(&xmodemPipe, (char *)AddressToDownload,
		                SizeToDownload, XmodemProtocol, 0);	
		while(XmodemComplete !=1);
		SizeToDownload = (unsigned int)((svcXmodem.pData) -
		                 (unsigned int)AddressToDownload);

		/* Modification of vector 6 */
		if ((DeviceAddress == CFG_DATAFLASH_LOGIC_ADDR_CS0)) {
			// Vector 6 must be compliant to the BootRom description (ref Datasheet)
	   		NbPage = 0;
	    	i = dataflash_info[device].Device.pages_number;
	    	while(i >>= 1)
	    		NbPage++;
			i = (SizeToDownload / 512)+1 + (NbPage << 13) +
			    (dataflash_info[device].Device.pages_size << 17); //+4 to add crc32
		    SizeToDownload = 512 * (i &0xFF);
		}	
		else
		{
			/* Save the contents of vector 6 ==> will be restored 
			 * at boot time (AT91F_StartBoot) */
			*(int *)(AddressToDownload + SizeToDownload + 4) =
				*(int *)(AddressToDownload + AT91C_OFFSET_VECT6);
			/* Modify Vector 6 to contain the size of the
			 * file to copy (Dataflash -> SDRAM)*/
			i = SizeToDownload;	
		}		

		*(int *)(AddressToDownload + AT91C_OFFSET_VECT6) = i;
//		printf("\nModification of Arm Vector 6 :%x\n", i);
			    
//		printf("\nWrite %d bytes in DataFlash [0x%x]\n",SizeToDownload, DeviceAddress);
		crc1 = 0;
		pAT91->CRC32((const unsigned char *)AddressToDownload, SizeToDownload , &crc1);

		/* Add the crc32 at the end of the code */
		*(char *)(AddressToDownload + SizeToDownload)     = (char)(crc1 & 0x000000FF);
		*(char *)(AddressToDownload + SizeToDownload + 1) = (char)((crc1 & 0x0000FF00) >> 8);
		*(char *)(AddressToDownload + SizeToDownload + 2) = (char)((crc1 & 0x00FF0000) >> 16);
		*(char *)(AddressToDownload + SizeToDownload + 3) = (char)((crc1 & 0xFF000000) >> 24);

		/* write dataflash */
		write_dataflash (DeviceAddress, AddressToDownload, (SizeToDownload + 8));

		/* clear the buffer before read */
		for(i=0; i <= SizeToDownload; i++)
			*(unsigned char *)(AddressToDownload + i) = 0;
					
		/* Read dataflash to check the validity of the data */
		read_dataflash (DeviceAddress, (SizeToDownload + 4), (char *)(AddressToDownload));

		printf("VFY: ");	
		crc2 = 0;
				
		pAT91->CRC32((const unsigned char *)AddressToDownload, SizeToDownload , &crc2);
 		crc1 = (int)(*(char *)(AddressToDownload + SizeToDownload))          +
			   (int)(*(char *)(AddressToDownload + SizeToDownload + 1) << 8) +
			   (int)(*(char *)(AddressToDownload + SizeToDownload + 2) << 16) +
			   (int)(*(char *)(AddressToDownload + SizeToDownload + 3) << 24);

		if (crc1 != crc2)
		 	printf("ERR");
		else
		 	printf("OK");
		 	
 		command = 0;
 		XmodemComplete = 0;
		AT91F_WaitKeyPressed();
#endif
 	}
}

