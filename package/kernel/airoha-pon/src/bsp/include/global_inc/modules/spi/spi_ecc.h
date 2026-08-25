
#include <asm/types.h>

typedef enum
{
	SPI_ECC_ENCODE_DISABLE=0,
	SPI_ECC_ENCODE_ENABLE
} SPI_ECC_ENCODE_T;

typedef enum
{
	SPI_ECC_ENCODE_ABILITY_4BITS = 4,
	SPI_ECC_ENCODE_ABILITY_6BITS = 6,
	SPI_ECC_ENCODE_ABILITY_8BITS = 8,
	SPI_ECC_ENCODE_ABILITY_10BITS = 10,
	SPI_ECC_ENCODE_ABILITY_12BITS = 12,
	SPI_ECC_ENCODE_ABILITY_14BITS = 14,
	SPI_ECC_ENCODE_ABILITY_16BITS = 16,
} SPI_ECC_ENCODE_ABILITY_T;

typedef enum
{
	SPI_ECC_ENCODE_ABILITY_4BITS_TRANSLATE = 0,
	SPI_ECC_ENCODE_ABILITY_6BITS_TRANSLATE,
	SPI_ECC_ENCODE_ABILITY_8BITS_TRANSLATE,
	SPI_ECC_ENCODE_ABILITY_10BITS_TRANSLATE,
	SPI_ECC_ENCODE_ABILITY_12BITS_TRANSLATE,
	SPI_ECC_ENCODE_ABILITY_14BITS_TRANSLATE,
	SPI_ECC_ENCODE_ABILITY_16BITS_TRANSLATE,
} SPI_ECC_ENCODE_ABILITY_TRANSLATE_T;

typedef enum
{
	SPI_ECC_ENCODE_STATUS_IDLE=0,
	SPI_ECC_ENCODE_STATUS_PROCESSING,
	SPI_ECC_ENCODE_STATUS_DONE,
} SPI_ECC_ENCODE_STATUS_T;


typedef struct SPI_ECC_ENCODE_CONF
{
	SPI_ECC_ENCODE_T 			encode_en; 				/* enable encode or not */
	u32							encode_block_size;		/* encode block size */
	SPI_ECC_ENCODE_ABILITY_T	encode_ecc_abiliry;		/* Indicate the ECC correction abiliry */
		
} SPI_ECC_ENCODE_CONF_T;


typedef enum
{
	SPI_ECC_DECODE_DISABLE=0,
	SPI_ECC_DECODE_ENABLE
} SPI_ECC_DECODE_T;

typedef enum
{
	SPI_ECC_DECODE_ABILITY_4BITS = 4,
	SPI_ECC_DECODE_ABILITY_6BITS = 6,
	SPI_ECC_DECODE_ABILITY_8BITS = 8,
	SPI_ECC_DECODE_ABILITY_10BITS = 10,
	SPI_ECC_DECODE_ABILITY_12BITS = 12,
	SPI_ECC_DECODE_ABILITY_14BITS = 14,
	SPI_ECC_DECODE_ABILITY_16BITS = 16,
} SPI_ECC_DECODE_ABILITY_T;


typedef struct SPI_ECC_DECODE_CONF
{
	SPI_ECC_DECODE_T 			decode_en; 				/* enable decode or not */
	u32							decode_block_size;		/* decode block size */
	SPI_ECC_DECODE_ABILITY_T	decode_ecc_abiliry;		/* Indicate the ECC correction abiliry */
		
} SPI_ECC_DECODE_CONF_T;


typedef enum
{
	SPI_ECC_DECODE_STATUS_IDLE=0,
	SPI_ECC_DECODE_STATUS_PROCESSING,
	SPI_ECC_DECODE_STATUS_DONE,
	SPI_ECC_DECODE_STATUS_TIMEOUT
} SPI_ECC_DECODE_STATUS_T;

typedef enum
{
	SPI_ECC_DECODE_CORRECTION_FAIL=0,
	SPI_ECC_DECODE_CORRECTION_OK,
	
} SPI_ECC_DECODE_CORRECTION_T;


typedef enum{
	SPI_ECC_RTN_NO_ERROR =0,
	SPI_ECC_RTN_CORRECTION_ERROR,

	SPI_ECC_RTN_DEF_NO
} SPI_ECC_RTN_T;

SPI_ECC_RTN_T SPI_ECC_Regs_Dump( void );
SPI_ECC_RTN_T SPI_ECC_Encode_Check_Idle( SPI_ECC_ENCODE_STATUS_T *prt_rtn_encode_status_t );
SPI_ECC_RTN_T SPI_ECC_Encode_Check_Done( SPI_ECC_ENCODE_STATUS_T *prt_rtn_encode_status_t );
SPI_ECC_RTN_T SPI_ECC_Encode_Get_Configure( SPI_ECC_ENCODE_CONF_T  *ptr_rtn_encode_conf_t );
SPI_ECC_RTN_T SPI_ECC_Encode_Set_Configure( SPI_ECC_ENCODE_CONF_T  *ptr_encode_conf_t );
SPI_ECC_RTN_T SPI_ECC_Encode_Enable( void);
SPI_ECC_RTN_T SPI_ECC_Encode_Disable( void);
SPI_ECC_RTN_T SPI_ECC_Encode_Init( void );
SPI_ECC_RTN_T SPI_ECC_Decode_Check_Idle( SPI_ECC_DECODE_STATUS_T *prt_rtn_decode_status_t );
SPI_ECC_RTN_T SPI_ECC_Decode_Check_Done( SPI_ECC_DECODE_STATUS_T *prt_rtn_decode_status_t, u8 sec_num );
SPI_ECC_RTN_T SPI_ECC_DECODE_Check_Correction_Status( void );
SPI_ECC_RTN_T SPI_ECC_Decode_Get_Configure( SPI_ECC_DECODE_CONF_T  *ptr_rtn_decode_conf_t );
SPI_ECC_RTN_T SPI_ECC_Decode_Set_Configure( SPI_ECC_DECODE_CONF_T  *ptr_decode_conf_t );
SPI_ECC_RTN_T SPI_ECC_Decode_Enable( void);
SPI_ECC_RTN_T SPI_ECC_Decode_Disable( void);
SPI_ECC_RTN_T SPI_ECC_Decode_Init( void );
void SPI_ECC_DEBUG_ENABLE( void );
void SPI_ECC_DEBUG_DISABLE( void );
u32 get_spi_ecc_deccnfg(void);

#if defined(CONFIG_ECNT_UBOOT)
extern void udelay(u32 usec);

#define delay1us udelay
#endif

