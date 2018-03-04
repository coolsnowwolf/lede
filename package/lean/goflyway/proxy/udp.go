package proxy

import (
	"fmt"
	"time"

	"github.com/coyove/goflyway/pkg/logg"

	"encoding/binary"
	"io"
	"net"
	"strconv"
	"strings"
)

type uAddr struct {
	ip   net.IP
	host string
	port int
	size int
}

func (a *uAddr) String() string {
	return a.HostString() + ":" + strconv.Itoa(a.port)
}

func (a *uAddr) HostString() string {
	if a.ip != nil {
		if len(a.ip) == net.IPv4len {
			return a.ip.String()
		}
		return "[" + a.ip.String() + "]"
	}

	if strings.Contains(a.host, ":") && a.host[0] != '[' {
		return "[" + a.host + "]"
	}
	return a.host
}

func (a *uAddr) IP() net.IP {
	if a.ip != nil {
		return a.ip
	}

	ip, err := net.ResolveIPAddr("ip", a.host)
	if err != nil {
		return nil
	}

	return ip.IP
}

func (a *uAddr) IsAllZeros() bool {
	if a.ip != nil {
		return a.ip.IsUnspecified() && a.port == 0
	}

	return false
}

func parseUDPHeader(conn net.Conn, buf []byte, omitCheck bool) (method byte, addr *uAddr, err error) {
	var n int

	if buf == nil {
		buf, n = make([]byte, 256+3+1+1+2), 0
		// conn.SetReadDeadline(time.Now().Add(30 * time.Second))
		if n, err = io.ReadAtLeast(conn, buf, 3+1+net.IPv4len+2); err != nil {
			return
		}
	}

	if !omitCheck {
		if buf[0] != socksVersion5 {
			return 0, nil, fmt.Errorf(socksVersionErr, buf[0])
		}

		if buf[1] != 0x01 && buf[1] != 0x03 {
			return 0, nil, fmt.Errorf(socksMethodErr, buf[1])
		}
	}

	addr = &uAddr{}
	switch buf[3] {
	case socksAddrIPv4:
		addr.size = 3 + 1 + net.IPv4len + 2
	case socksAddrIPv6:
		addr.size = 3 + 1 + net.IPv6len + 2
	case socksAddrDomain:
		addr.size = 3 + 1 + 1 + int(buf[4]) + 2
	default:
		return 0, nil, fmt.Errorf(socksAddressErr, buf[3])
	}

	if conn != nil {
		if _, err = io.ReadFull(conn, buf[n:addr.size]); err != nil {
			return
		}
	} else {
		if len(buf) < addr.size {
			return 0, nil, io.EOF
		}
	}

	rawaddr := buf[3 : addr.size-2]
	addr.port = int(binary.BigEndian.Uint16(buf[addr.size-2 : addr.size]))

	switch buf[3] {
	case socksAddrIPv4:
		addr.ip = net.IP(rawaddr[1:])
	case socksAddrIPv6:
		addr.ip = net.IP(rawaddr[1:])
	default:
		addr.host = string(rawaddr[2:])
	}

	return buf[1], addr, nil
}

type udpBridgeConn struct {
	*net.UDPConn
	udpSrc net.Addr

	initBuf []byte

	waitingMore struct {
		incompleteLen bool

		buf    []byte
		remain int
	}

	socks bool
	dst   *uAddr

	closed bool
}

func (c *udpBridgeConn) Read(b []byte) (n int, err error) {
	const expectedMaxPacketSize = 2050
	if len(b) < expectedMaxPacketSize {
		panic(fmt.Sprintf("goflyway expects that all UDP packet must be smaller than %d bytes", expectedMaxPacketSize-2))
	}

	if c.initBuf != nil {
		n = len(c.initBuf)
		copy(b[2:], c.initBuf)
		c.initBuf = nil
		goto PUT_HEADER
	}

	n, c.udpSrc, err = c.UDPConn.ReadFrom(b) // We assume that src never change
	if err != nil {
		return
	}

	if c.socks {
		_, dst, err := parseUDPHeader(nil, b[:n], true)
		if err != nil {
			return 0, err
		}

		copy(b[2:], b[dst.size:n])
		n -= dst.size
	} else {
		copy(b[2:], b[:n])
	}

PUT_HEADER:
	binary.BigEndian.PutUint16(b, uint16(n))
	logg.D("UDP read ", n, " bytes")
	return n + 2, err
}

