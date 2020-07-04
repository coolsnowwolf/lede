#ifndef	__ECC_H__
#define	__ECC_H__

#include "bn.h"
#include "sae_cmm.h"

typedef struct __EC_GROUP_INFO_BI {
	INT32 group_id;
	SAE_BN *prime;
	SAE_BN *order;
	SAE_BN *a;
	SAE_BN *b;
	SAE_BN *gx;
	SAE_BN *gy;
	SAE_BN *cofactor;
	MONT_STRUC *mont;
	UCHAR is_minus_3;
	UCHAR is_init;
} EC_GROUP_INFO_BI, *PEC_GROUP_INFO_BI;

typedef struct __EC_GROUP_INFO {
	INT32 group_id;
	const UCHAR *prime;
	UINT32 prime_len;
	const UCHAR *order;
	UINT32 order_len;
	const UCHAR *a;
	UINT32 a_len;
	const UCHAR *b;
	UINT32 b_len;
	const UCHAR *X;
	UINT32 X_len;
	const UCHAR *R;
	UINT32 R_len;
	const UCHAR *PInverse;
	UINT32 PInverse_len;
	UINT16 bits_of_R;
} EC_GROUP_INFO, *PEC_GROUP_INFO;

struct __EC_GROUP_INFO *get_ecc_group_info(
	IN INT32 group);

EC_GROUP_INFO_BI *get_ecc_group_info_bi(
	IN INT32 group);

VOID group_info_bi_deinit(
	VOID);

VOID ecc_point_init(
	IN BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_free(
	IN BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_copy(
	IN BIG_INTEGER_EC_POINT * point,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* if (x1, y2) != (x2, y2), but x1 = x2 => result is infinity */
/* if point1 + point2 and point1 is infinity => result is point2 */
VOID ecc_point_add(
	IN BIG_INTEGER_EC_POINT * point,
	IN BIG_INTEGER_EC_POINT * point2,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);
/* if point is infinity, result is infinity */
VOID ecc_point_double(
	IN BIG_INTEGER_EC_POINT * point,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_add_3d(
	IN BIG_INTEGER_EC_POINT * point,
	IN BIG_INTEGER_EC_POINT * point2,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_double_3d(
	IN BIG_INTEGER_EC_POINT * point,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_3d_to_2d(
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	INOUT BIG_INTEGER_EC_POINT * ec_point_res);

VOID ecc_point_set_z_to_one(
	INOUT BIG_INTEGER_EC_POINT * ec_point_res);

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* double and add */
VOID ecc_point_mul_dblandadd(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *scalar,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* w-ary non-adjacent form (wNAF) method */
VOID ecc_point_mul_wNAF(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *scalar,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* w-ary non-adjacent form (wNAF) method */
VOID ecc_point_mul_windowed(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *scalar,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* (x,y) => (x, p-y) */
VOID ecc_point_inverse(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *prime,
	OUT BIG_INTEGER_EC_POINT * *point_res);

UCHAR ecc_point_is_on_curve(
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT * point);

UCHAR ecc_point_is_on_curve_3d(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT * point);

/* y^2 = x^3 + ax + b  */
UCHAR ecc_point_find_by_x(
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	IN SAE_BN *x,
	IN SAE_BN **res_y,
	IN UCHAR need_res_y);

VOID ecc_point_dump_time(
	VOID);

BIG_INTEGER_EC_POINT *ecc_point_add_cmm(
	IN BIG_INTEGER_EC_POINT *point,
	IN BIG_INTEGER_EC_POINT *point2,
	IN SAE_BN *lamda,
	IN EC_GROUP_INFO_BI *ec_group_bi);

INT ecc_gen_key(EC_GROUP_INFO *ec_group,
		EC_GROUP_INFO_BI *ec_group_bi,
		INOUT SAE_BN **priv_key,
		BIG_INTEGER_EC_POINT *generator,
		INOUT VOID **pub_key);
#endif /* __ECC_H__ */
