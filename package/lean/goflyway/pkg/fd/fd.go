package fd

import (
	"net"
	"reflect"
	"syscall"
)

func getNetFD(conn net.Conn) reflect.Value {
	return reflect.ValueOf(conn).Elem().Field(0).Field(0).Elem()
}

func toSockaddr(addr *net.TCPAddr) syscall.Sockaddr {
	if len(addr.IP) == net.IPv4len {
		sa := &syscall.SockaddrInet4{Port: addr.Port}
		copy(sa.Addr[:], addr.IP.To4())
		return sa
	}

	sa := &syscall.SockaddrInet6{Port: addr.Port}
	copy(sa.Addr[:], addr.IP.To16())
	return sa
}
