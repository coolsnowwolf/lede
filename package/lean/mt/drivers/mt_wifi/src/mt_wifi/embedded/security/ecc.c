#include "rt_config.h"
#include "security/sae_cmm.h"
#include "security/ecc.h"

EC_POINT_OP_TIME_RECORD ec_op_ti_rec;

static EC_GROUP_INFO ec_groups[] = {
	EC_GROUP(19, EC_GROUP19_BITS_OF_R),
	EC_GROUP(20, EC_GROUP20_BITS_OF_R),
	EC_GROUP(21, EC_GROUP21_BITS_OF_R),
	EC_GROUP(25, EC_GROUP25_BITS_OF_R),
	EC_GROUP(26, EC_GROUP26_BITS_OF_R),
	EC_GROUP(27, EC_GROUP27_BITS_OF_R),
	EC_GROUP(28, EC_GROUP28_BITS_OF_R),
	EC_GROUP(29, EC_GROUP29_BITS_OF_R),
	EC_GROUP(30, EC_GROUP30_BITS_OF_R),
};

static EC_GROUP_INFO_BI ec_groups_bi[] = {
	EC_GROUP_BI(19),
	EC_GROUP_BI(20),
	EC_GROUP_BI(21),
	EC_GROUP_BI(25),
	EC_GROUP_BI(26),
	EC_GROUP_BI(27),
	EC_GROUP_BI(28),
	EC_GROUP_BI(29),
	EC_GROUP_BI(30),
};

int ECC_COST_TIME_DBG_LVL = DBG_LVL_INFO;


EC_GROUP_INFO *get_ecc_group_info(
	IN INT32 group)
{
	UINT32 i;
	EC_GROUP_INFO *ec_group = NULL;

	for (i = 0; i < EC_GROUP_NUM; i++)
		if (ec_groups[i].group_id == group)
			ec_group = &ec_groups[i];
	return ec_group;
}

EC_GROUP_INFO_BI *get_ecc_group_info_bi(
	IN INT32 group)
{
	UINT32 i;
	EC_GROUP_INFO_BI *ec_group_bi = NULL;
	EC_GROUP_INFO *ec_group = NULL;
	const UCHAR *gx = NULL;
	const UCHAR *gy = NULL;
	UINT32 gx_len;
	UINT32 gy_len;

	for (i = 0; i < EC_GROUP_NUM; i++)
		if (ec_groups[i].group_id == group) {
			ec_group = &ec_groups[i];
			ec_group_bi = &ec_groups_bi[i];
		}

	if (ec_group == NULL
		|| ec_group_bi == NULL)
		return NULL;

	if (ec_group_bi->is_init == FALSE) {
		SAE_BN *tmp = NULL;
		SAE_BN *pthree = NULL;
		UCHAR three[] = {0x3};

		SAE_BN_BIN2BI((UINT8 *)ec_group->prime,
						  ec_group->prime_len,
						  &ec_group_bi->prime);
		SAE_BN_BIN2BI((UINT8 *)ec_group->order,
						  ec_group->order_len,
						  &ec_group_bi->order);
		SAE_BN_BIN2BI((UINT8 *)ec_group->a,
						  ec_group->a_len,
						  &ec_group_bi->a);
		SAE_BN_BIN2BI((UINT8 *)ec_group->b,
						  ec_group->b_len,
						  &ec_group_bi->b);

		if (group == 19) {
			gx = ec_group19_gx;
			gy = ec_group19_gy;
			gx_len = sizeof(ec_group19_gx);
			gy_len = sizeof(ec_group19_gy);
		} else if (group == 20) {
			gx = ec_group20_gx;
			gy = ec_group20_gy;
			gx_len = sizeof(ec_group20_gx);
			gy_len = sizeof(ec_group20_gy);
		} else if (group == 21) {
			gx = ec_group21_gx;
			gy = ec_group21_gy;
			gx_len = sizeof(ec_group21_gx);
			gy_len = sizeof(ec_group21_gy);
		}

		if ((gx != NULL) && (gx != NULL)) {
			SAE_BN_BIN2BI((UINT8 *)gx, gx_len, &ec_group_bi->gx);
			SAE_BN_BIN2BI((UINT8 *)gy, gy_len, &ec_group_bi->gy);
		}

		ec_group_bi->is_init = TRUE;

		SAE_BN_BIN2BI(three, sizeof(three), &pthree);
		SAE_BN_INIT(&tmp);
		SAE_BN_ADD(ec_group_bi->a, pthree, &tmp);
		if (!SAE_BN_UCMP(tmp, ec_group_bi->prime))
			ec_group_bi->is_minus_3 = TRUE;

		SAE_BN_FREE(&tmp);
		SAE_BN_FREE(&pthree);

		/* mont structure init */
		os_alloc_mem(NULL, (UCHAR **) &ec_group_bi->mont, sizeof(MONT_STRUC));
		if (ec_group_bi->mont == NULL)
			return NULL;

		ec_group_bi->mont->Bits_Of_R = ec_group->bits_of_R;
		ec_group_bi->mont->pBI_X = NULL;
		ec_group_bi->mont->pBI_R = NULL;
		ec_group_bi->mont->pBI_PInverse = NULL;
		SAE_BN_BIN2BI((UINT8 *)ec_group->X,
						  ec_group->X_len,
						  &ec_group_bi->mont->pBI_X);
		SAE_BN_BIN2BI((UINT8 *)ec_group->R,
						  ec_group->R_len,
						  &ec_group_bi->mont->pBI_R);
		SAE_BN_BIN2BI((UINT8 *)ec_group->PInverse,
						  ec_group->PInverse_len,
						  &ec_group_bi->mont->pBI_PInverse);
	}

	return ec_group_bi;
}

