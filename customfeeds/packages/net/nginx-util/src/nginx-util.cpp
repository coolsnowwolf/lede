#include <iostream>
#include <numeric>

#include "nginx-ssl-util.hpp"
#include "nginx-util.hpp"

static auto constexpr file_comment_auto_created =
    std::string_view{"# This file is re-created when Nginx starts.\n"};

// TODO(pst) replace it with blobmsg_get_string if upstream takes const:
#ifndef NO_UBUS
static inline auto _pst_get_string(const blob_attr* attr) -> char*
{
    return static_cast<char*>(blobmsg_data(attr));
}
#endif

void create_lan_listen()  // create empty files for compatibility:
{
    // TODO(pst): replace by dummies after transitioning nginx config to UCI:
    std::vector<std::string> ips;

#ifndef NO_UBUS
    try {
        auto loopback_status = ubus::call("network.interface.loopback", "status");

        for (const auto* ip : loopback_status.filter("ipv4-address", "", "address")) {
            ips.emplace_back(_pst_get_string(ip));
        }

        for (const auto* ip : loopback_status.filter("ipv6-address", "", "address")) {
            ips.emplace_back(std::string{"["} + _pst_get_string(ip) + "]");
        }
    }
    catch (const std::runtime_error&) { /* do nothing about it */
    }

    try {
        auto lan_status = ubus::call("network.interface.lan", "status");

        for (const auto* ip : lan_status.filter("ipv4-address", "", "address")) {
            ips.emplace_back(_pst_get_string(ip));
        }

        for (const auto* ip : lan_status.filter("ipv6-address", "", "address")) {
            ips.emplace_back(std::string{"["} + _pst_get_string(ip) + "]");
        }

        for (const auto* ip :
             lan_status.filter("ipv6-prefix-assignment", "", "local-address", "address")) {
            ips.emplace_back(std::string{"["} + _pst_get_string(ip) + "]");
        }
    }
    catch (const std::runtime_error&) { /* do nothing about it */
    }
#else
    ips.emplace_back("127.0.0.1");
#endif

    std::string listen = std::string{file_comment_auto_created};
    std::string listen_default = std::string{file_comment_auto_created};
    for (const auto& ip : ips) {
        listen += "\tlisten " + ip + ":80;\n";
        listen_default += "\tlisten " + ip + ":80 default_server;\n";
    }
    write_file(LAN_LISTEN, listen);
    write_file(LAN_LISTEN_DEFAULT, listen_default);

    std::string ssl_listen = std::string{file_comment_auto_created};
    std::string ssl_listen_default = std::string{file_comment_auto_created};
    for (const auto& ip : ips) {
        ssl_listen += "\tlisten " + ip + ":443 ssl;\n";
        ssl_listen_default += "\tlisten " + ip + ":443 ssl default_server;\n";
    }
    write_file(LAN_SSL_LISTEN, ssl_listen);
    write_file(LAN_SSL_LISTEN_DEFAULT, ssl_listen_default);
}

inline auto change_if_starts_with(const std::string_view& subject,
                                  const std::string_view& prefix,
                                  const std::string_view& substitute,
                                  const std::string_view& seperator = " \t\n;") -> std::string
{
    auto view = subject;
    view = view.substr(view.find_first_not_of(seperator));
    if (view.rfind(prefix, 0) == 0) {
        if (view.size() == prefix.size()) {
            return std::string{substitute};
        }
        view = view.substr(prefix.size());
        if (seperator.find(view[0]) != std::string::npos) {
            auto ret = std::string{substitute};
            ret += view;
            return ret;
        }
    }
    return std::string{subject};
}

