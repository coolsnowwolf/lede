#!/bin/sh

failsafe_dropbear () {
	dropbearkey -t rsa -s 1024 -f /tmp/dropbear_failsafe_host_key
	dropbear -r /tmp/dropbear_failsafe_host_key <> /dev/null 2>&1
}

boot_hook_add failsafe failsafe_dropbear
