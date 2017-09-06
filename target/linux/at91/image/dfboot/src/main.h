//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : main.h
//* Object              :
//*
//* 1.0 27/03/03 HIi    : Creation
//* 1.01 03/05/04 HIi   : AT9C_VERSION incremented to 1.01
//* 1.02 15/06/04 HIi   : AT9C_VERSION incremented to 1.02 ==> 
//*                       Add crc32 to verify dataflash download
//* 1.03 18/04/05 MLC   : AT91C_VERSION incremented to 1.03g
//*			  Repeat boot on CRC Failure
//*			  Change Page Size to 1056
//*			  Reduce SPI speed to 4 Mbit
//*			  Change U-Boot boot address to a 1056 byte page boundary
//* 1.04 30/04/05 USA	: AT91C_VERSION incremented to 1.04
//* 1.05 07/08/06 USA	: AT91C_VERSION incremented to 1.05
//*			  Will only support loading Dataflashboot.bin and U-Boot
//*----------------------------------------------------------------------------

#ifndef main_h
#define main_h

#include    "embedded_services.h"

#define AT91C_DOWNLOAD_BASE_ADDRESS     0x20000000
#define AT91C_DOWNLOAD_MAX_SIZE         0x00040000

#define AT91C_OFFSET_VECT6              0x14        //* Offset for ARM vector 6

#define AT91C_VERSION   "VER 1.05"


// Global variables and functions definition
extern unsigned int GetTickCount(void);
#endif

