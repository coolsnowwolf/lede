#!/bin/sh
#
# This script creates a bundle storage of a given size.
#
# $1 = container file
# $2 = size of the container in MB
#

help_message() {
	echo "usage: "
	echo " $0 <container file> <size in MB>"
}

if [ $# -le 1 ]; then
	help_message
	exit 1
fi

CONTAINER=$(cd "$(dirname "$1")"; pwd)/$(basename $1)
SIZE=$2

# check if the container already exists
if [ -f $CONTAINER ]; then
	echo "Aborted! The specified container already exists."
	exit 1
fi

# create the container
echo -n "creating the container file..."
/bin/dd if=/dev/zero of=$CONTAINER bs=1M count=$SIZE >/dev/null 2>/dev/null
echo " done" 

# create file system
echo -n "initializing ext3 filesystem for the container..."
/usr/sbin/mkfs.ext3 -q -F $CONTAINER > /dev/null
echo " done"

# final hint
echo "The container is now ready. To use it with IBR-DTN set the container with:"
echo "# uci set ibrdtn.storage.container=$CONTAINER"
echo "# uci set ibrdtn.storage.container_size=$SIZE"

exit 0
