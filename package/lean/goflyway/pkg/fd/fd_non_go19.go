//+build !go1.9

package fd

import (
	"net"
	"unsafe"
)

func ConnFD(conn net.Conn) (fd int) {
	return int(getNetFD(conn).FieldByName("sysfd").Int())
}

func SetConnFD(conn net.Conn, fd int) {
	addr := getNetFD(conn).FieldByName("sysfd").UnsafeAddr()
	*(*int)(unsafe.Pointer(addr)) = fd
}