BIG_INTEGER_EC_POINT *ecc_point_add_cmm(
	IN BIG_INTEGER_EC_POINT *point,
	IN BIG_INTEGER_EC_POINT *point2,
	IN SAE_BN *lamda,
	IN EC_GROUP_INFO_BI *ec_group_bi)
{
	BIG_INTEGER_EC_POINT *res = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *prime = ec_group_bi->prime;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));
	ecc_point_init(&res);

	if (res == NULL)
		return NULL;

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[7]);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(tmp2);

	/*
	  * Xr = lamda ^ 2 - Xp - Xq
	  * Yr = lamda * (Xp - Xr) - Yp
	  */
	SAE_BN_MOD_MUL(lamda, lamda, prime, &tmp);
	SAE_BN_MOD_ADD_QUICK(point->x, point2->x, prime, &tmp2);
	SAE_BN_MOD_SUB(tmp, tmp2, prime, &res->x);
	SAE_BN_MOD_SUB_QUICK(point->x, res->x, prime, &tmp);
	SAE_BN_MOD_MUL(lamda, tmp, prime, &tmp2);
	SAE_BN_MOD_SUB(tmp2, point->y, prime, &res->y);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp2);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[7]);
	return res;
}

VOID ecc_point_init(
	IN BIG_INTEGER_EC_POINT **ec_point_res)
{
	if (*ec_point_res == NULL) {
		os_alloc_mem(NULL, (UCHAR **)ec_point_res, sizeof(BIG_INTEGER_EC_POINT));

		if (*ec_point_res == NULL) {
			panic("ecc_point_init alloc fail\n");
			return;
		}

		(*ec_point_res)->x = NULL;
		(*ec_point_res)->y = NULL;
		(*ec_point_res)->z = NULL;
		(*ec_point_res)->z_is_one = 0;
	}
}


VOID ecc_point_free(
	IN BIG_INTEGER_EC_POINT **ec_point_res)
{
	if (*ec_point_res == NULL)
		return;

	SAE_BN_FREE(&(*ec_point_res)->x);
	SAE_BN_FREE(&(*ec_point_res)->y);
	SAE_BN_FREE(&(*ec_point_res)->z);
	os_free_mem(*ec_point_res);
	*ec_point_res = NULL;
}

VOID ecc_point_copy(
	IN BIG_INTEGER_EC_POINT *point,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	if (point == NULL)
		return;

	if (*ec_point_res == NULL) {
		ecc_point_init(ec_point_res);

		if (*ec_point_res == NULL) {
			panic("ecc_point_init alloc fail\n");
			return;
		}
	}

	if (point->x)
		SAE_BN_COPY(point->x, &(*ec_point_res)->x);
	if (point->y)
		SAE_BN_COPY(point->y, &(*ec_point_res)->y);
	if (point->z)
		SAE_BN_COPY(point->z, &(*ec_point_res)->z);
	(*ec_point_res)->z_is_one = point->z_is_one;
}

/* if (x1, y2) != (x2, y2), but x1 = x2 => result is infinity */
/* if point1 + point2 and point1 is infinity => result is point2 */
VOID ecc_point_add(
	IN BIG_INTEGER_EC_POINT *point,
	IN BIG_INTEGER_EC_POINT *point2,
	IN EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	BIG_INTEGER_EC_POINT *res = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *lamda = NULL;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 ("==> %s()\n", __func__));
	SAE_LOG_TIME_BEGIN(&ec_op_ti_rec.add_op);

	/* if point1 + point2 and point1 is infinity => result is point2 */
	if (point == NULL && point2 != NULL) {
		ecc_point_copy(point2, ec_point_res);
		return;
	}

	/* if point1 + point2 and point2 is infinity => result is point1 */
	if (point != NULL && point2 == NULL) {
		ecc_point_copy(point, ec_point_res);
		return;
	}

	/* if point1 + point2 and point1 & point2 is infinity => result is infinity */
	if (point == NULL && point2 == NULL) {
		ecc_point_free(ec_point_res);
		return;
	}

	if (point->z && point2->z) {
		ecc_point_add_3d(point, point2, ec_group_bi, ec_point_res);
		return;
	}

	if (SAE_BN_UCMP(point->x, point2->x) == 0) {
		if (SAE_BN_UCMP(point->y, point2->y) == 0)
			ecc_point_double(point, ec_group_bi, ec_point_res);
		else
			/* if (x1, y2) != (x2, y2), but x1 = x2 => result is infinity */
			ecc_point_free(ec_point_res);
		return;
	}

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[8]);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(tmp2);
	GET_BI_INS_FROM_POOL(lamda);
	SAE_BN_INIT(&tmp);
	SAE_BN_INIT(&tmp2);
	SAE_BN_INIT(&lamda);

	/* lamda = (Yp - Yq) / (Xp - Xq) */
	SAE_BN_MOD_SUB_QUICK(point->x, point2->x, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_SUB_QUICK(point->y, point2->y, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_DIV(tmp2, tmp, ec_group_bi->prime, &lamda);
	res = ecc_point_add_cmm(point, point2, lamda, ec_group_bi);
	ecc_point_copy(res, ec_point_res);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp2);
	SAE_BN_RELEASE_BACK_TO_POOL(&lamda);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[8]);
	ecc_point_free(&res);
	SAE_LOG_TIME_END(&ec_op_ti_rec.add_op);
}

