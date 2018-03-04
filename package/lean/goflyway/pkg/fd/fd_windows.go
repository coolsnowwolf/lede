package fd

import (
	"net"
	"syscall"
)

func Socket(family int) (syscall.Handle, error) {
	return syscall.Handle(0), nil
}

func DialWithFD(sock syscall.Handle, address string) (net.Conn, error) {
	return net.Dial("tcp", address)
}
