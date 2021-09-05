#ifndef __NGINX_SSL_UTIL_HPP
#define __NGINX_SSL_UTIL_HPP

#ifdef NO_PCRE
#include <regex>
namespace rgx = std;
#else
#include "regex-pcre.hpp"
#endif

#include "nginx-util.hpp"
#include "px5g-openssl.hpp"

#ifndef NO_UBUS
static constexpr auto UBUS_TIMEOUT = 1000;
#endif

// once a year:
static constexpr auto CRON_INTERVAL = std::string_view{"3 3 12 12 *"};

static constexpr auto LAN_SSL_LISTEN = std::string_view{"/var/lib/nginx/lan_ssl.listen"};

static constexpr auto LAN_SSL_LISTEN_DEFAULT =  // TODO(pst) deprecate
    std::string_view{"/var/lib/nginx/lan_ssl.listen.default"};

static constexpr auto ADD_SSL_FCT = std::string_view{"add_ssl"};

static constexpr auto SSL_SESSION_CACHE_ARG = [](const std::string_view & /*name*/) -> std::string {
    return "shared:SSL:32k";
};

static constexpr auto SSL_SESSION_TIMEOUT_ARG = std::string_view{"64m"};

using _Line = std::array<std::string (*)(const std::string&, const std::string&), 2>;

class Line {
  private:
    _Line _line;

  public:
    explicit Line(const _Line& line) noexcept : _line{line} {}

    template <const _Line&... xn>
    static auto build() noexcept -> Line
    {
        return Line{_Line{[](const std::string& p, const std::string& b) -> std::string {
                              return (... + xn[0](p, b));
                          },
                          [](const std::string& p, const std::string& b) -> std::string {
                              return (... + xn[1](p, b));
                          }}};
    }

    [[nodiscard]] auto STR(const std::string& param, const std::string& begin) const -> std::string
    {
        return _line[0](param, begin);
    }

    [[nodiscard]] auto RGX() const -> rgx::regex
    {
        return rgx::regex{_line[1]("", "")};
    }
};

auto get_if_missed(const std::string& conf,
                   const Line& LINE,
                   const std::string& val,
                   const std::string& indent = "\n    ",
                   bool compare = true) -> std::string;

auto replace_if(const std::string& conf,
                const rgx::regex& rgx,
                const std::string& val,
                const std::string& insert) -> std::string;

auto replace_listen(const std::string& conf, const std::array<const char*, 2>& ngx_port)
    -> std::string;

auto check_ssl_certificate(const std::string& crtpath, const std::string& keypath) -> bool;

auto contains(const std::string& sentence, const std::string& word) -> bool;

auto get_uci_section_for_name(const std::string& name) -> uci::section;

void add_ssl_if_needed(const std::string& name);

void add_ssl_if_needed(const std::string& name,
                       std::string_view manage,
                       std::string_view crt,
                       std::string_view key);

void install_cron_job(const Line& CRON_LINE, const std::string& name = "");

void remove_cron_job(const Line& CRON_LINE, const std::string& name = "");

auto del_ssl_legacy(const std::string& name) -> bool;

void del_ssl(const std::string& name);

void del_ssl(const std::string& name, std::string_view manage);

auto check_ssl(const uci::package& pkg, bool is_enabled) -> bool;

inline void check_ssl(const uci::package& pkg)
{
    if (!check_ssl(pkg, is_enabled(pkg))) {
#ifndef NO_UBUS
        if (ubus::call("service", "list", UBUS_TIMEOUT).filter("nginx")) {
            call("/etc/init.d/nginx", "reload");
            std::cerr << "Reload Nginx.\n";
        }
#endif
    }
}

constexpr auto _begin = _Line{
    [](const std::string& /*param*/, const std::string& begin) -> std::string { return begin; },

    [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
        return R"([{;](?:\s*#[^\n]*(?=\n))*(\s*))";
    }};

constexpr auto _space = _Line{[](const std::string& /*param*/, const std::string &
                                 /*begin*/) -> std::string { return std::string{" "}; },

                              [](const std::string& /*param*/, const std::string &
                                 /*begin*/) -> std::string { return R"(\s+)"; }};

constexpr auto _newline = _Line{
    [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
        return std::string{"\n"};
    },

    [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
        return std::string{"(\n)"};
    }  // capture it as _end captures it, too.
};

constexpr auto _end =
    _Line{[](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
              return std::string{";"};
          },

          [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
              return std::string{R"(\s*(;(?:[\t ]*#[^\n]*)?))"};
          }};

