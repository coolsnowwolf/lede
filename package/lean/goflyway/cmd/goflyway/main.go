package main

import (
	"encoding/json"
	"net"
	"net/http"
	"os"

	"github.com/coyove/goflyway/cmd/goflyway/lib"
	"github.com/coyove/goflyway/pkg/aclrouter"
	"github.com/coyove/goflyway/pkg/config"
	"github.com/coyove/goflyway/pkg/logg"
	"github.com/coyove/goflyway/pkg/lru"
	"github.com/coyove/goflyway/proxy"

	"flag"
	"fmt"
	"io/ioutil"
	"strings"
)

var version = "__devel__"

var (
	cmdGenCA = flag.Bool("gen-ca", false, "generate certificate (ca.pem) and private key (key.pem)")
	cmdDebug = flag.Bool("debug", false, "turn on debug mode")

	// General flags
	cmdConfig    = flag.String("c", "", "[SC] config file path")
	cmdLogLevel  = flag.String("lv", "log", "[SC] logging level: {dbg, log, warn, err, off}")
	cmdLogFile   = flag.String("lf", "", "[SC] log to file")
	cmdAuth      = flag.String("a", "", "[SC] proxy authentication, form: username:password (remember the colon)")
	cmdKey       = flag.String("k", "0123456789abcdef", "[SC] password, do not use the default one")
	cmdLocal     = flag.String("l", ":8100", "[SC] local listening address")
	cmdCloseConn = flag.Int64("t", 20, "[SC] close connections when they go idle for at least N sec")

	// Server flags
	cmdThrot     = flag.Int64("throt", 0, "[S] traffic throttling in bytes")
	cmdThrotMax  = flag.Int64("throt-max", 1024*1024, "[S] traffic throttling token bucket max capacity")
	cmdDiableUDP = flag.Bool("disable-udp", false, "[S] disable UDP relay")
	cmdProxyPass = flag.String("proxy-pass", "", "[S] use goflyway as a reverse HTTP proxy")

	// Client flags
	cmdGlobal     = flag.Bool("g", false, "[C] global proxy")
	cmdUpstream   = flag.String("up", "", "[C] upstream server address")
	cmdPartial    = flag.Bool("partial", false, "[C] partially encrypt the tunnel traffic")
	cmdUDPonTCP   = flag.Int64("udp-tcp", 1, "[C] use N TCP connections to relay UDP")
	cmdWebConPort = flag.Int64("web-port", 8101, "[C] web console listening port, 0 to disable")
	cmdDNSCache   = flag.Int64("dns-cache", 1024, "[C] DNS cache size")
	cmdMux        = flag.Int64("mux", 0, "[C] limit the total number of TCP connections, 0 means no limit")
	cmdVPN        = flag.Bool("vpn", false, "[C] vpn mode, used on Android only")
	cmdACL        = flag.String("acl", "chinalist.txt", "[C] load ACL file")

	// Shadowsocks compatible flags
	cmdLocal2 = flag.String("p", "", "server listening address")

	_ = flag.Bool("u", true, "placeholder")
	_ = flag.String("m", "", "placeholder")
	_ = flag.String("b", "", "placeholder")
	_ = flag.Bool("V", true, "placeholder")
	_ = flag.Bool("fast-open", true, "placeholder")
)