/* if point is infinity, result is infinity */
VOID ecc_point_double(
	IN BIG_INTEGER_EC_POINT *point,
	IN EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	BIG_INTEGER_EC_POINT *res = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *lamda = NULL;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 ("==> %s()\n", __func__));
	SAE_LOG_TIME_BEGIN(&ec_op_ti_rec.dbl_op);

	if (point == NULL) {
		/* if point is infinity, result is infinity */
		ecc_point_free(ec_point_res);
		return;
	}

	if (point->z) {
		ecc_point_double_3d(point, ec_group_bi, ec_point_res);
		return;
	}

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[9]);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(tmp2);
	GET_BI_INS_FROM_POOL(lamda);
	SAE_BN_INIT(&tmp);
	SAE_BN_INIT(&tmp2);
	SAE_BN_INIT(&lamda);

	/* lamda = (3Xp^2 + a) / 2Yp */
	SAE_BN_MOD_SQR(point->x, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_ADD_QUICK(tmp, tmp, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_ADD_QUICK(tmp, tmp2, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_ADD(tmp2, ec_group_bi->a, ec_group_bi->prime, &tmp);
	SAE_BN_MOD_ADD_QUICK(point->y, point->y, ec_group_bi->prime, &tmp2);
	SAE_BN_MOD_DIV(tmp, tmp2, ec_group_bi->prime, &lamda);
	res = ecc_point_add_cmm(point, point, lamda, ec_group_bi);
	ecc_point_copy(res, ec_point_res);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp2);
	SAE_BN_RELEASE_BACK_TO_POOL(&lamda);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[9]);
	ecc_point_free(&res);
	SAE_LOG_TIME_END(&ec_op_ti_rec.dbl_op);
}

VOID ecc_point_add_3d(
	IN BIG_INTEGER_EC_POINT *point,
	IN BIG_INTEGER_EC_POINT *point2,
	IN EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	SAE_BN *n0 = NULL;
	SAE_BN *n1 = NULL;
	SAE_BN *n2 = NULL;
	SAE_BN *n3 = NULL;
	SAE_BN *n4 = NULL;
	SAE_BN *n5 = NULL;
	SAE_BN *n6 = NULL;
	BIG_INTEGER_EC_POINT *res = NULL;

	/* if point1 + point2 and point1 is infinity => result is point2 */
	if (point == NULL && point2 != NULL) {
		ecc_point_copy(point2, ec_point_res);
		return;
	}

	/* if point1 + point2 and point2 is infinity => result is point1 */
	if (point != NULL && point2 == NULL) {
		ecc_point_copy(point, ec_point_res);
		return;
	}

	/* if point1 + point2 and point1 & point2 is infinity => result is infinity */
	if (point == NULL && point2 == NULL) {
		ecc_point_free(ec_point_res);
		return;
	}

	SAE_BN_INIT(&n0);
	SAE_BN_INIT(&n1);
	SAE_BN_INIT(&n2);
	SAE_BN_INIT(&n3);
	SAE_BN_INIT(&n4);
	SAE_BN_INIT(&n5);
	SAE_BN_INIT(&n6);

	if (n6 == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			("%s: allocate fail\n", __func__));
		ecc_point_free(ec_point_res);
		return;
	}

	/* n1, n2 */
	if (point2->z_is_one) {
		SAE_BN_COPY(point->x, &n1);
		SAE_BN_COPY(point->y, &n2);
		/* n1 = X_a */
		/* n2 = Y_a */
	} else {
		SAE_BN_MOD_SQR(point2->z, ec_group_bi->prime, &n0);
		SAE_BN_MOD_MUL(point->x, n0, ec_group_bi->prime, &n1);
		/* n1 = X_a * Z_b^2 */
		SAE_BN_MOD_MUL(n0, point2->z, ec_group_bi->prime, &n0);
		SAE_BN_MOD_MUL(point->y, n0, ec_group_bi->prime, &n2);
		/* n2 = Y_a * Z_b^3 */
	}

	/* n3, n4 */
	if (point->z_is_one) {
		SAE_BN_COPY(point2->x, &n3);
		SAE_BN_COPY(point2->y, &n4);
		/* n3 = X_b */
		/* n4 = Y_b */
	} else {
		SAE_BN_MOD_SQR(point->z, ec_group_bi->prime, &n0);
		SAE_BN_MOD_MUL(point2->x, n0, ec_group_bi->prime, &n3);
		/* n3 = X_b * Z_a^2 */
		SAE_BN_MOD_MUL(n0, point->z, ec_group_bi->prime, &n0);
		SAE_BN_MOD_MUL(point2->y, n0, ec_group_bi->prime, &n4);
		/* n4 = Y_b * Z_a^3 */
	}

	/* n5, n6 */
	SAE_BN_MOD_SUB_QUICK(n1, n3, ec_group_bi->prime, &n5);
	SAE_BN_MOD_SUB_QUICK(n2, n4, ec_group_bi->prime, &n6);
	/* n5 = n1 - n3 */
	/* n6 = n2 - n4 */

	if (SAE_BN_IS_ZERO(n5)) {
		if (SAE_BN_IS_ZERO(n6)) {
			/* a is the same point as b */
			ecc_point_double_3d(point, ec_group_bi, ec_point_res);
		} else {
			/* a is the inverse of b */
			ecc_point_free(ec_point_res);
		}
		return;
	}
	/* 'n7', 'n8' */
	SAE_BN_MOD_ADD_QUICK(n1, n3, ec_group_bi->prime, &n1);
	SAE_BN_MOD_ADD_QUICK(n2, n4, ec_group_bi->prime, &n2);
	/* 'n7' = n1 + n3 */
	/* 'n8' = n2 + n4 */

	ecc_point_init(&res);

	if (res == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			("%s: ecc allocate fail\n", __func__));
		ecc_point_free(ec_point_res);
		return;
	}

	SAE_BN_INIT(&res->x);
	SAE_BN_INIT(&res->y);
	SAE_BN_INIT(&res->z);

	/* Z_r */
	if (point->z_is_one && point2->z_is_one)
		SAE_BN_COPY(n5, &res->z);
	else {
		if (point->z_is_one)
			SAE_BN_COPY(point2->z, &n0);
		else if (point2->z_is_one)
			SAE_BN_COPY(point->z, &n0);
		else
			SAE_BN_MOD_MUL(point->z, point2->z, ec_group_bi->prime, &n0);
		SAE_BN_MOD_MUL(n0, n5, ec_group_bi->prime, &res->z);
	}
	res->z_is_one = 0;
	/* Z_r = Z_a * Z_b * n5 */

	/* X_r */
	SAE_BN_MOD_SQR(n6, ec_group_bi->prime, &n0);
	SAE_BN_MOD_SQR(n5, ec_group_bi->prime, &n4);
	SAE_BN_MOD_MUL(n1, n4, ec_group_bi->prime, &n3);
	SAE_BN_MOD_SUB_QUICK(n0, n3, ec_group_bi->prime, &res->x);
	/* X_r = n6^2 - n5^2 * 'n7' */

	/* 'n9' */
	SAE_BN_MOD_LSHIFT1(res->x, ec_group_bi->prime, &n0);
	SAE_BN_MOD_SUB_QUICK(n3, n0, ec_group_bi->prime, &n0);
	/* n9 = n5^2 * 'n7' - 2 * X_r */

	/* Y_r */
	SAE_BN_MOD_MUL(n0, n6, ec_group_bi->prime, &n0);
	SAE_BN_MOD_MUL(n4, n5, ec_group_bi->prime, &n5); /* now n5 is n5^3 */
	SAE_BN_MOD_MUL(n2, n5, ec_group_bi->prime, &n1);
	SAE_BN_MOD_SUB_QUICK(n0, n1, ec_group_bi->prime, &n0);
	if (SAE_BN_IS_ODD(n0)) {
		SAE_BN_ADD(n0, ec_group_bi->prime, &n3); /* BI: input can not equal to output */
		SAE_BN_COPY(n3, &n0);
	}
	/* now  0 <= n0 < 2*p,  and n0 is even */
	SAE_BN_RSHIFT1(n0, &res->y);
	/* Y_r = (n6 * 'n9' - 'n8' * 'n5^3') / 2 */

	SAE_BN_FREE(&n0);
	SAE_BN_FREE(&n1);
	SAE_BN_FREE(&n2);
	SAE_BN_FREE(&n3);
	SAE_BN_FREE(&n4);
	SAE_BN_FREE(&n5);
	SAE_BN_FREE(&n6);

	ecc_point_copy(res, ec_point_res);

	ecc_point_free(&res);
}

