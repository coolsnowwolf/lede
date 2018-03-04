package proxy

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net/http"
)

func (proxy *ProxyClient) servePACFile(w http.ResponseWriter, r *http.Request) {
	table, _ := json.Marshal(proxy.ACL.White.DomainFastMatch)
	table2 := proxy.ACL.White.DomainSlowMatch
	t := "SOCKS5"
	if proxy.Policy.IsSet(PolicyManInTheMiddle) {
		t = "PROXY"
	}

	pac := &bytes.Buffer{}
	pac.WriteString("function slowMatch(host) { ")
	if len(table2) > 0 {
		pac.WriteString("return ")
		for _, r := range table2 {
			pac.WriteString("host.match(/" + r.String() + "/)||")
		}
		pac.Truncate(pac.Len() - 2)
	} else {
		pac.WriteString("return false")
	}
	pac.WriteString(` } 
		var table = `)
	pac.Write(table)
	pac.WriteString(fmt.Sprintf(`;
		function isInTable(host) {
			var cands = host.split('.');
			if (cands.length <= 1)
				return slowMatch(host);
		
			var _table = table;
			for (var i = cands.length - 1; i >= 0; i--) {
				var cand = cands[i];
				if (!(cand in _table))
					return slowMatch(host);
		
				if (_table[cand] === 0)
					return true;
				
				_table = _table[cand];
			}

			return slowMatch(host);
		}
		
		function FindProxyForURL(url, host) {
			if (isPlainHostName(host) ||
				shExpMatch(host, "*.local") ||
				isInNet(dnsResolve(host), "10.0.0.0", "255.0.0.0") ||
				isInNet(dnsResolve(host), "172.16.0.0",  "255.240.0.0") ||
				isInNet(dnsResolve(host), "192.168.0.0",  "255.255.0.0") ||
				isInNet(dnsResolve(host), "127.0.0.0", "255.255.255.0") ||
				isInTable(host))
				return "DIRECT";
			
			return "%s %s";
		}`, t, r.Host))

	w.Write(pac.Bytes())
}
