package lib

import (
	"strconv"

	"github.com/coyove/goflyway/pkg/aclrouter"

	"github.com/coyove/goflyway/pkg/lru"
	pp "github.com/coyove/goflyway/proxy"

	"bytes"
	"fmt"
	"net/http"
	"strings"
	"text/template"
)

var webConsoleHTML, _ = template.New("console").Parse(`<!DOCTYPE html>
    <html><title>{{.I18N.Title}}</title>
    <link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAFoAAABaAQMAAAACZtNBAAAABlBMVEVycYL///9g0YTYAAAANUlEQVQ4y2MYBMD+/x8Q8f//wHE+MP8HEQPFgbgERAwQZ1AAoEvgAUJ/zmBJiQwDwxk06QAA91Y8PCo+T/8AAAAASUVORK5CYII='>

    <style>
        *                                  { font-family: Arial, Helvetica, sans-serif; box-sizing: border-box; font-size: 12px; }
        #traffic                           { width: 100%; overflow: hidden; height: auto; cursor: pointer; vertical-align: bottom; }
        #search                            { width: 100%; border: none; padding: 4px; left: 8px; top: 4px; height: 100%; margin: -4px -8px; position: absolute; background: #fafafa; min-height: 20px; }
        table#dns                          { border-collapse: collapse; margin: 4px auto; width: 100%; max-width: 600px }
        table#dns td, table#dns th         { border: solid 1px rgba(0,0,0,0.1); padding: 4px 8px; }
        table#dns td.fit, table#dns th.fit { white-space: nowrap; }
        table#dns td.ip                    { width: 135px; text-align: right; }
        table#dns td.ip, table#dns td.ip * { font-family: "Lucida Console", Monaco, monospace; border-left: none; }
        table#dns td.host                  { border-right: none; text-align: left; max-width: 380px; overflow: hidden; }
        table#dns td.ip a 	               { text-decoration: none; color: black; }
        table#dns td.ip a span:before, table#dns td.ip a span.p1:after { content: '\00a0'; }
        table#dns td.rule                        { text-align: center; padding: 0; }
        table#dns td.rule[rule=Block] 		     { background: #F44336; color:white; }
        table#dns td.rule[rule=Private] 		 { background: #5D4037; color:white; }
        table#dns td.rule[rule=MatchedPass] 	 { background: #00796B; color:white; }
        table#dns td.rule[rule=Pass] 		     { background: #00796B; color:white; }
        table#dns td.rule[rule=MatchedProxy]     { background: #FBC02D; }
        table#dns td.rule[rule=Proxy] 		     { background: #FBC02D; }
        table#dns td.rule[rule=IPv6] 		     { background: #7B1FA2; color:white; }
        table#dns td.rule[rule=Unknown] 		 { background: #512DA8; color:white; }
        table#dns td.side-rule     		         { width: 6px; min-width: 6px; max-width: 6px; padding: 0; cursor: pointer }
        table#dns td.side-rule[rule=Pass]        { background: #0EAB99; }
        table#dns td.side-rule[rule=Proxy]       { background: #FDD97F; }
        table#dns td.side-rule[rule=Block]       { background: #EB918A; }
        table#dns td.side-rule[rule=Pass]:hover  { background: #00796B; }
        table#dns td.side-rule[rule=Proxy]:hover { background: #FBC02D; }
        table#dns td.side-rule[rule=Block]:hover { background: #F44336; }
        table#dns td.side-rule.old[rule=Pass]    { background: repeating-linear-gradient(45deg, #5d4037,#5d4037 2.5px,#0EAB99 2.5px,#0EAB99 4px); }
        table#dns td.side-rule.old[rule=Proxy]   { background: repeating-linear-gradient(45deg, #5d4037,#5d4037 2.5px,#FDD97F 2.5px,#FDD97F 4px); }
        table#dns td.side-rule.old[rule=Block]   { background: repeating-linear-gradient(45deg, #5d4037,#5d4037 2.5px,#EB918A 2.5px,#EB918A 4px); }
        table#dns tr:nth-child(odd) 	   { background-color: #e3e4e5; }
        table#dns tr.traffic td            { padding: 0 }
        table#dns tr.last-tr               { visibility: hidden; }
        table#dns tr.last-tr td            { border: 0; }
        .dropdown                          { position: relative; }
        .dropdown ul                       { padding: 0; margin: 0; list-style: none; display: none; position: absolute; right: 0; border: solid 1px #ccc; background: #f1f2f3; }
        .dropdown:hover ul                 { display: inherit; box-shadow: 0 1px 2px #ccc; }
        .dropdown ul li a                  { display: block; border-bottom: solid 1px #ccc; text-align: left; text-decoration: none; color: black }
        .dropdown ul li:last-child a       { border: none; }
        .dropdown ul li a.item             { padding: 4px 12px 4px 8px; background: #f1f2f3; }
        .dropdown ul li a.sep              { background: #ddd; font-size: 0.8em; padding: 2px; }
        .dropdown ul li a.item:before      { content: '\00a0'; display: inline-block; width: 16px; }
        .dropdown ul li a.checked:before   { content: '\25cf'; }
        .dropdown ul li a.item:hover       { background: #676677; color: white; }
    </style>

    <body style='text-align: center'>
    <a href="https://github.com/coyove/goflyway/wiki" target="_blank">
    <svg viewBox="0 0 9 9" width=80 height=80><path fill="#667" d="M0 5h4v1H3v1H2v1H1V5h5v1h1V5h1v3H5V2h1v1h1V2H2v1h1V2h1v2H1V1h2v1h2V1h3v3H5v1H0v4h9V0H0"/></svg>
    </a>

    <script>
    function iter(sel, callback) {
        var items = document.querySelectorAll(sel);
        for (var i = 0; i < items.length; i++) callback(items[i], i);
    }

    function search(e) {
        try {
            var re = new RegExp(e.value.toLowerCase() || ".*");
            iter("#dns .citem", function (e) { e.style.display = e.childNodes[0].innerHTML.match(re) ? "" : "none"; });
        } catch (ex) {}
    }

    function post(data, callback) {
        var http = new XMLHttpRequest();
        http.open("POST", "", true);
        http.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        http.onreadystatechange = function () { callback(http) };
        http.send(data);
    }
    
    function update(el) {
        var rule = el.getAttribute("rule"), tdr = el.parentNode.querySelectorAll("td.r"), rules = ["Proxy", "Pass", "Block"];

        post("target=" + el.parentNode.childNodes[0].getAttribute("host") + "&update=" + rule, function(http) {
            if (rules.indexOf(http.responseText) == -1) return;
            var setter = function(e,c,o,h) { e.setAttribute("colspan", c); e.setAttribute("onclick", o); e.innerHTML = h;}
            for (var i = 0 ; i < 3; i++) {
                tdr[i].className = "r side-rule " + rules[i] + (rules[i] == http.responseText ? " old" : "");
                tdr[i].setAttribute("rule", rules[i]);
                setter(tdr[i], "1", "update(this)", "");
            }
            el.className = el.className.replace("side-", "");
            setter(el, "11", "", rule);
        });
    }

    function updateRuleFilter(el) {
        el.className = el.className.indexOf("checked") > -1 ? "item rule" : "item rule checked";
        var rules = [];
        iter("#rule-menu .rule", function (e) { 
            if (e.className.indexOf("checked") > -1) { 
                rules.push(e.innerHTML); 
                rules.push("M-" + e.innerHTML); 
            }
        });

        iter("#dns .citem", function (e) { e.style.display = rules.indexOf((e.querySelector("td.rule") || {}).innerHTML) > -1 ? "" : "none"; });
    }

    function toggle(t) {
        post(t + "=" + t, function() { location.reload(); });
    }
    </script>
    
    <table id=dns>
        <tr>
            <th class=fit colspan=2 style="position:relative;min-width:100px;height:100%;overflow:hidden;text-align:left">
            <input onkeyup="search(this)" id="search" placeholder="{{.I18N.Filter}} ({{.Entries}} {{.I18N.Host}})"/>
            </th>
            <th colspan=13 class=fit>
                <div id=rule-menu class=dropdown>{{.I18N.Rule}} &#9662;<ul>
                    <li><a href="#" class="sep">{{.I18N.Basic}}</a></li>
                    <li><a href="#" onclick="toggle('proxy')" class="item {{if .Global}}checked{{end}}">{{.I18N.GlobalOn}}</a></li>
                    <li><a href="#" onclick="toggle('cleardns')" class="item">{{.I18N.ClearDNS}}</a></li>
                    <li><a href="#" onclick="toggle('reset')" class="item">{{.I18N.Reset}}</a></li>
                    <li><a href="#" class="sep">{{.I18N.Show}}</a></li>
                    <li><a href="#" onclick="updateRuleFilter(this)" class="item checked rule">Pass</a></li>
                    <li><a href="#" onclick="updateRuleFilter(this)" class="item checked rule">Proxy</a></li>
                    <li><a href="#" onclick="updateRuleFilter(this)" class="item checked rule">Block</a></li>
                    <li><a href="#" onclick="updateRuleFilter(this)" class="item checked rule">Private</a></li>
                    <li><a href="#" onclick="updateRuleFilter(this)" class="item checked rule">IPv6</a></li>
                    <li><a href="#" onclick="updateRuleFilter(this)" class="item checked rule">Unknown</a></li>
                </ul></div>
            </th>
        </tr>
        <tr class=traffic>
            <td colspan=15><img id="traffic" src="" log=0 onclick="switchSVG(this)"/></td>
        </tr>
        {{.DNS}}
    </table>

    <script>
    function switchSVG(el) {
        if (el) el.setAttribute("log", Math.abs(el.getAttribute("log") - 1));
        
        var log = document.getElementById('traffic').getAttribute("log") == 1;
        document.getElementById('traffic').src = "/traffic.svg?" + (log ? "log=1&c=" : "c=") + (new Date().getTime());
        document.cookie = "log=" + (log ? "1" : "0") + "; expires=Sat, 1 Jan 2050 00:00:00 GMT; path=/";
    }

    document.getElementById('traffic').setAttribute("log", (/log[^;]+/.exec(document.cookie)||"").toString() == "log=1" ? 1 : 0);
    switchSVG();
    setInterval(switchSVG, 5000);

    iter("#dns .citem", function (e) {
        var o = e.childNodes[e.childNodes.length - 1].nodeValue;
        if (e.querySelector(".rule").innerHTML == o) return;
        (e.querySelector("[rule=" + o + "]") || {}).className += " old";
    });
    </script>
    </body>
`)

