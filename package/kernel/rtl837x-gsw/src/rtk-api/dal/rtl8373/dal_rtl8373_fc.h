#ifndef __DAL_RTL8373_FC_H__
#define __DAL_RTL8373_FC_H__


/* Function Name:
 *      dal_rtl8373_asicFCPubPage_set
 * Description:
 *      Set public page number
 * Input:
 *      number     - public page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPubPage_set(rtk_uint32 number);



/* Function Name:
 *      dal_rtl8373_asicFCPubPage_get
 * Description:
 *      Get public page number
 * Input:
 *      number     - public page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPubPage_get(rtk_uint32* pNumber);


/* Function Name:
 *      dal_rtl8373_asicFCDropAll_set
 * Description:
 *      Set drop all  page number
 * Input:
 *      number     - drop all page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCDropAll_set(rtk_uint32 number);



/* Function Name:
 *      dal_rtl8373_asicFCDropAll_get
 * Description:
 *      get drop all  page number
 * Input:
 *      number     - drop all page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCDropAll_get(rtk_uint32* pNumber);




/* Function Name:
 *      dal_rtl8373_asicFCOnHiThr_set
 * Description:
 *      Set flow control on high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnHiThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOnHiThr_get
 * Description:
 *      Get flow control on high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnHiThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOnLoThr_set
 * Description:
 *      Set flow control on low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnLoThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOnLoThr_get
 * Description:
 *      Get flow control on low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnLoThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber);



/* Function Name:
 *      dal_rtl8373_asicFCOffHiThr_set
 * Description:
 *      Set flow control off high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffHiThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOffHiThr_get
 * Description:
 *      Get flow control off high threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffHiThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber);



/* Function Name:
 *      dal_rtl8373_asicFCOffLoThr_set
 * Description:
 *      Set flow control off low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffLoThr_set(rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOffLoThr_get
 * Description:
 *      Get flow control off low threshold  page number
 * Input:
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffLoThr_get(rtk_uint32* onNumber, rtk_uint32* offNumber);




/* Function Name:
 *      dal_rtl8373_asicFCOnPortHiThr_set
 * Description:
 *      Set flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnPortHiThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOnPortHiThr_get
 * Description:
 *      Get flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnPortHiThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOnPortLoThr_set
 * Description:
 *      Set flow control on low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnPortLoThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOnPortLoThr_get
 * Description:
 *      Get flow control on low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOnPortLoThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber);



/* Function Name:
 *      dal_rtl8373_asicFCOffPortHiThr_set
 * Description:
 *      Set flow control off high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffPortHiThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOffPortHiThr_get
 * Description:
 *      Get flow control off high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffPortHiThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber);



/* Function Name:
 *      dal_rtl8373_asicFCOffPortLoThr_set
 * Description:
 *      Set flow control off low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffPortLoThr_set(rtk_uint32 index, rtk_uint32 onNumber, rtk_uint32 offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCOffPortLoThr_get
 * Description:
 *      Get flow control off low threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCOffPortLoThr_get(rtk_uint32 index, rtk_uint32* onNumber, rtk_uint32* offNumber);



/* Function Name:
 *      dal_rtl8373_asicFCPortGua_set
 * Description:
 *      Set port guarantee page
 * Input:
 *      index           - 0 ~ 3
 *      number        - guarantee
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortGua_set(rtk_uint32 index, rtk_uint32 number);




/* Function Name:
 *      dal_rtl8373_asicFCPortGua_get
 * Description:
 *      Set port guarantee page
 * Input:
 *      index           - 0 ~ 3
 *      number        - guarantee
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortGua_get(rtk_uint32 index, rtk_uint32* pNumber);


/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_set
 * Description:
 *      Set port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortThrSel_set(rtk_uint32 port, rtk_uint32 index);


/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_get
 * Description:
 *      Set port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortThrSel_get(rtk_uint32 port, rtk_uint32* pIndex);



/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_set
 * Description:
 *      Set port threshold select
 * Input:
 *      type             - 0:unknown unicast     1: l2 multicast      2: broadcast
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCHOLPrvnt_set(rtk_uint32 type, rtk_uint32 enable);



/* Function Name:
 *      dal_rtl8373_asicFCPortThrSel_get
 * Description:
 *      Set port threshold select
 * Input:
 *      type             - 0:unknown unicast     1: l2 multicast      2: broadcast
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCHOLPrvnt_get(rtk_uint32 type, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_asicFCPortHOLPrvntEn_set
 * Description:
 *      Set port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortHOLPrvntEn_set(rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_asicFCPortHOLPrvntEn_get
 * Description:
 *      Get port threshold select
 * Input:
 *      port             - 0 ~ 9
 *      enable           - 0:disable hol prevention    1: enable hol prevention
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortHOLPrvntEn_get(rtk_uint32 port, rtk_uint32* enable);



/* Function Name:
 *      dal_rtl8373_asicFCPortAct_set
 * Description:
 *      Set port flow control action
 * Input:
 *      portnum             - 0 ~ 9
 *      act                    - 0:receive    1: drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortAct_set(rtk_uint32 portnum, rtk_uint32 act);

/* Function Name:
 *      dal_rtl8373_asicFCPortAct_get
 * Description:
 *      Get port flow control action
 * Input:
 *      portnum             - 0 ~ 9
 *      act                    - 0:receive    1: drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortAct_get(rtk_uint32 portnum, rtk_uint32* pAct);



/* Function Name:
 *      dal_rtl8373_asicFCPortAllowPage_set
 * Description:
 *      Set port allow page number
 * Input:
 *      portnum             - 0 ~ 9
 *      pagenum           - allow page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortAllowPage_set(rtk_uint32 portnum, rtk_uint32 pagenum);



/* Function Name:
 *      dal_rtl8373_asicFCPortAllowPage_get
 * Description:
 *      Get port allow page number
 * Input:
 *      portnum             - 0 ~ 9
 *      pagenum           - allow page number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortAllowPage_get(rtk_uint32 portnum, rtk_uint32* pPagenum);


/* Function Name:
 *      dal_rtl8373_asicFCQEgrDropThr_set
 * Description:
 *      Set flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      queueid       - 0 ~ 7
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCQEgrDropThr_set(rtk_uint32 index, rtk_uint32 queueid, rtk_uint32 onNumber, rtk_uint32 offNumber);



/* Function Name:
 *      dal_rtl8373_asicFCQEgrDropThr_get
 * Description:
 *      Get flow control on high threshold  page number
 * Input:
 *      index           - 0 ~ 3
 *      queueid       - 0 ~ 7
 *      onNumber     - pause on number
 *      offNumber     - pause off number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCQEgrDropThr_get(rtk_uint32 index, rtk_uint32 queueid, rtk_uint32* onNumber, rtk_uint32* offNumber);


/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropThrSel_set
 * Description:
 *      Select port queue egress drop threshold
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortQEgrDropThrSel_set(rtk_uint32 port, rtk_uint32 index);


/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropThrSel_get
 * Description:
 *      Gelect port queue egress drop threshold
 * Input:
 *      port             - 0 ~ 9
 *      index           - 0 ~ 3
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortQEgrDropThrSel_get(rtk_uint32 port, rtk_uint32* pIndex);


/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropEn_set
 * Description:
 *      Set port queue egress drop enable
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortQEgrDropEn_set(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enable);



/* Function Name:
 *      dal_rtl8373_asicFCPortQEgrDropEn_get
 * Description:
 *      Get port queue egress drop enable
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortQEgrDropEn_get(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* enable);



/* Function Name:
 *      dal_rtl8373_asicFCPortQForceEgrDrop_set
 * Description:
 *      Set port queue force egress drop ability
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortQForceEgrDrop_set(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enable);

/* Function Name:
 *      dal_rtl8373_asicFCPortQForceEgrDrop_get
 * Description:
 *      Get port queue force egress drop ability
 * Input:
 *      port             - 0 ~ 9
 *      qid              - 0 ~ 7
 *      enable         - 1: enable    0: disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_asicFCPortQForceEgrDrop_get(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* enable);




#endif