VOID ecc_point_double_3d(
	IN BIG_INTEGER_EC_POINT *point,
	IN EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	SAE_BN *n0 = NULL;
	SAE_BN *n1 = NULL;
	SAE_BN *n2 = NULL;
	SAE_BN *n3 = NULL;
	BIG_INTEGER_EC_POINT *res = NULL;

	if (point == NULL) {
		/* if point is infinity, result is infinity */
		ecc_point_free(ec_point_res);
		return;
	}

	SAE_BN_INIT(&n0);
	SAE_BN_INIT(&n1);
	SAE_BN_INIT(&n2);
	SAE_BN_INIT(&n3);

	if (n3 == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			("%s: allocate fail\n", __func__));
		ecc_point_free(ec_point_res);
		return;
	}

	ecc_point_init(&res);

	if (res == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			("%s: ecc allocate fail\n", __func__));
		ecc_point_free(ec_point_res);
		return;
	}

	SAE_BN_INIT(&res->x);
	SAE_BN_INIT(&res->y);
	SAE_BN_INIT(&res->z);

	/* n1 */
	if (point->z_is_one) {
		SAE_BN_MOD_SQR(point->x, ec_group_bi->prime, &n0);
		SAE_BN_MOD_LSHIFT1(n0, ec_group_bi->prime, &n1);
		SAE_BN_MOD_ADD_QUICK(n0, n1, ec_group_bi->prime, &n0);
		SAE_BN_MOD_ADD_QUICK(n0, ec_group_bi->a, ec_group_bi->prime, &n1);
		/* n1 = 3 * X_a^2 + a_curve */
	} else if (ec_group_bi->is_minus_3) {
		SAE_BN_MOD_SQR(point->z, ec_group_bi->prime, &n1);
		SAE_BN_MOD_ADD_QUICK(point->x, n1, ec_group_bi->prime, &n0);
		SAE_BN_MOD_SUB_QUICK(point->x, n1, ec_group_bi->prime, &n2);
		SAE_BN_MOD_MUL(n0, n2, ec_group_bi->prime, &n1);
		SAE_BN_MOD_LSHIFT1(n1, ec_group_bi->prime, &n0);
		SAE_BN_MOD_ADD_QUICK(n0, n1, ec_group_bi->prime, &n1);
		/*-
		 * n1 = 3 * (X_a + Z_a^2) * (X_a - Z_a^2)
		 *    = 3 * X_a^2 - 3 * Z_a^4
		 */
	} else {
		SAE_BN_MOD_SQR(point->x, ec_group_bi->prime, &n0);
		SAE_BN_MOD_LSHIFT1(n0, ec_group_bi->prime, &n1);
		SAE_BN_MOD_ADD_QUICK(n0, n1, ec_group_bi->prime, &n0);
		SAE_BN_MOD_SQR(point->z, ec_group_bi->prime, &n1);
		SAE_BN_MOD_SQR(n1, ec_group_bi->prime, &n1);
		SAE_BN_MOD_MUL(n1, ec_group_bi->a, ec_group_bi->prime, &n1);
		SAE_BN_MOD_ADD_QUICK(n1, n0, ec_group_bi->prime, &n1);
		/* n1 = 3 * X_a^2 + a_curve * Z_a^4 */
	}

	/* Z_r */
	if (point->z_is_one)
		SAE_BN_COPY(point->y, &n0);
	else
		SAE_BN_MOD_MUL(point->y, point->z, ec_group_bi->prime, &n0);
	SAE_BN_MOD_LSHIFT1(n0, ec_group_bi->prime, &res->z);
	res->z_is_one = 0;
	/* Z_r = 2 * Y_a * Z_a */

	/* n2 */
	SAE_BN_MOD_SQR(point->y, ec_group_bi->prime, &n3);
	SAE_BN_MOD_MUL(point->x, n3, ec_group_bi->prime, &n2);
	SAE_BN_MOD_LSHIFT(n2, 2, ec_group_bi->prime, &n2);
	/* n2 = 4 * X_a * Y_a^2 */

	/* X_r */
	SAE_BN_MOD_LSHIFT1(n2, ec_group_bi->prime, &n0);
	SAE_BN_MOD_SQR(n1, ec_group_bi->prime, &res->x);
	SAE_BN_MOD_SUB_QUICK(res->x, n0, ec_group_bi->prime, &res->x);
	/* X_r = n1^2 - 2 * n2 */

	/* n3 */
	SAE_BN_MOD_SQR(n3, ec_group_bi->prime, &n0);
	SAE_BN_MOD_LSHIFT(n0, 3, ec_group_bi->prime, &n3);
	/* n3 = 8 * Y_a^4 */

	/* Y_r */
	SAE_BN_MOD_SUB_QUICK(n2, res->x, ec_group_bi->prime, &n0);
	SAE_BN_MOD_MUL(n1, n0, ec_group_bi->prime, &n0);
	SAE_BN_MOD_SUB_QUICK(n0, n3, ec_group_bi->prime, &res->y);
	/* Y_r = n1 * (n2 - X_r) - n3 */

	SAE_BN_FREE(&n0);
	SAE_BN_FREE(&n1);
	SAE_BN_FREE(&n2);
	SAE_BN_FREE(&n3);

	ecc_point_copy(res, ec_point_res);

	ecc_point_free(&res);
}


