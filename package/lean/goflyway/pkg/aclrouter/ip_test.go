package aclrouter

import (
	"math/rand"
	"strconv"
	"strings"
	"testing"
	"time"
)

func TestIPLookup(t *testing.T) {
	table := linesToRange(PrivateIP)
	table = sortLookupTable(table)

	in := 0
	for i := 0; i < 1<<32; i++ {

		if isIPInLookupTableI(uint32(i), table) {
			in++
		}
	}

	//    10.0.0.0 ~ 10.255.255.255  = 256 * 256 * 256
	//   127.0.0.0 ~ 127.255.255.255 = 256 * 256 * 256
	//  172.16.0.0 ~ 172.31.255.255  = 256 * 256 * 16
	// 192.168.0.0 ~ 192.168.255.255 = 256 * 256 * 16

	if in != 34668544 {
		t.Error("error IP lookup")
	}
}

func TestHostLookup(t *testing.T) {

	acl := &ACL{}
	acl.init()

	test := func(m string, assert bool) {
		if acl.Black.Match(m) != assert {
			t.Error("host lookup failed:", m)
		}
	}

	acl.Black.tryAddACLSingleRule(`(^|\.)sub\.abc\.com$`)
	test("abc.com", false)
	test("sub.abc.com", true)
	test("subsub.sub.abc.com", true)

	acl.Black.tryAddACLSingleRule(`(^|\.)ab[cd]\.com$`)
	test("abe.com", false)
	test("sub.abc.com", true)
	test("sub.abd.com", true)

	acl.Black.tryAddACLSingleRule(`(^|\.)abc\.com$`) // this sould override (^|\.)sub\.abc\.com$
	test("abc.com", true)
	test("sub.abc.com", true)

	acl.Black.tryAddACLSingleRule(`1.0.0.0/8`)
	acl.Black.tryAddACLSingleRule(`4.0.0.0/17`)
	acl.Black.IPv4Table = sortLookupTable(acl.Black.IPv4Table)

	test2 := func(m string, assert bool) {
		if isIPInLookupTable(m, acl.Black.IPv4Table) != assert {
			t.Error("host lookup failed:", m)
		}
	}

	test2("1.0.0.0", true)
	test2("2.0.0.0", false)
	test2("4.0.127.255", true)
	test2("4.0.128.0", false)
	test2("1000.0.0", false)
}

func TestIPv4ToInt(t *testing.T) {
	test := func(m string, assert bool) {
		if (IPv4ToInt(m) > 0) != assert {
			t.Error("IPv4ToInt failed:", m)
		}
	}

	test("0.0.0.0", false) // 0.0.0.0 is not considered here

	rand.Seed(time.Now().UnixNano())
	for i := 0; i < 1000; i++ {
		s := [4]string{}
		for j := 0; j < 4; j++ {
			s[j] = strconv.Itoa(rand.Intn(255) + 1)
		}
		test(strings.Join(s[:], "."), true)
	}

	test("0.1.2.a", false)
	test("0.1..2", false)
	test("0.1.2.3.4", false)
	test("0.1.2.257", false)
}
