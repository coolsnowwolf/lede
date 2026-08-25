
/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************
	Module Name:
	xpon_igmp_lib.h
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name				Date			Modification logs
	lidong.hu		2012/7/28	Create
*/
#ifndef _XPON_IGMP_LIB_H_
#define _XPON_IGMP_LIB_H_


//////////////////////////////////////////////////////////////////////////////////////////

int igmp_get_ver(int port); 


int igmp_set_ver(int port ,int ver);

 
int igmp_get_func(int port);


int igmp_set_func(int port,int mode);


int igmp_get_fastleave(int port);


int igmp_set_fastleave(int port,int mode);


int igmp_get_up_tci(int port);


int igmp_set_up_tci(int port,int tci);


int igmp_get_up_tagctrl(int port);


int igmp_set_up_tagctrl(int port,int mode);


int igmp_get_down_tci(int port);


int igmp_set_down_tci(int port,int tci);


int igmp_get_down_tagctrl(int port);


int igmp_set_down_tagctrl(int port,int mode);


int igmp_get_maxrate(int port);


int igmp_set_maxrate(int port,int maxRate);


int igmp_clear_maxrate(int port);


int igmp_get_robust(int port);


int igmp_set_robust(int port,int robust);


int igmp_get_unauthor(int port);


int igmp_set_unauthor(int port,int unauthor);


int igmp_get_query_ip(int port,unsigned char* queryip);


int igmp_set_query_ip(int port,unsigned char* queryip);


int igmp_get_query_interval(int port);


int igmp_set_query_interval(int port,int interval);


int igmp_get_last_interval(int port);


int igmp_set_last_interval(int port,int interval);


int igmp_get_query_maxresp(int port);


int igmp_set_query_maxresp(int port,int maxresp);


int igmp_get_max_playgroup(int port);


int igmp_set_max_playgroup(int port,int maxgroup);


int igmp_get_max_bw(int port);
int igmp_set_max_bw(int port,int bw);
int igmp_get_bwe(int port);
int igmp_set_bwe(int port,unsigned char bwe);
int igmp_get_counter_bw_exceeded(int port);
int igmp_get_counter_curr_mcast_bw(int port);
int igmp_get_counter_join_msg(int port);
int igmp_get_user_subscribe_cnt(int port);
int igmp_get_user_subscribe_by_index(int port, int index, char * src_ip, char * program_ip);
int igmp_set_xpon_ds_bw_ctrl(int val);


int igmp_get_tagstrip(int port);


int igmp_set_tagstrip(int port,int mode);


int igmp_get_fastleave_ctc(void);


int igmp_set_fastleave_ctc(int fastleave);


int igmp_get_fastleave_ability(void);


////////////////////////////////////////////////////////////////////////////////////////////

int igmp_get_dyctrlist(int port, int maxlen, int*  num, unsigned char* val);

int igmp_set_dyctrlist(int port, void*  ptr);

int igmp_del_dyctrlist(int port, int index);

int igmp_clear_dyctrlist(int port);

int igmp_get_staticlist(int port, int maxlen, int*  num, unsigned char* val);

int igmp_set_staticlist(int port, void* ptr);

int igmp_del_staticlist(int port, int index);

int igmp_clear_staticlist(int port);

/////////////////////////////////////////////////////////////////////////////////////////////
int igmp_add_portvlan(int port,int vid);

int igmp_del_portvlan(int port,int vid);

int igmp_clear_portvlan(int port);

int igmp_get_portvlan_cnt(int port);

int igmp_get_portvlan_id(int port,int idx,int* vid,int* newid);

int igmp_set_portvlan_switchid(int port,int vid,int newvid);

int igmp_get_portvlan_switchcnt(int port);

int igmp_get_portvlan_switchid(int port,int idx,int* vid,int* newid);

int igmp_set_portvlan_flag(int port,int vflag);

////////////////////////////////////////////////////////////////////////////////////////////////
int igmp_get_fwdmode(void);

int igmp_set_fwdmode(int mode);

int igmp_add_fwdentry(int type,int port,int vid, unsigned char* grp_addr,unsigned char* src_ip);

int igmp_del_fwdentry(int type,int port,int vid, unsigned char* grp_addr,unsigned char* src_ip);

int igmp_get_fwdentry_cnt(void);

int igmp_clear_fwdentry(void);

int igmp_get_fwdentry(int idx,int* type,int* port,int* vid, unsigned char* grp_addr,unsigned char* src_ip,unsigned char* client_ip,int* join_time_sec);

int igmp_get_fwdentry_ext(int idx,int* type,int* port,int* vid, unsigned char* grp_addr,unsigned char* src_ip,int* flag);

/////////////////////////////////////////////////////////////////////////////////////////////////
int igmp_add_mulvlan(int vid);

int igmp_del_mulvlan(int vid);

int igmp_clear_mulvlan(void);

int igmp_get_mulvlan_cnt(void);

int igmp_se_mulvlan_flag(int vid,int flag);
	
int igmp_ge_mulvlan_flag(int vid);
	
int igmp_get_mulvlan_id(int idx);

///////////////////////////////////////////////////////////////////////////////////////////////////
int igmp_set_epon_mode(void); 

int igmp_set_xpon_mode(int mode);

int igmp_get_onu_type(void);

int igmp_set_xpon_debug(int val);

int igmp_clear_hw_entry(void);

int igmp_clear_hw_drop_entry(void);

int igmp_set_care_ver_dy_stalist(int val);


#endif
