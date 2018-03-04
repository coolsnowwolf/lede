package proxy

import (
	"crypto/tls"

	acr "github.com/coyove/goflyway/pkg/aclrouter"
	"github.com/coyove/goflyway/pkg/logg"
	"github.com/coyove/goflyway/pkg/lru"
	"github.com/coyove/tcpmux"

	"bytes"
	"encoding/base64"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/url"
	"strconv"
	"strings"
	"time"
)

type ClientConfig struct {
	Upstream string
	Policy   Options
	UserAuth string

	Connect2     string
	Connect2Auth string
	DummyDomain  string
	URLHeader    string

	UDPRelayCoconn int

	Mux int

	DNSCache *lru.Cache
	CA       tls.Certificate
	CACache  *lru.Cache
	ACL      *acr.ACL

	*Cipher
}

type ProxyClient struct {
	*ClientConfig

	rkeyHeader string
	tp         *http.Transport // to upstream
	tpq        *http.Transport // to upstream used for dns query
	tpd        *http.Transport // to host directly
	dummies    *lru.Cache
	pool       *tcpmux.DialPool

	Localaddr string
	Listener  *listenerWrapper
}

func (proxy *ProxyClient) dialUpstream() (net.Conn, error) {
	lat := time.Now().UnixNano()
	if proxy.Connect2 == "" {
		upstreamConn, err := proxy.pool.DialTimeout(timeoutDial)
		if err != nil {
			return nil, err
		}

		proxy.IO.Tr.AddLatency(time.Now().UnixNano() - lat)
		return upstreamConn, nil
	}

	connectConn, err := net.DialTimeout("tcp", proxy.Connect2, timeoutDial)
	if err != nil {
		return nil, err
	}

	up, auth := proxy.Upstream, ""
	if proxy.Connect2Auth != "" {
		// if proxy.Connect2Auth == "socks5" {
		// 	connectConn.SetWriteDeadline(time.Now().Add(timeoutOp))
		// 	if _, err = connectConn.Write(SOCKS5_HANDSHAKE); err != nil {
		// 		connectConn.Close()
		// 		return nil, err
		// 	}

		// 	buf := make([]byte, 263)
		// 	connectConn.SetReadDeadline(time.Now().Add(timeoutOp))
		// 	if _, err = connectConn.Read(buf); err != nil {
		// 		connectConn.Close()
		// 		return nil, err
		// 	}

		// 	if buf[0] != socksVersion5 || buf[1] != 0 {
		// 		connectConn.Close()
		// 		return nil, errors.New("unsupported SOCKS5 authentication: " + strconv.Itoa(int(buf[1])))
		// 	}

		// 	host, _port, err := net.SplitHostPort(proxy.Upstream)
		// 	port, _ := strconv.Atoi(_port)
		// 	if err != nil {
		// 		connectConn.Close()
		// 		return nil, err
		// 	}

		// 	payload := []byte{socksVersion5, 1, 0, socksAddrDomain, byte(len(host))}
		// 	payload = append(payload, []byte(host)...)
		// 	payload = append(payload, 0, 0)
		// 	binary.BigEndian.PutUint16(payload[len(payload)-2:], uint16(port))

		// 	connectConn.SetWriteDeadline(time.Now().Add(timeoutOp))
		// 	if _, err = connectConn.Write(payload); err != nil {
		// 		connectConn.Close()
		// 		return nil, err
		// 	}

		// 	connectConn.SetReadDeadline(time.Now().Add(timeoutOp))
		// 	if n, err := io.ReadAtLeast(connectConn, buf, 5); err != nil || n < 5 {
		// 		connectConn.Close()
		// 		return nil, err
		// 	}

		// 	if buf[1] != 0 {
		// 		connectConn.Close()
		// 		return nil, errors.New("SOCKS5 returned error: " + strconv.Itoa(int(buf[1])))
		// 	}

		// 	ln := 0
		// 	switch buf[3] {
		// 	case socksAddrIPv4:
		// 		ln = net.IPv4len - 1 + 2
		// 	case socksAddrIPv6:
		// 		ln = net.IPv6len - 1 + 2
		// 	case socksAddrDomain:
		// 		ln = int(buf[4]) + 2
		// 	default:
		// 		connectConn.Close()
		// 		return nil, errors.New("unexpected address type: " + strconv.Itoa(int(buf[3])))
		// 	}

		// 	connectConn.SetReadDeadline(time.Now().Add(timeoutOp))
		// 	if n, err := io.ReadAtLeast(connectConn, buf, ln); err != nil || n < ln {
		// 		connectConn.Close()
		// 		return nil, err
		// 	}

		// 	return connectConn, nil
		// }

		x := base64.StdEncoding.EncodeToString([]byte(proxy.Connect2Auth))
		auth = fmt.Sprintf("Proxy-Authorization: Basic %s\r\nAuthorization: Basic %s\r\n", x, x)
	}

	connect := fmt.Sprintf("CONNECT %s HTTP/1.1\r\nHost: %s\r\n%s\r\n", up, up, auth)
	connectConn.SetWriteDeadline(time.Now().Add(timeoutOp))
	if _, err = connectConn.Write([]byte(connect)); err != nil {
		connectConn.Close()
		return nil, err
	}

	respbuf, err := readUntil(connectConn, "\r\n\r\n")
	if err != nil {
		connectConn.Close()
		return nil, err
	}

	if !bytes.Contains(respbuf, okHTTP[9:14]) { // []byte(" 200 ")
		x := string(respbuf)
		if x = x[strings.Index(x, " ")+1:]; len(x) > 3 {
			x = x[:3]
		}

		connectConn.Close()
		return nil, errors.New("connect2: cannot connect to the HTTPS proxy (" + x + ")")
	}

	proxy.IO.Tr.AddLatency(time.Now().UnixNano() - lat)
	return connectConn, nil
}