var _i18n = map[string]map[string]string{
	"en": {
		"Title":     "goflyway web console",
		"Basic":     "Basic",
		"ClearDNS":  "Clear rules cache",
		"Host":      "Host(s)",
		"Hits":      "Hits",
		"Clear":     "Clear",
		"Filter":    "Filter string",
		"Show":      "Show",
		"CertCache": "Cert Cache",
		"Rule":      "Rule",
		"OldRule":   "Old Rule",
		"GlobalOn":  "Enable global proxy",
		"Reset":     "Reset changed rules",
	},
	"zh": {
		"Title":     "goflyway 控制台",
		"Basic":     "基本设置",
		"ClearDNS":  "清除规则缓存",
		"Host":      "域名",
		"Hits":      "访问次数",
		"Clear":     "清除",
		"Filter":    "过滤",
		"Show":      "显示",
		"CertCache": "证书缓存",
		"Rule":      "规则",
		"OldRule":   "旧规则",
		"GlobalOn":  "全局代理",
		"Reset":     "重置规则",
	},
}

var ruleMappingLeft = []string{
	"<td onclick=update(this) class='r side-rule' rule=Proxy ></td>",
	"",
	"<td onclick=update(this) class='r side-rule' rule=Proxy ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Proxy ></td>",
	"<td colspan=11 class='r rule' rule=MatchedProxy >M-Proxy</td>",
	"<td colspan=11 class='r rule' rule=Proxy >Proxy</td>",
	"<td colspan=11 class='r rule' rule=IPv6 >IPv6</td>",
	"<td colspan=11 class='r rule' rule=Unknown >Unknown</td>",
}

