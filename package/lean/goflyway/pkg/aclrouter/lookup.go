package aclrouter

import (
	"errors"
	"io/ioutil"
	"net"
	"regexp"
	"strings"

	"github.com/coyove/goflyway/pkg/config"
)

type matchTree map[string]interface{}

type ipRange struct{ start, end uint32 }

type lookup struct {
	Always          bool
	DomainFastMatch matchTree
	DomainSlowMatch []*regexp.Regexp
	IPv4Table       []ipRange
}

func (lk *lookup) init() {
	lk.IPv4Table = make([]ipRange, 0)
	lk.DomainSlowMatch = make([]*regexp.Regexp, 0)
	lk.DomainFastMatch = make(matchTree)
}

const (
	RuleBlock = iota
	RulePrivate
	RuleMatchedPass
	RulePass
	RuleMatchedProxy
	RuleProxy
	RuleIPv6
	RuleUnknown
)

// ACL stands for Access Control List
type ACL struct {
	Black lookup // Black are those which should be banned, first to check
	White lookup // White are those which should be accessed directly
	Gray  lookup // Gray are those which should be proxied

	PrivateIPv4Table []ipRange
	RemoteDNS        bool
	Legacy           bool
	OmitRules        []string
}

func (acl *ACL) init() {
	acl.White.init()
	acl.Gray.init()
	acl.Black.init()
	acl.PrivateIPv4Table = sortLookupTable(linesToRange(PrivateIP))
	acl.RemoteDNS = true
	acl.OmitRules = make([]string, 0)
}

func (acl *ACL) postInit() *ACL {
	acl.White.IPv4Table = sortLookupTable(linesToRange(ChinaIP))
	acl.Gray.Always = true
	acl.White.Always = false
	acl.RemoteDNS = true
	acl.Legacy = true
	return acl
}

// LoadACL loads ACL config, which can be chinalist.txt or SS ACL,
// note that it will always return a valid ACL struct, but may be empty.
// An empty ACL checks China IP only, if failed, it goes Gray as always.
func LoadACL(path string) (*ACL, error) {
	acl := &ACL{}
	acl.init()

	buf, err := ioutil.ReadFile(path)
	if err != nil {
		return acl.postInit(), err
	}

	if strings.HasSuffix(path, "chinalist.txt") {
		return loadChinaList(buf)
	}

	cf, err := config.ParseConf(string(buf))
	if err != nil {
		return acl.postInit(), err
	}

	acl.Gray.Always = cf.HasSection("proxy_all")
	acl.White.Always = cf.HasSection("bypass_all")
	acl.RemoteDNS = !cf.HasSection("local_dns")

	if acl.Gray.Always && acl.White.Always {
		return acl.postInit(), errors.New("proxy_all and bypass_all collide")
	}

	adder := func(src *lookup) func(string) {
		return func(key string) {
			if src.tryAddACLSingleRule(key) != nil {
				acl.OmitRules = append(acl.OmitRules, key)
			}
		}
	}

	cf.Iterate("bypass_list", adder(&acl.White))
	acl.White.sortLookupTable()

	cf.Iterate("proxy_list", adder(&acl.Gray))
	acl.Gray.sortLookupTable()

	cf.Iterate("outbound_block_list", adder(&acl.Black))
	acl.Black.sortLookupTable()

	// fmt.Println(IPv4ToInt("47.97.161.219"))
	// fmt.Println(acl.White.IPv4Table)
	return acl, nil
}

func loadChinaList(buf []byte) (*ACL, error) {
	acl := &ACL{}
	acl.init()
	acl.postInit()

	raw := string(buf)
	for _, domain := range strings.Split(raw, "\n") {
		subs := strings.Split(strings.Trim(domain, "\r "), ".")
		if len(subs) == 0 || len(domain) == 0 || domain[0] == '#' {
			continue
		}

		top := acl.White.DomainFastMatch
		for i := len(subs) - 1; i >= 0; i-- {
			if top[subs[i]] == nil {
				top[subs[i]] = make(matchTree)
			}

			if i == 0 {
				// end
				top[subs[0]] = 0
			} else {
				top = top[subs[i]].(matchTree)
			}
		}
	}

	return acl, nil
}