template <char clim = '\0'>
static constexpr auto _capture = _Line{
    [](const std::string& param, const std::string & /*begin*/) -> std::string {
        return '\'' + param + '\'';
    },

    [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
        const auto lim = clim == '\0' ? std::string{"\\s"} : std::string{clim};
        return std::string{R"(((?:(?:"[^"]*")|(?:[^'")"} + lim + "][^" + lim + "]*)|(?:'[^']*'))+)";
    }};

template <const std::string_view& strptr, char clim = '\0'>
static constexpr auto _escape = _Line{
    [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
        return clim == '\0' ? std::string{strptr.data()} : clim + std::string{strptr.data()} + clim;
    },

    [](const std::string& /*param*/, const std::string & /*begin*/) -> std::string {
        std::string ret{};
        for (char c : strptr) {
            switch (c) {
                case '^':
                    ret += '\\';
                    ret += c;
                    break;
                case '_':
                case '-':
                    ret += c;
                    break;
                default:
                    if ((isalpha(c) != 0) || (isdigit(c) != 0)) {
                        ret += c;
                    }
                    else {
                        ret += std::string{"["} + c + "]";
                    }
            }
        }
        return "(?:" + ret + "|'" + ret + "'" + "|\"" + ret + "\"" + ")";
    }};

constexpr std::string_view _check_ssl = "check_ssl";

constexpr std::string_view _server_name = "server_name";

constexpr std::string_view _listen = "listen";

constexpr std::string_view _include = "include";

constexpr std::string_view _ssl_certificate = "ssl_certificate";

constexpr std::string_view _ssl_certificate_key = "ssl_certificate_key";

constexpr std::string_view _ssl_session_cache = "ssl_session_cache";

constexpr std::string_view _ssl_session_timeout = "ssl_session_timeout";

// For a compile time regex lib, this must be fixed, use one of these options:
// * Hand craft or macro concat them (loosing more or less flexibility).
// * Use Macro concatenation of __VA_ARGS__ with the help of:
//   https://p99.gforge.inria.fr/p99-html/group__preprocessor__for.html
// * Use constexpr---not available for strings or char * for now---look at lib.

static const auto CRON_CHECK =
    Line::build<_space, _escape<NGINX_UTIL>, _space, _escape<_check_ssl, '\''>, _newline>();

static const auto CRON_CMD = Line::build<_space,
                                         _escape<NGINX_UTIL>,
                                         _space,
                                         _escape<ADD_SSL_FCT, '\''>,
                                         _space,
                                         _capture<>,
                                         _newline>();

static const auto NGX_SERVER_NAME =
    Line::build<_begin, _escape<_server_name>, _space, _capture<';'>, _end>();

static const auto NGX_INCLUDE_LAN_LISTEN =
    Line::build<_begin, _escape<_include>, _space, _escape<LAN_LISTEN, '\''>, _end>();

static const auto NGX_INCLUDE_LAN_LISTEN_DEFAULT =
    Line::build<_begin, _escape<_include>, _space, _escape<LAN_LISTEN_DEFAULT, '\''>, _end>();

static const auto NGX_INCLUDE_LAN_SSL_LISTEN =
    Line::build<_begin, _escape<_include>, _space, _escape<LAN_SSL_LISTEN, '\''>, _end>();

static const auto NGX_INCLUDE_LAN_SSL_LISTEN_DEFAULT =
    Line::build<_begin, _escape<_include>, _space, _escape<LAN_SSL_LISTEN_DEFAULT, '\''>, _end>();

static const auto NGX_SSL_CRT =
    Line::build<_begin, _escape<_ssl_certificate>, _space, _capture<';'>, _end>();

static const auto NGX_SSL_KEY =
    Line::build<_begin, _escape<_ssl_certificate_key>, _space, _capture<';'>, _end>();

static const auto NGX_SSL_SESSION_CACHE =
    Line::build<_begin, _escape<_ssl_session_cache>, _space, _capture<';'>, _end>();

static const auto NGX_SSL_SESSION_TIMEOUT =
    Line::build<_begin, _escape<_ssl_session_timeout>, _space, _capture<';'>, _end>();

static const auto NGX_LISTEN = Line::build<_begin, _escape<_listen>, _space, _capture<';'>, _end>();

static const auto NGX_PORT_80 = std::array<const char*, 2>{
    R"(^\s*([^:]*:|\[[^\]]*\]:)?80(\s|$|;))",
    "$01443 ssl$2",
};

static const auto NGX_PORT_443 = std::array<const char*, 2>{
    R"(^\s*([^:]*:|\[[^\]]*\]:)?443(\s.*)?\sssl(\s|$|;))",
    "$0180$2$3",
};

// ------------------------- implementation: ----------------------------------

auto get_if_missed(const std::string& conf,
                   const Line& LINE,
                   const std::string& val,
                   const std::string& indent,
                   bool compare) -> std::string
{
    if (!compare || val.empty()) {
        return rgx::regex_search(conf, LINE.RGX()) ? "" : LINE.STR(val, indent);
    }

    rgx::smatch match;  // assuming last capture has the value!

    for (auto pos = conf.begin(); rgx::regex_search(pos, conf.end(), match, LINE.RGX());
         pos += match.position(0) + match.length(0))
    {
        const std::string value = match.str(match.size() - 2);

        if (value == val || value == "'" + val + "'" || value == '"' + val + '"') {
            return "";
        }
    }

    return LINE.STR(val, indent);
}

auto replace_if(const std::string& conf,
                const rgx::regex& rgx,
                const std::string& val,
                const std::string& insert) -> std::string
{
    std::string ret{};
    auto pos = conf.begin();

    auto skip = 0;
    for (rgx::smatch match; rgx::regex_search(pos, conf.end(), match, rgx);
         pos += match.position(match.size() - 1))
    {
        auto i = match.size() - 2;
        const std::string value = match.str(i);

        bool compare = !val.empty();
        if (compare && value != val && value != "'" + val + "'" && value != '"' + val + '"') {
            ret.append(pos + skip, pos + match.position(i) + match.length(i));
            skip = 0;
        }
        else {
            ret.append(pos + skip, pos + match.position(match.size() > 2 ? 1 : 0));
            ret += insert;
            skip = 1;
        }
    }

    ret.append(pos + skip, conf.end());
    return ret;
}

auto replace_listen(const std::string& conf, const std::array<const char*, 2>& ngx_port)
    -> std::string
{
    std::string ret{};
    auto pos = conf.begin();

    for (rgx::smatch match; rgx::regex_search(pos, conf.end(), match, NGX_LISTEN.RGX());
         pos += match.position(match.size() - 1))
    {
        auto i = match.size() - 2;
        ret.append(pos, pos + match.position(i));
        ret += rgx::regex_replace(match.str(i), rgx::regex{ngx_port[0]}, ngx_port[1]);
    }

    ret.append(pos, conf.end());
    return ret;
}

inline void add_ssl_directives_to(const std::string& name)
{
    const std::string prefix = std::string{CONF_DIR} + name;

    const std::string const_conf = read_file(prefix + ".conf");

    rgx::smatch match;  // captures str(1)=indentation spaces, str(2)=server name
    for (auto pos = const_conf.begin();
         rgx::regex_search(pos, const_conf.end(), match, NGX_SERVER_NAME.RGX());
         pos += match.position(0) + match.length(0))
    {
        if (!contains(match.str(2), name)) {
            continue;
        }  // else:

        const std::string indent = match.str(1);

        auto adds = std::string{};

        adds += get_if_missed(const_conf, NGX_SSL_CRT, prefix + ".crt", indent);

        adds += get_if_missed(const_conf, NGX_SSL_KEY, prefix + ".key", indent);

        adds += get_if_missed(const_conf, NGX_SSL_SESSION_CACHE, SSL_SESSION_CACHE_ARG(name),
                              indent, false);

        adds += get_if_missed(const_conf, NGX_SSL_SESSION_TIMEOUT,
                              std::string{SSL_SESSION_TIMEOUT_ARG}, indent, false);

        pos += match.position(0) + match.length(0);
        std::string conf =
            std::string(const_conf.begin(), pos) + adds + std::string(pos, const_conf.end());

        conf = replace_if(conf, NGX_INCLUDE_LAN_LISTEN_DEFAULT.RGX(), "",
                          NGX_INCLUDE_LAN_SSL_LISTEN_DEFAULT.STR("", indent));

        conf = replace_if(conf, NGX_INCLUDE_LAN_LISTEN.RGX(), "",
                          NGX_INCLUDE_LAN_SSL_LISTEN.STR("", indent));

        conf = replace_listen(conf, NGX_PORT_80);

        if (conf != const_conf) {
            write_file(prefix + ".conf", conf);
            std::cerr << "Added SSL directives to " << prefix << ".conf\n";
        }

        return;
    }

    auto errmsg = std::string{"add_ssl_directives_to error: "};
    errmsg += "cannot add SSL directives to " + name + ".conf, missing: ";
    errmsg += NGX_SERVER_NAME.STR(name, "\n    ") + "\n";
    throw std::runtime_error(errmsg);
}

template <typename T>
inline auto num2hex(T bytes) -> std::array<char, 2 * sizeof(bytes) + 1>
{
    constexpr auto n = 2 * sizeof(bytes);
    std::array<char, n + 1> str{};

    for (size_t i = 0; i < n; ++i) {
        static const std::array<char, 17> hex{"0123456789ABCDEF"};
        static constexpr auto get = 0x0fU;
        str.at(i) = hex.at(bytes & get);

        static constexpr auto move = 4U;
        bytes >>= move;
    }

    str[n] = '\0';
    return str;
}

template <typename T>
inline auto get_nonce(const T salt = 0) -> T
{
    T nonce = 0;

    std::ifstream urandom{"/dev/urandom"};

    static constexpr auto move = 6U;

    constexpr size_t steps = (sizeof(nonce) * 8 - 1) / move + 1;

    for (size_t i = 0; i < steps; ++i) {
        if (!urandom.good()) {
            throw std::runtime_error("get_nonce error");
        }
        nonce = (nonce << move) + static_cast<unsigned>(urandom.get());
    }

    nonce ^= salt;

    return nonce;
}

inline void create_ssl_certificate(const std::string& crtpath,
                                   const std::string& keypath,
                                   const int days = 792)
{
    size_t nonce = 0;

    try {
        nonce = get_nonce(nonce);
    }

    catch (...) {  // the address of a variable should be random enough:
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) sic:
        nonce += reinterpret_cast<size_t>(&crtpath);
    }

    auto noncestr = num2hex(nonce);

    const auto tmpcrtpath = crtpath + ".new-" + noncestr.data();
    const auto tmpkeypath = keypath + ".new-" + noncestr.data();

    try {
        auto pkey = gen_eckey(NID_secp384r1);

        write_key(pkey, tmpkeypath);

        std::string subject{"/C=ZZ/ST=Somewhere/L=None/CN=OpenWrt/O=OpenWrt"};
        subject += noncestr.data();

        selfsigned(pkey, days, subject, tmpcrtpath);

        static constexpr auto to_seconds = 24 * 60 * 60;
        static constexpr auto leeway = 42;
        if (!checkend(tmpcrtpath, days * to_seconds - leeway)) {
            throw std::runtime_error("bug: created certificate is not valid!!");
        }
    }
    catch (...) {
        std::cerr << "create_ssl_certificate error: ";
        std::cerr << "cannot create selfsigned certificate, ";
        std::cerr << "removing temporary files ..." << std::endl;

        if (remove(tmpcrtpath.c_str()) != 0) {
            auto errmsg = "\t cannot remove " + tmpcrtpath;
            perror(errmsg.c_str());
        }

        if (remove(tmpkeypath.c_str()) != 0) {
            auto errmsg = "\t cannot remove " + tmpkeypath;
            perror(errmsg.c_str());
        }

        throw;
    }

    if (rename(tmpcrtpath.c_str(), crtpath.c_str()) != 0 ||
        rename(tmpkeypath.c_str(), keypath.c_str()) != 0)
    {
        auto errmsg = std::string{"create_ssl_certificate warning: "};
        errmsg += "cannot move " + tmpcrtpath + " to " + crtpath;
        errmsg += " or " + tmpkeypath + " to " + keypath + ", continuing ... ";
        perror(errmsg.c_str());
    }

    std::cerr << "Created self-signed SSL certificate '" << crtpath;
    std::cerr << "' with key '" << keypath << "'.\n";
}

