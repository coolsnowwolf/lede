#ifndef _UBUS_CXX_HPP
#define _UBUS_CXX_HPP

#include <libubus.h>
#include <cassert>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#endif

namespace ubus {

static constexpr int call_timeout = 500;

using msg_ptr = std::shared_ptr<const blob_attr>;

using strings = std::vector<std::string>;

inline auto concat(strings dest)
{
    return dest;
}

template <class... Strings>
inline auto concat(strings dest, strings src, Strings... more)
{
    dest.reserve(dest.size() + src.size());
    dest.insert(std::end(dest), std::make_move_iterator(std::begin(src)),
                std::make_move_iterator(std::end(src)));
    return concat(std::move(dest), std::move(more)...);
}

template <class S, class... Strings>
inline auto concat(strings dest, S src, Strings... more)
{
    dest.emplace_back(std::move(src));
    return concat(std::move(dest), std::move(more)...);
}

class iterator {
  private:
    const strings& keys;

    const size_t n = 0;

    size_t i = 0;

    const blob_attr* pos = nullptr;

    std::unique_ptr<iterator> cur{};

    iterator* parent = nullptr;

    size_t rem = 0;

    [[nodiscard]] inline auto matches() const -> bool
    {
        return (keys[i].empty() || blobmsg_name(cur->pos) == keys[i]);
    }

    explicit iterator(iterator* par)
        : keys{par->keys}, n{par->n}, pos{par->pos}, cur{this}, parent{par}
    {
        if (pos != nullptr) {
            rem = blobmsg_data_len(pos);
            pos = static_cast<blob_attr*>(blobmsg_data(pos));
        }
    }

  public:
    explicit iterator(const blob_attr* msg, const strings& key_filter = {""})
        : keys{key_filter}, n{keys.size() - 1}, pos{msg}, cur{this}
    {
        if (pos != nullptr) {
            rem = blobmsg_data_len(pos);
            pos = static_cast<blob_attr*>(blobmsg_data(pos));

            if (rem == 0) {
                pos = nullptr;
            }
            else if (i != n || !matches()) {
                ++*this;
            }
        }
    }

    inline iterator(iterator&&) noexcept = default;

    inline iterator(const iterator&) = delete;

    inline auto operator=(const iterator&) -> iterator& = delete;

    inline auto operator=(iterator &&) -> iterator& = delete;

    inline auto operator*()
    {
        return cur->pos;
    }

    inline auto operator!=(const iterator& rhs)
    {
        return (cur->rem != rhs.cur->rem || cur->pos != rhs.cur->pos);
    }

    auto operator++() -> iterator&;

    inline ~iterator()
    {
        if (cur.get() == this) {
            static_cast<void>(cur.release());
        }
    }
};

class message {
  private:
    const msg_ptr msg{};  // initialized by callback.

    const strings keys{};

  public:
    inline explicit message(msg_ptr message_ptr, strings key_filter = {""})
        : msg{std::move(message_ptr)}, keys{std::move(key_filter)}
    {}

    inline message(message&&) = default;

    inline message(const message&) = delete;

    inline auto operator=(message &&) -> message& = delete;

    inline auto operator=(const message&) -> message& = delete;

    [[nodiscard]] inline auto begin() const -> iterator
    {
        return iterator{msg.get(), keys};
    }

    [[nodiscard]] inline auto end() const -> iterator
    {
        return iterator{nullptr, keys};
    }

    inline explicit operator bool() const
    {
        return begin() != end();
    }

    template <class... Strings>
    auto filter(Strings... key_filter)
    {
        strings both{};
        if (keys.size() != 1 || !keys[0].empty()) {
            both = keys;
        }
        both = concat(std::move(both), std::move(key_filter)...);
        return std::move(message{msg, std::move(both)});
    }

    inline ~message() = default;
};

class lock_shared_resources {
  private:
    static std::mutex inuse;

  public:
    inline lock_shared_resources()
    {
        inuse.lock();
    }

    inline lock_shared_resources(lock_shared_resources&&) noexcept = default;

    inline lock_shared_resources(const lock_shared_resources&) = delete;

    inline auto operator=(const lock_shared_resources&) -> auto& = delete;

