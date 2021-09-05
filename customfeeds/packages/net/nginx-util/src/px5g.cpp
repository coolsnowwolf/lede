#include <unistd.h>
#include <array>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include "px5g-openssl.hpp"

class argv_view {  // TODO(pst): use std::span when available.

  private:
    std::basic_string_view<const char*> data;

  public:
    argv_view(const argv_view&) = delete;

    argv_view(argv_view&&) = delete;

    auto operator=(const argv_view&) -> argv_view& = delete;

    auto operator=(argv_view &&) -> argv_view& = delete;

    argv_view(const char** argv, int argc) : data{argv, static_cast<size_t>(argc)} {}

    inline auto operator[](size_t pos) const -> std::string_view
    {
        return std::string_view{data[pos]};
    }

    [[nodiscard]] inline constexpr auto size() const noexcept -> size_t
    {
        return data.size();
    }

    ~argv_view() = default;
};

static const auto default_validity = 30;

auto checkend(const argv_view& argv) -> int;

void eckey(const argv_view& argv);

void rsakey(const argv_view& argv);

void selfsigned(const argv_view& argv);

inline auto parse_int(const std::string_view& arg) -> int
{
    size_t pos = 0;
    int ret = stoi(std::string{arg}, &pos);
    if (pos < arg.size()) {
        throw std::runtime_error("number has trailing char");
    }
    return ret;
}

inline auto parse_curve(const std::string_view& name) -> int
{
    if (name == "P-384") {
        return NID_secp384r1;
    }
    if (name == "P-521") {
        return NID_secp521r1;
    }
    if (name == "P-256" || name == "secp256r1") {
        return NID_X9_62_prime256v1;
    }
    if (name == "secp192r1") {
        return NID_X9_62_prime192v1;
    }
    return OBJ_sn2nid(name.data());
    // not: if (curve == 0) { curve = EC_curve_nist2nid(name.c_str()); }
}

auto checkend(const argv_view& argv) -> int
{
    bool use_pem = true;
    std::string crtpath{};
    time_t seconds = 0;

    for (size_t i = 2; i < argv.size(); ++i) {
        if (argv[i] == "-der") {
            use_pem = false;
        }
        else if (argv[i] == "-in") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("checkend error: -in misses filename");
            }

            if (!crtpath.empty()) {
                if (argv[i] == crtpath) {
                    std::cerr << "checkend warning: repeated same -in file\n";
                }
                else {
                    throw std::runtime_error("checkend error: more than one -in file");
                }
            }

            crtpath = argv[i];
        }

        else if (argv[i][0] == '-') {
            std::cerr << "checkend warning: skipping option " << argv[i] << std::endl;
        }
        else {  // main option:
            intmax_t num = 0;

            try {
                num = parse_int(argv[i]);
            }
            catch (...) {
                auto errmsg = std::string{"checkend error: invalid time "};
                errmsg += argv[i];
                std::throw_with_nested(std::runtime_error(errmsg));
            }

            seconds = static_cast<time_t>(num);

            if (num != static_cast<intmax_t>(seconds)) {
                auto errmsg = std::string{"checkend error: time too big "};
                errmsg += argv[i];
                throw std::runtime_error(errmsg);
            }
        }
    }

    bool valid = checkend(crtpath, seconds, use_pem);
    std::cout << "Certificate will" << (valid ? " not " : " ") << "expire" << std::endl;

    return (valid ? 0 : 1);
}

void eckey(const argv_view& argv)
{
    bool has_main_option = false;
    bool use_pem = true;
    std::string keypath{};
    int curve = NID_X9_62_prime256v1;

    for (size_t i = 2; i < argv.size(); ++i) {
        if (argv[i] == "-der") {
            use_pem = false;
        }
        else if (argv[i] == "-out") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("eckey error: -out misses filename");
            }

            if (!keypath.empty()) {
                if (argv[i] == keypath) {
                    std::cerr << "eckey warning: repeated same -out file\n";
                }
                else {
                    throw std::runtime_error("eckey error: more than one -out file");
                }
            }

            keypath = argv[i];
        }

        else if (argv[i][0] == '-') {
            std::cerr << "eckey warning: skipping option " << argv[i] << std::endl;
        }
        else {  // main option:

            if (has_main_option) {
                throw std::runtime_error("eckey error: more than one main option");
            }  // else:
            has_main_option = true;

            curve = parse_curve(argv[i]);
        }
    }

    write_key(gen_eckey(curve), keypath, use_pem);
}

void rsakey(const argv_view& argv)
{
    bool has_main_option = false;
    bool use_pem = true;
    std::string keypath{};
    BN_ULONG exponent = RSA_F4;
    int keysize = rsa_min_modulus_bits;

    for (size_t i = 2; i < argv.size(); ++i) {
        if (argv[i] == "-der") {
            use_pem = false;
        }
        else if (argv[i] == "-3") {
            exponent = 3;
        }
        else if (argv[i] == "-out") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("rsakey error: -out misses filename");
            }

            if (!keypath.empty()) {
                if (argv[i] == keypath) {
                    std::cerr << "rsakey warning: repeated -out file" << std::endl;
                }
                else {
                    throw std::runtime_error("rsakey error: more than one -out file");
                }
            }

            keypath = argv[i];
        }

        else if (argv[i][0] == '-') {
            std::cerr << "rsakey warning: skipping option " << argv[i] << std::endl;
        }
        else {  // main option:

            if (has_main_option) {
                throw std::runtime_error("rsakey error: more than one keysize");
            }  // else:
            has_main_option = true;

            try {
                keysize = parse_int(argv[i]);
            }
            catch (...) {
                std::string errmsg{"rsakey error: invalid keysize "};
                errmsg += argv[i];
                std::throw_with_nested(std::runtime_error(errmsg));
            }
        }
    }

    write_key(gen_rsakey(keysize, exponent), keypath, use_pem);
}

