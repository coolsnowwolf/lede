#if A60972_SERDES  //mark A60972 FPGA code

#ifndef _A60972_H_
#define _A60972_H_



int xpon_pma_debug_init(void);

int xpon_pma_init(void);


void a60972_trans_tx_control(unchar val);

void a60972_dump(void);

#endif
#endif