func (proxy *ProxyClient) dialUpstreamAndBridge(downstreamConn net.Conn, host string, resp []byte, extra byte) net.Conn {
	upstreamConn, err := proxy.dialUpstream()
	if err != nil {
		logg.E(err)
		downstreamConn.Close()
		return nil
	}

	opt := Options(doConnect | extra)
	if proxy.Partial {
		opt.Set(doPartial)
	}

	rkey, rkeybuf := proxy.Cipher.NewIV(opt, nil, proxy.UserAuth)
	pl := make([]string, 0, len(dummyHeaders)+3)
	pl = append(pl,
		"GET /"+proxy.Cipher.EncryptCompress(host, rkeybuf...)+" HTTP/1.1\r\n",
		"Host: "+proxy.genHost()+"\r\n")

	for _, i := range proxy.Rand.Perm(len(dummyHeaders)) {
		if h := dummyHeaders[i]; h == "ph" {
			pl = append(pl, proxy.rkeyHeader+": "+rkey+"\r\n")
		} else if v, ok := proxy.dummies.Get(h); ok && v.(string) != "" {
			pl = append(pl, h+": "+v.(string)+"\r\n")
		}
	}

	upstreamConn.Write([]byte(strings.Join(pl, "") + "\r\n"))

	buf, err := readUntil(upstreamConn, "\r\n\r\n")
	// the first 15 bytes MUST be "HTTP/1.1 200 OK"
	if err != nil || len(buf) < 15 || !bytes.Equal(buf[:15], okHTTP[:15]) {
		if err != nil {
			logg.E(host, ": ", err)
		}

		upstreamConn.Close()
		downstreamConn.Close()
		return nil
	}

	if resp != nil {
		downstreamConn.Write(resp)
	}

	go proxy.Cipher.IO.Bridge(downstreamConn, upstreamConn, rkeybuf, IOConfig{Partial: proxy.Partial})

	return upstreamConn
}

