#!/bin/sh
#
# safety wrapper for IBR-DTN daemon
#
# Tasks:
#  * start IBR-DTN daemon
#  * restart the daemon after a crash
#  * if respawning to fast, then slow down with backoff
#  * check for enough space on disk and delete bundles if necessary.
#  * clean the blob directory on startup
#

DTND=/usr/sbin/dtnd
TMPCONF=/tmp/ibrdtn.config
UCI=/sbin/uci

getstate() {
	$UCI -P/var/state -q get ibrdtn.$1
	return $?
}

setstate() {
	$UCI -P/var/state -q set ibrdtn.$1=$2
	return $?
}

getconfig() {
	$UCI -q get ibrdtn.$1
	return $?
}

setconfig() {
	$UCI -q set ibrdtn.$1=$2
	return $?
}

# remove the old state file
/bin/rm /var/state/ibrdtn

# read uci configuration
BLOB_PATH=`getconfig storage.blobs`
BUNDLE_PATH=`getconfig storage.bundles`
CONTAINER_PATH=`getconfig storage.path`
CONTAINER_FILE=`getconfig storage.container`
LOG_FILE=`getconfig main.logfile`
ERR_FILE=`getconfig main.errfile`
DEBUG_LEVEL=`getconfig main.debug`
SAFEMODE=no

# run a system check
/bin/sh /usr/share/ibrdtn/systemcheck.sh

if [ $? -eq 0 ]; then
	# mount container if specified
	if [ -n "$CONTAINER_FILE" ] && [ -n "$CONTAINER_PATH" ]; then
		/bin/sh /usr/share/ibrdtn/mountcontainer.sh

		# if the mount of the container failed
		# switch to safe mode!
		if [ $? -gt 0 ]; then
			SAFEMODE=yes
		fi
	fi
else
	SAFEMODE=yes
fi

# create blob & bundle path
if [ -n "$BLOB_PATH" ]; then
	/bin/mkdir -p $BLOB_PATH

	# clean the blob directory on startup
	/bin/rm -f $BLOB_PATH/file*
fi

if [ -n "$BUNDLE_PATH" ]; then
	/bin/mkdir -p $BUNDLE_PATH
fi

LOGGING=""
if [ -n "$LOG_FILE" ]; then
	LOGGING="$LOGGING > $LOG_FILE"
else
	LOGGING="$LOGGING > /dev/null"
fi

if [ -n "$ERR_FILE" ]; then
	LOGGING="$LOGGING 2> $ERR_FILE"
else
	LOGGING="$LOGGING 2> /dev/null"
fi

if [ -z "$LOG_FILE" ] && [ -z "$ERR_FILE" ]; then
	LOGGING="-q"
fi

# check for debugging option
if [ -n "$DEBUG_LEVEL" ]; then
	DEBUG_ARGS="-v -d ${DEBUG_LEVEL}"
else
	DEBUG_ARGS=""
fi

# create configuration
if [ "$SAFEMODE" == "yes" ]; then
	/bin/sh /usr/share/ibrdtn/build-config.sh --safe-mode $TMPCONF
else
	/bin/sh /usr/share/ibrdtn/build-config.sh $TMPCONF
fi

# set the crash counter to zero
CRASH=0

# run the daemon
setstate state running

while [ "`getstate state`" == "running" ]; do
	# run a system check
	/bin/sh /usr/share/ibrdtn/systemcheck.sh

	# run in safe mode if the system check has failed
	if [ $? -gt 0 ] && [ "$SAFEMODE" == "no" ]; then
		SAFEMODE=yes
		/usr/bin/logger -t "ibrdtn-safe-wrapper" -p 2 "system check failed! Switch to safe-mode settings."
		/bin/sh /usr/share/ibrdtn/build-config.sh --safe-mode $TMPCONF
	fi

	# measure the running time
	TIMESTART=`/bin/date +%s`
	
	# run the daemon
	echo "${DTND} ${DEBUG_ARGS} -c ${TMPCONF} ${LOGGING}" | /bin/sh
	
	# measure the stopping time
	TIMESTOP=`/bin/date +%s`
	
	# calc the running time
	let TIMERUN=$TIMESTOP-$TIMESTART
	
	# reset the CRASH counter if there is one hour between the crashes
	if [ $TIMERUN -ge 3600 ]; then
		CRASH=0
	fi
	
	# check if the daemon is crashed
	if [ "`getstate state`" == "running" ]; then
		# if the crash counter is higher than 20 switch to safe-mode settings
		if [ $CRASH -eq 20 ] && [ "$SAFEMODE" == "no" ]; then
			SAFEMODE=yes
			/usr/bin/logger -t "ibrdtn-safe-wrapper" -p 2 "IBR-DTN daemon crashed 20 times! Switch to safe-mode settings."
			/bin/sh /usr/share/ibrdtn/build-config.sh --safe-mode $TMPCONF
		fi

		# increment the crash counter
		let CRASH=$CRASH+1
		
		# backoff wait timer
		let WAIT=2**$CRASH
		
		# set a upper limit for the wait time
		if [ $WAIT -ge 1800 ]; then
			WAIT=1800
		fi

		# log the crash
		/usr/bin/logger -t "ibrdtn-safe-wrapper" -p 2 "IBR-DTN daemon crashed $CRASH times! Wait $WAIT seconds."
		
		# wait sometime
		/bin/sleep $WAIT
	fi
done

