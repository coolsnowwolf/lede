//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : embedded_sevices.h
//* Object              : Header File with all the embedded software services definitions
//*
//* 1.0 24 Jan 2003 FB  : Creation
//*----------------------------------------------------------------------------
#ifndef embedded_sevices_h
#define embedded_sevices_h

#include "AT91RM9200.h"

#define AT91C_BASE_ROM	(char *)0x00100000

/* Return values */
#define AT91C_BUFFER_SUCCESS		   0
#define AT91C_BUFFER_ERROR_SHIFT      16
#define AT91C_BUFFER_ERROR            (0x0F << AT91C_BUFFER_ERROR_SHIFT)

#define AT91C_BUFFER_OVERFLOW         (0x01 << AT91C_BUFFER_ERROR_SHIFT)
#define AT91C_BUFFER_UNDERRUN         (0x02 << AT91C_BUFFER_ERROR_SHIFT)

typedef unsigned int AT91S_BufferStatus;

struct _AT91S_Pipe;

// This structure is a virtual object of a buffer
typedef struct _AT91S_Buffer
{
	struct _AT91S_Pipe *pPipe;
	void *pChild;

	// Functions invoked by the pipe
	AT91S_BufferStatus (*SetRdBuffer)     (struct _AT91S_Buffer *pSBuffer, char *pBuffer, unsigned int Size);
	AT91S_BufferStatus (*SetWrBuffer)     (struct _AT91S_Buffer *pSBuffer, char const *pBuffer, unsigned int Size);
	AT91S_BufferStatus (*RstRdBuffer)     (struct _AT91S_Buffer *pSBuffer);
	AT91S_BufferStatus (*RstWrBuffer)     (struct _AT91S_Buffer *pSBuffer);
	char (*MsgWritten)      (struct _AT91S_Buffer *pSBuffer, char const *pBuffer);
	char (*MsgRead)         (struct _AT91S_Buffer *pSBuffer, char const *pBuffer);
	// Functions invoked by the peripheral
	AT91S_BufferStatus (*GetWrBuffer)     (struct _AT91S_Buffer *pSBuffer, char const **pData, unsigned int *pSize);
	AT91S_BufferStatus (*GetRdBuffer)     (struct _AT91S_Buffer *pSBuffer, char **pData, unsigned int *pSize);
	AT91S_BufferStatus (*EmptyWrBuffer)   (struct _AT91S_Buffer *pSBuffer, unsigned int size);
	AT91S_BufferStatus (*FillRdBuffer)    (struct _AT91S_Buffer *pSBuffer, unsigned int size);
	char (*IsWrEmpty)      (struct _AT91S_Buffer *pSBuffer);
	char (*IsRdFull)       (struct _AT91S_Buffer *pSBuffer);
} AT91S_Buffer, *AT91PS_Buffer;

// ===========================================================================================
// SimpleBuffer definition
//
// This structure is pointed by pRealBuffer field in the SBuffer
// It contains usefull information for a real implementation of
// a SBuffer object.
// The application just create an instance of SSBUffer and SBuffer,
// call OpenSimpleBuffer, and continue using SBuffer instance

typedef struct _AT91S_SBuffer
{
	AT91S_Buffer parent;
	char         *pRdBuffer;
	char const   *pWrBuffer;
	unsigned int szRdBuffer;
	unsigned int szWrBuffer;
	unsigned int stRdBuffer;
	unsigned int stWrBuffer;
} AT91S_SBuffer, *AT91PS_SBuffer;

typedef AT91PS_Buffer (*AT91PF_OpenSBuffer) (AT91PS_SBuffer);

// This function is called by the application
extern AT91PS_Buffer AT91F_OpenSBuffer(AT91PS_SBuffer pBuffer);

