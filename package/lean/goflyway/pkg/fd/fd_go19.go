//+build go1.9

package fd

import (
	"net"
	"unsafe"
)

func ConnFD(conn net.Conn) (fd int) {
	return int(getNetFD(conn).Field(0).FieldByName("Sysfd").Int())
}

func SetConnFD(conn net.Conn, fd int) {
	addr := getNetFD(conn).Field(0).FieldByName("Sysfd").UnsafeAddr()
	*(*int)(unsafe.Pointer(addr)) = fd
}