auto check_ssl_certificate(const std::string& crtpath, const std::string& keypath) -> bool
{
    {  // paths are relative to dir:
        auto dir = std::string_view{"/etc/nginx"};
        auto crt_rel = crtpath[0] != '/';
        auto key_rel = keypath[0] != '/';
        if ((crt_rel || key_rel) && (chdir(dir.data()) != 0)) {
            auto errmsg = std::string{"check_ssl_certificate error: entering "};
            errmsg += dir;
            perror(errmsg.c_str());
            errmsg += " (need to change directory since the given ";
            errmsg += crt_rel ? "ssl_certificate '" + crtpath : std::string{};
            errmsg += crt_rel && key_rel ? "' and " : "";
            errmsg += key_rel ? "ssl_certificate_key '" + keypath : std::string{};
            errmsg += crt_rel && key_rel ? "' are" : "' is a";
            errmsg += " relative path";
            errmsg += crt_rel && key_rel ? "s)" : ")";
            throw std::runtime_error(errmsg);
        }
    }

    constexpr auto remaining_seconds = (365 + 32) * 24 * 60 * 60;
    constexpr auto validity_days = 3 * (365 + 31);

    bool is_valid = true;

    if (access(keypath.c_str(), R_OK) != 0 || access(crtpath.c_str(), R_OK) != 0) {
        is_valid = false;
    }

    else {
        try {
            if (!checkend(crtpath, remaining_seconds)) {
                is_valid = false;
            }
        }
        catch (...) {  // something went wrong, maybe it is in DER format:
            try {
                if (!checkend(crtpath, remaining_seconds, false)) {
                    is_valid = false;
                }
            }
            catch (...) {  // it has neither DER nor PEM format, rebuild.
                is_valid = false;
            }
        }
    }

    if (!is_valid) {
        create_ssl_certificate(crtpath, keypath, validity_days);
    }

    return is_valid;
}

