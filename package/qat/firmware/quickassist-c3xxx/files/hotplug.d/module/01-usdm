#!/bin/sh

HUGE_PAGE_DIR="/dev/hugepages"

[ "$DEVICENAME" != "usdm_drv" ] && exit 0

[ "$ACTION" == "add" ] && {

	if [ -d ${HUGE_PAGE_DIR} ]; then

		mkdir ${HUGE_PAGE_DIR}/qat 2> /dev/null

		if [ $? -ne 0]; then
			logger -t "quickassist(usdm_drv): error creating ${HUGE_PAGE_DIR}/qat"
		else
			chgrp qat ${HUGE_PAGE_DIR}/qat
			chmod 0770 ${HUGE_PAGE_DIR}/qat
		fi

	else
		logger -t "quickassist(usdm_drv): ${HUGE_PAGE_DIR} not found"
		exit 1
	fi


}

[ "$ACTION" == "remove" ] && {

	rmdir ${HUGE_PAGE_DIR}/qat

}
