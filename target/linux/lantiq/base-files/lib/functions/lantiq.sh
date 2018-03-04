#!/bin/sh

lantiq_is_vdsl_system() {
	grep -qE "system type.*: (VR9|xRX200)" /proc/cpuinfo
}