func (proxy *ProxyClient) dialUpstreamAndBridgeWS(downstreamConn net.Conn, host string, resp []byte, extra byte) net.Conn {
	upstreamConn, err := proxy.dialUpstream()
	if err != nil {
		logg.E(err)
		downstreamConn.Close()
		return nil
	}

	opt := Options(doConnect | doWebSocket | extra)
	if proxy.Partial {
		opt.Set(doPartial)
	}

	rkey, rkeybuf := proxy.Cipher.NewIV(opt, nil, proxy.UserAuth)
	var pl string
	if proxy.URLHeader == "" {
		pl = "GET /" + proxy.Cipher.EncryptCompress(host, rkeybuf...) + " HTTP/1.1\r\n" +
			"Host: " + proxy.genHost() + "\r\n"
	} else {
		pl = "GET http://" + proxy.Upstream + "/ HTTP/1.1\r\n" +
			"Host: " + proxy.Upstream + "\r\n" +
			proxy.URLHeader + ": http://" + proxy.genHost() + "/" + proxy.Cipher.EncryptCompress(host, rkeybuf...) + "\r\n"
	}

	pl += "Upgrade: websocket\r\n" +
		"Connection: Upgrade\r\n" +
		"Sec-WebSocket-Key: " + rkey[:24] + "\r\n" +
		"Sec-WebSocket-Version: 13\r\n" +
		proxy.rkeyHeader + ": " + rkey + "\r\n\r\n"

	upstreamConn.Write([]byte(pl))

	buf, err := readUntil(upstreamConn, "\r\n\r\n")
	if err != nil || !strings.HasPrefix(string(buf), "HTTP/1.1 101 Switching Protocols") {
		if err != nil {
			logg.E(host, ": ", err)
		}

		upstreamConn.Close()
		downstreamConn.Close()
		return nil
	}

	if resp != nil {
		downstreamConn.Write(resp)
	}

	go proxy.Cipher.IO.Bridge(downstreamConn, upstreamConn, rkeybuf, IOConfig{
		Partial: proxy.Partial,
		WSCtrl:  wsClient,
	})
	return upstreamConn
}

func (proxy *ProxyClient) dialHostAndBridge(downstreamConn net.Conn, host string, resp []byte) {
	targetSiteConn, err := net.Dial("tcp", host)
	if err != nil {
		logg.E(err)
		downstreamConn.Close()
		return
	}

	downstreamConn.Write(resp)
	go proxy.Cipher.IO.Bridge(downstreamConn, targetSiteConn, nil, IOConfig{})
}

