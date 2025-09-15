/*
 * Copyright (c) 2011 Quantenna Communications, Inc.
 *
 * Shared datastructure between lhost, MuC and ADM module for CCA measurement
 */

#ifndef _QTN_CCA_H
#define _QTN_CCA_H

struct out_cca_info {
	u_int64_t	start_tsf;
	u_int64_t	end_tsf;
	u_int32_t	cnt_pri_cca;
	u_int32_t	cnt_sec_cca;
	u_int32_t	cca_sample_cnt;
};

#endif	// _QTN_CCA_H

