// +build darwin dragonfly freebsd linux netbsd openbsd solaris

package proxy

import (
	"errors"
	"net"
	"strconv"
	"syscall"
	"unsafe"

	"github.com/coyove/goflyway/pkg/fd"
)

func vpnDial(address string) (net.Conn, error) {
	sock, err := fd.Socket(syscall.AF_INET)
	if err != nil {
		return nil, err
	}

	// Send our conn fd to shadowsocks vpn thread for protection
	if err := protectFD(sock); err != nil {
		return nil, err
	}

	// If succeeded, this fd will be closed while we still need it.
	// So we dial a new conn, replace its fd with this one
	return fd.DialWithFD(sock, address)
}

func protectFD(fd int) error {
	sock, err := syscall.Socket(syscall.AF_UNIX, syscall.SOCK_STREAM, 0)
	if err != nil {
		return err
	}

	var addr syscall.SockaddrUnix
	addr.Name = "protect_path"

	if err := (syscall.Connect(sock, &addr)); err != nil {
		return err
	}

	if err := sendFD(sock, fd); err != nil {
		return err
	}

	ret := []byte{9}
	if n, err := syscall.Read(sock, ret); err != nil {
		return err
	} else if n != 1 {
		return errors.New("protecting failed")
	}

	syscall.Close(sock)

	if ret[0] != 0 {
		return errors.New("protecting failed")
	}

	return nil
}

var _int_value_one int = 1
var _little_endian = *(*byte)(unsafe.Pointer(&_int_value_one)) == 1

func sendFD(sock int, fd int) error {
	cmsg := &syscall.Cmsghdr{
		Level: syscall.SOL_SOCKET,
		Type:  syscall.SCM_RIGHTS,
	}

	const hdrsize = syscall.SizeofCmsghdr
	ln := byte(hdrsize + strconv.IntSize/8)
	h := (*[8]byte)(unsafe.Pointer(&cmsg.Len))

	if _little_endian {
		h[0] = ln
	} else {
		h[3+4*(^cmsg.Len<<32>>63)] = ln
	}

	buffer := make([]byte, cmsg.Len)

	copy(buffer, (*[hdrsize]byte)(unsafe.Pointer(cmsg))[:])
	*(*int)(unsafe.Pointer(&buffer[hdrsize])) = fd

	return syscall.Sendmsg(sock, []byte{'!'}, buffer, nil, 0)
}