auto contains(const std::string& sentence, const std::string& word) -> bool
{
    auto pos = sentence.find(word);
    if (pos == std::string::npos) {
        return false;
    }
    if (pos != 0 && (isgraph(sentence[pos - 1]) != 0)) {
        return false;
    }
    if (isgraph(sentence[pos + word.size()]) != 0) {
        return false;
    }
    // else:
    return true;
}

auto get_uci_section_for_name(const std::string& name) -> uci::section
{
    auto pkg = uci::package{"nginx"};  // let it throw.

    auto uci_enabled = is_enabled(pkg);

    if (uci_enabled) {
        for (auto sec : pkg) {
            if (sec.name() == name) {
                return sec;
            }
        }
        // try interpreting 'name' as FQDN:
        for (auto sec : pkg) {
            for (auto opt : sec) {
                if (opt.name() == "server_name") {
                    for (auto itm : opt) {
                        if (contains(itm.name(), name)) {
                            return sec;
                        }
                    }
                }
            }
        }
    }

    auto errmsg = std::string{"lookup error: neither there is a file named '"};
    errmsg += std::string{CONF_DIR} + name + ".conf' nor the UCI config has ";
    if (uci_enabled) {
        errmsg += "a nginx server with section name or 'server_name': " + name;
    }
    else {
        errmsg += "been enabled by:\n\tuci set nginx.global.uci_enable=true";
    }
    throw std::runtime_error(errmsg);
}

