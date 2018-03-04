package proxy

import (
	"net"
	"net/http"

	acr "github.com/coyove/goflyway/pkg/aclrouter"
	"github.com/coyove/goflyway/pkg/logg"
)

const (
	ruleProxy = iota
	rulePass
	ruleBlock
)

type Rule struct {
	IP     string
	Ans    byte
	OldAns byte
	R      byte
}

func (proxy *ProxyClient) canDirectConnect(host string) (r byte, ext string) {
	host, _ = splitHostPort(host)

	if c, ok := proxy.DNSCache.Get(host); ok && c.(*Rule) != nil {
		return c.(*Rule).Ans, " (cache-" + c.(*Rule).IP + ")"
	}

	rule, ipstr, err := proxy.ACL.Check(host, !proxy.ACL.RemoteDNS)
	if err != nil {
		logg.E(err)
	}

	priv := false
	defer func() {
		if proxy.Policy.IsSet(PolicyGlobal) && !priv {
			r = ruleProxy
			ext += " (global)"
		} else {
			proxy.DNSCache.Add(host, &Rule{ipstr, r, r, rule})
		}
	}()

	switch rule {
	case acr.RuleIPv6:
		return ruleProxy, " (ipv6-proxy)" // By default we proxy IPv6 destination
	case acr.RuleMatchedPass:
		return rulePass, " (match-pass)"
	case acr.RuleProxy, acr.RuleMatchedProxy:
		return ruleProxy, " (match-proxy)"
	case acr.RuleBlock:
		return ruleBlock, " (match-block)"
	case acr.RulePrivate:
		priv = true
		return rulePass, " (private-ip)"
	case acr.RulePass:
		if !proxy.ACL.RemoteDNS {
			return rulePass, " (trust-local-pass)"
		}
		r = rulePass
	default:
		r = ruleProxy
	}

	if proxy.Policy.IsSet(PolicyGlobal) {
		return
	}

	// We have doubts, so query the upstream
	dnsloc := "http://" + proxy.genHost()
	rkey, _ := proxy.Cipher.NewIV(doDNS, []byte(host), proxy.UserAuth)
	if proxy.URLHeader != "" {
		dnsloc = "http://" + proxy.Upstream
	}

	req, _ := http.NewRequest("GET", dnsloc, nil)
	req.Header.Add(proxy.rkeyHeader, rkey)
	if proxy.URLHeader != "" {
		req.Header.Add(proxy.URLHeader, "http://"+proxy.genHost())
	}

	resp, err := proxy.tpq.RoundTrip(req)
	if err != nil {
		if e, _ := err.(net.Error); e != nil && e.Timeout() {
			// proxy.tpq.Dial = (&net.Dialer{Timeout: 2 * time.Second}).Dial
		} else {
			logg.E(err)
		}
		return r, " (network-err)"
	}

	tryClose(resp.Body)
	ip, _ := base32Decode(resp.Header.Get(dnsRespHeader), true)
	if ip == nil || len(ip) != net.IPv4len {
		return r, " (remote-err)"
	}

	ipstr = net.IP(ip).String()
	switch rule, _, _ = proxy.ACL.Check(ipstr, true); rule {
	case acr.RulePass, acr.RuleMatchedPass:
		return rulePass, " (remote-pass)"
	case acr.RuleProxy, acr.RuleMatchedProxy:
		return ruleProxy, " (remote-proxy)"
	case acr.RuleBlock:
		return ruleBlock, " (remote-block)"
	case acr.RulePrivate:
		return ruleProxy, " (remote-private-ip)"
	default:
		return ruleProxy, " (remote-unknown)"
	}
}
