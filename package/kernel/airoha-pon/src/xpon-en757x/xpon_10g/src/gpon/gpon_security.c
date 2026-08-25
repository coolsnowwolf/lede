/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include "common/xpon_global.h"
#include "gpon/gpon_security.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**	function name
**		gf_mulx
**	description:
**		the algorithms needed by the enctyption  aes_128_cmac_vector
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void gf_mulx(u8 *pad)
{
	int i = 0 , carry = 0;

	carry = pad[0] & 0x80;
	for (i = 0; i < AES_BLOCK_SIZE - 1; i++)
		pad[i] = (pad[i] << 1) | (pad[i + 1] >> 7);

	pad[AES_BLOCK_SIZE - 1] <<= 1;
	if (carry)
		pad[AES_BLOCK_SIZE - 1] ^= 0x87;
}

/******************************************************************************
 * omac1_aes_128_vector - One-Key CBC MAC (OMAC1) hash with AES-128
 * @key: 128-bit key for the hash operation
 * @num_elem: Number of elements in the data vector
 * @addr: Pointers to the data areas
 * @len: Lengths of the data blocks
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 ******************************************************************************/
static int aes_128_cmac_vector(struct crypto_cipher *tfm, const u8 *key, size_t num_elem,
		const u8 *addr[], const size_t *len, u8 *mac)
{
	u8 cbc[AES_BLOCK_SIZE] = {0}, pad[AES_BLOCK_SIZE] = {0};
	const u8 *pos = NULL, *end = NULL ;
	size_t i, e, left, total_len;
	int ret = 0;

	ret = crypto_cipher_setkey(tfm, key, AES_CMAC_KEY_LEN);
	if(ret){
		PON_MSG(MSG_ERR, "failed to set key for AES CMAC\n") ;
		return -1;
	}
	memset(cbc, 0, AES_BLOCK_SIZE);

	total_len = 0;
	for (e = 0; e < num_elem; e++)
		total_len += len[e];
	left = total_len;

	e = 0;
	pos = addr[0];
	end = pos + len[0];

	while (left >= AES_BLOCK_SIZE) {
		for (i = 0; i < AES_BLOCK_SIZE; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		if (left > AES_BLOCK_SIZE)
			crypto_cipher_encrypt_one(tfm, cbc, cbc);
		left -= AES_BLOCK_SIZE;
	}

	memset(pad, 0, AES_BLOCK_SIZE);
	crypto_cipher_encrypt_one(tfm, pad, pad);
	gf_mulx(pad);

	if (left || total_len == 0) {
		for (i = 0; i < left; i++) {
			cbc[i] ^= *pos++;
			if (pos >= end) {
				e++;
				pos = addr[e];
				end = pos + len[e];
			}
		}
		cbc[left] ^= 0x80;
		gf_mulx(pad);
	}

	for (i = 0; i < AES_BLOCK_SIZE; i++)
		pad[i] ^= cbc[i];
	
	crypto_cipher_encrypt_one(tfm, pad, pad);
	memcpy(mac, pad, CMAC_TLEN);	
	return 0;
}
/******************************************************************************
 * omac1_aes_128 - One-Key CBC MAC (OMAC1) hash with AES-128 (aka AES-CMAC)
 * @key: 128-bit key for the hash operation
 * @data: Data buffer for which a MAC is determined
 * @data_len: Length of data buffer in bytes
 * @mac: Buffer for MAC (128 bits, i.e., 16 bytes)
 * Returns: 0 on success, -1 on failure
 *
 * This is a mode for using block cipher (AES in this case) for authentication.
 * OMAC1 was standardized with the name CMAC by NIST in a Special Publication
 * (SP) 800-38B.
 ******************************************************************************/
int gpon_aes_cmac_encrypt(struct crypto_cipher *tfm, const u8* key, 
		const u8 *data, size_t data_len, u8 *mac){

	const u8 *addr[3]={0};
	size_t len[3] ={0};

	addr[0] = data;
	len[0] = data_len;
	addr[1] = NULL;
	len[1] = 0;
	addr[2] = NULL;
	len[2] = 0;
	
	return aes_128_cmac_vector(tfm, key, 1, addr, len ,mac);
}
/*______________________________________________________________________________
**	function name
**		gpon_aes_cmac_setup
**	description:
**		setup the aes cmac crypto
**	parameters:
**		none
**	global:
**		None
**	return:
**		tfm: pointer of the crypto transform
**		NULL: failure.
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
struct crypto_cipher * gpon_aes_cmac_setup(void)
{
	struct crypto_cipher *tfm = NULL;

	tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_ASYNC);
	if (IS_ERR(tfm)){
		PON_MSG(MSG_ERR, "failed to load transform for AES CMAC\n") ;
		return NULL;
	}
	
	return tfm;
}
/*______________________________________________________________________________
**	function name
**		gpon_aes_ecb_encrypt
**	description:
**		encrypt data by aes 
**	parameters:
**		none
**	global:
**		None
**	return:
**		0: ok
**		-ohter: failure.
**	call:
**		crypto_cipher_setkey
**		crypto_cipher_encrypt_one
**	revision:
**		v1.0
**____________________________________________________________________________*/
int gpon_aes_ecb_encrypt(struct crypto_cipher *tfm, const u8* key, const u8 *data, size_t data_len, u8 *mac){
	int ret = 0;
	
	ret = crypto_cipher_setkey(tfm, key, AES_ECB_KEY_LEN);
	if(ret){
		PON_MSG(MSG_ERR, "failed to set key for AES ECB\n") ;
		return -1;
	}
	
	crypto_cipher_encrypt_one(tfm, mac, data);

	return 0;
}
/*______________________________________________________________________________
**	function name
**		gpon_aes_ecb_decrypt
**	description:
**		decrypt data by aes 
**	parameters:
**		none
**	global:
**		None
**	return:
**		0: ok
**		-ohter: failure.
**	call:
**		crypto_cipher_setkey
**		crypto_cipher_decrypt_one
**	revision:
**		v1.0
**____________________________________________________________________________*/

int gpon_aes_ecb_decrypt(struct crypto_cipher *tfm, const u8* key, const u8 *data, size_t data_len, u8 *mac){
	int ret = 0;
	
	ret = crypto_cipher_setkey(tfm, key, AES_ECB_KEY_LEN);
	if(ret){
		PON_MSG(MSG_ERR, "failed to set key for AES ECB\n") ;
		return -1;
	}
	
	crypto_cipher_decrypt_one(tfm, mac, data);

	return 0;
}

/*______________________________________________________________________________
**	function name
**		gpon_aes_ecb_setup
**	description:
**		setup the aes ecb crypto
**	parameters:
**		none
**	global:
**		None
**	return:
**		tfm: pointer of the crypto transform
**		NULL: failure.
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
struct crypto_cipher * gpon_aes_ecb_setup(void)
{
	struct crypto_cipher *tfm = NULL;

	tfm = crypto_alloc_cipher("aes", 0, CRYPTO_ALG_TYPE_CIPHER);  /*CRYPTO_ALG_TYPE_CIPHER singer block cipher*/
	if (IS_ERR(tfm)){		
		PON_MSG(MSG_ERR, "failed to load transform for AES ECB\n") ;
		return NULL;
	}
	
	return tfm;
}
/*______________________________________________________________________________
**	function name
**		gpon_aes_cmac_key_free
**	description:
**		free the crypto transform
**	parameters:
**		tfm: the crypto to free
**	global:
**		None
**	return:
**		None
**	call:
**		crypto_free_cipher
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_aes_cmac_key_free(struct crypto_cipher *tfm)
{
	if (tfm)
		crypto_free_cipher(tfm);
}
/*______________________________________________________________________________
**	function name
**		gpon_key_index_change_by_hw
**	description:
**		changer the ploam,omci and key ik in O4
**	parameters:
**		none
**	global:
**		None
**	return:
**		None
**	call:
**		gponDevGetPloamIkIdx
**		gponDevGetOmciIkIdx
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_key_index_change_by_hw(GPON_Security_t *gponSecurity)
{
	unchar ploamIkIdx = 0;
	unchar omciIkIdx = 0;
	
	PON_MSG(MSG_SECUR, "key index change by HW \n");

	if(GPON_CURR_STATE == GPON_10G_STATE_O4){
		gponDevGetPloamIkIdx(&ploamIkIdx);
		gponDevGetOmciIkIdx(&omciIkIdx);
		PON_MSG(MSG_SECUR, "HW ploamIkIdx:%d omciIkId:%d \nSW ploamIkIdx:%d omciIkId:%d \n",\
            ploamIkIdx,omciIkIdx,gponSecurity->ploamIkIdx,gponSecurity->omciIkIdx);
		if((ploamIkIdx != gponSecurity->ploamIkIdx) && (omciIkIdx != gponSecurity->omciIkIdx)){
			gponSecurity->ploamIkIdx = GPON_PLOAM_IK_IDX0;
			gponSecurity->omciIkIdx = GPON_OMCI_IK_IDX0;
            gponSecurity->kekIdx = GPON_KEK_IK_IDX0;
			PON_MSG(MSG_SECUR, "update sw ploamIkIdx to:%d omciIkId to:%d in O4\n",\
                gponSecurity->ploamIkIdx,gponSecurity->omciIkIdx);
		}
        gponSecurity->registerIDState = GPON_REG_ID_REPORTED;
	}	
}
/*______________________________________________________________________________
**	function name
**		gpon_ploamIk_index_change_by_OMCI_base_secure
**	description:
**		changer the ploam ik cased by omci secure mutual authentication
**	parameters:
**		gponSecurity:the security info struct
**	global:
**		None
**	return:
**		None
**	call:
**		gponDevGetPloamIkIdx
**		gponDevSetPloamIkIdx
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_ploamIk_index_change_by_OMCI_base_secure(GPON_Security_t *gponSecurity)
{
	unchar ploamIkIdx = 0;

	PON_MSG(MSG_SECUR, "key index change by OMCI_base secure \n");
	gponDevGetPloamIkIdx(&ploamIkIdx);
    
    gponSecurity->ploamIkIdx = (gponSecurity->ploamIkIdx == GPON_PLOAM_IK_IDX0)? GPON_PLOAM_IK_IDX1 : GPON_PLOAM_IK_IDX0;
    gponSecurity->kekIdx = (gponSecurity->kekIdx == GPON_KEK_IK_IDX0)? GPON_KEK_IK_IDX1 : GPON_KEK_IK_IDX0;
    gponDevSetPloamIkIdx(gponSecurity->ploamIkIdx);   
}
/*______________________________________________________________________________
**	function name
**		gpon_omciIk_index_change_by_OMCI_base_secures
**	description:
**		changer omci  ik cased by omci secure mutual authentication
**	parameters:
**		gponSecurity:the security info struct
**	global:
**		None
**	return:
**		None
**	call:
**		gponDevGetOmciIkIdx
**		gponDevSetOmciIkIdx
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_omciIk_index_change_by_OMCI_base_secure(GPON_Security_t *gponSecurity)
{
	unchar omciIkIdx = 0;

	PON_MSG(MSG_SECUR, "key index change by OMCI_base secure \n");
	gponDevGetOmciIkIdx(&omciIkIdx);
	if(omciIkIdx == GPON_OMCI_IK_IDX0){
		PON_MSG(MSG_SECUR, "omciIkIdx  changed form 0 to 1 \n");
		gponSecurity->omciIkIdx = GPON_OMCI_IK_IDX1;
		gponDevSetOmciIkIdx(GPON_OMCI_IK_IDX1);
	}
	else{
		PON_MSG(MSG_SECUR, "omciIkIdx  changed form 1 to 0 \n");
		gponSecurity->omciIkIdx = GPON_OMCI_IK_IDX0;
		gponDevSetOmciIkIdx(GPON_OMCI_IK_IDX0);
	}
}
/*_____________________________________________________________________________
**      function name: 
**             gpon_encryption_key_change_state
**      descriptions:
**           It's used to change the data encryption state
**      parameters:
**            encryption state
**      global:
**             None
**      return:
**             None
**      call:
**   	          None
**      revision:
**            None
**____________________________________________________________________________
*/
void gpon_encryption_key_change_state(const uint new_state)
{
    uint lastState = 0;

    lastState = gpGponPriv->gponSecurity.state;
    gpGponPriv->gponSecurity.state = new_state;
	PON_MSG(MSG_SECUR, "DATAKEY: Change the current state: KN%d. Last State:KN%d\n", new_state, lastState) ; 
}
/*_____________________________________________________________________________
**      function name: 
**             gpon_security_tk4_timer_expires
**      descriptions:
**           It's used to set the function when the TK4 timer expires
**      parameters:
**             None
**      global:
**             None
**      return:
**             None
**      call:
**   	          gpon_encryption_key_change_state
**   	          gponDevSetAesRxKeyInvalid
**   	          gponDevSetAesRxKeyInvalid
**   	          gponDevClearAesUcKey0
**   	          gponDevClearAesUcKey1
**      revision:
**            None
**____________________________________________________________________________
*/
void gpon_security_tk4_timer_expires(TIMER_FUN_PAAM arg)
{
	PON_MSG((MSG_OAM|MSG_SECUR), "DATAKEY: TK4 timer timeout. \n") ;
    GPON_STOP_TIMER(gpGponPriv->gponSecurity.TK5_timer);/*stop TK5*/
    gpon_encryption_key_change_state(KEY_STATE_KN0);  /*change to KN0*/    
    gponDevSetAesRxKeyInvalid(GPON_AES_UC_FIRST_KEY);  /*clear all unicast key info*/
    gponDevSetAesRxKeyInvalid(GPON_AES_UC_SECOND_KEY);        
    gponDevClearAesUcKey0();
    gponDevClearAesUcKey1();
}
/*_____________________________________________________________________________
**      function name: 
**             gpon_security_tk5_timer_expires
**      descriptions:
**           It's used to set the function when the TK5 timer expires
**      parameters:
**             None
**      global:
**             None
**      return:
**             None
**      call:
**   	          ploam_send_key_report_msg
**   	          GPON_START_TIMER
**      revision:
**            None
**____________________________________________________________________________
*/
void gpon_security_tk5_timer_expires(TIMER_FUN_PAAM arg)
{
    Key_Report_Config_t * keyConfig = &gpGponPriv->gponCfg.keyReport ;
    PON_MSG((MSG_OAM|MSG_SECUR), "DATAKEY: TK5 timer timeout. \n") ;
     /*resend new key report ploam*/
    ploam_send_key_report_msg(keyConfig->seqNo, GPON_UC_KEY_GEN, keyConfig->keyIndex,
            PLOAM_KEY_FRAGMENT_NUM, keyConfig->dataEncryptedKey, PLOAM_NEW_ENCRYPTED_KEY_LEN);
    GPON_START_TIMER(gpGponPriv->gponSecurity.TK5_timer,GPON_SECURITY_TK5_TIMER) ;  /*start TK5*/
}
/*______________________________________________________________________________
**	function name
**		gpon_security_init
**	description:
**		init the ploam and omci ik index,and set up the cmac and ecb crypto transfor
**	parameters:
**		None
**	global:
**		None
**	return:
**		0: scuess
**		-1: fail
**	call:
**		gponDevGetOmciIkIdx
**		gponDevSetOmciIkIdx
**		gpon_security_tk4_timer_expires
**		gpon_security_tk5_timer_expires
**	revision:
**		v1.0
**____________________________________________________________________________*/
int gpon_security_init(void){

	gpGponPriv->gponSecurity.ploamIkIdx = GPON_PLOAM_IK_IDX1;
	gpGponPriv->gponSecurity.omciIkIdx = GPON_OMCI_IK_IDX1;
	gponDevSetPloamIkIdx(GPON_PLOAM_IK_IDX1);
	gponDevSetOmciIkIdx(GPON_OMCI_IK_IDX1);
	
	gpGponPriv->gponSecurity.tfm = gpon_aes_cmac_setup();
	if(gpGponPriv->gponSecurity.tfm == NULL)
		return -1;
	
	gpGponPriv->gponSecurity.aesEcbTfm = gpon_aes_ecb_setup();
	if(gpGponPriv->gponSecurity.tfm == NULL)
		return -1;

	GPON_CREATE_TIMER(&gpGponPriv->gponSecurity.TK4_timer,gpon_security_tk4_timer_expires,GPON_SECURITY_TK4_TIMER);/* 10,0 ms */
	GPON_CREATE_TIMER(&gpGponPriv->gponSecurity.TK5_timer,gpon_security_tk5_timer_expires,GPON_SECURITY_TK5_TIMER);/* 20 ms */

	return 0;
}
/*______________________________________________________________________________
**	function name
**		gpon_security_exit
**	description:
**		free the crtpto tfm and the TK4 and TK5 
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		gpon_aes_cmac_key_free
**		del_timer
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_security_exit(void){
	
	gpon_aes_cmac_key_free(gpGponPriv->gponSecurity.tfm);
	gpon_aes_cmac_key_free(gpGponPriv->gponSecurity.aesEcbTfm);
    del_timer(&gpGponPriv->gponSecurity.TK4_timer);
	del_timer(&gpGponPriv->gponSecurity.TK5_timer);

	return;
}
