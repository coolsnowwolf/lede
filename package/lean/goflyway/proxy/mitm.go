package proxy

import (
	"github.com/coyove/goflyway/pkg/logg"

	"bufio"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"io"
	"math/big"
	"net"
	"net/http"
	"net/http/httputil"
	"net/url"
	"strings"
	"time"
)

type bufioConn struct {
	m io.Reader
	net.Conn
}

func (c *bufioConn) Read(buf []byte) (int, error) {
	return c.m.Read(buf)
}

func publicKey(priv interface{}) interface{} {
	switch k := priv.(type) {
	case *rsa.PrivateKey:
		return &k.PublicKey
	case *ecdsa.PrivateKey:
		return &k.PublicKey
	default:
		return nil
	}
}

func (proxy *ProxyClient) sign(host string) *tls.Certificate {
	if cert, ok := proxy.CACache.Get(host); ok {
		return cert.(*tls.Certificate)
	}

	logg.D("self signing: ", host)

	serialNumberLimit := new(big.Int).Lsh(big.NewInt(1), 128)
	serialNumber, err := rand.Int(rand.Reader, serialNumberLimit)
	if err != nil {
		logg.E(err)
		return nil
	}

	x509ca, err := x509.ParseCertificate(proxy.CA.Certificate[0])
	if err != nil {
		return nil
	}

	template := x509.Certificate{
		SerialNumber: serialNumber,
		Issuer:       x509ca.Subject,
		Subject:      pkix.Name{Organization: []string{"goflyway"}},
		NotBefore:    time.Now().AddDate(0, 0, -1),
		NotAfter:     time.Now().AddDate(1, 0, 0),

		KeyUsage:              x509.KeyUsageKeyEncipherment | x509.KeyUsageDigitalSignature,
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
		BasicConstraintsValid: true,
		DNSNames:              []string{host},
	}

	pubKey := publicKey(proxy.CA.PrivateKey)
	derBytes, err := x509.CreateCertificate(rand.Reader, &template, x509ca, pubKey, proxy.CA.PrivateKey)
	if err != nil {
		logg.E("create certificate: ", err)
		return nil
	}

	cert := &tls.Certificate{
		Certificate: [][]byte{derBytes, proxy.CA.Certificate[0]},
		PrivateKey:  proxy.CA.PrivateKey,
	}

	proxy.CACache.Add(host, cert)
	return cert
}

func (proxy *ProxyClient) manInTheMiddle(client net.Conn, host string) {
	_host, _ := splitHostPort(host)
	// try self signing a cert of this host
	cert := proxy.sign(_host)
	if cert == nil {
		return
	}

	client.Write(okHTTP)

	go func() {

		tlsClient := tls.Server(client, &tls.Config{
			InsecureSkipVerify: true,
			Certificates:       []tls.Certificate{*cert},
		})

		if err := tlsClient.Handshake(); err != nil {
			logg.E("handshake failed: ", host, ", ", err)
			return
		}

		bufTLSClient := bufio.NewReader(tlsClient)

		for {
			proxy.Cipher.IO.markActive(tlsClient, 0)

			var err error
			var rURL string
			var buf []byte
			if buf, err = bufTLSClient.Peek(3); err == io.EOF || len(buf) != 3 {
				break
			}

			// switch string(buf) {
			// case "GET", "POS", "HEA", "PUT", "OPT", "DEL", "PAT", "TRA":
			// 	// good
			// default:
			// 	proxy.dialUpstreamAndBridge(&bufioConn{Conn: tlsClient, m: bufTLSClient}, host, auth, []byte{})
			// 	return
			// }

			req, err := http.ReadRequest(bufTLSClient)
			if err != nil {
				if !isClosedConnErr(err) && buf[0] != ')' {
					logg.E("cannot read request: ", err)
				}
				break
			}

			rURL = req.URL.String()
			req.Header.Del("Proxy-Authorization")
			req.Header.Del("Proxy-Connection")

			if !isHTTPSSchema.MatchString(req.URL.String()) {
				// we can ignore 443 since it's by default
				h := req.Host
				if strings.HasSuffix(h, ":443") {
					h = h[:len(h)-4]
				}

				req.URL, err = url.Parse("https://" + h + req.URL.String())
				rURL = req.URL.String()
			}

			logg.D(req.Method, "^ ", rURL)

			resp, rkeybuf, err := proxy.encryptAndTransport(req)
			if err != nil {
				logg.E("proxy pass: ", rURL, ", ", err)
				tlsClient.Write([]byte("HTTP/1.1 500 Internal Server Error\r\n\r\n" + err.Error()))
				break
			}

			resp.Header.Del("Content-Length")
			resp.Header.Set("Transfer-Encoding", "chunked")

			if strings.ToLower(resp.Header.Get("Connection")) != "upgrade" {
				resp.Header.Set("Connection", "close")
				tlsClient.Write([]byte("HTTP/1.1 " + resp.Status + "\r\n"))
			} else {
				// we don't support upgrade in mitm
				tlsClient.Write([]byte("HTTP/1.1 403 Forbidden\r\n\r\n"))
				break
			}

			// buf, _ := httputil.DumpResponse(resp, true)
			_ = httputil.DumpResponse

			hdr := http.Header{}
			copyHeaders(hdr, resp.Header, proxy.Cipher, false, rkeybuf)
			if err := hdr.Write(tlsClient); err != nil {
				logg.W("write header: ", err)
				break
			}
			if _, err = io.WriteString(tlsClient, "\r\n"); err != nil {
				logg.W("write header: ", err)
				break
			}

			nr, err := proxy.Cipher.IO.Copy(tlsClient, resp.Body, rkeybuf, IOConfig{Partial: false, Chunked: true})
			if err != nil {
				logg.E("copy ", nr, " bytes: ", err)
			}
			tryClose(resp.Body)
		}

		tlsClient.Close()
	}()
}