func (proxy *ProxyClient) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if proxy.UserAuth != "" {
		if proxy.basicAuth(r.Header.Get("Proxy-Authorization")) == "" {
			w.Header().Set("Proxy-Authenticate", "Basic realm=goflyway")
			w.WriteHeader(http.StatusProxyAuthRequired)
			return
		}
	}

	if r.RequestURI == "/proxy.pac" {
		proxy.servePACFile(w, r)
		return
	}

	if r.Method == "CONNECT" {
		hij, ok := w.(http.Hijacker)
		if !ok {
			http.Error(w, "webserver doesn't support hijacking", http.StatusInternalServerError)
			return
		}

		proxyClient, _, err := hij.Hijack()
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		// we are inside GFW and should pass data to upstream
		host := r.URL.Host
		if !hasPort.MatchString(host) {
			host += ":80"
		}

		if ans, ext := proxy.canDirectConnect(host); ans == ruleBlock {
			logg.D("BLACKLIST ", host, ext)
			proxyClient.Close()
		} else if ans == rulePass {
			logg.D("CONNECT ", r.RequestURI, ext)
			proxy.dialHostAndBridge(proxyClient, host, okHTTP)
		} else if proxy.Policy.IsSet(PolicyManInTheMiddle) {
			proxy.manInTheMiddle(proxyClient, host)
		} else if proxy.Policy.IsSet(PolicyWebSocket) {
			logg.D("WS^ ", r.RequestURI, ext)
			proxy.dialUpstreamAndBridgeWS(proxyClient, host, okHTTP, 0)
		} else {
			logg.D("CONNECT^ ", r.RequestURI, ext)
			proxy.dialUpstreamAndBridge(proxyClient, host, okHTTP, 0)
		}
	} else {
		// normal http requests
		if !r.URL.IsAbs() {
			http.Error(w, "request URI must be absolute", http.StatusInternalServerError)
			return
		}

		// borrow some headers from real browsings
		proxy.addToDummies(r)

		r.URL.Host = r.Host
		rURL := r.URL.String()
		r.Header.Del("Proxy-Authorization")
		r.Header.Del("Proxy-Connection")

		var resp *http.Response
		var err error
		var rkeybuf []byte

		if ans, ext := proxy.canDirectConnect(r.Host); ans == ruleBlock {
			logg.D("BLACKLIST ", r.Host, ext)
			w.WriteHeader(http.StatusServiceUnavailable)
			return
		} else if ans == rulePass {
			logg.D(r.Method, " ", r.Host, ext)
			resp, err = proxy.tpd.RoundTrip(r)
		} else {
			logg.D(r.Method, "^ ", r.Host, ext)
			resp, rkeybuf, err = proxy.encryptAndTransport(r)
		}

		if err != nil {
			logg.E("HTTP forward: ", rURL, ", ", err)
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		if resp.StatusCode >= 400 {
			logg.D("[", resp.Status, "] - ", rURL)
		}

		copyHeaders(w.Header(), resp.Header, proxy.Cipher, false, rkeybuf)
		w.WriteHeader(resp.StatusCode)

		if nr, err := proxy.Cipher.IO.Copy(w, resp.Body, rkeybuf, IOConfig{Partial: false}); err != nil {
			logg.E("copy ", nr, " bytes: ", err)
		}

		tryClose(resp.Body)
	}
}

func (proxy *ProxyClient) authSocks(conn net.Conn) bool {
	buf := make([]byte, 1+1+255+1+255)
	n, err := io.ReadAtLeast(conn, buf, 2)
	if err != nil {
		logg.E(err)
		return false
	}

	ulen := int(buf[1])
	if buf[0] != 0x01 || 2+ulen+1 > n {
		return false
	}

	username := string(buf[2 : 2+ulen])
	plen := int(buf[2+ulen])
	if 2+ulen+1+plen > n {
		return false
	}

	password := string(buf[2+ulen+1 : 2+ulen+1+plen])
	return proxy.UserAuth == username+":"+password
}

func (proxy *ProxyClient) handleSocks(conn net.Conn) {
	logClose := func(args ...interface{}) {
		logg.E(args...)
		conn.Close()
	}

	buf := make([]byte, 2)
	if _, err := io.ReadAtLeast(conn, buf, 2); err != nil {
		logClose(err)
		return
	} else if buf[0] != socksVersion5 {
		logClose(fmt.Sprintf(socksVersionErr, buf[0]))
		return
	}

	numMethods := int(buf[1])
	methods := make([]byte, numMethods)
	if _, err := io.ReadAtLeast(conn, methods, numMethods); err != nil {
		logClose(err)
		return
	}

	if proxy.UserAuth != "" {
		conn.Write([]byte{socksVersion5, 0x02}) // username & password auth

		if !proxy.authSocks(conn) {
			conn.Write([]byte{1, 1})
			logClose("invalid auth data from: ", conn.RemoteAddr)
			return
		}

		// auth success
		conn.Write([]byte{1, 0})
	} else {
		conn.Write([]byte{socksVersion5, 0})
	}
	// handshake over
	// tunneling start
	method, addr, err := parseUDPHeader(conn, nil, false)
	if err != nil {
		logClose(err)
		return
	}

	host := addr.String()
	switch method {
	case 1:
		if ans, ext := proxy.canDirectConnect(host); ans == ruleBlock {
			logg.D("BLACKLIST ", host, ext)
			conn.Close()
		} else if ans == rulePass {
			logg.D("SOCKS ", host, ext)
			proxy.dialHostAndBridge(conn, host, okSOCKS)
		} else if proxy.Policy.IsSet(PolicyWebSocket) {
			logg.D("WS^ ", host, ext)
			proxy.dialUpstreamAndBridgeWS(conn, host, okSOCKS, 0)
		} else {
			logg.D("SOCKS^ ", host, ext)
			proxy.dialUpstreamAndBridge(conn, host, okSOCKS, 0)
		}
	case 3:
		relay, err := net.ListenUDP("udp", &net.UDPAddr{IP: net.IPv6zero, Port: 0})
		if err != nil {
			logClose("can't create UDP relay server: ", err)
			return
		}

		proxy.handleUDPtoTCP(relay, conn)
	}
}