// Functions invoked by the pipe
extern AT91S_BufferStatus AT91F_SbSetRdBuffer  (AT91PS_Buffer pBuffer, char *pData, unsigned int Size);
extern AT91S_BufferStatus AT91F_SbSetWrBuffer  (AT91PS_Buffer pBuffer, char const *pData, unsigned int Size);
extern AT91S_BufferStatus AT91F_SbRstRdBuffer  (AT91PS_Buffer pBuffer);
extern AT91S_BufferStatus AT91F_SbRstWrBuffer  (AT91PS_Buffer pBuffer);
extern char AT91F_SbMsgWritten   (AT91PS_Buffer pBuffer, char const *pMsg);
extern char AT91F_SbMsgRead      (AT91PS_Buffer pBuffer, char const *pMsg);
// Functions invoked by the peripheral
extern AT91S_BufferStatus AT91F_SbGetWrBuffer  (AT91PS_Buffer pBuffer, char const **pData, unsigned int *pSize);
extern AT91S_BufferStatus AT91F_SbGetRdBuffer  (AT91PS_Buffer pBuffer, char **pData, unsigned int *pSize);
extern AT91S_BufferStatus AT91F_SbEmptyWrBuffer(AT91PS_Buffer pBuffer, unsigned int size);
extern AT91S_BufferStatus AT91F_SbFillRdBuffer (AT91PS_Buffer pBuffer, unsigned int size);
extern char AT91F_SbIsWrEmpty   (AT91PS_Buffer pBuffer);
extern char AT91F_SbIsRdFull    (AT91PS_Buffer pBuffer);

#ifdef DBG_DRV_BUFFER
extern char const *AT91F_SbGetError(AT91S_BufferStatus errorNumber);
#endif


#define AT91C_OPEN_CTRLTEMPO_SUCCESS	0
#define AT91C_ERROR_OPEN_CTRLTEMPO		1
#define AT91C_START_OK					2
#define AT91C_STOP_OK					3
#define AT91C_TIMEOUT_REACHED			4

typedef enum _AT91E_SvcTempo {
	AT91E_SVCTEMPO_DIS,
	AT91E_SVCTEMPO_EN
} AT91E_SvcTempo;

typedef unsigned int AT91S_TempoStatus;

// AT91S_SvcTempo
typedef struct _AT91S_SvcTempo
{

	// Methods:
	AT91S_TempoStatus (*Start)  (
		struct _AT91S_SvcTempo *pSvc,
		unsigned int timeout,
		unsigned int reload,
		void (*callback) (AT91S_TempoStatus, void *),
		void *pData);
	AT91S_TempoStatus (*Stop)   (struct _AT91S_SvcTempo *pSvc);

	struct _AT91S_SvcTempo *pPreviousTempo;
	struct _AT91S_SvcTempo *pNextTempo;

	// Data
	unsigned int TickTempo;	//* timeout value
	unsigned int ReloadTempo;//* Reload value for periodic execution
	void (*TempoCallback)(AT91S_TempoStatus, void *);
	void *pPrivateData;
	AT91E_SvcTempo flag;
} AT91S_SvcTempo, *AT91PS_SvcTempo;


