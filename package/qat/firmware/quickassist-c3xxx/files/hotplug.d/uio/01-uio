#!/bin/sh

if [ "$ACTION" != "add" ] && [ "$MAJOR" != "249" ]; then exit 0; fi

chgrp qat /dev/${DEVICENAME}
chmod 0660 /dev/${DEVICENAME}