VOID ecc_point_3d_to_2d(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	INOUT BIG_INTEGER_EC_POINT *ec_point_res)
{
	SAE_BN *Z_1 = NULL;
	SAE_BN *Z_2 = NULL;
	SAE_BN *Z_3 = NULL;
	SAE_BN *Z = ec_point_res->z;
	SAE_BN *res_x = NULL;
	SAE_BN *res_y = NULL;
	SAE_BN *res_z = NULL;
	UCHAR one[] = {1};

	if (ec_point_res == NULL)
		return;
	if (ec_point_res->z_is_one)
		return;

	if (ec_point_res->z == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			("%s: z is null\n", __func__));
		return;
	}

	SAE_BN_INIT(&Z_1);
	SAE_BN_INIT(&Z_2);
	SAE_BN_INIT(&Z_3);
	SAE_BN_INIT(&res_x);
	SAE_BN_INIT(&res_y);
	SAE_BN_INIT(&res_z);

	/*-
	* We have a curve defined by a Weierstrass equation
	*      y^2 = x^3 + a*x + b.
	* The point to consider is given in Jacobian projective coordinates
	* where  (X, Y, Z)  represents  (x, y) = (X/Z^2, Y/Z^3).
	* Substituting this and multiplying by  Z^6  transforms the above equation into
	*      Y^2 = X^3 + a*X*Z^4 + b*Z^6.
	* To test this, we add up the right-hand side in 'rh'.
	*/
	SAE_BN_MOD_MUL_INV(Z, ec_group_bi->prime, &Z_1);
	SAE_BN_MOD_SQR(Z_1, ec_group_bi->prime, &Z_2);
	SAE_BN_MOD_MUL(ec_point_res->x, Z_2, ec_group_bi->prime, &res_x);
	SAE_BN_MOD_MUL(Z_2, Z_1, ec_group_bi->prime, &Z_3);
	SAE_BN_MOD_MUL(ec_point_res->y, Z_3, ec_group_bi->prime, &res_y);

	SAE_BN_BIN2BI(one, sizeof(one), &res_z);

	SAE_BN_COPY(res_x, &ec_point_res->x);
	SAE_BN_COPY(res_y, &ec_point_res->y);
	SAE_BN_COPY(res_z, &ec_point_res->z);
	ec_point_res->z_is_one = 1;

	SAE_BN_FREE(&Z_1);
	SAE_BN_FREE(&Z_2);
	SAE_BN_FREE(&Z_3);
	SAE_BN_FREE(&res_x);
	SAE_BN_FREE(&res_y);
	SAE_BN_FREE(&res_z);
}