inline auto add_ssl_to_config(const std::string& name,
                              const std::string_view manage = "self-signed",
                              const std::string_view crt = "",
                              const std::string_view key = "")
{
    auto sec = get_uci_section_for_name(name);  // let it throw.
    auto secname = sec.name();

    struct {
        std::string crt;
        std::string key;
    } ret;

    std::cerr << "Adding SSL directives to UCI server: nginx." << secname << "\n";

    std::cerr << "\t" << MANAGE_SSL << "='" << manage << "'\n";
    sec.set(MANAGE_SSL.data(), manage.data());

    if (!crt.empty() && !key.empty()) {
        sec.set("ssl_certificate", crt.data());
        std::cerr << "\tssl_certificate='" << crt << "'\n";
        sec.set("ssl_certificate_key", key.data());
        std::cerr << "\tssl_certificate_key='" << key << "'\n";
    }

    auto cache = false;
    auto timeout = false;
    for (auto opt : sec) {
        if (opt.name() == "ssl_session_cache") {
            cache = true;
            continue;
        }  // else:

        if (opt.name() == "ssl_session_timeout") {
            timeout = true;
            continue;
        }

        // else:
        for (auto itm : opt) {
            if (opt.name() == "ssl_certificate_key") {
                ret.key = itm.name();
            }

            else if (opt.name() == "ssl_certificate") {
                ret.crt = itm.name();
            }

            else if (opt.name() == "listen") {
                auto val = regex_replace(itm.name(), rgx::regex{NGX_PORT_80[0]}, NGX_PORT_80[1]);
                if (val != itm.name()) {
                    std::cerr << "\t" << opt.name() << "='" << val << "' (replacing)\n";
                    itm.rename(val.c_str());
                }
            }
        }
    }

    if (ret.crt.empty()) {
        ret.crt = std::string{CONF_DIR} + name + ".crt";
        std::cerr << "\tssl_certificate='" << ret.crt << "'\n";
        sec.set("ssl_certificate", ret.crt.c_str());
    }

    if (ret.key.empty()) {
        ret.key = std::string{CONF_DIR} + name + ".key";
        std::cerr << "\tssl_certificate_key='" << ret.key << "'\n";
        sec.set("ssl_certificate_key", ret.key.c_str());
    }

    if (!cache) {
        std::cerr << "\tssl_session_cache='" << SSL_SESSION_CACHE_ARG(name) << "'\n";
        sec.set("ssl_session_cache", SSL_SESSION_CACHE_ARG(name).data());
    }

    if (!timeout) {
        std::cerr << "\tssl_session_timeout='" << SSL_SESSION_TIMEOUT_ARG << "'\n";
        sec.set("ssl_session_timeout", SSL_SESSION_TIMEOUT_ARG.data());
    }

    sec.commit();

    return ret;
}

