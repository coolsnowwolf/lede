#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include "api/mgr_api.h"
#include "hal_itf_ctqos.h"

/**
* To creat a qos policer
* @param  policer_info 
* @see  hal_qos_policer_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/

int hal_hgu_qos_create_policer(hal_qos_policer_t policer_info)
{
	int ret = EXEC_OK;
	struct XMCS_QosPolicerCreat_S QosPolicerCreat;

	memset(&QosPolicerCreat, 0, sizeof(struct XMCS_QosPolicerCreat_S));
    QosPolicerCreat.policer_id = policer_info.policer_id;
	QosPolicerCreat.cir = policer_info.cir;

	ret = XMCS_IOCTL_SDI(IO_IOS_QOS_CREAT_POLICER, &QosPolicerCreat);
	if(EXEC_OK != ret)
	{
		return HAL_RET_FAILED;
	}

	return HAL_RET_SUCCESS;
}

/**
* To delete a qos policer
* @param  policer_id 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
int hal_hgu_qos_delete_policer(int policer_id)
{
	int ret = EXEC_OK;
	struct XMCS_QosPolicerDelete_S QosPolicerDelete;

	memset(&QosPolicerDelete, 0, sizeof(struct XMCS_QosPolicerDelete_S));
    QosPolicerDelete.policer_id = policer_id;
	
	ret = XMCS_IOCTL_SDI(IO_IOS_QOS_DELETE_POLICER, &QosPolicerDelete);
	if(EXEC_OK != ret)
	{
		return HAL_RET_FAILED;
	}

	return HAL_RET_SUCCESS;
}

/**
* To config the uplink overall ratelimit
* @param  bandwidth
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
int hal_hgu_qos_config_overall_ratelimit(int bandwidth)
{
	int ret = EXEC_OK;
	struct XMCS_OverallRatelimitConfig_S OverallRatelimit;

	memset(&OverallRatelimit, 0, sizeof(struct XMCS_OverallRatelimitConfig_S));
    OverallRatelimit.bandwidth = bandwidth;
	
	ret = XMCS_IOCTL_SDI(IO_IOS_QOS_OVERALL_RATELIMIT_CONFIG, &OverallRatelimit);
	if(EXEC_OK != ret)
	{
		return HAL_RET_FAILED;
	}

	return HAL_RET_SUCCESS;
}