var ruleMapping = []string{
	"<td onclick=update(this) class='r side-rule' rule=Pass ></td>",
	"<td colspan=13 class='r rule' rule=Private >Private</td>",
	"<td colspan=11 class='r rule' rule=MatchedPass >M-Pass</td>",
	"<td colspan=11 class='r rule' rule=Pass >Pass</td>",
	"<td onclick=update(this) class='r side-rule' rule=Pass ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Pass ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Pass ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Pass ></td>",
}

var ruleMappingRight = []string{
	"<td colspan=11 class='r rule' rule=Block >Block</td>",
	"",
	"<td onclick=update(this) class='r side-rule' rule=Block ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Block ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Block ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Block ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Block ></td>",
	"<td onclick=update(this) class='r side-rule' rule=Block ></td>",
}

func toString(ans byte) string {
	return []string{"Proxy", "Pass", "Block"}[ans]
}

func WebConsoleHTTPHandler(proxy *pp.ProxyClient) func(w http.ResponseWriter, r *http.Request) {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method == "GET" {

			if strings.HasPrefix(r.RequestURI, "/traffic.svg") {
				w.Header().Add("Content-Type", "image/svg+xml")
				w.Write(proxy.IO.Tr.SVG(300, 50, r.FormValue("log") == "1").Bytes())
				return
			}

			payload := struct {
				Global       bool
				Entries      int
				EntriesRatio int
				DNS          string
				I18N         map[string]string
			}{}

			buf, count := &bytes.Buffer{}, 0

			proxy.DNSCache.Info(func(k lru.Key, v interface{}, h int64) {
				count++
				rule := v.(*pp.Rule)
				ip, r := rule.IP, rule.R

				if aclrouter.IPv4ToInt(ip) > 0 {
					ips := make([]string, 4)
					for i, s := range strings.Split(ip, ".") {
						if ips[i] = s; len(s) < 3 {
							ips[i] = "<span class=p" + strconv.Itoa(len(s)) + ">" + s + "</span>"
						}
					}
					ip = fmt.Sprintf("<a href='http://freeapi.ipip.net/%v' target=_blank>%v</a>", ip, strings.Join(ips, "."))
				} else {
					ip = "<a><span class=p1>-</span>.<span class=p1>-</span>.<span class=p1>-</span>.<span class=p1>-</span></a>"
				}

				buf.WriteString(fmt.Sprintf(`<tr class=citem><td class="fit host" host="%v">%v (%d)</td><td class="fit ip">%s</td>%s%s%s<!--%s--></tr>`,
					k, k, h, ip, ruleMappingLeft[r], ruleMapping[r], ruleMappingRight[r], toString(rule.OldAns)))
			})

			buf.WriteString(fmt.Sprintf("<tr class=last-tr><td></td><td></td>%s</tr>", strings.Repeat("<td class=side-rule></td>", 13)))

			payload.DNS = buf.String()
			payload.Global = proxy.Policy.IsSet(pp.PolicyGlobal)
			payload.Entries = count
			payload.EntriesRatio = count * 100 / proxy.DNSCache.MaxEntries

			// use lang=en to force english display
			if strings.Contains(r.Header.Get("Accept-Language"), "zh") && r.FormValue("lang") != "en" {
				payload.I18N = _i18n["zh"]
			} else {
				payload.I18N = _i18n["en"]
			}

			webConsoleHTML.Execute(w, payload)
		} else if r.Method == "POST" {
			if r.FormValue("cleardns") != "" {
				proxy.DNSCache.Clear()
				w.WriteHeader(200)
				return
			}

			if r.FormValue("reset") != "" {
				keys := []string{}
				proxy.DNSCache.Info(func(k lru.Key, v interface{}, h int64) {
					if r := v.(*pp.Rule); r.OldAns != r.Ans {
						keys = append(keys, k.(string))
					}
				})

				for _, k := range keys {
					if v, ok := proxy.DNSCache.Get(k); ok {
						v.(*pp.Rule).Ans = v.(*pp.Rule).OldAns
						proxy.DNSCache.Add(k, v)
					}
				}

				w.WriteHeader(200)
				return
			}

			if r.FormValue("proxy") != "" {
				if proxy.Policy.IsSet(pp.PolicyGlobal) {
					proxy.Policy.UnSet(pp.PolicyGlobal)
				} else {
					proxy.Policy.Set(pp.PolicyGlobal)
				}
				w.WriteHeader(200)
				return
			}

			if rule := r.FormValue("update"); rule != "" {
				target := r.FormValue("target")
				if v, ok := proxy.DNSCache.Get(target); ok {
					oldRule := v.(*pp.Rule)
					old := oldRule.OldAns
					switch rule {
					case "Proxy":
						oldRule.Ans = 0
						oldRule.R = aclrouter.RuleProxy
					case "Pass":
						oldRule.Ans = 1
						oldRule.R = aclrouter.RulePass
					case "Block":
						oldRule.Ans = 2
						oldRule.R = aclrouter.RuleBlock
					}
					proxy.DNSCache.Add(target, oldRule)
					w.Write([]byte(toString(old)))
				} else {
					w.Write([]byte("error"))
				}
				return
			}

			w.Write([]byte("error"))
		}
	}
}
