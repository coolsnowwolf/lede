#ifndef _UCI_CXX_HPP
#define _UCI_CXX_HPP

#include <uci.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>

namespace uci {

template <class T>
class iterator {  // like uci_foreach_element_safe.

  private:
    const uci_ptr& _ptr;

    uci_element* _it = nullptr;

    uci_element* _next = nullptr;

    // wrapper for clang-tidy
    inline auto _list_to_element(const uci_list* cur) -> uci_element*
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-cstyle-cast)
        return list_to_element(cur);  // macro casting container=pointer-offset.
    }

  public:
    inline explicit iterator(const uci_ptr& ptr, const uci_list* cur)
        : _ptr{ptr}, _it{_list_to_element(cur)}
    {
        _next = _list_to_element(_it->list.next);
    }

    inline iterator(iterator&&) noexcept = default;

    inline iterator(const iterator&) = delete;

    inline auto operator=(const iterator&) -> iterator& = delete;

    inline auto operator=(iterator &&) -> iterator& = delete;

    auto operator*() -> T
    {
        return T{_ptr, _it};
    }

    inline auto operator!=(const iterator& rhs) -> bool
    {
        return (&_it->list != &rhs._it->list);
    }

    inline auto operator++() -> iterator&
    {
        _it = _next;
        _next = _list_to_element(_next->list.next);
        return *this;
    }

    inline ~iterator() = default;
};

class locked_context {
  private:
    static std::mutex inuse;

  public:
    inline locked_context()
    {
        inuse.lock();
    }

    inline locked_context(locked_context&&) noexcept = default;

    inline locked_context(const locked_context&) = delete;

    inline auto operator=(const locked_context&) -> locked_context& = delete;

    inline auto operator=(locked_context &&) -> locked_context& = delete;

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    inline auto get() -> uci_context*  // is member to enforce inuse
    {
        static auto free_ctx = [](uci_context* ctx) { uci_free_context(ctx); };
        static std::unique_ptr<uci_context, decltype(free_ctx)> lazy_ctx{uci_alloc_context(),
                                                                         free_ctx};

        if (!lazy_ctx) {  // it could be available on a later call:
            lazy_ctx.reset(uci_alloc_context());
            if (!lazy_ctx) {
                throw std::runtime_error("uci error: cannot allocate context");
            }
        }

        return lazy_ctx.get();
    }

    inline ~locked_context()
    {
        inuse.unlock();
    }
};

template <class T>
class element {
  private:
    uci_list* _begin = nullptr;

    uci_list* _end = nullptr;

    uci_ptr _ptr{};

  protected:
    [[nodiscard]] inline auto ptr() -> uci_ptr&
    {
        return _ptr;
    }

    [[nodiscard]] inline auto ptr() const -> const uci_ptr&
    {
        return _ptr;
    }

    void init_begin_end(uci_list* begin, uci_list* end)
    {
        _begin = begin;
        _end = end;
    }

    inline explicit element(const uci_ptr& pre, uci_element* last) : _ptr{pre}
    {
        _ptr.last = last;
    }

    inline explicit element() = default;

  public:
    inline element(element&&) noexcept = default;

    inline element(const element&) = delete;

    inline auto operator=(const element&) -> element& = delete;

    inline auto operator=(element &&) -> element& = delete;

    auto operator[](std::string_view key) const -> T;

    [[nodiscard]] inline auto name() const -> std::string
    {
        return _ptr.last->name;
    }

    void rename(const char* value) const;

    void commit() const;

    [[nodiscard]] inline auto begin() const -> iterator<T>
    {
        return iterator<T>{_ptr, _begin};
    }

    [[nodiscard]] inline auto end() const -> iterator<T>
    {
        return iterator<T>{_ptr, _end};
    }

    inline ~element() = default;
};

class section;

class option;

class item;

class package : public element<section> {
  public:
    inline package(const uci_ptr& pre, uci_element* last) : element{pre, last}
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-cstyle-cast)
        ptr().p = uci_to_package(ptr().last);  // macro casting pointer-offset.
        ptr().package = ptr().last->name;

        auto* end = &ptr().p->sections;
        auto* begin = end->next;
        init_begin_end(begin, end);
    }

    explicit package(const char* name);

    auto set(const char* key, const char* type) const -> section;
};