// AT91S_CtrlTempo
typedef struct _AT91S_CtlTempo
{
	// Members:

	// Start and stop for Timer	hardware
	AT91S_TempoStatus (*CtlTempoStart)  (void *pTimer);
	AT91S_TempoStatus (*CtlTempoStop)   (void *pTimer);

	// Start and stop for Tempo service
	AT91S_TempoStatus (*SvcTempoStart)  (
		struct _AT91S_SvcTempo *pSvc,
		unsigned int timeout,
		unsigned int reload,
		void (*callback) (AT91S_TempoStatus, void *),
		void *pData);
	AT91S_TempoStatus (*SvcTempoStop)   (struct _AT91S_SvcTempo *pSvc);
	AT91S_TempoStatus (*CtlTempoSetTime)(struct _AT91S_CtlTempo *pCtrl, unsigned int NewTime);
	AT91S_TempoStatus (*CtlTempoGetTime)(struct _AT91S_CtlTempo *pCtrl);
	AT91S_TempoStatus (*CtlTempoIsStart)(struct _AT91S_CtlTempo *pCtrl);
	AT91S_TempoStatus (*CtlTempoCreate) (
								struct _AT91S_CtlTempo *pCtrl,
								struct _AT91S_SvcTempo *pTempo);
	AT91S_TempoStatus (*CtlTempoRemove) (
								struct _AT91S_CtlTempo *pCtrl,
								struct _AT91S_SvcTempo *pTempo);
	AT91S_TempoStatus (*CtlTempoTick)   (struct _AT91S_CtlTempo *pCtrl);

	// Data:

	void *pPrivateData;     // Pointer to devived class
	void const *pTimer;			// hardware
	AT91PS_SvcTempo pFirstTempo;
	AT91PS_SvcTempo pNewTempo;
} AT91S_CtlTempo, *AT91PS_CtlTempo;
typedef AT91S_TempoStatus (*AT91PF_OpenCtlTempo)   ( AT91PS_CtlTempo, void const *);

// This function is called by the application.
extern AT91S_TempoStatus AT91F_OpenCtlTempo( AT91PS_CtlTempo pCtrlTempo, void const *pTempoTimer );

extern AT91S_TempoStatus AT91F_STStart   (void *);
extern AT91S_TempoStatus AT91F_STStop    (void *);
extern AT91S_TempoStatus AT91F_STSetTime (AT91PS_CtlTempo, unsigned int);
extern AT91S_TempoStatus AT91F_STGetTime (AT91PS_CtlTempo);
extern AT91S_TempoStatus AT91F_STIsStart (AT91PS_CtlTempo);
extern AT91S_TempoStatus AT91F_CtlTempoCreate (AT91PS_CtlTempo, AT91PS_SvcTempo);
extern AT91S_TempoStatus AT91F_CtlTempoRemove (AT91PS_CtlTempo, AT91PS_SvcTempo);
extern AT91S_TempoStatus AT91F_CtlTempoTick   (AT91PS_CtlTempo);
extern AT91S_TempoStatus AT91F_SvcTempoStart (
		AT91PS_SvcTempo pSvc,
		unsigned int timeout,
		unsigned int reload,
		void (*callback) (AT91S_TempoStatus, void *),
		void *pData);
extern AT91S_TempoStatus AT91F_SvcTempoStop (AT91PS_SvcTempo);


// Following types are defined in another header files
struct _AT91S_Buffer;

// Constants:
#define AT91C_COMMSVC_SUCCESS     0
#define AT91C_COMMSVC_ERROR_SHIFT 8
#define AT91C_COMMSVC_ERROR       (0x0f << AT91C_COMMSVC_ERROR_SHIFT)

typedef unsigned int AT91S_SvcCommStatus;

// AT91S_Service definition
// This structure is an abstraction of a communication peripheral
typedef struct _AT91S_Service
{
	// Methods:
	AT91S_SvcCommStatus (*Reset)  (struct _AT91S_Service *pService);
	AT91S_SvcCommStatus (*StartTx)(struct _AT91S_Service *pService);
	AT91S_SvcCommStatus (*StartRx)(struct _AT91S_Service *pService);
	AT91S_SvcCommStatus (*StopTx) (struct _AT91S_Service *pService);
	AT91S_SvcCommStatus (*StopRx) (struct _AT91S_Service *pService);
	char                (*TxReady)(struct _AT91S_Service *pService);
	char                (*RxReady)(struct _AT91S_Service *pService);
	// Data:
	struct _AT91S_Buffer *pBuffer; // Link to a buffer object
	void *pChild;
} AT91S_SvcComm, *AT91PS_SvcComm;

