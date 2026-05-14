#ifndef __DAL_RTL8373_HSB_H__
#define __DAL_RTL8373_HSB_H__



#define RTL8373_REASON_NUMBER	0x40

typedef struct hsab_latch_s{
	rtk_uint8 latch_always;
	rtk_uint8 latch_first;
	rtk_uint8 spa_en;
	rtk_uint8 spa;
	rtk_uint8 fwd_en;
	rtk_uint8 fwd;
	rtk_uint8 reason_en;
	rtk_uint8 reason;

}hsab_latch_t;


typedef struct rtl8373_hsb_s{

    rtk_uint16 pktlen;
    rtk_uint8 dmac[6];
	rtk_uint8 smac[6];
    rtk_uint8   cputag_if;
    rtk_uint32  cputag;
    rtk_uint8   stag_if;
    rtk_uint16  stag;
    rtk_uint8   ctag_if;
    rtk_uint16  ctag;
	rtk_uint8 	rtag_if;
    rtk_uint16  ethertype;
    rtk_uint8   snap;
    rtk_uint8   pppoe;
    rtk_uint8   rrcp;
	rtk_uint8   rldp;
	rtk_uint8   rlpp;
	rtk_uint8   oam;
	rtk_uint8   arp;
    rtk_uint8   ip_type;
	rtk_uint8   tcp;
	rtk_uint8   udp;
	rtk_uint8   igmp;
	rtk_uint8   icmp;
    rtk_uint32  dip;
    rtk_uint32  sip;
    rtk_uint16  l4_dport;
    rtk_uint16  l4_sport;
	rtk_uint16	tos;
    rtk_uint8   usr0_valid;
    rtk_uint8   usr1_valid;
    rtk_uint8   usr2_valid;
    rtk_uint8   usr3_valid;
    rtk_uint8   usr4_valid;
    rtk_uint8   usr5_valid;
    rtk_uint8   usr6_valid;
    rtk_uint8   usr7_valid;
	rtk_uint8   usr8_valid; 
    rtk_uint8   usr9_valid;
    rtk_uint8   usr10_valid;
    rtk_uint8   usr11_valid;
    rtk_uint8   usr12_valid;
    rtk_uint8   usr13_valid;
    rtk_uint8   usr14_valid;
    rtk_uint8   usr15_valid;
    rtk_uint16  usr0_field;
    rtk_uint16  usr1_field;
    rtk_uint16  usr2_field;
    rtk_uint16  usr3_field;
    rtk_uint16  usr4_field;
    rtk_uint16  usr5_field;
    rtk_uint16  usr6_field;
    rtk_uint16  usr7_field;
	rtk_uint16  usr8_field;
    rtk_uint16  usr9_field;
    rtk_uint16  usr10_field;
    rtk_uint16  usr11_field;
    rtk_uint16  usr12_field;
    rtk_uint16  usr13_field;
    rtk_uint16  usr14_field;
    rtk_uint16  usr15_field;
    rtk_uint8  	spa;
	rtk_uint8	wol;
	rtk_uint8	l2ptp;
	rtk_uint8	udpptp;
	rtk_uint8	ingr_err;
	rtk_uint8	errpkt;
	rtk_uint8	l3err;
    rtk_uint16  begin_dsc;
    rtk_uint16  end_dsc;

}rtl8373_hsb_t;



typedef struct rtl8373_hsa_s
{
    rtk_uint16  pktlen;
    rtk_uint8  dpm0;
    rtk_uint8  dpm1;
    rtk_uint8  dpm2;
    rtk_uint8  dpm3;
    rtk_uint8  dpm4;
    rtk_uint8  dpm5;
    rtk_uint8  dpm6;
    rtk_uint8  dpm7;
	rtk_uint8  dpm8;
	rtk_uint8  dpm9;
	rtk_uint8  trappkt;
	rtk_uint8  rfc1024;
	rtk_uint8  llcother_if;
	rtk_uint8  pppoe_if;
	rtk_uint8  l3fmt;

	rtk_uint8  acl_dscprmk_en;
	rtk_uint8  acl_1prmk_en;
	rtk_uint8  dscp_1p;
	rtk_uint8  internal_pri;
	rtk_uint8  user_pri0;
	rtk_uint8  user_pri1;
	rtk_uint8  user_pri2;
	rtk_uint8  user_pri3;
	rtk_uint8  user_pri4;
	rtk_uint8  user_pri5;
	rtk_uint8  user_pri6;
	rtk_uint8  user_pri7;
	rtk_uint8  user_pri8;
	rtk_uint8  user_pri9;
	rtk_uint8  ingress_cpri;

	rtk_uint8  cvid_en;
	rtk_uint8  ctag_if;
	rtk_uint8  cvid_if;
	rtk_uint16  cvid;
	rtk_uint16  cvlan_untagset;
	rtk_uint8  cvid_zero;
	rtk_uint8  cfmt_en;
	rtk_uint8  cfmt;
	rtk_uint8  ctag_act;
	rtk_uint16 ingress_ctag;

	rtk_uint8  stag_if;
	rtk_uint16 svid;
	rtk_uint8  spri;
	rtk_uint16 svlan_untag;
	rtk_uint8  ingress_stag;

	rtk_uint8  keep;
	rtk_uint8  cputag_if;
	rtk_uint8  qid0;
	rtk_uint8  qid1;
	rtk_uint8  qid2;
	rtk_uint8  qid3;
	rtk_uint8  qid4;
	rtk_uint8  qid5;
	rtk_uint8  qid6;
	rtk_uint8  qid7;
	rtk_uint8  qid8;
	rtk_uint8  qid9;
	rtk_uint8  qid10;
	rtk_uint8  qid11;
	rtk_uint8  qid12;
	rtk_uint8  qid13;
	rtk_uint8  qid14;
	rtk_uint8  qid15;

	rtk_uint8  rspan;

 
	rtk_uint8  spa;
    rtk_uint16  end_dsc;
    rtk_uint16  begin_dsc;
	rtk_uint8 	pg_id;

	rtk_uint8	reason;

} rtl8373_hsa_t;


extern ret_t rtl8373_aleLatch_set(hsab_latch_t * latchParam);
extern ret_t rtl8373_aleLatch_get(hsab_latch_t * latchParam);
extern ret_t rtl8373_aleHsb_get(rtl8373_hsb_t * pHsb);
extern ret_t rtl8373_aleHsb_display(rtl8373_hsb_t * pHsb);
extern ret_t rtl8373_aleHsa_get(rtl8373_hsa_t * pHsa);
extern ret_t rtl8373_aleHsa_display(rtl8373_hsa_t *hsa);





#endif
