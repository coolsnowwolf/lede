#ifndef __NGINX_UTIL_H
#define __NGINX_UTIL_H

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
// #include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <vector>

#ifndef NO_UBUS
#include "ubus-cxx.hpp"
#endif

#include "uci-cxx.hpp"

static constexpr auto NGINX_UTIL = std::string_view{"/usr/bin/nginx-util"};

static constexpr auto VAR_UCI_CONF = std::string_view{"/var/lib/nginx/uci.conf"};

static constexpr auto UCI_CONF = std::string_view{"/etc/nginx/uci.conf"};

static constexpr auto NGINX_CONF = std::string_view{"/etc/nginx/nginx.conf"};

static constexpr auto CONF_DIR = std::string_view{"/etc/nginx/conf.d/"};

static constexpr auto LAN_NAME = std::string_view{"_lan"};

static auto constexpr MANAGE_SSL = std::string_view{"uci_manage_ssl"};

static constexpr auto LAN_LISTEN = std::string_view{"/var/lib/nginx/lan.listen"};

static constexpr auto LAN_LISTEN_DEFAULT =  // TODO(pst) deprecate
    std::string_view{"/var/lib/nginx/lan.listen.default"};

// mode: optional ios::binary and/or ios::app (default ios::trunc)
void write_file(const std::string_view& name,
                const std::string& str,
                std::ios_base::openmode flag = std::ios::trunc);

// mode: optional ios::binary (internally ios::ate|ios::in)
auto read_file(const std::string_view& name, std::ios_base::openmode mode = std::ios::in)
    -> std::string;

// all S must be convertible to const char[]
template <typename... S>
auto call(const std::string& program, S... args) -> pid_t;

void create_lan_listen();

void init_uci(const uci::package& pkg);

auto is_enabled(const uci::package& pkg) -> bool;

void init_lan();

void get_env();

// --------------------- partial implementation: ------------------------------

void write_file(const std::string_view& name,
                const std::string& str,
                const std::ios_base::openmode flag)
{
    auto tmp = std::string{name};

    if ((flag & std::ios::ate) == 0 && (flag & std::ios::app) == 0) {
        tmp += ".tmp-XXXXXX";
        auto fd = mkstemp(&tmp[0]);
        if (fd == -1 || close(fd) != 0) {
            throw std::runtime_error("write_file error: cannot access " + tmp);
        }
    }

    try {
        std::ofstream file(tmp.data(), flag);
        if (!file.good()) {
            throw std::ofstream::failure("write_file error: cannot open " + std::string{tmp});
        }

        file << str << std::flush;

        file.close();
    }
    catch (...) {
        if (tmp != name) {
            remove(tmp.c_str());
        }  // remove can fail.
        throw;
    }

    if (rename(tmp.c_str(), name.data()) != 0) {
        throw std::runtime_error("write_file error: cannot move " + tmp + " to " + name.data());
    }
}

auto read_file(const std::string_view& name, const std::ios_base::openmode mode) -> std::string
{
    std::ifstream file(name.data(), mode | std::ios::ate);
    if (!file.good()) {
        throw std::ifstream::failure("read_file error: cannot open " + std::string{name});
    }

    std::string ret{};
    const size_t size = file.tellg();
    ret.reserve(size);

    file.seekg(0);
    ret.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();
    return ret;
}

template <typename... S>
auto call(const char* program, S... args) -> pid_t
{
    pid_t pid = fork();

    if (pid == 0) {  // child:
        std::array<char*, sizeof...(args) + 2> argv = {strdup(program), strdup(args)..., nullptr};

        execv(program, argv.data());  // argv cannot be const char * const[]!

        _exit(EXIT_FAILURE);  // exec never returns.
    }
    else if (pid > 0) {  // parent:
        return pid;
    }

    std::string errmsg = "call error: cannot fork (";
    errmsg += std::to_string(errno) + "): " + std::strerror(errno);
    throw std::runtime_error(errmsg);
}

#endif