// Constants:
#define AT91C_XMODEM_SOH         0x01         /* Start of Heading for 128 bytes */
#define AT91C_XMODEM_STX         0x02         /* Start of heading for 1024 bytes */
#define AT91C_XMODEM_EOT         0x04         /* End of transmission */
#define AT91C_XMODEM_ACK         0x06         /* Acknowledge */
#define AT91C_XMODEM_NAK         0x15         /* Negative Acknowledge */
#define AT91C_XMODEM_CRCCHR      'C'

#define AT91C_XMODEM_PACKET_SIZE 2                 // packet + packetCRC
#define AT91C_XMODEM_CRC_SIZE    2                 // crcLSB + crcMSB
#define AT91C_XMODEM_DATA_SIZE_SOH    128          // data 128 corresponding to SOH header
#define AT91C_XMODEM_DATA_SIZE_STX    1024         // data 1024 corresponding to STX header

//* Following structure is used by SPipe to refer to the USB device peripheral endpoint
typedef struct _AT91PS_SvcXmodem {

	// Public Methods:
	AT91S_SvcCommStatus (*Handler) (struct _AT91PS_SvcXmodem *, unsigned int);
	AT91S_SvcCommStatus (*StartTx) (struct _AT91PS_SvcXmodem *, unsigned int);
	AT91S_SvcCommStatus (*StopTx)  (struct _AT91PS_SvcXmodem *, unsigned int);

	// Private Methods:
	AT91S_SvcCommStatus (*ReadHandler)  (struct _AT91PS_SvcXmodem *, unsigned int csr);
	AT91S_SvcCommStatus (*WriteHandler) (struct _AT91PS_SvcXmodem *, unsigned int csr);
	unsigned short      (*GetCrc)       (char *ptr, unsigned int count);
	char                (*CheckHeader)  (unsigned char currentPacket, char *packet);
	char                (*CheckData)    (struct _AT91PS_SvcXmodem *);

	AT91S_SvcComm  parent;      // Base class
	AT91PS_USART  pUsart;

	AT91S_SvcTempo tempo; // Link to a AT91S_Tempo object

	char          *pData;
	unsigned int  dataSize;        // = XMODEM_DATA_STX or XMODEM_DATA_SOH
	char          packetDesc[AT91C_XMODEM_PACKET_SIZE];
	unsigned char packetId;         // Current packet
	char          packetStatus;
	char          isPacketDesc;
	char          eot;            // end of transmition
} AT91S_SvcXmodem, *AT91PS_SvcXmodem;

typedef AT91PS_SvcComm      (*AT91PF_OpenSvcXmodem) ( AT91PS_SvcXmodem, AT91PS_USART, AT91PS_CtlTempo);

// This function is called by the application.
extern AT91PS_SvcComm AT91F_OpenSvcXmodem( AT91PS_SvcXmodem, AT91PS_USART, AT91PS_CtlTempo);

extern unsigned short AT91F_SvcXmodemGetCrc     (char *ptr, unsigned int count);
extern char           AT91F_SvcXmodemCheckHeader(unsigned char currentPacket, char *packet);
extern char           AT91F_SvcXmodemCheckData  (AT91PS_SvcXmodem pSvcXmodem);
extern AT91S_SvcCommStatus AT91F_SvcXmodemReadHandler(AT91PS_SvcXmodem pSvcXmodem, unsigned int csr);
extern AT91S_SvcCommStatus AT91F_SvcXmodemWriteHandler(AT91PS_SvcXmodem pSvcXmodem, unsigned int csr);
extern AT91S_SvcCommStatus AT91F_SvcXmodemStartTx(AT91PS_SvcComm pSvcComm);
extern AT91S_SvcCommStatus AT91F_SvcXmodemStopTx(AT91PS_SvcComm pSvcComm);
extern AT91S_SvcCommStatus AT91F_SvcXmodemStartRx(AT91PS_SvcComm pSvcComm);
extern AT91S_SvcCommStatus AT91F_SvcXmodemStopRx(AT91PS_SvcComm pSvcComm);
extern char AT91F_SvcXmodemTxReady(AT91PS_SvcComm pService);
extern char AT91F_SvcXmodemRxReady(AT91PS_SvcComm pSvcComm);