class section : public element<option> {
  public:
    inline section(const uci_ptr& pre, uci_element* last) : element{pre, last}
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-cstyle-cast)
        ptr().s = uci_to_section(ptr().last);  // macro casting pointer-offset.
        ptr().section = ptr().last->name;

        auto* end = &ptr().s->options;
        auto* begin = end->next;
        init_begin_end(begin, end);
    }

    auto set(const char* key, const char* value) const -> option;

    void del();

    [[nodiscard]] inline auto anonymous() const -> bool
    {
        return ptr().s->anonymous;
    }

    [[nodiscard]] inline auto type() const -> std::string
    {
        return ptr().s->type;
    }
};

class option : public element<item> {
  public:
    inline option(const uci_ptr& pre, uci_element* last) : element{pre, last}
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,cppcoreguidelines-pro-type-cstyle-cast)
        ptr().o = uci_to_option(ptr().last);  // macro casting pointer-offset.
        ptr().option = ptr().last->name;

        if (ptr().o->type == UCI_TYPE_LIST) {  // use union ptr().o->v as list:
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            auto* end = &ptr().o->v.list;
            auto* begin = end->next;
            init_begin_end(begin, end);
        }
        else {
            auto* begin = &ptr().last->list;
            auto* end = begin->next;
            init_begin_end(begin, end);
        }
    }

    void del();

    [[nodiscard]] inline auto type() const -> std::string
    {
        return (ptr().o->type == UCI_TYPE_LIST ? "list" : "option");
    }
};

class item : public element<item> {
  public:
    inline item(const uci_ptr& pre, uci_element* last) : element{pre, last}
    {
        ptr().value = ptr().last->name;
    }

    [[nodiscard]] inline auto type() const -> std::string
    {
        return (ptr().o->type == UCI_TYPE_LIST ? "list" : "option");
    }

    [[nodiscard]] inline auto name() const -> std::string
    {
        return (ptr().last->type == UCI_TYPE_ITEM
                    ? ptr().last->name
                    :
                    // else: use union ptr().o->v as string:
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                    ptr().o->v.string);
    }

    inline explicit operator bool() const
    {
        const auto x = std::string_view{name()};

        if (x == "0" || x == "off" || x == "false" || x == "no" || x == "disabled") {
            return false;
        }

        if (x == "1" || x == "on" || x == "true" || x == "yes" || x == "enabled") {
            return true;
        }

        auto errmsg = std::string{"uci_error: item is not bool "} + name();
        throw std::runtime_error(errmsg);
    }

    void rename(const char* value) const;
};

// ------------------------- implementation: ----------------------------------

std::mutex locked_context::inuse{};

inline auto uci_error(uci_context* ctx, const char* prefix = nullptr) -> std::runtime_error
{
    char* errmsg = nullptr;
    uci_get_errorstr(ctx, &errmsg, prefix);

    std::unique_ptr<char, decltype(&std::free)> auto_free{errmsg, std::free};
    return std::runtime_error{errmsg};
}

template <class T>
auto element<T>::operator[](std::string_view key) const -> T
{
    for (auto elmt : *this) {
        if (elmt.name() == key) {
            return elmt;
        }
    }

    auto errmsg = std::string{"uci error: cannot find "}.append(key);
    throw uci_error(locked_context{}.get(), errmsg.c_str());
}

template <class T>
void element<T>::rename(const char* value) const
{
    if (value == name()) {
        return;
    }

    auto ctx = locked_context{};
    auto tmp_ptr = uci_ptr{_ptr};
    tmp_ptr.value = value;
    if (uci_rename(ctx.get(), &tmp_ptr) != 0) {
        auto errmsg = std::string{"uci error: cannot rename "}.append(name());
        throw uci_error(ctx.get(), errmsg.c_str());
    }
}

