#!/bin/sh

START_LOG="/tmp/openclash_start.log"
LOG_FILE="/tmp/openclash.log"
		
LOG_OUT()
{
	if [ -n "${1}" ]; then
		echo -e "${1}" > $START_LOG
		echo -e "$(date "+%Y-%m-%d %H:%M:%S") ${1}" >> $LOG_FILE
	fi
}

SLOG_CLEAN()
{
	echo "" > $START_LOG
}