VOID ecc_point_set_z_to_one(
	INOUT BIG_INTEGER_EC_POINT *ec_point_res)
{
	UCHAR one[] = {1};

	SAE_BN_BIN2BI(one, sizeof(one), &ec_point_res->z);
	ec_point_res->z_is_one = 1;
}


#define W_POW 16

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* w-ary non-adjacent form (wNAF) method */
VOID ecc_point_mul_wNAF(
	IN BIG_INTEGER_EC_POINT *point,
	IN SAE_BN *scalar,
	IN EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	BIG_INTEGER_EC_POINT *p[W_POW / 2] = {NULL};
	BIG_INTEGER_EC_POINT *res = NULL;
	SAE_BN *scalar_copy = NULL;
	UINT32 i;
	UINT32 *d;
	UINT32 record = 0;

	SAE_LOG_TIME_BEGIN(&ec_op_ti_rec.mul_op);

	/* pre-compute {1, 3, ..., 2^(w-1) -1}P */
	ecc_point_copy(point, &p[0]);
	ecc_point_double(p[0], ec_group_bi, &p[W_POW / 2 - 1]);
	for (i = 0; i < W_POW / 4 - 1; i++)
		ecc_point_add(p[i], p[W_POW / 2  - 1], ec_group_bi, &p[i + 1]);
	for (i = 0; i < W_POW / 4; i++) {
		ecc_point_copy(p[i], &p[W_POW / 2 - i - 1]);
		ecc_point_inverse(p[W_POW / 2 - i - 1], ec_group_bi->prime, &p[W_POW / 2 - i - 1]);
	}
	os_alloc_mem(NULL, (UCHAR **)&d, sizeof(UINT32) * SAE_BN_GET_LEN(scalar) * 8);
	NdisZeroMemory(d, sizeof(UINT32) * SAE_BN_GET_LEN(scalar) * 8);
	SAE_BN_COPY(scalar, &scalar_copy);

	i = 0;
	while (SAE_BN_IS_ZERO(scalar_copy) == FALSE) {
		if (SAE_BN_IS_ODD(scalar_copy)) {
			SAE_BN_MOD_DW(scalar_copy, W_POW, &d[i]);
			if (d[i] >= W_POW / 2)
				SAE_BN_ADD_DW(scalar_copy, W_POW); /* fix me */
		} else
			d[i] = 0;
		SAE_BN_RSHIFT1(scalar_copy, &scalar_copy);
		i++;
	}
	i = SAE_BN_GET_LEN(scalar) * 8 - 1;
	ecc_point_copy(point, &res);
	do {

		ecc_point_double(res, ec_group_bi, &res);
		if (d[i]) {
			ecc_point_add(res, p[d[i]], ec_group_bi, &res);
			record++;
		}
		i--;
	} while (i != 0);

	ecc_point_copy(res, ec_point_res);

	os_free_mem(d);

	for (i = 0; i < W_POW / 2; i++)
		ecc_point_free(&p[i]);
	ecc_point_free(&res);
	SAE_LOG_TIME_END(&ec_op_ti_rec.mul_op);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF, ("total cnt = %d!!!!!!\n", record + W_POW / 2));
}

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* windowed method */
VOID ecc_point_mul_windowed(
	IN BIG_INTEGER_EC_POINT *point,
	IN SAE_BN *scalar,
	IN EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT **ec_point_res)
{
	BIG_INTEGER_EC_POINT *p[W_POW / 2] = {NULL};
	BIG_INTEGER_EC_POINT *res = NULL;
	SAE_BN *scalar_copy = NULL;
	INT16 i;
	UINT32 *d;

	SAE_LOG_TIME_BEGIN(&ec_op_ti_rec.mul_op);

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 ("==> %s()\n", __func__));

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[11]);
	GET_BI_INS_FROM_POOL(scalar_copy);

	/* pre-compute {1, 3, ..., 2^(w-1) -1}P */
	ecc_point_copy(point, &p[0]);
	ecc_point_double(p[0], ec_group_bi, &p[W_POW / 2 - 1]);

	for (i = 0; i < W_POW / 2 - 1; i++)
		ecc_point_add(p[i], p[W_POW / 2  - 1], ec_group_bi, &p[i + 1]);

	os_alloc_mem(NULL, (UCHAR **)&d, sizeof(UINT32) * SAE_BN_GET_LEN(scalar) * 8);
	NdisZeroMemory(d, sizeof(UINT32) * SAE_BN_GET_LEN(scalar) * 8);
	SAE_BN_COPY(scalar, &scalar_copy);

	i = 0;
	while (SAE_BN_IS_ZERO(scalar_copy) == FALSE) {
		if (SAE_BN_IS_ODD(scalar_copy))
			SAE_BN_MOD_DW(scalar_copy, W_POW, &d[i]);
		else
			d[i] = 0;
		SAE_BN_RSHIFT1(scalar_copy, &scalar_copy);
		i++;
	}

	i = SAE_BN_GET_LEN(scalar) * 8 - 1;

	do {

		ecc_point_double(res, ec_group_bi, &res);
		if (d[i])
			ecc_point_add(res, p[(d[i] - 1) / 2], ec_group_bi, &res);
		i--;
	} while (i >= 0);

	ecc_point_copy(res, ec_point_res);

	os_free_mem(d);

	for (i = 0; i < W_POW / 2; i++)
		ecc_point_free(&p[i]);
	ecc_point_free(&res);
	SAE_BN_RELEASE_BACK_TO_POOL(&scalar_copy);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[11]);
	SAE_LOG_TIME_END(&ec_op_ti_rec.mul_op);
}



