#include <iostream>

#include "ubus-cxx.hpp"

inline void example_for_checking_if_there_is_a_key()
{
    if (ubus::call("service", "list").filter("cron")) {
        std::cout << "Cron is active (with or without instances) " << std::endl;
    }
}

inline void example_for_getting_values()
{
    auto lan_status = ubus::call("network.interface.lan", "status");
    for (const auto* t : lan_status.filter("ipv6-address", "", "address")) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        auto* x = const_cast<blob_attr*>(t);
        std::cout << "[" << blobmsg_get_string(x) << "] ";
    }
    for (const auto* t : lan_status.filter("ipv4-address", "").filter("address")) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        auto* x = const_cast<blob_attr*>(t);
        std::cout << blobmsg_get_string(x) << " ";
    }
    std::cout << std::endl;
}

inline void example_for_sending_message()
{
    auto set_arg = [](blob_buf* buf) -> int { return blobmsg_add_string(buf, "config", "nginx"); };
    for (const auto* t : ubus::call("uci", "get", set_arg).filter("values")) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        auto* x = const_cast<blob_attr*>(t);
        std::cout << blobmsg_get_string(x) << "\n";
    }
}

inline void example_for_exploring()
{
    ubus::strings keys{"ipv4-address", "", ""};
    for (const auto* t : ubus::call("network.interface.lan", "status").filter(keys)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        auto* x = const_cast<blob_attr*>(t);
        std::cout << blobmsg_name(x) << ": ";
        switch (blob_id(x)) {
            case BLOBMSG_TYPE_UNSPEC: std::cout << "[unspecified]"; break;
            case BLOBMSG_TYPE_ARRAY: std::cout << "[array]"; break;
            case BLOBMSG_TYPE_TABLE: std::cout << "[table]"; break;
            case BLOBMSG_TYPE_STRING: std::cout << blobmsg_get_string(x); break;
            case BLOBMSG_TYPE_INT64: std::cout << blobmsg_get_u64(x); break;
            case BLOBMSG_TYPE_INT32: std::cout << blobmsg_get_u32(x); break;
            case BLOBMSG_TYPE_INT16: std::cout << blobmsg_get_u16(x); break;
            case BLOBMSG_TYPE_BOOL: std::cout << blobmsg_get_bool(x); break;
            case BLOBMSG_TYPE_DOUBLE: std::cout << blobmsg_get_double(x); break;
            default: std::cout << "[unknown]";
        }
        std::cout << std::endl;
    }
}

inline void example_for_recursive_exploring()
{  // output like from the original ubus call:
    const auto explore = [](auto message) -> void {
        auto end = message.end();
        auto explore_internal = [&end](auto& explore_ref, auto it, size_t depth = 1) -> void {
            std::cout << std::endl;
            bool first = true;
            for (; it != end; ++it) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                auto* attr = const_cast<blob_attr*>(*it);
                if (first) {
                    first = false;
                }
                else {
                    std::cout << ",\n";
                }
                std::cout << std::string(depth, '\t');
                std::string name = blobmsg_name(attr);
                if (!name.empty()) {
                    std::cout << "\"" << name << "\": ";
                }
                switch (blob_id(attr)) {
                    case BLOBMSG_TYPE_UNSPEC: std::cout << "(unspecified)"; break;
                    case BLOBMSG_TYPE_ARRAY:
                        std::cout << "[";
                        explore_ref(explore_ref, ubus::iterator{attr}, depth + 1);
                        std::cout << "\n" << std::string(depth, '\t') << "]";
                        break;
                    case BLOBMSG_TYPE_TABLE:
                        std::cout << "{";
                        explore_ref(explore_ref, ubus::iterator{attr}, depth + 1);
                        std::cout << "\n" << std::string(depth, '\t') << "}";
                        break;
                    case BLOBMSG_TYPE_STRING:
                        std::cout << "\"" << blobmsg_get_string(attr) << "\"";
                        break;
                    case BLOBMSG_TYPE_INT64: std::cout << blobmsg_get_u64(attr); break;
                    case BLOBMSG_TYPE_INT32: std::cout << blobmsg_get_u32(attr); break;
                    case BLOBMSG_TYPE_INT16: std::cout << blobmsg_get_u16(attr); break;
                    case BLOBMSG_TYPE_BOOL:
                        std::cout << (blobmsg_get_bool(attr) ? "true" : "false");
                        break;
                    case BLOBMSG_TYPE_DOUBLE: std::cout << blobmsg_get_double(attr); break;
                    default: std::cout << "(unknown)"; break;
                }
            }
        };
        std::cout << "{";
        explore_internal(explore_internal, message.begin());
        std::cout << "\n}" << std::endl;
    };
    explore(ubus::call("network.interface.lan", "status"));
}

auto main() -> int
{
    try {
        example_for_checking_if_there_is_a_key();

        example_for_getting_values();

        example_for_sending_message();

        example_for_exploring();

        example_for_recursive_exploring();

        return 0;
    }

    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    catch (...) {
        perror("main error");
    }

    return 1;
}
