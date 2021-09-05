#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "uci-cxx.hpp"

auto main() -> int
{
    uci::element p = uci::package{"nginx"};
    std::cout << "package " << p.name() << "\n\n";
    for (auto s : p) {
        std::cout << "config " << s.type() << " '" << s.name() << "'\n";
        for (auto o : s) {
            for (auto i : o) {
                std::cout << "\t" << o.type() << " " << o.name() << " '" << i.name() << "'\n";
            }
        }
        std::cout << "\n";
    }
}
