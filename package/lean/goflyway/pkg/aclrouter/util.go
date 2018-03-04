package aclrouter

import (
	"fmt"
	"regexp"
	"sort"
	"strconv"
	"strings"
)

var validDomain = regexp.MustCompile(`^[a-zA-Z0-9.\-_]+$`)

func linesToRange(lines string) []ipRange {
	l := strings.Split(lines, "\n")
	ret := make([]ipRange, 0, len(l))

	for _, iprange := range l {
		ret = append(ret, tryParseIPRange(iprange))
	}

	return ret
}

func sortLookupTable(iplist []ipRange) []ipRange {
	sort.Slice(iplist, func(i, j int) bool {
		return iplist[i].start < iplist[j].start
	})

	for i := 1; i < len(iplist); {
		if iplist[i-1].end+1 == iplist[i].start {
			// merge
			iplist[i-1].end = iplist[i].end
			iplist = append(iplist[:i], iplist[i+1:]...)
			continue
		}
		i++
	}

	return iplist
}

func tryParseIPRange(iprange string) ipRange {
	p := strings.Split(iprange, " ")
	if len(p) >= 2 {
		// form: "0.0.0.0 1.2.3.4"
		ipstart, ipend := IPv4ToInt(p[0]), IPv4ToInt(p[1])
		if ipstart > 0 && ipend > 0 {
			return ipRange{ipstart, ipend}
		}
	}

	p = strings.Split(iprange, "/")
	if len(p) >= 2 {
		// form: "1.2.3.4/12", ipv4 only
		ipstart := IPv4ToInt(p[0])
		if ipstart == 0 {
			return ipRange{}
		}

		mask, _ := strconv.Atoi(p[1])

		if mask >= 0 && mask <= 32 {
			ipend := ipstart + (1<<(32-uint(mask)) - 1)
			return ipRange{ipstart, ipend}
		}
	}

	panic(iprange)
}

func (lk *lookup) sortLookupTable() {
	lk.IPv4Table = sortLookupTable(lk.IPv4Table)
}

func (lk *lookup) tryAddACLSingleRule(r string) error {
	rx := strings.Replace(r, "\\.", ".", -1)
	if strings.HasPrefix(rx, "(^|.)") && strings.HasSuffix(rx, "$") {
		rx = rx[5 : len(rx)-1]
		if validDomain.MatchString(rx) {
			subs := strings.Split(strings.Trim(rx, "\r "), ".")
			fast := lk.DomainFastMatch
			for i := len(subs) - 1; i >= 0; i-- {
				if fast[subs[i]] == nil {
					fast[subs[i]] = make(matchTree)
				}

				if i == 0 {
					// end
					fast[subs[0]] = 0
				} else {
					it := fast[subs[i]]
					switch it.(type) {
					case int:
						fast[subs[i]] = make(matchTree)
						fast = fast[subs[i]].(matchTree)
					case matchTree:
						fast = it.(matchTree)
					}
				}
			}

			return nil
		}
	}

	if idx := strings.Index(r, "/"); idx > -1 {
		if _, err := strconv.Atoi(r[idx+1:]); err == nil {
			lk.IPv4Table = append(lk.IPv4Table, tryParseIPRange(r))
			return nil
		}
	}

	re, err := regexp.Compile(r)
	if err == nil {
		lk.DomainSlowMatch = append(lk.DomainSlowMatch, re)
		return nil
	}

	return fmt.Errorf("invalid rule: %s", r)
}

func (lk *lookup) Match(domain string) bool {
	slowMatch := func() bool {
		for _, r := range lk.DomainSlowMatch {
			if r.MatchString(domain) {
				return true
			}
		}

		return false
	}

	subs := strings.Split(domain, ".")
	if len(subs) <= 1 {
		return slowMatch()
	}

	top := lk.DomainFastMatch
	if top == nil {
		return slowMatch()
	}

	for i := len(subs) - 1; i >= 0; i-- {
		sub := subs[i]
		if top[sub] == nil {
			return slowMatch()
		}

		switch top[sub].(type) {
		case matchTree:
			top = top[sub].(matchTree)
		case int:
			return top[sub].(int) == 0
		default:
			return slowMatch()
		}
	}

	return slowMatch()
}