inline auto create_server_conf(const uci::section& sec, const std::string& indent = "")
    -> std::string
{
    auto secname = sec.name();

    auto legacypath = std::string{CONF_DIR} + secname + ".conf";
    if (access(legacypath.c_str(), R_OK) == 0) {
        auto message = std::string{"skipped UCI server 'nginx."} + secname;
        message += "' as it could conflict with: " + legacypath + "\n";

        // TODO(pst) std::cerr<<"create_server_conf notice: "<<message;

        return indent + "# " + message;
    }  // else:

    auto conf = indent + "server { #see uci show 'nginx." + secname + "'\n";

    for (auto opt : sec) {
        for (auto itm : opt) {
            if (opt.name().rfind("uci_", 0) == 0) {
                continue;
            }
            // else: standard opt.name()

            auto val = itm.name();

            if (opt.name() == "error_log") {
                val = change_if_starts_with(val, "logd", "/proc/self/fd/1");
            }

            else if (opt.name() == "access_log") {
                val = change_if_starts_with(val, "logd", "stderr");
            }

            conf += indent + "\t" + opt.name() + " " + itm.name() + ";\n";
        }
    }

    conf += indent + "}\n";

    return conf;
}

void init_uci(const uci::package& pkg)
{
    auto conf = std::string{file_comment_auto_created};

    static const auto uci_http_config = std::string_view{"#UCI_HTTP_CONFIG\n"};

    const auto tmpl = read_file(std::string{UCI_CONF} + ".template");
    auto pos = tmpl.find(uci_http_config);

    if (pos == std::string::npos) {
        conf += tmpl;
    }

    else {
        const auto index = tmpl.find_last_not_of(" \t", pos - 1);

        const auto before = tmpl.begin() + index + 1;
        const auto middle = tmpl.begin() + pos;
        const auto after = middle + uci_http_config.length();

        conf.append(tmpl.begin(), before);

        const auto indent = std::string{before, middle};
        for (auto sec : pkg) {
            if (sec.type() == std::string_view{"server"}) {
                conf += create_server_conf(sec, indent) + "\n";
            }
        }

        conf.append(after, tmpl.end());
    }

    write_file(VAR_UCI_CONF, conf);
}

auto is_enabled(const uci::package& pkg) -> bool
{
    for (auto sec : pkg) {
        if (sec.type() != std::string_view{"main"}) {
            continue;
        }
        if (sec.name() != std::string_view{"global"}) {
            continue;
        }
        for (auto opt : sec) {
            if (opt.name() != "uci_enable") {
                continue;
            }
            for (auto itm : opt) {
                if (itm) {
                    return true;
                }
            }
        }
    }
    return false;
}

/*
 * ___________main_thread________________|______________thread_1________________
 *  create_lan_listen() or do nothing    | config = uci::package("nginx")
 *  if config_enabled (set in thread_1): | config_enabled = is_enabled(config)
 *  then init_uci(config)                | check_ssl(config, config_enabled)
 */
void init_lan()
{
    std::exception_ptr ex;
    std::unique_ptr<uci::package> config;
    bool config_enabled = false;
    std::mutex configuring;

    configuring.lock();
    auto thrd = std::thread([&config, &config_enabled, &configuring, &ex] {
        try {
            config = std::make_unique<uci::package>("nginx");
            config_enabled = is_enabled(*config);
            configuring.unlock();
            check_ssl(*config, config_enabled);
        }
        catch (...) {
            std::cerr << "init_lan error: checking UCI file /etc/config/nginx\n";
            ex = std::current_exception();
        }
    });

    try {
        create_lan_listen();
    }
    catch (...) {
        std::cerr << "init_lan error: cannot create listen files of local IPs.\n";
        ex = std::current_exception();
    }

    configuring.lock();
    if (config_enabled) {
        try {
            init_uci(*config);
        }
        catch (...) {
            std::cerr << "init_lan error: cannot create " << VAR_UCI_CONF << " from ";
            std::cerr << UCI_CONF << ".template using UCI file /etc/config/nginx\n";
            ex = std::current_exception();
        }
    }

    thrd.join();
    if (ex) {
        std::rethrow_exception(ex);
    }
}