/* (x,y) => (x, p-y) */
VOID ecc_point_inverse(
	IN BIG_INTEGER_EC_POINT *point,
	IN SAE_BN *prime,
	OUT BIG_INTEGER_EC_POINT **point_res)
{
	BIG_INTEGER_EC_POINT *res = NULL;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));

	if (point == NULL) {
		ecc_point_free(point_res);
		return;
	}

	ecc_point_copy(point, &res);
	SAE_BN_SUB(prime, point->y, &res->y);
	ecc_point_copy(res, point_res);
	ecc_point_free(&res);
}

/* y^2 = x^3 + ax + b  */
UCHAR ecc_point_is_on_curve(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT *point)
{
	SAE_BN *right = NULL;
	SAE_BN *left = NULL;
	SAE_BN *left2 = NULL;
	UCHAR res;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[12]);
	GET_BI_INS_FROM_POOL(right);
	GET_BI_INS_FROM_POOL(left);
	GET_BI_INS_FROM_POOL(left2);
	SAE_BN_INIT(&right);
	SAE_BN_INIT(&left);
	SAE_BN_INIT(&left2);

	SAE_LOG_TIME_BEGIN(&ec_op_ti_rec.on_curve_check_op);
	SAE_BN_MOD_SQR(point->x, ec_group_bi->prime, &right); /* x^2 */
	SAE_BN_MOD_ADD(right, ec_group_bi->a, ec_group_bi->prime, &right); /* X^2+a */
	SAE_BN_MOD_MUL(point->x, right, ec_group_bi->prime, &right); /* x^3+ax */
	SAE_BN_MOD_ADD(right, ec_group_bi->b, ec_group_bi->prime, &right); /* x^3+ax+b */
	SAE_BN_MOD_SQR(point->y, ec_group_bi->prime, &left); /* y^2 */
	SAE_BN_SUB(ec_group_bi->prime, point->y, &left2);
	SAE_BN_MOD_SQR(left2, ec_group_bi->prime, &left2); /* (p - y)^2 */

	if (SAE_BN_UCMP(left, right) == 0
		|| SAE_BN_UCMP(left2, right) == 0)
		res = TRUE;
	else
		res = FALSE;

	SAE_BN_RELEASE_BACK_TO_POOL(&right);
	SAE_BN_RELEASE_BACK_TO_POOL(&left);
	SAE_BN_RELEASE_BACK_TO_POOL(&left2);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[12]);
	SAE_LOG_TIME_END(&ec_op_ti_rec.on_curve_check_op);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("%s(): res = %d\n", __func__, res));
	return res;
}

/* Y^2 = X^3 + a*X*Z^4 + b*Z^6  */
UCHAR ecc_point_is_on_curve_3d(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT *point)
{
	SAE_BN *right = NULL;
	SAE_BN *left = NULL;
	SAE_BN *left2 = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *Z4 = NULL;
	UCHAR res;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));

	if (point->z == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			("ecc_point_is_on_curve_3d: z is NULL\n"));
		return FALSE;
	}

	GET_BI_INS_FROM_POOL(right);
	GET_BI_INS_FROM_POOL(left);
	GET_BI_INS_FROM_POOL(left2);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(Z4);
	SAE_BN_INIT(&right);
	SAE_BN_INIT(&left);
	SAE_BN_INIT(&left2);
	SAE_BN_INIT(&tmp);
	SAE_BN_INIT(&Z4);

	SAE_BN_MOD_SQR(point->x, ec_group_bi->prime, &right); /* x^2 */
	SAE_BN_MOD_SQR(point->z, ec_group_bi->prime, &tmp);   /* z^2 */
	SAE_BN_MOD_SQR(tmp, ec_group_bi->prime, &Z4);   /* z^4 */
	SAE_BN_MOD_MUL(Z4, tmp, ec_group_bi->prime, &tmp); /* z^6 */
	SAE_BN_MOD_MUL(ec_group_bi->a, Z4, ec_group_bi->prime, &Z4); /* az^4  */
	SAE_BN_MOD_ADD(right, Z4, ec_group_bi->prime, &right); /* X^2+az^4  */
	SAE_BN_MOD_MUL(point->x, right, ec_group_bi->prime, &right); /* X^3+axz^4 */
	SAE_BN_MOD_MUL(ec_group_bi->b, tmp, ec_group_bi->prime, &tmp); /* bz^6 */
	SAE_BN_MOD_ADD(right, tmp, ec_group_bi->prime, &right); /* X^3+axz^4 + bz^6 */

	SAE_BN_MOD_SQR(point->y, ec_group_bi->prime, &left); /* y^2 */
	SAE_BN_SUB(ec_group_bi->prime, point->y, &left2);
	SAE_BN_MOD_SQR(left2, ec_group_bi->prime, &left2); /* (p - y)^2 */


	if (SAE_BN_UCMP(left, right) == 0
		|| SAE_BN_UCMP(left2, right) == 0)
		res = TRUE;
	else
		res = FALSE;

	SAE_BN_RELEASE_BACK_TO_POOL(&right);
	SAE_BN_RELEASE_BACK_TO_POOL(&left);
	SAE_BN_RELEASE_BACK_TO_POOL(&left2);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	SAE_BN_RELEASE_BACK_TO_POOL(&Z4);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("%s(): res = %d\n", __func__, res));
	return res;
}


