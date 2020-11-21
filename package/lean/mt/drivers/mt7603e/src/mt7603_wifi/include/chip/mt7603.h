#ifndef __MT7603_H__
#define __MT7603_H__

#include "../mcu/andes_core.h"
//#include "../mcu/andes_rlt.h"
#include "../phy/mt_rf.h"

#ifdef RTMP_FLASH_SUPPORT
#ifdef RTMP_MAC_PCI
#define EEPROM_DEFAULT_7603_FILE_PATH     "/etc_ro/wlan/MT7603E.bin"
#endif /* RTMP_MAC_PCI */
#endif /* RTMP_FLASH_SUPPORT */

struct _RTMP_ADAPTER;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt7603_init(struct _RTMP_ADAPTER *pAd);
void mt7603_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7603_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7603_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
int mt7603_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7603_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7603_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7603_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7603_set_ed_cca(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);

#ifdef LED_CONTROL_SUPPORT
INT Set_MT7603LED_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT7603LED_Enhance_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT7603LED_Behavor_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* LED_CONTROL_SUPPORT */



#endif // __MT7603_H__

