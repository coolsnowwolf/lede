package rand

import (
	"syscall"
	"unsafe"
)

var CLOCK_MONOTONIC = 1

func GetCounter() int64 {
	var ts syscall.Timespec
	syscall.Syscall(syscall.SYS_CLOCK_GETTIME, uintptr(CLOCK_MONOTONIC), uintptr(unsafe.Pointer(&ts)), 0)

	// even we are calling ts.Nano() here, we just want a high reoslution int64 number, not a precise time
	return ts.Nano()
}