template <class T>
void element<T>::commit() const
{
    auto ctx = locked_context{};
    // TODO(pst) use when possible:
    // if (uci_commit(ctx.get(), &_ptr.p, true) != 0) {
    //    auto errmsg = std::string{"uci error: cannot commit "} + _ptr.package;
    //    throw uci_error(ctx.get(), errmsg.c_str());
    // }
    auto err = uci_save(ctx.get(), _ptr.p);
    if (err == 0) {
        uci_package* tmp_pkg = nullptr;
        uci_context* tmp_ctx = uci_alloc_context();
        err = (tmp_ctx == nullptr ? 1 : 0);
        if (err == 0) {
            err = uci_load(tmp_ctx, _ptr.package, &tmp_pkg);
        }
        if (err == 0) {
            err = uci_commit(tmp_ctx, &tmp_pkg, false);
        }
        if (err == 0) {
            err = uci_unload(tmp_ctx, tmp_pkg);
        }
        if (tmp_ctx != nullptr) {
            uci_free_context(tmp_ctx);
        }
    }

    if (err != 0) {
        auto errmsg = std::string{"uci error: cannot commit "} + _ptr.package;
        throw uci_error(ctx.get(), errmsg.c_str());
    }
}

package::package(const char* name)
{
    auto ctx = locked_context{};

    auto* pkg = uci_lookup_package(ctx.get(), name);
    if (pkg == nullptr) {
        if (uci_load(ctx.get(), name, &pkg) != 0) {
            auto errmsg = std::string{"uci error: cannot load package "} + name;
            throw uci_error(ctx.get(), errmsg.c_str());
        }
    }

    ptr().package = name;
    ptr().p = pkg;
    ptr().last = &pkg->e;

    auto* end = &ptr().p->sections;
    auto* begin = end->next;
    init_begin_end(begin, end);
}

auto package::set(const char* key, const char* type) const -> section
{
    auto ctx = locked_context{};

    auto tmp_ptr = uci_ptr{ptr()};
    tmp_ptr.section = key;
    tmp_ptr.value = type;
    if (uci_set(ctx.get(), &tmp_ptr) != 0) {
        auto errmsg = std::string{"uci error: cannot set section "} + type + "'" + key +
                      "' in package " + name();
        throw uci_error(ctx.get(), errmsg.c_str());
    }

    return section{ptr(), tmp_ptr.last};
}

auto section::set(const char* key, const char* value) const -> option
{
    auto ctx = locked_context{};

    auto tmp_ptr = uci_ptr{ptr()};
    tmp_ptr.option = key;
    tmp_ptr.value = value;
    if (uci_set(ctx.get(), &tmp_ptr) != 0) {
        auto errmsg = std::string{"uci error: cannot set option "} + key + "'" + value +
                      "' in package " + name();
        throw uci_error(ctx.get(), errmsg.c_str());
    }

    return option{ptr(), tmp_ptr.last};
}

void section::del()
{
    auto ctx = locked_context{};
    if (uci_delete(ctx.get(), &ptr()) != 0) {
        auto errmsg = std::string{"uci error: cannot delete section "} + name();
        throw uci_error(ctx.get(), errmsg.c_str());
    }
}

void option::del()
{
    auto ctx = locked_context{};
    if (uci_delete(ctx.get(), &ptr()) != 0) {
        auto errmsg = std::string{"uci error: cannot delete option "} + name();
        throw uci_error(ctx.get(), errmsg.c_str());
    }
}

void item::rename(const char* value) const
{
    if (value == name()) {
        return;
    }

    auto ctx = locked_context{};
    auto tmp_ptr = uci_ptr{ptr()};

    if (tmp_ptr.last->type != UCI_TYPE_ITEM) {
        tmp_ptr.value = value;
        if (uci_set(ctx.get(), &tmp_ptr) != 0) {
            auto errmsg = std::string{"uci error: cannot rename item "} + name();
            throw uci_error(ctx.get(), errmsg.c_str());
        }
        return;
    }  // else:

    tmp_ptr.value = tmp_ptr.last->name;
    if (uci_del_list(ctx.get(), &tmp_ptr) != 0) {
        auto errmsg = std::string{"uci error: cannot rename (del) "} + name();
        throw uci_error(ctx.get(), errmsg.c_str());
    }

    tmp_ptr.value = value;
    if (uci_add_list(ctx.get(), &tmp_ptr) != 0) {
        auto errmsg = std::string{"uci error: cannot rename (add) "} + value;
        throw uci_error(ctx.get(), errmsg.c_str());
    }
}

}  // namespace uci

#endif
