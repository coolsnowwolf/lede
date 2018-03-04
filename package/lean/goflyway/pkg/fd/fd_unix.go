// +build darwin dragonfly freebsd linux nacl netbsd openbsd solaris

package fd

import (
	"net"
	"syscall"
)

func Socket(family int) (int, error) {
	fd, err := syscall.Socket(family, syscall.SOCK_STREAM, 0)
	if err != nil {
		return 0, err
	}

	if err := syscall.SetsockoptInt(fd, syscall.SOL_SOCKET, syscall.SO_REUSEADDR, 1); err != nil {
		return 0, err
	}

	return fd, nil
}

func DialWithFD(sock int, address string) (net.Conn, error) {
	conn, err := net.Dial("tcp", address)
	if err != nil {
		return nil, err
	}

	if err := syscall.Connect(sock, toSockaddr(conn.RemoteAddr().(*net.TCPAddr))); err != nil {
		return nil, err
	}

	oldfd := ConnFD(conn)
	SetConnFD(conn, sock)
	syscall.Close(oldfd)

	return conn, err
}