void install_cron_job(const Line& CRON_LINE, const std::string& name)
{
    static const char* filename = "/etc/crontabs/root";

    std::string conf{};
    try {
        conf = read_file(filename);
    }
    catch (const std::ifstream::failure&) { /* is ok if not found, create. */
    }

    const std::string add = get_if_missed(conf, CRON_LINE, name);

    if (add.length() > 0) {
#ifndef NO_UBUS
        if (!ubus::call("service", "list", UBUS_TIMEOUT).filter("cron")) {
            std::string errmsg{"install_cron_job error: "};
            errmsg += "Cron unavailable to re-create the ssl certificate";
            errmsg += (name.empty() ? std::string{"s\n"} : " for '" + name + "'\n");
            throw std::runtime_error(errmsg);
        }  // else active with or without instances:
#endif

        const auto* pre = (conf.length() == 0 || conf.back() == '\n' ? "" : "\n");
        write_file(filename, pre + std::string{CRON_INTERVAL} + add, std::ios::app);

#ifndef NO_UBUS
        call("/etc/init.d/cron", "reload");
#endif

        std::cerr << "Rebuild the self-signed SSL certificate";
        std::cerr << (name.empty() ? std::string{"s"} : " for '" + name + "'");
        std::cerr << " annually with cron." << std::endl;
    }
}

void add_ssl_if_needed(const std::string& name)
{
    const auto legacypath = std::string{CONF_DIR} + name + ".conf";
    if (access(legacypath.c_str(), R_OK) == 0) {
        add_ssl_directives_to(name);  // let it throw.

        const auto crtpath = std::string{CONF_DIR} + name + ".crt";
        const auto keypath = std::string{CONF_DIR} + name + ".key";
        check_ssl_certificate(crtpath, keypath);  // let it throw.

        try {
            install_cron_job(CRON_CMD, name);
        }
        catch (...) {
            std::cerr << "add_ssl_if_needed warning: cannot use cron to rebuild ";
            std::cerr << "the self-signed SSL certificate for " << name << "\n";
        }
        return;
    }  // else:

    auto paths = add_ssl_to_config(name);  // let it throw.

    check_ssl_certificate(paths.crt, paths.key);  // let it throw.

    try {
        install_cron_job(CRON_CHECK);
    }
    catch (...) {
        std::cerr << "add_ssl_if_needed warning: cannot use cron to rebuild ";
        std::cerr << "the self-signed SSL certificates.\n";
    }
}

void add_ssl_if_needed(const std::string& name,
                       const std::string_view manage,
                       const std::string_view crt,
                       const std::string_view key)
{
    if (crt[0] != '/') {
        auto errmsg = std::string{"add_ssl_if_needed error: ssl_certificate "};
        errmsg += "path cannot be relative '" + std::string{crt} + "'";
        throw std::runtime_error(errmsg);
    }

    if (key[0] != '/') {
        auto errmsg = std::string{"add_ssl_if_needed error: path to ssl_key "};
        errmsg += "cannot be relative '" + std::string{key} + "'";
        throw std::runtime_error(errmsg);
    }

    const auto legacypath = std::string{CONF_DIR} + name + ".conf";

    if (access(legacypath.c_str(), R_OK) != 0) {
        add_ssl_to_config(name, manage, crt, key);  // let it throw.
        return;
    }  // else:

    // symlink crt+key to the paths that add_ssl_directives_to uses (if needed):

    auto crtpath = std::string{CONF_DIR} + name + ".crt";
    if (crtpath != crt && /* then */ symlink(crt.data(), crtpath.c_str()) != 0) {
        auto errmsg = std::string{"add_ssl_if_needed error: cannot link "};
        errmsg += "ssl_certificate " + crtpath + " -> " + crt.data() + " (";
        errmsg += std::to_string(errno) + "): " + std::strerror(errno);
        throw std::runtime_error(errmsg);
    }

    auto keypath = std::string{CONF_DIR} + name + ".key";
    if (keypath != key && /* then */ symlink(key.data(), keypath.c_str()) != 0) {
        auto errmsg = std::string{"add_ssl_if_needed error: cannot link "};
        errmsg += "ssl_certificate_key " + keypath + " -> " + key.data() + " (";
        errmsg += std::to_string(errno) + "): " + std::strerror(errno);
        throw std::runtime_error(errmsg);
    }

    add_ssl_directives_to(name);  // let it throw.
}