void get_env()
{
    std::cout << "UCI_CONF="
              << "'" << UCI_CONF << "'" << std::endl;
    std::cout << "NGINX_CONF="
              << "'" << NGINX_CONF << "'" << std::endl;
    std::cout << "CONF_DIR="
              << "'" << CONF_DIR << "'" << std::endl;
    std::cout << "LAN_NAME="
              << "'" << LAN_NAME << "'" << std::endl;
    std::cout << "LAN_LISTEN="
              << "'" << LAN_LISTEN << "'" << std::endl;
    std::cout << "LAN_SSL_LISTEN="
              << "'" << LAN_SSL_LISTEN << "'" << std::endl;
    std::cout << "SSL_SESSION_CACHE_ARG="
              << "'" << SSL_SESSION_CACHE_ARG(LAN_NAME) << "'" << std::endl;
    std::cout << "SSL_SESSION_TIMEOUT_ARG="
              << "'" << SSL_SESSION_TIMEOUT_ARG << "'\n";
    std::cout << "ADD_SSL_FCT="
              << "'" << ADD_SSL_FCT << "'" << std::endl;
    std::cout << "MANAGE_SSL="
              << "'" << MANAGE_SSL << "'" << std::endl;
}

auto main(int argc, char* argv[]) -> int
{
    // TODO(pst): use std::span when available:
    auto args = std::basic_string_view<char*>{argv, static_cast<size_t>(argc)};

    auto cmds = std::array{
        std::array<std::string_view, 2>{"init_lan", ""},
        std::array<std::string_view, 2>{"get_env", ""},
        std::array<std::string_view, 2>{
            ADD_SSL_FCT, "server_name [manager /path/to/ssl_certificate /path/to/ssl_key]"},
        std::array<std::string_view, 2>{"del_ssl", "server_name [manager]"},
        std::array<std::string_view, 2>{"check_ssl", ""},
    };

    try {
        if (argc == 2 && args[1] == cmds[0][0]) {
            init_lan();
        }

        else if (argc == 2 && args[1] == cmds[1][0]) {
            get_env();
        }

        else if (argc == 3 && args[1] == cmds[2][0]) {
            add_ssl_if_needed(std::string{args[2]});
        }

        // NOLINTNEXTLINE(readability-magic-numbers,cppcoreguidelines-avoid-magic-numbers): 6
        else if (argc == 6 && args[1] == cmds[2][0]) {
            // NOLINTNEXTLINE(readability-magic-numbers,cppcoreguidelines-avoid-magic-numbers): 5
            add_ssl_if_needed(std::string{args[2]}, args[3], args[4], args[5]);
        }

        else if (argc == 3 && args[1] == cmds[3][0]) {
            del_ssl(std::string{args[2]});
        }

        else if (argc == 4 && args[1] == cmds[3][0]) {
            del_ssl(std::string{args[2]}, args[3]);
        }

        else if (argc == 2 && args[1] == cmds[3][0])  // TODO(pst) deprecate
        {
            try {
                auto name = std::string{LAN_NAME};
                if (del_ssl_legacy(name)) {
                    auto crtpath = std::string{CONF_DIR} + name + ".crt";
                    remove(crtpath.c_str());
                    auto keypath = std::string{CONF_DIR} + name + ".key";
                    remove(keypath.c_str());
                }
            }
            catch (...) { /* do nothing. */
            }
        }

        else if (argc == 2 && args[1] == cmds[4][0]) {
            check_ssl(uci::package{"nginx"});
        }

        else {
            std::cerr << "Tool for creating Nginx configuration files (";
#ifdef VERSION
            std::cerr << "version " << VERSION << " ";
#endif
            std::cerr << "with libuci, ";
#ifndef NO_UBUS
            std::cerr << "libubus, ";
#endif
            std::cerr << "libopenssl, ";
#ifndef NO_PCRE
            std::cerr << "PCRE, ";
#endif
            std::cerr << "pthread and libstdcpp)." << std::endl;

            auto usage =
                std::accumulate(cmds.begin(), cmds.end(), std::string{"usage: "} + *argv + " [",
                                [](const auto& use, const auto& cmd) {
                                    return use + std::string{cmd[0]} + (cmd[1].empty() ? "" : " ") +
                                           std::string{cmd[1]} + "|";
                                });
            usage[usage.size() - 1] = ']';
            std::cerr << usage << std::endl;

            throw std::runtime_error("main error: argument not recognized");
        }

        return 0;
    }

    catch (const std::exception& e) {
        std::cerr << " * " << *argv << " " << e.what() << "\n";
    }

    catch (...) {
        std::cerr << " * * " << *argv;
        perror(" main error");
    }

    return 1;
}
