#ifndef __MT7636_H__
#define __MT7636_H__

#include "../mcu/andes_core.h"
#include "../phy/mt_rf.h"

struct _RTMP_ADAPTER;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

void mt7636_init(struct _RTMP_ADAPTER *pAd);
void mt7636_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7636_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7636_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
int mt7636_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7636_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7636_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7636_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);

#endif // __MT7636_H__