func loadConfig() {
	flag.Parse()

	path := *cmdConfig
	if path == "" {
		return
	}

	buf, err := ioutil.ReadFile(path)
	if err != nil {
		fmt.Println("* can't load config file:", err)
		return
	}

	if strings.Contains(path, "shadowsocks.conf") {
		cmds := make(map[string]interface{})
		if err := json.Unmarshal(buf, &cmds); err != nil {
			fmt.Println("* can't parse config file:", err)
			return
		}

		*cmdKey = cmds["password"].(string)
		*cmdUpstream = fmt.Sprintf("%v:%v", cmds["server"], cmds["server_port"])
		*cmdMux = 10
		*cmdLogLevel = "dbg"
		*cmdVPN = true
		*cmdGlobal = true
		return
	}

	cf, err := config.ParseConf(string(buf))
	if err != nil {
		fmt.Println("* can't parse config file:", err)
		return
	}

	*cmdKey = cf.GetString("default", "password", *cmdKey)
	*cmdAuth = cf.GetString("default", "auth", *cmdAuth)
	*cmdLocal = cf.GetString("default", "listen", *cmdLocal)
	*cmdUpstream = cf.GetString("default", "upstream", *cmdUpstream)
	*cmdDiableUDP = cf.GetBool("default", "disableudp", *cmdDiableUDP)
	*cmdUDPonTCP = cf.GetInt("default", "udptcp", *cmdUDPonTCP)
	*cmdGlobal = cf.GetBool("default", "global", *cmdGlobal)
	*cmdACL = cf.GetString("default", "acl", *cmdACL)
	*cmdPartial = cf.GetBool("default", "partial", *cmdPartial)

	*cmdProxyPass = cf.GetString("misc", "proxypass", *cmdProxyPass)
	*cmdWebConPort = cf.GetInt("misc", "webconport", *cmdWebConPort)
	*cmdDNSCache = cf.GetInt("misc", "dnscache", *cmdDNSCache)
	*cmdMux = cf.GetInt("misc", "mux", *cmdMux)
	*cmdLogLevel = cf.GetString("misc", "loglevel", *cmdLogLevel)
	*cmdLogFile = cf.GetString("misc", "logfile", *cmdLogFile)
	*cmdThrot = cf.GetInt("misc", "throt", *cmdThrot)
	*cmdThrotMax = cf.GetInt("misc", "throtmax", *cmdThrotMax)

	*cmdCloseConn = cf.GetInt("misc", "closeconn", *cmdCloseConn)
}