void remove_cron_job(const Line& CRON_LINE, const std::string& name)
{
    static const char* filename = "/etc/crontabs/root";

    const auto const_conf = read_file(filename);

    bool changed = false;
    auto conf = std::string{};

    size_t prev = 0;
    size_t curr = 0;
    while ((curr = const_conf.find('\n', prev)) != std::string::npos) {
        auto line = const_conf.substr(prev, curr - prev + 1);

        if (line == replace_if(line, CRON_LINE.RGX(), name, "")) {
            conf += line;
        }
        else {
            changed = true;
        }

        prev = curr + 1;
    }

    if (changed) {
        write_file(filename, conf);

        std::cerr << "Do not rebuild the self-signed SSL certificate";
        std::cerr << (name.empty() ? std::string{"s"} : " for '" + name + "'");
        std::cerr << " annually with cron anymore." << std::endl;

#ifndef NO_UBUS
        if (ubus::call("service", "list", UBUS_TIMEOUT).filter("cron")) {
            call("/etc/init.d/cron", "reload");
        }
#endif
    }
}

inline void del_ssl_directives_from(const std::string& name)
{
    const std::string prefix = std::string{CONF_DIR} + name;

    const std::string const_conf = read_file(prefix + ".conf");

    rgx::smatch match;  // captures str(1)=indentation spaces, str(2)=server name
    for (auto pos = const_conf.begin();
         rgx::regex_search(pos, const_conf.end(), match, NGX_SERVER_NAME.RGX());
         pos += match.position(0) + match.length(0))
    {
        if (!contains(match.str(2), name)) {
            continue;
        }  // else:

        const std::string indent = match.str(1);

        std::string conf = const_conf;

        conf = replace_listen(conf, NGX_PORT_443);

        conf = replace_if(conf, NGX_INCLUDE_LAN_SSL_LISTEN_DEFAULT.RGX(), "",
                          NGX_INCLUDE_LAN_LISTEN_DEFAULT.STR("", indent));

        conf = replace_if(conf, NGX_INCLUDE_LAN_SSL_LISTEN.RGX(), "",
                          NGX_INCLUDE_LAN_LISTEN.STR("", indent));

        // NOLINTNEXTLINE(performance-inefficient-string-concatenation) prefix:
        conf = replace_if(conf, NGX_SSL_CRT.RGX(), prefix + ".crt", "");

        // NOLINTNEXTLINE(performance-inefficient-string-concatenation) prefix:
        conf = replace_if(conf, NGX_SSL_KEY.RGX(), prefix + ".key", "");

        conf = replace_if(conf, NGX_SSL_SESSION_CACHE.RGX(), "", "");

        conf = replace_if(conf, NGX_SSL_SESSION_TIMEOUT.RGX(), "", "");

        if (conf != const_conf) {
            write_file(prefix + ".conf", conf);
            std::cerr << "Deleted SSL directives from " << prefix << ".conf\n";
        }

        return;
    }

    auto errmsg = std::string{"del_ssl_directives_from error: "};
    errmsg += "cannot delete SSL directives from " + name + ".conf, missing: ";
    errmsg += NGX_SERVER_NAME.STR(name, "\n    ") + "\n";
    throw std::runtime_error(errmsg);
}

inline auto del_ssl_from_config(const std::string& name,
                                const std::string_view manage = "self-signed")
{
    auto sec = get_uci_section_for_name(name);  // let it throw.
    auto secname = sec.name();

    struct {
        std::string crt;
        std::string key;
    } ret;

    std::cerr << "Deleting SSL directives from UCI server: nginx." << secname << "\n";

    auto manage_match = false;
    for (auto opt : sec) {
        for (auto itm : opt) {
            if (opt.name() == "ssl_certificate_key") {
                ret.key = itm.name();
            }

            else if (opt.name() == "ssl_certificate") {
                ret.crt = itm.name();
            }

            else if (opt.name() == "ssl_session_cache" || opt.name() == "ssl_session_timeout") {
            }

            else if (opt.name() == MANAGE_SSL && itm.name() == manage) {
                manage_match = true;
            }

            else if (opt.name() == "listen") {
                auto val = regex_replace(itm.name(), rgx::regex{NGX_PORT_443[0]}, NGX_PORT_443[1]);
                if (val != itm.name()) {
                    std::cerr << "\t" << opt.name() << " (set back to '" << val << "')\n";
                    itm.rename(val.c_str());
                }
                continue; /* not deleting opt, look at other itm : opt */
            }

            else {
                continue; /* not deleting opt, look at other itm : opt */
            }

            // Delete matching opt (not skipped by continue):
            std::cerr << "\t" << opt.name() << " (was '" << itm.name() << "')\n";
            opt.del();
            break;
        }
    }
    if (manage_match) {
        sec.commit();
        return ret;
    }  // else:

    auto errmsg = std::string{"del_ssl error: not changing config wihtout: "};
    errmsg += "uci set nginx." + secname + "." + MANAGE_SSL.data() + "='" + manage.data();
    errmsg += "'";
    throw std::runtime_error(errmsg);
}