// Constants:
#define AT91C_PIPE_SUCCESS	      0
#define AT91C_PIPE_ERROR_SHIFT    8
#define AT91C_PIPE_ERROR          (0x0F << AT91C_PIPE_ERROR_SHIFT)

#define AT91C_PIPE_OPEN_FAILED    (1 << AT91C_PIPE_ERROR_SHIFT)
#define AT91C_PIPE_WRITE_FAILED   (2 << AT91C_PIPE_ERROR_SHIFT)
#define AT91C_PIPE_WRITE_ABORTED  (3 << AT91C_PIPE_ERROR_SHIFT)
#define AT91C_PIPE_READ_FAILED    (4 << AT91C_PIPE_ERROR_SHIFT)
#define AT91C_PIPE_READ_ABORTED   (5 << AT91C_PIPE_ERROR_SHIFT)
#define AT91C_PIPE_ABORT_FAILED   (6 << AT91C_PIPE_ERROR_SHIFT)
#define AT91C_PIPE_RESET_FAILED   (7 << AT91C_PIPE_ERROR_SHIFT)

/* _AT91S_Pipe stucture */
typedef unsigned int AT91S_PipeStatus;

typedef struct _AT91S_Pipe
{
	// A pipe is linked with a peripheral and a buffer
	AT91PS_SvcComm pSvcComm;
	AT91PS_Buffer  pBuffer;

	// Callback functions with their arguments
	void (*WriteCallback) (AT91S_PipeStatus, void *);
	void (*ReadCallback)  (AT91S_PipeStatus, void *);
	void *pPrivateReadData;
	void *pPrivateWriteData;

	// Pipe methods
	AT91S_PipeStatus (*Write) (
		struct _AT91S_Pipe   *pPipe,
		char const *         pData,
		unsigned int         size,
		void                 (*callback) (AT91S_PipeStatus, void *),
		void                 *privateData);
	AT91S_PipeStatus (*Read) (
		struct _AT91S_Pipe  *pPipe,
		char                *pData,
		unsigned int        size,
		void                (*callback) (AT91S_PipeStatus, void *),
		void                *privateData);
	AT91S_PipeStatus (*AbortWrite) (
		struct _AT91S_Pipe  *pPipe);
	AT91S_PipeStatus (*AbortRead) (
		struct _AT91S_Pipe *pPipe);
	AT91S_PipeStatus (*Reset) (
		struct _AT91S_Pipe *pPipe);
	char (*IsWritten) (
		struct _AT91S_Pipe *pPipe,
		char const *pVoid);
	char (*IsReceived) (
		struct _AT91S_Pipe *pPipe,
		char const *pVoid);
} AT91S_Pipe, *AT91PS_Pipe;

// types used in AT91S_Pipe
typedef AT91PS_Pipe (*AT91PF_OpenPipe)   (AT91PS_Pipe, AT91PS_SvcComm, AT91PS_Buffer);
typedef void (*AT91PF_PipeWriteCallBack) (AT91S_PipeStatus, void *);
typedef void (*AT91PF_PipeReadCallBack)  (AT91S_PipeStatus, void *);
typedef AT91S_PipeStatus (*AT91PF_PipeWrite) (AT91PS_Pipe, char const *, unsigned int, void (*) (AT91S_PipeStatus, void *),	void *);
typedef AT91S_PipeStatus (*AT91PF_PipeRead)  (AT91PS_Pipe, char const *, unsigned int, void (*) (AT91S_PipeStatus, void *),	void *);
typedef AT91S_PipeStatus (*AT91PF_PipeAbortWrite) (AT91PS_Pipe);
typedef AT91S_PipeStatus (*AT91PF_PipeAbortRead)  (AT91PS_Pipe);
typedef AT91S_PipeStatus (*AT91PF_PipeReset)      (AT91PS_Pipe);
typedef char (*AT91PF_PipeIsWritten)              (AT91PS_Pipe, char const *);
typedef char (*AT91PF_PipeIsReceived)             (AT91PS_Pipe, char const *);

