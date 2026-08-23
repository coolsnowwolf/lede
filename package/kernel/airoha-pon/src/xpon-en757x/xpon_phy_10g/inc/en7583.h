#ifndef _EN7583_H_
#define _EN7583_H_

extern ponPhyFuncTbl an7583_gepon_func[];
extern ponPhyFuncTbl an7583_xgpon_func[];
extern ponPhyFuncTbl an7583_xepon_func[];

void an7583_pon_phy_clear_rogueonu_BEN(void);   //julia_20231010
void an7583_pon_phy_rogueonu_int_en_BEN(unchar rogue_int_en);  //julia_20231010
void an7583_pon_phy_rogueonu_detect(void);  //julia_20240510
void pma_no_los_no_ready_reset(void);

#endif /* _EN7583_H_ */