auto del_ssl_legacy(const std::string& name) -> bool
{
    const auto legacypath = std::string{CONF_DIR} + name + ".conf";

    if (access(legacypath.c_str(), R_OK) != 0) {
        return false;
    }

    try {
        remove_cron_job(CRON_CMD, name);
    }
    catch (...) {
        std::cerr << "del_ssl warning: cannot remove cron job rebuilding ";
        std::cerr << "the self-signed SSL certificate for " << name << "\n";
    }

    try {
        del_ssl_directives_from(name);
    }
    catch (...) {
        std::cerr << "del_ssl error: ";
        std::cerr << "cannot delete SSL directives from " << name << ".conf\n";
        throw;
    }

    return true;
}

void del_ssl(const std::string& name)
{
    auto crtpath = std::string{};
    auto keypath = std::string{};

    if (del_ssl_legacy(name)) {  // let it throw.
        crtpath = std::string{CONF_DIR} + name + ".crt";
        keypath = std::string{CONF_DIR} + name + ".key";
    }

    else {
        auto paths = del_ssl_from_config(name);  // let it throw.
        crtpath = paths.crt;
        keypath = paths.key;
    }

    if (remove(crtpath.c_str()) != 0) {
        auto errmsg = "del_ssl warning: cannot remove " + crtpath;
        perror(errmsg.c_str());
    }

    if (remove(keypath.c_str()) != 0) {
        auto errmsg = "del_ssl warning: cannot remove " + keypath;
        perror(errmsg.c_str());
    }
}

void del_ssl(const std::string& name, const std::string_view manage)
{
    const auto legacypath = std::string{CONF_DIR} + name + ".conf";

    if (access(legacypath.c_str(), R_OK) != 0) {
        del_ssl_from_config(name, manage);  // let it throw.
        return;
    }  // else:

    del_ssl_directives_from(name);  // let it throw.

    for (const auto* ext : {".crt", ".key"}) {
        struct stat sb {};

        auto path = std::string{CONF_DIR} + name + ext;

        // managed version of add_ssl_if_needed created symlinks (if needed):
        // NOLINTNEXTLINE(hicpp-signed-bitwise) S_ISLNK macro:
        if (lstat(path.c_str(), &sb) == 0 && S_ISLNK(sb.st_mode)) {
            if (remove(path.c_str()) != 0) {
                auto errmsg = "del_ssl warning: cannot remove " + path;
                perror(errmsg.c_str());
            }
        }
    }
}

auto check_ssl(const uci::package& pkg, bool is_enabled) -> bool
{
    auto are_valid = true;
    auto is_enabled_and_at_least_one_has_manage_ssl = false;

    if (is_enabled) {
        for (auto sec : pkg) {
            if (sec.anonymous() || sec.type() != "server") {
                continue;
            }  // else:

            const auto legacypath = std::string{CONF_DIR} + sec.name() + ".conf";
            if (access(legacypath.c_str(), R_OK) == 0) {
                continue;
            }  // else:

            auto keypath = std::string{};
            auto crtpath = std::string{};
            auto self_signed = false;

            for (auto opt : sec) {
                for (auto itm : opt) {
                    if (opt.name() == "ssl_certificate_key") {
                        keypath = itm.name();
                    }

                    else if (opt.name() == "ssl_certificate") {
                        crtpath = itm.name();
                    }

                    else if (opt.name() == MANAGE_SSL) {
                        if (itm.name() == "self-signed") {
                            self_signed = true;
                        }

                        // else if (itm.name()=="???") { /* manage other */ }

                        else {
                            continue;
                        }  // no supported manage_ssl string.

                        is_enabled_and_at_least_one_has_manage_ssl = true;
                    }
                }
            }

            if (self_signed && !crtpath.empty() && !keypath.empty()) {
                try {
                    if (!check_ssl_certificate(crtpath, keypath)) {
                        are_valid = false;
                    }
                }
                catch (...) {
                    std::cerr << "check_ssl warning: cannot build certificate '";
                    std::cerr << crtpath << "' or key '" << keypath << "'.\n";
                }
            }
        }
    }

    auto suffix = std::string_view{" the cron job checking the managed SSL certificates.\n"};

    if (is_enabled_and_at_least_one_has_manage_ssl) {
        try {
            install_cron_job(CRON_CHECK);
        }
        catch (...) {
            std::cerr << "check_ssl warning: cannot install" << suffix;
        }
    }

    else if (access("/etc/crontabs/root", R_OK) == 0) {
        try {
            remove_cron_job(CRON_CHECK);
        }
        catch (...) {
            std::cerr << "check_ssl warning: cannot remove" << suffix;
        }
    }  // else: do nothing

    return are_valid;
}

#endif