func (proxy *ProxyClient) UpdateKey(newKey string) {
	proxy.Cipher.Init(newKey)
	proxy.rkeyHeader = "X-" + proxy.Cipher.Alias
}

func (proxy *ProxyClient) Start() error {
	return http.Serve(proxy.Listener, proxy)
}

func NewClient(localaddr string, config *ClientConfig) *ProxyClient {
	var mux net.Listener
	var err error

	upURL, err := url.Parse("http://" + config.Upstream)
	if err != nil {
		logg.F(err)
		return nil
	}

	proxyURL := http.ProxyURL(upURL)
	proxy := &ProxyClient{
		pool: tcpmux.NewDialer(config.Upstream, config.Mux),

		tp:  &http.Transport{TLSClientConfig: tlsSkip, Proxy: proxyURL},
		tpd: &http.Transport{TLSClientConfig: tlsSkip},
		tpq: &http.Transport{TLSClientConfig: tlsSkip, Proxy: proxyURL, ResponseHeaderTimeout: timeoutOp, Dial: (&net.Dialer{Timeout: timeoutDial}).Dial},

		dummies:    lru.NewCache(len(dummyHeaders)),
		rkeyHeader: "X-" + config.Cipher.Alias,

		ClientConfig: config,
	}

	if config.Mux > 0 {
		proxy.Cipher.IO.Ob = proxy.pool
	}

	tcpmux.Version = checksum1b([]byte(config.Cipher.Alias)) | 0x80

	if proxy.Connect2 != "" || proxy.Mux != 0 {
		proxy.tp.Proxy, proxy.tpq.Proxy = nil, nil
		proxy.tpq.Dial = func(network, address string) (net.Conn, error) { return proxy.dialUpstream() }
		proxy.tp.Dial = proxy.tpq.Dial
	}

	if config.Policy.IsSet(PolicyManInTheMiddle) {
		// plus other fds, we should have a number smaller than 100
		proxy.tp.MaxIdleConns = 20
		proxy.tpd.MaxIdleConns = 20
		proxy.tpq.MaxIdleConns = 20
	}

	if proxy.UDPRelayCoconn <= 0 {
		proxy.UDPRelayCoconn = 1
	}

	if port, lerr := strconv.Atoi(localaddr); lerr == nil {
		mux, err = net.ListenTCP("tcp", &net.TCPAddr{IP: net.IPv6zero, Port: port})
		localaddr = "127.0.0.1:" + localaddr
	} else {
		mux, err = net.Listen("tcp", localaddr)
		if localaddr[0] == ':' {
			localaddr = "127.0.0.1" + localaddr
		}
	}

	if err != nil {
		logg.F(err)
		return nil
	}

	proxy.Listener = &listenerWrapper{mux, proxy}
	proxy.Localaddr = localaddr

	if proxy.Policy.IsSet(PolicyVPN) {
		proxy.pool.OnDial = vpnDial
		// proxy.tp.MaxIdleConns = 2
		// proxy.tpd.MaxIdleConns = 2
		// proxy.tpq.MaxIdleConns = 2
		// proxy.tpd.Dial = func(network, address string) (net.Conn, error) { return vpnDial(address) }

		proxy.pool.DialTimeout(time.Second)
	}

	return proxy
}