func isIPInLookupTable(ip string, table []ipRange) bool {
	m := IPv4ToInt(ip)
	if m == 0 {
		return false
	}

	return isIPInLookupTableI(m, table)
}

func isIPInLookupTableI(ip uint32, table []ipRange) bool {
	var rec func([]ipRange) bool
	rec = func(r []ipRange) bool {
		if len(r) == 0 {
			return false
		}

		mid := len(r) / 2
		if ip >= r[mid].start && ip <= r[mid].end {
			return true
		}

		if ip < r[mid].start {
			return rec(r[:mid])
		}

		return rec(r[mid+1:])
	}

	return rec(table)
}

// IsPrivateIP checks if the given IPv4 is private
func (acl *ACL) IsPrivateIP(ip string) bool {
	return isIPInLookupTable(ip, acl.PrivateIPv4Table)
}

// Check returns a route rule for the given host
func (acl *ACL) Check(host string, trustIP bool) (rule byte, strIP string, err error) {
	var ip *net.IPAddr
	iip := IPv4ToInt(host)

	if iip > 0 {
		if isIPInLookupTableI(iip, acl.PrivateIPv4Table) {
			return RulePrivate, host, nil
		}

		trustIP = true
		strIP = host
		goto IP_CHECK
	}

	if acl.Black.Match(host) {
		return RuleBlock, host, nil
	} else if acl.Gray.Match(host) {
		return RuleMatchedProxy, host, nil
	} else if acl.White.Match(host) {
		return RuleMatchedPass, host, nil
	}

	if host[0] == '[' && host[len(host)-1] == ']' {
		// Naive match: host is IPv6
		// We assume that local don't have the ability to resolve IPv6,
		// so return RuleIPv6 and let the caller deal with it
		return RuleIPv6, host, nil
	}

	// Resolve at local in case host points to a private ip
	ip, err = net.ResolveIPAddr("ip4", host)
	if err != nil {
		return RuleUnknown, "unknown", err
	}

	iip = NetIPv4ToInt(ip.IP)
	if isIPInLookupTableI(iip, acl.PrivateIPv4Table) {
		return RulePrivate, ip.String(), nil
	}

	strIP = ip.String()
IP_CHECK:
	if isIPInLookupTableI(iip, acl.Black.IPv4Table) {
		return RuleBlock, strIP, nil
	} else if isIPInLookupTableI(iip, acl.Gray.IPv4Table) {
		return RuleMatchedProxy, strIP, nil
	}

	if trustIP {
		if isIPInLookupTableI(iip, acl.White.IPv4Table) {
			return RuleMatchedPass, strIP, nil
		} else if acl.Gray.Always {
			return RuleProxy, strIP, nil
		}
		return RulePass, strIP, nil
	}

	if isIPInLookupTableI(iip, acl.White.IPv4Table) {
		return RulePass, strIP, nil
	}
	return RuleUnknown, strIP, nil
}

// IPv4ToInt converts an IPv4 string to its integer representation
func IPv4ToInt(ip string) uint32 {
	buf, idx, last := [4]uint32{}, 0, rune(0)
	for _, r := range ip {
		if r == '.' {
			if idx++; idx > 3 || last == r {
				return 0
			}
			last = r
			continue
		}

		if r >= '0' && r <= '9' {
			buf[idx] = buf[idx]*10 + uint32(r-'0')
			last = r
			continue
		}

		return 0
	}

	if idx != 3 || buf[0] > 255 || buf[1] > 255 || buf[2] > 255 || buf[3] > 255 {
		return 0
	}

	return buf[0]<<24 + buf[1]<<16 + buf[2]<<8 + buf[3]
}

// NetIPv4ToInt converts net.IP(v4) to its integer representation
func NetIPv4ToInt(ip net.IP) uint32 {
	v4 := ip.To4()
	if v4 == nil {
		return 0
	}

	buf := []byte(v4)
	return uint32(buf[0])<<24 + uint32(buf[1])<<16 + uint32(buf[2])<<8 + uint32(buf[3])
}