    inline auto operator=(lock_shared_resources &&) -> auto&& = delete;

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    inline auto get_context() -> ubus_context*  // is member to enforce inuse.
    {
        static auto ubus_freeing = [](ubus_context* ctx) { ubus_free(ctx); };
        static std::unique_ptr<ubus_context, decltype(ubus_freeing)> lazy_ctx{ubus_connect(nullptr),
                                                                              ubus_freeing};

        if (!lazy_ctx) {  // it could be available on a later call:

            lazy_ctx.reset(ubus_connect(nullptr));

            if (!lazy_ctx) {
                throw std::runtime_error("ubus error: cannot connect context");
            }
        }

        return lazy_ctx.get();
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    inline auto get_blob_buf() -> blob_buf*  // is member to enforce inuse.
    {
        static blob_buf buf;

        static auto blob_buf_freeing = [](blob_buf* b) { blob_buf_free(b); };
        static std::unique_ptr<blob_buf, decltype(blob_buf_freeing)>
            created_to_free_on_the_end_of_life{&buf, blob_buf_freeing};

        blob_buf_init(&buf, 0);

        return &buf;
    }

    inline ~lock_shared_resources()
    {
        inuse.unlock();
    }
};

template <class F>
auto call(const char* path, const char* method, F set_arguments, int timeout = call_timeout)
    -> message;

inline auto call(const char* path, const char* method, int timeout = call_timeout) -> message
{
    return call(
        path, method, [](blob_buf* /*buf*/) { return 0; }, timeout);
}

inline auto call(const char* path, int timeout = call_timeout) -> message
{
    return call(path, "", timeout);
}

// ------------------------- implementation: ----------------------------------

std::mutex lock_shared_resources::inuse;

inline auto iterator::operator++() -> iterator&
{
    for (;;) {
#ifndef NDEBUG
        std::cout << std::string(i, '>') << " look for " << keys[i] << " at ";
        std::cout << blobmsg_name(cur->pos) << std::endl;
#endif

        auto id = blob_id(cur->pos);
        if ((id == BLOBMSG_TYPE_TABLE || id == BLOBMSG_TYPE_ARRAY) && i < n && matches() &&
            blobmsg_data_len(cur->pos) > 0)
        {  // immmerge:
            ++i;

            auto* tmp = cur.release();

            struct new_iterator : public iterator  // use private constructor:
            {
                explicit new_iterator(iterator* par) : iterator{par} {}
            };
            cur = std::make_unique<new_iterator>(tmp);
        }
        else {
            while (true) {
                cur->rem -= blob_pad_len(cur->pos);
                cur->pos = blob_next(cur->pos);
                auto len = blob_pad_len(cur->pos);

                if (cur->rem > 0 && len <= cur->rem && len >= sizeof(blob_attr)) {
                    break;
                }

                // emerge:
                auto* tmp = cur->parent;

                if (tmp == nullptr) {
                    cur->pos = nullptr;
                    return *cur;
                }

                cur.reset(tmp);

                --i;
            }
        }
        if (i == n && matches()) {
            return *cur;
        }
    }
}

template <class F>
inline auto call(const char* path, const char* method, F set_arguments, int timeout) -> message
{
    auto shared = lock_shared_resources{};

    auto* ctx = shared.get_context();

    uint32_t id = 0;
    int err = ubus_lookup_id(ctx, path, &id);

    if (err == 0) {  // call
        ubus_request request{};

        auto* buf = shared.get_blob_buf();
        err = set_arguments(buf);
        if (err == 0) {
            err = ubus_invoke_async(ctx, id, method, buf->head, &request);
        }

        if (err == 0) {
            msg_ptr message_ptr;

            /* Cannot capture message_ptr, the lambda would be another type.
             * Pass a location where to save the message as priv pointer when
             * invoking and get it back here:
             */
            request.priv = &message_ptr;

            request.data_cb = [](ubus_request* req, int /*type*/, blob_attr* msg) {
                if (req == nullptr || msg == nullptr) {
                    return;
                }

                auto* saved = static_cast<msg_ptr*>(req->priv);
                if (saved == nullptr || *saved) {
                    return;
                }

                saved->reset(blob_memdup(msg), free);
                if (!*saved) {
                    throw std::bad_alloc();
                }
            };

            err = ubus_complete_request(ctx, &request, timeout);

            if (err == 0) {
                return message{message_ptr};
            }
        }
    }

    std::string errmsg = "ubus::call error: cannot invoke";
    errmsg += " (" + std::to_string(err) + ") " + path + " " + method;
    throw std::runtime_error(errmsg);
}

}  // namespace ubus

#endif
