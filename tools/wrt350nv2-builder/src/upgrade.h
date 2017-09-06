#ifndef _UPGRADE_H_
#define _UPGRADE_H_

#define FLASH_END_ADDR       0xffffffff 
#define FLASH_ADDR_BASE      0xff800000
#define BOOT_ADDR_BASE			 0xfffc0000
#define BOOT_ADDR_BASE_OFF   0x7c0000
#define FLASH_SIZE           0x800000
#define BOOT_SIZE            0x40000
//NVRAM in boot area
//#define NVRAM_ADDR_BASE      0xfff90000
//#define NVRAM_ADDR_BASE_OFF  0x790000
//#define NVRAM_SIZE           0x10000

#define PID_OFFSET         (BOOT_SIZE- 0x46)
#define NODE_ADDRESS       (BOOT_SIZE-0x60)
#define NODE_BASE_OFF   (FLASH_SIZE-0x60)
#define PIN_ADDRESS         (BOOT_SIZE-0x70)//WPS PIN,8bytes
#define PIN_OFF 				(FLASH_SIZE-0x70)
#define KERNEL_CODE_OFFSET      0         
#define SN_ADDRESS (BOOT_SIZE-0x80) //12bytes
#define SN_OFF (FLASH_SIZE-0x80)


#define UPGRADE_START_OFFSET  0
#define UPGRADE_END_OFFSET    0x7A0000
#define PRODUCT_ID_OFFSET     0x75ffe0//(UPGRADE_END_OFFSET  - 0x10)
#define PROTOCOL_ID_OFFSET   (PRODUCT_ID_OFFSET + 0x02)
#define FW_VERSION_OFFSET    (PRODUCT_ID_OFFSET + 0x04)
#define SIGN_OFFSET          (PRODUCT_ID_OFFSET + 0x08)   /* eRcOmM */

//#define LANG_UPGRADE
#ifdef LANG_UPGRADE
#define LANGBUF_BEGIN   0x1300000
#define LANGBUF_END     0x1f00000
#define LANGBUF_LENTH   (LANGBUF_END - LANGBUF_BEGIN)
#endif
#ifndef ERROR
#define ERROR -1
#endif

#ifndef OK
#define OK 0
#endif

#ifndef NULL
#define NULL 0
#endif


void do_boot(void);
void Download(void);
void Assign(void);

void gpio_init(void);
void Led_Power(int value);
void Led_security(int value);
int PushButton(void);

static unsigned short xchg ( unsigned short dwData);
int FlashDriver(unsigned long dlAddress,unsigned char *dbData,unsigned long dlLength,unsigned long dlFlag);
int ProgramChip(unsigned long dlAddress,unsigned char * dbData,unsigned long dlLength);
int dl_Initialize(void);
void dl_GetAddr(unsigned char *node);
int dl_Receive(void);
int dl_Transmit(char *buf,int len);
void reset(void);
void AssignHWAddress(unsigned char *psBuffer);
int ResetChip(unsigned long ulRomDstAddr);
int GetFlashType(void);

#ifdef LANG_UPGRADE
int save_lang_buf(unsigned long flash_addr, char *mem_addr,unsigned long length);
int LangDriver(unsigned long flash_addr, char *mem_addr,unsigned long length, unsigned long dlFlag);
#endif 
#endif