void selfsigned(const argv_view& argv)
{
    bool use_pem = true;
    int days = default_validity;
    std::string keypath{};
    std::string crtpath{};
    std::string subject{};

    bool use_rsa = true;
    int keysize = rsa_min_modulus_bits;
    BN_ULONG exponent = RSA_F4;

    int curve = NID_X9_62_prime256v1;

    for (size_t i = 2; i < argv.size(); ++i) {
        if (argv[i] == "-der") {
            use_pem = false;
        }
        else if (argv[i] == "-days") {
            ++i;
            try {
                days = parse_int(argv[i]);
            }
            catch (...) {
                std::string errmsg{"selfsigned error: not a number for -days "};
                errmsg += argv[i].substr(4);
                std::throw_with_nested(std::runtime_error(errmsg));
            }
        }

        else if (argv[i] == "-newkey") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("selfsigned error: -newkey misses algorithm option");
            }

            static constexpr auto rsa_prefix = std::string_view{"rsa:"};

            if (argv[i] == "ec") {
                use_rsa = false;
            }
            else if (argv[i].rfind(rsa_prefix, 0) == 0) {
                use_rsa = true;
                try {
                    keysize = parse_int(argv[i].substr(rsa_prefix.size()));
                }
                catch (...) {
                    std::string errmsg{"selfsigned error: invalid keysize "};
                    errmsg += argv[i].substr(4);
                    std::throw_with_nested(std::runtime_error(errmsg));
                }
            }
            else {
                throw std::runtime_error("selfsigned error: invalid algorithm");
            }
        }

        else if (argv[i] == "-pkeyopt") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("selfsigned error: -pkeyopt misses value");
            }

            static constexpr auto curve_prefix = std::string_view{"ec_paramgen_curve:"};

            if (argv[i].rfind(curve_prefix, 0) != 0) {
                throw std::runtime_error("selfsigned error: -pkeyopt invalid");
            }

            curve = parse_curve(argv[i].substr(curve_prefix.size()));
        }

        else if (argv[i] == "-keyout") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("selfsigned error: -keyout misses path");
            }

            if (!keypath.empty()) {
                if (argv[i] == keypath) {
                    std::cerr << "selfsigned warning: repeated -keyout file\n";
                }
                else {
                    throw std::runtime_error("selfsigned error: more than one -keyout file");
                }
            }

            keypath = argv[i];
        }

        else if (argv[i] == "-out") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("selfsigned error: -out misses filename");
            }

            if (!crtpath.empty()) {
                if (argv[i] == crtpath) {
                    std::cerr << "selfsigned warning: repeated same -out file\n";
                }
                else {
                    throw std::runtime_error("selfsigned error: more than one -out file");
                }
            }

            crtpath = argv[i];
        }

        else if (argv[i] == "-subj") {
            ++i;

            if (i >= argv.size()) {
                throw std::runtime_error("selfsigned error: -subj misses value");
            }

            if (!subject.empty()) {
                if (argv[i] == subject) {
                    std::cerr << "selfsigned warning: repeated same -subj\n";
                }
                else {
                    throw std::runtime_error("selfsigned error: more than one -subj value");
                }
            }

            subject = argv[i];
        }

        else {
            std::cerr << "selfsigned warning: skipping option " << argv[i] << std::endl;
        }
    }

    auto pkey = use_rsa ? gen_rsakey(keysize, exponent) : gen_eckey(curve);

    selfsigned(pkey, days, subject, crtpath, use_pem);

    if (!keypath.empty()) {
        write_key(pkey, keypath, use_pem);
    }
}

auto main(int argc, const char** argv) -> int
{
    auto args = argv_view{argv, argc};

    auto cmds = std::array{
        std::array<std::string, 2>{"checkend",
                                   " [-der] [-in certificate_path] [seconds_remaining]"},
        std::array<std::string, 2>{"eckey", " [-der] [-out key_path] [curve_name]"},
        std::array<std::string, 2>{"rsakey", " [-der] [-out key_path] [-3] [key_size]"},
        std::array<std::string, 2>{
            "selfsigned",
            " [-der] [-keyout key_path] [-out certificate_path]"
            " [-newkey ec|rsa:key_size] [-pkeyopt ec_paramgen_curve:name]"
            " [-days validity] [-subj /C=.../ST=.../L=.../O=.../CN=.../... ]"},
    };

    try {
        if (argc < 2) {
            throw std::runtime_error("error: no argument");
        }

        if (args[1] == cmds[0][0]) {
            return checkend(args);
        }

        if (args[1] == cmds[1][0]) {
            eckey(args);
        }

        else if (args[1] == cmds[2][0]) {
            rsakey(args);
        }

        else if (args[1] == cmds[3][0]) {
            selfsigned(args);
        }

        else {
            throw std::runtime_error("error: argument not recognized");
        }
    }

    catch (const std::exception& e) {
        auto usage = std::accumulate(
            cmds.begin(), cmds.end(), std::string{"usage: \n"},
            [=](const auto& use, const auto& cmd) {
                return use + std::string{4, ' '} + *argv + " " + cmd[0] + cmd[1] + "\n";
            });

        std::cerr << usage << std::flush;

        auto print_nested = [](auto&& self, const std::exception& outer, int depth = 0) -> void {
            std::cerr << std::string(depth, '\t') << outer.what() << std::endl;
            try {
                std::rethrow_if_nested(outer);
            }
            catch (const std::exception& inner) {
                self(self, inner, depth + 1);
            }
        };

        print_nested(print_nested, e);

        return 1;
    }

    catch (...) {
        std::cerr << *argv << " unknown error." << std::endl;
        return 2;
    }

    return 0;
}
