#ifndef _ECNT_LOG_H_
#define _ECNT_LOG_H_
/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************/
#define FLASH_SIZE 0x500000
#define BLOCK_SIZE 256 * 1024
#define PAGE_SIZE 4096

#define LOG_BUF_LEN 5 * 1024 * 1024
#define BUF_LEN BLOCK_SIZE
#define SEPARATE_NUM 8
#define MSG_LEN 256
#define VAL_LEN 8 * MSG_LEN
#define RAW_DATA_LEN 4 * MSG_LEN
#define NORMAL 0
#define BIN 1

#define FLASH_ADDR_BEGIN 0x6b80000
#define FLASH_ADDR_END 0x7080000
#define FLASH_ADDR_PANIC (FLASH_ADDR_END - BLOCK_SIZE)

#define PROC_PATH "ecnt_log/%s"
#define PROC_PATH_F "/proc/ecnt_log/%s"
#define FILE_PATH "/tmp/%s"

#define ONE_MINUT 60 * 1000
#define ENUM_TO_STRING(x) case x: return(#x);
/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************/
char log_buf[LOG_BUF_LEN];

typedef struct {
	unsigned char module_num;
	char module_name[16];
	unsigned char log_type;
	unsigned char separate_save;
	int file_size;
	char file_dir[32];
} log_conf;
	
enum log_module_type{
	LOG_OMCI,
	LOG_OMCI_RAW,
	LOG_WIFI,
	LOG_DP,
	LOG_WAN,
	LOG_VOIP,
    LOG_XPON,
	LOG_OTHERS
};

static inline const char *enum_to_string(enum log_module_type type){
	switch(type){
		ENUM_TO_STRING(LOG_OMCI)
		ENUM_TO_STRING(LOG_OMCI_RAW)
		ENUM_TO_STRING(LOG_WIFI)
		ENUM_TO_STRING(LOG_DP)
		ENUM_TO_STRING(LOG_WAN)
		ENUM_TO_STRING(LOG_VOIP)
        ENUM_TO_STRING(LOG_XPON)
		ENUM_TO_STRING(LOG_OTHERS)
		}
	return "error";
}
log_conf ecnt_log_conf[SEPARATE_NUM] = {
	{LOG_OMCI, "omci", NORMAL, 1, BUF_LEN, "omci_log"},
	{LOG_OMCI_RAW, "omci_raw", BIN, 1, BUF_LEN, "omci_raw"},
	{LOG_WIFI, "wifi", NORMAL, 1, BUF_LEN, "wifi_log"},
	{LOG_DP, "data_path", NORMAL, 1, BUF_LEN, "dp_log"},
	{LOG_WAN, "wan", NORMAL, 1, BUF_LEN, "wan_log"},
	{LOG_VOIP, "voip", NORMAL, 1, BUF_LEN, "voip_log"},
    {LOG_VOIP, "xpon_drv_log", NORMAL, 1, BUF_LEN, "xpon_log"},
	{LOG_OTHERS, "normal_log", NORMAL, 0, 0, "ecnt_log"}
	};
#endif