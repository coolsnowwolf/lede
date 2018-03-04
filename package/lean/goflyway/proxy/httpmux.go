package proxy

import (
	"time"

	"github.com/coyove/goflyway/pkg/logg"
	"github.com/coyove/tcpmux"

	"io"
	"net"
)

type listenerWrapper struct {
	net.Listener
	proxy *ProxyClient
}

func (l *listenerWrapper) Accept() (net.Conn, error) {

CONTINUE:
	c, err := l.Listener.Accept()
	if err != nil || c == nil {
		logg.E("listener: ", err)

		if isClosedConnErr(err) {
			return nil, err
		}

		goto CONTINUE
	}

	wrapper := &tcpmux.Conn{Conn: c}

	wrapper.SetReadDeadline(time.Now().Add(2000 * time.Millisecond))
	b, err := wrapper.FirstByte()
	wrapper.SetReadDeadline(time.Time{})

	if err != nil {
		if err != io.EOF {
			logg.E("prefetch err: ", err)
		}

		wrapper.Close()
		goto CONTINUE
	}

	switch b {
	case 0x04, 0x05:
		// we are accepting SOCKS4 in case it goes to the HTTP handler
		go l.proxy.handleSocks(wrapper)
		goto CONTINUE
	default:
		return wrapper, err
	}
}
