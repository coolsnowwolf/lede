package rand

import (
	"syscall"
	"time"
	"unsafe"
)

var qpc uintptr

func init() {
	k32, _ := syscall.LoadLibrary("kernel32.dll")
	qpc, _ = syscall.GetProcAddress(k32, "QueryPerformanceCounter")
}

func GetCounter() (k int64) {
	syscall.Syscall(qpc, 1, uintptr(unsafe.Pointer(&k)), 0, 0)

	if k == 0 {
		k = time.Now().UnixNano()
	}

	return
}