func (c *udpBridgeConn) write(b []byte) (n int, err error) {
	if !c.socks {
		n, err = c.UDPConn.Write(b)
		if err == nil {
			n += 2
		}

		return
	}

	if c.udpSrc == nil {
		logg.W("UDP early write")
		return
	}

	xbuf := make([]byte, len(b)+256)
	ln := 0

	if c.dst.host != "" {
		hl := len(c.dst.host)

		xbuf[3] = 0x03
		xbuf[4] = byte(hl)
		copy(xbuf[5:], []byte(c.dst.host))

		binary.BigEndian.PutUint16(xbuf[5+hl:], uint16(c.dst.port))
		ln = 5 + hl + 2
		copy(xbuf[ln:], b)
		//
	} else if len(c.dst.ip) == net.IPv4len {
		ln = len(udpHeaderIPv4)

		copy(xbuf, udpHeaderIPv4)
		copy(xbuf[4:8], c.dst.ip)
		binary.BigEndian.PutUint16(xbuf[8:], uint16(c.dst.port))
		copy(xbuf[ln:], b)

	} else {
		ln = len(udpHeaderIPv6)

		copy(xbuf, udpHeaderIPv6)
		copy(xbuf[4:20], c.dst.ip)
		binary.BigEndian.PutUint16(xbuf[20:], uint16(c.dst.port))
		copy(xbuf[ln:], b)
	}

	n, err = c.WriteTo(xbuf[:ln+len(b)], c.udpSrc)
	if err == nil {
		n += 2 - ln
	}
	return
}

func (c *udpBridgeConn) Write(b []byte) (n int, err error) {
	// For simplicity, when return "n", it should always equal to the total length of "b" if writing succeeded
	// No ErrShortWrite shall happen
	defer func() {
		if err == nil {
			n = len(b)
			logg.D("UDP write ", n, " bytes")
		} else {
			logg.D("UDP write error: ", err)
		}
	}()

	if b == nil || len(b) == 0 {
		return
	}

	var ln int
	var buf []byte

	if c.waitingMore.incompleteLen {
		c.waitingMore.incompleteLen = false

		ln = int(binary.BigEndian.Uint16([]byte{c.waitingMore.buf[0], b[0]}))
		buf = b[1:]

		goto TEST
	}

	if c.waitingMore.remain > 0 {
		remain := c.waitingMore.remain
		c.waitingMore.remain -= len(b)

		if c.waitingMore.remain == 0 {
			// Best case
			return c.write(append(c.waitingMore.buf, b...))
		}

		if c.waitingMore.remain > 0 {
			// We still don't have enough data to write
			c.waitingMore.buf = append(c.waitingMore.buf, b...)
			return len(b), nil
		}

		// b contains more than what we need
		if n, err = c.write(append(c.waitingMore.buf, b[:remain]...)); err != nil {
			return
		}

		b = b[remain:]
		// Let's deal with the trailing bytes
	}

	if len(b) == 1 {
		logg.D("rare case: incomplete header")
		c.waitingMore.buf = b
		c.waitingMore.incompleteLen = true // "len" should have 2 bytes, we got 1
		return 1, nil
	}

	ln = int(binary.BigEndian.Uint16(b))
	buf = b[2:]

TEST:
	if len(buf) < ln {
		logg.D("rare case: incomplete buffer")
		c.waitingMore.buf = buf
		c.waitingMore.remain = ln - len(buf)
		return len(b), nil
	}

	if len(buf) == ln {
		return c.write(buf)
	}

	// len(buf) > ln
	logg.D("rare case: overflow buffer")
	if n, err = c.write(buf[:ln]); err != nil {
		return
	}

	return c.Write(buf[ln:])
}

func (c *udpBridgeConn) Close() error {
	c.closed = true
	return c.UDPConn.Close()
}

func (proxy *ProxyClient) handleUDPtoTCP(relay *net.UDPConn, client net.Conn) {
	defer relay.Close()
	defer client.Close()

	// prepare the response to answer the client
	response, port := make([]byte, len(okSOCKS)), relay.LocalAddr().(*net.UDPAddr).Port

	copy(response, okSOCKS)
	binary.BigEndian.PutUint16(response[8:], uint16(port))
	client.Write(response)

	buf := make([]byte, 2048)
	n, src, err := relay.ReadFrom(buf)
	if err != nil {
		logg.E("can't read initial packet: ", err)
		return
	}

	_, dst, err := parseUDPHeader(nil, buf[:n], true)
	if err != nil {
		logg.E(err)
		return
	}

	logg.D("UDP relay listening port: ", port, ", destination: ", dst.String())

	maxConns := proxy.UDPRelayCoconn
	srcs := make([]*udpBridgeConn, maxConns)
	conns := make([]net.Conn, maxConns)

	for i := 0; i < maxConns; i++ {
		srcs[i] = &udpBridgeConn{
			UDPConn: relay,
			socks:   true,
			udpSrc:  src,
			dst:     dst,
		}

		if i == 0 {
			// The first connection will be responsible for sending the initial buffer
			srcs[0].initBuf = buf[dst.size:n]
		}

		if proxy.Policy.IsSet(PolicyWebSocket) {
			conns[i] = proxy.dialUpstreamAndBridgeWS(srcs[i], dst.String(), nil, doUDPRelay)
		} else {
			conns[i] = proxy.dialUpstreamAndBridge(srcs[i], dst.String(), nil, doUDPRelay)
		}
	}

	// Connections may be double closed, so we manually check them
	for {
		count := 0
		for _, src := range srcs {
			if src.closed {
				count++
			}
		}

		if count == maxConns {
			break
		}

		time.Sleep(time.Second)
	}

	logg.D("closing relay port: ", port)
}