// This function is called by the application
extern AT91PS_Pipe AT91F_OpenPipe(
	AT91PS_Pipe    pPipe,
	AT91PS_SvcComm pSvcComm,
	AT91PS_Buffer  pBuffer);

// Following functions are called through AT91S_Pipe pointers

extern AT91S_PipeStatus AT91F_PipeWrite(
	AT91PS_Pipe pPipe,
	char const *pVoid,
	unsigned int size,
	AT91PF_PipeWriteCallBack callback,
	void *privateData);
extern AT91S_PipeStatus AT91F_PipeRead(
	AT91PS_Pipe pPipe,
	char *pVoid,
	unsigned int Size,
	AT91PF_PipeReadCallBack callback,
	void *privateData);
extern AT91S_PipeStatus AT91F_PipeAbortWrite(AT91PS_Pipe pPipe);
extern AT91S_PipeStatus AT91F_PipeAbortRead(AT91PS_Pipe pPipe);
extern AT91S_PipeStatus AT91F_PipeReset(AT91PS_Pipe pPipe);
extern char AT91F_PipeMsgWritten(AT91PS_Pipe pPipe, char const *pVoid);
extern char AT91F_PipeMsgReceived(AT91PS_Pipe pPipe, char const *pVoid);

#ifdef DBG_DRV_PIPE
// This function parse the error number and return a string
// describing the error message
extern char const *AT91F_PipeGetError(AT91S_PipeStatus msgId);
#endif

extern const unsigned char bit_rev[256];

extern void CalculateCrc32(const unsigned char *,unsigned int, unsigned int *);
extern void CalculateCrc16(const unsigned char *, unsigned int , unsigned short *); 
extern void CalculateCrcHdlc(const unsigned char *, unsigned int, unsigned short *);
extern void CalculateCrc16ccitt(const unsigned char *, unsigned int , unsigned short *);

typedef const unsigned char* AT91PS_SVC_CRC_BIT_REV ;

typedef void  (*AT91PF_SVC_CRC32)   (const unsigned char *, unsigned int, unsigned int *);
typedef void  (*AT91PF_SVC_CRC16)   (const unsigned char *, unsigned int, unsigned short *);
typedef void  (*AT91PF_SVC_CRCHDLC) (const unsigned char *, unsigned int, unsigned short *);
typedef	void  (*AT91PF_SVC_CRCCCITT)(const unsigned char *, unsigned int , unsigned short *);


typedef short (*AT91PF_Sinus) (int angle);
typedef const short * AT91PS_SINE_TAB;

extern short AT91F_Sinus(int angle);
extern const short AT91C_SINUS180_TAB[256];


typedef void (TypeAICHandler) (void) ;


// ROM BOOT Structure Element Definition (liv v2)
typedef struct _AT91S_MEMCDesc
{
    AT91PS_MC		memc_base ;		/* Peripheral base */
    unsigned char	periph_id ;		/* MC Peripheral Identifier */
} AT91S_MEMCDesc, *AT91PS_MEMCDesc ;

typedef struct _AT91S_Pio2Desc
{
   AT91PS_PIO      pio_base ;       /* Base Address */
   unsigned char   periph_id ;      /* Peripheral Identifier */
   unsigned char   pio_number ;     /* Total Pin Number */
} AT91S_Pio2Desc, *AT91PS_Pio2Desc ;