func main() {
	fmt.Println("goflyway (build " + version + ")")

	loadConfig()

	if *cmdGenCA {
		fmt.Println("* generating CA...")

		cert, key, err := lib.GenCA("goflyway")
		if err != nil {
			fmt.Println(err)
			return
		}

		err1, err2 := ioutil.WriteFile("ca.pem", cert, 0755), ioutil.WriteFile("key.pem", key, 0755)
		if err1 != nil || err2 != nil {
			fmt.Println("* error ca.pem:", err1)
			fmt.Println("* error key.pem:", err2)
			return
		}

		fmt.Println("* successfully generated ca.pem/key.pem, please leave them in the same directory with goflyway")
		fmt.Println("* goflyway will automatically read them when launched")
		return
	}

	if *cmdUpstream != "" {
		fmt.Println("* launched as client")
	} else {
		fmt.Println("* launched as server (aka upstream)")
	}

	if *cmdKey == "0123456789abcdef" {
		fmt.Println("* you are using the default password, it is recommended to change it: -k=<NEW PASSWORD>")
	}

	cipher := &proxy.Cipher{Partial: *cmdPartial}
	cipher.Init(*cmdKey)

	var cc *proxy.ClientConfig
	var sc *proxy.ServerConfig

	if *cmdMux > 0 {
		fmt.Println("* TCP multiplexer enabled, limit:", *cmdMux, ", note that you must directly connect to the upstream")
	}

	if *cmdUpstream != "" || *cmdDebug {
		acl, err := aclrouter.LoadACL(*cmdACL)
		if err != nil {
			fmt.Println("* failed to read ACL config (but it's fine, you can ignore this message)")
			fmt.Println("*   err:", err)
		}

		for _, r := range acl.OmitRules {
			fmt.Println("* ACL omit rule:", r)
		}

		cc = &proxy.ClientConfig{
			UserAuth:       *cmdAuth,
			Upstream:       *cmdUpstream,
			UDPRelayCoconn: int(*cmdUDPonTCP),
			Cipher:         cipher,
			DNSCache:       lru.NewCache(int(*cmdDNSCache)),
			CACache:        lru.NewCache(256),
			ACL:            acl,
			Mux:            int(*cmdMux),
		}

		if is := func(in string) bool { return strings.HasPrefix(*cmdUpstream, in) }; is("https://") {
			cc.Connect2Auth, cc.Connect2, _, cc.Upstream = parseAuthURL(*cmdUpstream)
			fmt.Println("* use HTTPS proxy [", cc.Connect2, "] as the frontend, proxy auth: [", cc.Connect2Auth, "]")
		} else if gfw, http, ws, cf, fwd, fwdws :=
			is("gfw://"), is("http://"), is("ws://"), is("cf://"), is("fwd://"), is("fwds://"); gfw || http || ws || cf || fwd || fwdws {

			cc.Connect2Auth, cc.Upstream, cc.URLHeader, cc.DummyDomain = parseAuthURL(*cmdUpstream)

			switch true {
			case cf:
				fmt.Println("* connect to the upstream [", cc.Upstream, "] hosted on cloudflare")
				cc.DummyDomain = cc.Upstream
			case fwdws, fwd:
				if cc.URLHeader == "" {
					cc.URLHeader = "X-Forwarded-Url"
				}
				fmt.Println("* forward request to [", cc.Upstream, "], store the true URL in [",
					cc.URLHeader+": http://"+cc.DummyDomain+"/... ]")
			case cc.DummyDomain != "":
				fmt.Println("* use dummy host [", cc.DummyDomain, "] to connect [", cc.Upstream, "]")
			}

			switch true {
			case fwdws, cf, ws:
				cc.Policy.Set(proxy.PolicyWebSocket)
				fmt.Println("* use WebSocket protocol to transfer data")
			case fwd, http:
				cc.Policy.Set(proxy.PolicyManInTheMiddle)
				fmt.Println("* use MITM to intercept HTTPS (HTTP proxy mode only)")
				cc.CA = lib.TryLoadCert()
			}
		}

		if *cmdGlobal {
			fmt.Println("* global proxy: goflyway will proxy everything except private IPs")
			cc.Policy.Set(proxy.PolicyGlobal)
		}

		if *cmdVPN {
			cc.Policy.Set(proxy.PolicyVPN)
		}
	}

	if *cmdUpstream == "" || *cmdDebug {
		sc = &proxy.ServerConfig{
			Cipher:        cipher,
			Throttling:    *cmdThrot,
			ThrottlingMax: *cmdThrotMax,
			ProxyPassAddr: *cmdProxyPass,
			DisableUDP:    *cmdDiableUDP,
		}

		if *cmdAuth != "" {
			sc.Users = map[string]proxy.UserConfig{
				*cmdAuth: {},
			}
		}
	}

	if *cmdLogFile != "" {
		logg.Redirect(*cmdLogFile)
		fmt.Println("* redirect log to", *cmdLogFile)
	}

	logg.SetLevel(*cmdLogLevel)
	logg.Start()

	if *cmdDebug {
		fmt.Println("* debug mode on")

		cc.Upstream = "127.0.0.1:8101"
		client := proxy.NewClient(":8100", cc)
		go func() {
			logg.F(client.Start())
		}()

		server := proxy.NewServer(":8101", sc)
		logg.F(server.Start())
		return
	}

	if *cmdCloseConn > 0 {
		cipher.IO.StartPurgeConns(int(*cmdCloseConn))
	}

	var localaddr string
	if *cmdLocal2 != "" {
		// -p has higher priority than -l, for the sack of SS users
		localaddr = *cmdLocal2
	} else {
		localaddr = *cmdLocal
	}

	if *cmdUpstream != "" {
		client := proxy.NewClient(localaddr, cc)

		if *cmdWebConPort != 0 {
			go func() {
				addr := fmt.Sprintf("127.0.0.1:%d", *cmdWebConPort)
				http.HandleFunc("/", lib.WebConsoleHTTPHandler(client))
				fmt.Println("* access client web console at [", addr, "]")
				logg.F(http.ListenAndServe(addr, nil))
			}()
		}

		fmt.Println("* proxy", client.Cipher.Alias, "started at [", client.Localaddr, "], upstream: [", client.Upstream, "]")
		logg.F(client.Start())
	} else {
		server := proxy.NewServer(localaddr, sc)
		fmt.Println("* upstream", server.Cipher.Alias, "started at [", server.Localaddr, "]")
		if strings.HasPrefix(sc.ProxyPassAddr, "http") {
			fmt.Println("* alternatively act as a reverse proxy:", sc.ProxyPassAddr)
		} else if sc.ProxyPassAddr != "" {
			fmt.Println("* alternatively act as a file server:", sc.ProxyPassAddr)
		}
		logg.F(server.Start())
	}
}

func parseAuthURL(in string) (auth string, upstream string, header string, dummy string) {
	// <scheme>://[<username>:<password>@]<host>:<port>[/[?<header>=]<dummy_host>:<dummy_port>]
	if idx := strings.Index(in, "://"); idx > -1 {
		in = in[idx+3:]
	}

	if idx := strings.Index(in, "/"); idx > -1 {
		dummy = in[idx+1:]
		in = in[:idx]
		if idx = strings.Index(dummy, "="); dummy[0] == '?' && idx > -1 {
			header = dummy[1:idx]
			dummy = dummy[idx+1:]
		}
	}

	upstream = in
	if idx := strings.Index(in, "@"); idx > -1 {
		auth = in[:idx]
		upstream = in[idx+1:]
	}

	if _, _, err := net.SplitHostPort(upstream); err != nil {
		fmt.Println("* invalid upstream destination:", upstream, err)
		os.Exit(1)
	}

	return
}
