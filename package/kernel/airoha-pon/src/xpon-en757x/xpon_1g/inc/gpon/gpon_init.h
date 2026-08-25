#ifndef _GPON_INIT_H_
#define _GPON_INIT_H_


void __gpon_dump_csr(void) ;
int gpon_init(void) ;
void gpon_deinit(void) ;
int register_gpon_isr(void);
int unregister_gpon_isr(void);

#endif /* _GPON_INIT_H_ */