typedef struct _AT91S_SPIDesc
{
    AT91PS_SPI         		spi_base ;
    const AT91PS_PIO        pio_base ;
    unsigned char           periph_id ;
    unsigned char           pin_spck ;
    unsigned char           pin_miso ;
    unsigned char           pin_mosi ;
    unsigned char           pin_npcs[4] ;
} AT91S_SPIDesc, *AT91PS_SPIDesc ;

typedef struct _AT91S_USART2Desc
{
    AT91PS_USART           	usart_base ;   	/* Peripheral base */
    const AT91PS_PIO   		pio_base ;     	/* IO controller descriptor */
    unsigned int            pin_rxd ;       /* RXD pin number in the PIO */
    unsigned int            pin_txd ;       /* TXD pin number in the PIO */
    unsigned int            pin_sck ;       /* SCK pin number in the PIO */
    unsigned int            pin_rts ;       /* RTS pin number in the PIO */
    unsigned int            pin_cts ;       /* CTS pin number in the PIO */
    unsigned int            pin_dtr ;       /* DTR pin number in the PIO */
    unsigned int            pin_ri ;        /* RI pin number in the PIO */
    unsigned int            pin_dsr ;       /* DSR pin number in the PIO */
    unsigned int            pin_dcd ;       /* DCD pin number in the PIO */
    unsigned int            periph_id ;     /* USART Peripheral Identifier */
} AT91S_USART2Desc, *AT91PS_USART2Desc ;

typedef struct _AT91S_TWIDesc
{
    AT91PS_TWI 				TWI_base ;
    const AT91PS_PIO        pio_base ;
    unsigned int			pin_sck ;
    unsigned int			pin_sda ;
    unsigned int 			periph_id;
}AT91S_TWIDesc, *AT91PS_TWIDesc;

typedef struct _AT91S_STDesc
{
    AT91PS_ST  		st_base ;          	/* Peripheral base address */
    TypeAICHandler  *AsmSTHandler ;     /* Assembly interrupt handler */
    unsigned char   PeriphId ;          /* Peripheral Identifier */
} AT91S_STDesc, *AT91PS_STDesc;

typedef struct _AT91S_RomBoot {
	const unsigned int     version;
	// Peripheral descriptors
	const AT91S_MEMCDesc   MEMC_DESC;
	const AT91S_STDesc     SYSTIMER_DESC;
	const AT91S_Pio2Desc   PIOA_DESC;
	const AT91S_Pio2Desc   PIOB_DESC;
	const AT91S_USART2Desc DBGU_DESC;
	const AT91S_USART2Desc USART0_DESC;
	const AT91S_USART2Desc USART1_DESC;
	const AT91S_USART2Desc USART2_DESC;
	const AT91S_USART2Desc USART3_DESC;
	const AT91S_TWIDesc    TWI_DESC;
	const AT91S_SPIDesc    SPI_DESC;

	// Objects entry
	const AT91PF_OpenPipe      		OpenPipe;
	const AT91PF_OpenSBuffer   		OpenSBuffer;
	const unsigned int				reserved1;
	const AT91PF_OpenSvcXmodem 		OpenSvcXmodem;
	const AT91PF_OpenCtlTempo  		OpenCtlTempo;
	const unsigned int				reserved2;
	const unsigned int				reserved3;
	const unsigned int				reserved4;
	const AT91PF_SVC_CRC16			CRC16;
	const AT91PF_SVC_CRCCCITT		CRCCCITT;
	const AT91PF_SVC_CRCHDLC		CRCHDLC;
	const AT91PF_SVC_CRC32			CRC32;
	const AT91PS_SVC_CRC_BIT_REV	Bit_Reverse_Array;
	const AT91PS_SINE_TAB			SineTab; 
	const AT91PF_Sinus              Sine;
} AT91S_RomBoot, *AT91PS_RomBoot;

#define AT91C_ROM_BOOT_ADDRESS ((const AT91S_RomBoot *) ( *((unsigned int *) (AT91C_BASE_ROM + 0x20))) )

#endif

