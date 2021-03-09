#!/bin/sh
#
# This script is called by dsl_cpe_control whenever there is a DSL event
# and calls any available hotplug script(s) in /etc/hotplug.d/dsl.

exec /sbin/hotplug-call dsl