/* ec_GFp_simple_set_compressed_coordinates */
/* y^2 = x^3 + ax + b  */
UCHAR ecc_point_find_by_x(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN SAE_BN *x,
	IN SAE_BN **res_y,
	IN UCHAR need_res_y)
{
	SAE_BN *res = NULL;
	UCHAR has_y;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
			 ("==> %s()\n", __func__));

	SAE_LOG_TIME_BEGIN(&ec_op_ti_rec.find_y_op);
	SAE_BN_MOD_SQR(x, ec_group_bi->prime, &res); /* x^2 */
	SAE_BN_MOD_ADD(res, ec_group_bi->a, ec_group_bi->prime, &res); /* X^2+a */
	SAE_BN_MOD_MUL(x, res, ec_group_bi->prime, &res); /* x^3+ax */
	SAE_BN_MOD_ADD(res, ec_group_bi->b, ec_group_bi->prime, &res); /* x^3+ax+b */

	if (SAE_BN_IS_QUADRATIC_RESIDE(res, ec_group_bi->prime, ec_group_bi->mont))
		has_y = TRUE;
	else
		has_y = FALSE;

	if (need_res_y && has_y) {
		SAE_BN_MOD_SQRT(res, ec_group_bi->prime, ec_group_bi->mont, &res);
		if (res) {
			SAE_BN_COPY(res, res_y);
			has_y = TRUE;
		} else {
			SAE_BN_FREE(res_y);
			has_y = FALSE;
		}
	}

	SAE_BN_FREE(&res);
	SAE_LOG_TIME_END(&ec_op_ti_rec.find_y_op);
	return has_y;
}


VOID ecc_point_dump_time(
	VOID)
{
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, ECC_COST_TIME_DBG_LVL, ("ecc_point time record:\n"));

	if (ec_op_ti_rec.add_op.exe_times) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("add_op time record:\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 ("\tavg_time=%lu jiffies", ec_op_ti_rec.add_op.avg_time_interval));
#ifdef LINUX
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 (", %u msec",	jiffies_to_msecs(ec_op_ti_rec.add_op.avg_time_interval)));
#endif
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("\n\texe_times=%u\n", ec_op_ti_rec.add_op.exe_times));
		ec_op_ti_rec.add_op.avg_time_interval = 0;
		ec_op_ti_rec.add_op.time_interval = 0;
		ec_op_ti_rec.add_op.exe_times = 0;
	}

	if (ec_op_ti_rec.dbl_op.exe_times) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("dbl_op time record:\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 ("\tavg_time=%lu jiffies", ec_op_ti_rec.dbl_op.avg_time_interval));
#ifdef LINUX
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 (", %u msec", jiffies_to_msecs(ec_op_ti_rec.dbl_op.avg_time_interval)));
#endif
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("\n\texe_times=%u\n", ec_op_ti_rec.dbl_op.exe_times));
		ec_op_ti_rec.dbl_op.avg_time_interval = 0;
		ec_op_ti_rec.dbl_op.time_interval = 0;
		ec_op_ti_rec.dbl_op.exe_times = 0;
	}

	if (ec_op_ti_rec.mul_op.exe_times) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("mul_op time record:\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 ("\tavg_time=%lu jiffies", ec_op_ti_rec.mul_op.avg_time_interval));
#ifdef LINUX
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 (", %u msec", jiffies_to_msecs(ec_op_ti_rec.mul_op.avg_time_interval)));
#endif
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("\n\texe_times=%u\n", ec_op_ti_rec.mul_op.exe_times));
		ec_op_ti_rec.mul_op.avg_time_interval = 0;
		ec_op_ti_rec.mul_op.time_interval = 0;
		ec_op_ti_rec.mul_op.exe_times = 0;
	}

	if (ec_op_ti_rec.find_y_op.exe_times) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("find_y_op time record:\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 ("\tavg_time=%lu jiffies", ec_op_ti_rec.find_y_op.avg_time_interval));
#ifdef LINUX
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 (", %u msec", jiffies_to_msecs(ec_op_ti_rec.find_y_op.avg_time_interval)));
#endif
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
			("\n\texe_times=%u\n", ec_op_ti_rec.find_y_op.exe_times));
		ec_op_ti_rec.find_y_op.avg_time_interval = 0;
		ec_op_ti_rec.find_y_op.time_interval = 0;
		ec_op_ti_rec.find_y_op.exe_times = 0;
	}

	if (ec_op_ti_rec.on_curve_check_op.exe_times) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL, ("on_curve_check_op time record:\n"));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 ("\tavg_time=%lu jiffies",
					ec_op_ti_rec.on_curve_check_op.avg_time_interval));
#ifdef LINUX
		MTWF_LOG(DBG_CAT_SEC, CATSEC_ECC, ECC_COST_TIME_DBG_LVL,
				 (", %u msec",
					jiffies_to_msecs(ec_op_ti_rec.on_curve_check_op.avg_time_interval)));
#endif
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, ECC_COST_TIME_DBG_LVL,
			("\n\texe_times=%u\n",
				ec_op_ti_rec.on_curve_check_op.exe_times));
		ec_op_ti_rec.on_curve_check_op.avg_time_interval = 0;
		ec_op_ti_rec.on_curve_check_op.time_interval = 0;
		ec_op_ti_rec.on_curve_check_op.exe_times = 0;
	}
}


