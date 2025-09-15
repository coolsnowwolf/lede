//*----------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : init.c
//* Object              : Low level initialisations written in C
//* Creation            : HIi   10/10/2003
//*
//*----------------------------------------------------------------------------
#include "config.h"
#include "AT91RM9200.h"
#include "lib_AT91RM9200.h"
#include "stdio.h"

//*----------------------------------------------------------------------------
//* \fn    AT91F_DataAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
static void AT91F_SpuriousHandler() 
{
	puts("ISI");
	while (1);
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_DataAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
static void AT91F_DataAbort() 
{
	puts("IDA");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_FetchAbort
//* \brief This function reports an Abort
//*----------------------------------------------------------------------------
static void AT91F_FetchAbort()
{
	puts("IFA");
	while (1);
}

//*----------------------------------------------------------------------------
//* \fn    AT91F_UndefHandler
//* \brief This function reports that no handler have been set for current IT
//*----------------------------------------------------------------------------
static void AT91F_UndefHandler() 
{
	puts("IUD");
	while (1);
}


//*--------------------------------------------------------------------------------------
//* Function Name       : AT91F_InitSdram
//* Object              : Initialize the SDRAM
//* Input Parameters    :
//* Output Parameters   :
//*--------------------------------------------------------------------------------------
static void AT91F_InitSdram()
{
	int *pRegister;
	
	//* Configure PIOC as peripheral (D16/D31)
	
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOC, // PIO controller base address
		0xFFFF0030,
		0
	);
	
	//*Init SDRAM
	pRegister = (int *)0xFFFFFF98;
	*pRegister = 0x2188c155; 
	pRegister = (int *)0xFFFFFF90;
	*pRegister = 0x2; 
	pRegister = (int *)0x20000000;
	*pRegister = 0; 
	pRegister = (int *)0xFFFFFF90;
	*pRegister = 0x4; 
	pRegister = (int *)0x20000000;
	*pRegister = 0; 
	*pRegister = 0; 
	*pRegister = 0; 
	*pRegister = 0; 
	*pRegister = 0; 
	*pRegister = 0; 
	*pRegister = 0; 
	*pRegister = 0; 
	pRegister = (int *)0xFFFFFF90;
	*pRegister = 0x3; 
	pRegister = (int *)0x20000080;
	*pRegister = 0; 

	pRegister = (int *)0xFFFFFF94;
	*pRegister = 0x2e0; 
	pRegister = (int *)0x20000000;
	*pRegister = 0; 

	pRegister = (int *)0xFFFFFF90;
	*pRegister = 0x00; 
	pRegister = (int *)0x20000000;
	*pRegister = 0; 
}


//*----------------------------------------------------------------------------
//* \fn    AT91F_InitFlash
//* \brief This function performs low level HW initialization
//*----------------------------------------------------------------------------
static void AT91F_InitMemories()
{
	int *pEbi = (int *)0xFFFFFF60;

	//* Setup MEMC to support all connected memories (CS0 = FLASH; CS1=SDRAM)
	pEbi  = (int *)0xFFFFFF60;
	*pEbi = 0x00000002;

	//* CS0 cs for flash
	pEbi  = (int *)0xFFFFFF70;
	*pEbi = 0x00003284;
	
	AT91F_InitSdram();
}



//*----------------------------------------------------------------------------
//* \fn    AT91F_LowLevelInit
//* \brief This function performs very low level HW initialization
//*----------------------------------------------------------------------------
void AT91F_LowLevelInit(void)
{
	int i;

	// Init Interrupt Controller
	AT91F_AIC_Open(
		AT91C_BASE_AIC,          // pointer to the AIC registers
		AT91C_AIC_BRANCH_OPCODE, // IRQ exception vector
		AT91F_UndefHandler,      // FIQ exception vector
		AT91F_UndefHandler,      // AIC default handler
		AT91F_SpuriousHandler,   // AIC spurious handler
		0);                      // Protect mode

	// Perform 8 End Of Interrupt Command to make sýre AIC will not Lock out nIRQ 
	for(i=0; i<8; i++)
		AT91F_AIC_AcknowledgeIt(AT91C_BASE_AIC);

	AT91F_AIC_SetExceptionVector((unsigned int *)0x0C, AT91F_FetchAbort);
	AT91F_AIC_SetExceptionVector((unsigned int *)0x10, AT91F_DataAbort);
	AT91F_AIC_SetExceptionVector((unsigned int *)0x4, AT91F_UndefHandler);

	//Initialize SDRAM and Flash
	AT91F_InitMemories();

}

