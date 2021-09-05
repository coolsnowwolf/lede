#ifndef __REGEXP_PCRE_HPP
#define __REGEXP_PCRE_HPP

#include <pcre.h>
#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace rgx {
/* partially implement the std::regex interface using PCRE for performance
 * (=> pass "match" as non-const reference)
 */

namespace regex_constants {
enum error_type {
    _enum_error_collate,
    _enum_error_ctype,
    _enum_error_escape,
    _enum_error_backref,
    _enum_error_brack,
    _enum_error_paren,
    _enum_error_brace,
    _enum_error_badbrace,
    _enum_error_range,
    _enum_error_space,
    _enum_error_badrepeat,
    _enum_error_complexity,
    _enum_error_stack,
    _enum_error_last
};
static const error_type error_collate(_enum_error_collate);
static const error_type error_ctype(_enum_error_ctype);
static const error_type error_escape(_enum_error_escape);
static const error_type error_backref(_enum_error_backref);
static const error_type error_brack(_enum_error_brack);
static const error_type error_paren(_enum_error_paren);
static const error_type error_brace(_enum_error_brace);
static const error_type error_badbrace(_enum_error_badbrace);
static const error_type error_range(_enum_error_range);
static const error_type error_space(_enum_error_space);
static const error_type error_badrepeat(_enum_error_badrepeat);
static const error_type error_complexity(_enum_error_complexity);
static const error_type error_stack(_enum_error_stack);
}  // namespace regex_constants

class regex_error : public std::runtime_error {
  private:
    regex_constants::error_type errcode;

  public:
    explicit regex_error(regex_constants::error_type code, const char* what = "regex error")
        : runtime_error(what), errcode(code)
    {}

    [[nodiscard]] auto virtual code() const -> regex_constants::error_type;
};

[[nodiscard]] auto regex_error::code() const -> regex_constants::error_type
{
    return errcode;
}

class regex {
  private:
    int errcode = 0;

    const char* errptr = nullptr;

    int erroffset = 0;

    pcre* const re = nullptr;

    static const std::array<regex_constants::error_type, 86> errcode_pcre2regex;

    static const auto BASE = 10;

  public:
    inline regex() = default;

    inline regex(const regex&) = delete;

    inline regex(regex&&) = default;

    inline auto operator=(const regex&) -> regex& = delete;

    inline auto operator=(regex &&) -> regex& = delete;

    explicit regex(const std::string& str) : regex(str.c_str()) {}

    explicit regex(const char* const str)
        : re{pcre_compile2(str, 0, &errcode, &errptr, &erroffset, nullptr)}
    {
        if (re == nullptr) {
            std::string what = std::string("regex error: ") + errptr + '\n';
            what += "    '" + std::string{str} + "'\n";
            what += "     " + std::string(erroffset, ' ') + '^';

            throw regex_error(errcode_pcre2regex.at(errcode), what.c_str());
        }
    }

    ~regex()
    {
        if (re != nullptr) {
            pcre_free(re);
        }
    }

    inline auto operator()() const -> const pcre*
    {
        return re;
    }
};

class smatch {
    friend auto regex_search(std::string::const_iterator begin,
                             std::string::const_iterator end,
                             smatch& match,      // NOLINT(google-runtime-references)
                             const regex& rgx);  // match std::regex interface.

  private:
    std::string::const_iterator begin;

    std::string::const_iterator end;

    std::vector<int> vec{};

    int n = 0;

  public:
    [[nodiscard]] inline auto position(int i = 0) const
    {
        return (i < 0 || i >= n) ? std::string::npos : vec[2 * i];
    }

    [[nodiscard]] inline auto length(int i = 0) const
    {
        return (i < 0 || i >= n) ? 0 : vec[2 * i + 1] - vec[2 * i];
    }

    [[nodiscard]] auto str(int i = 0) const -> std::string
    {  // should we throw?
        if (i < 0 || i >= n) {
            return "";
        }
        int x = vec[2 * i];
        if (x < 0) {
            return "";
        }
        int y = vec[2 * i + 1];
        return std::string{begin + x, begin + y};
    }

    [[nodiscard]] auto format(const std::string& fmt) const;

    [[nodiscard]] auto size() const -> int
    {
        return n;
    }

    [[nodiscard]] inline auto empty() const
    {
        return n < 0;
    }

    [[nodiscard]] inline auto ready() const
    {
        return !vec.empty();
    }
};

inline auto regex_search(const std::string& subj, const regex& rgx);

auto regex_replace(const std::string& subj, const regex& rgx, const std::string& insert);

inline auto regex_search(const std::string& subj,
                         smatch& match,      // NOLINT(google-runtime-references)
                         const regex& rgx);  // match std::regex interface.

auto regex_search(std::string::const_iterator begin,
                  std::string::const_iterator end,
                  smatch& match,      // NOLINT(google-runtime-references)
                  const regex& rgx);  // match std::regex interface.

// ------------------------- implementation: ----------------------------------

inline auto regex_search(const std::string& subj, const regex& rgx)
{
    if (rgx() == nullptr) {
        throw std::runtime_error("regex_search error: no regex given");
    }
    int n =
        pcre_exec(rgx(), nullptr, subj.c_str(), static_cast<int>(subj.length()), 0, 0, nullptr, 0);
    return n >= 0;
}

auto regex_search(const std::string::const_iterator begin,
                  const std::string::const_iterator end,
                  smatch& match,
                  const regex& rgx)
{
    if (rgx() == nullptr) {
        throw std::runtime_error("regex_search error: no regex given");
    }

    int sz = 0;
    pcre_fullinfo(rgx(), nullptr, PCRE_INFO_CAPTURECOUNT, &sz);
    sz = 3 * (sz + 1);

    match.vec.reserve(sz);

    const char* subj = &*begin;
    int len = static_cast<int>(&*end - subj);

    match.begin = begin;
    match.end = end;

    match.n = pcre_exec(rgx(), nullptr, subj, len, 0, 0, &match.vec[0], sz);

    if (match.n < 0) {
        return false;
    }
    if (match.n == 0) {
        match.n = sz / 3;
    }

    return true;
}

inline auto regex_search(const std::string& subj, smatch& match, const regex& rgx)
{
    return regex_search(subj.begin(), subj.end(), match, rgx);
}

auto smatch::format(const std::string& fmt) const
{
    std::string ret{};
    size_t index = 0;

    size_t pos = 0;
    while ((pos = fmt.find('$', index)) != std::string::npos) {
        ret.append(fmt, index, pos - index);
        index = pos + 1;

        char chr = fmt[index++];
        switch (chr) {
            case '&':  // match
                ret += str(0);
                break;

            case '`':  // prefix
                ret.append(begin, begin + vec[0]);
                break;

            case '\'':  // suffix
                ret.append(begin + vec[1], end);
                break;

            default:
                if (isdigit(chr) != 0) {  // one or two digits => submatch:
                    int num = chr - '0';
                    chr = fmt[index];
                    if (isdigit(chr) != 0) {  // second digit:
                        ++index;
                        static const auto base = 10;
                        num = num * base + chr - '0';
                    }
                    ret += str(num);
                    break;
                }  // else:

                ret += '$';
                [[fallthrough]];

            case '$':  // escaped
                ret += chr;
        }
    }
    ret.append(fmt, index);
    return ret;
}

auto regex_replace(const std::string& subj, const regex& rgx, const std::string& insert)
{
    if (rgx() == nullptr) {
        throw std::runtime_error("regex_replace error: no regex given");
    }

    std::string ret{};
    auto pos = subj.begin();

    for (smatch match; regex_search(pos, subj.end(), match, rgx);
         pos += match.position(0) + match.length(0))
    {
        ret.append(pos, pos + match.position(0));
        ret.append(match.format(insert));
    }

    ret.append(pos, subj.end());
    return ret;
}

// ------------ There is only the translation table below : -------------------

const std::array<regex_constants::error_type, 86> regex::errcode_pcre2regex = {
    //   0  no error
    regex_constants::error_type::_enum_error_last,
    //   1  \ at end of pattern
    regex_constants::error_escape,
    //   2  \c at end of pattern
    regex_constants::error_escape,
    //   3  unrecognized character follows \ .
    regex_constants::error_escape,
    //   4  numbers out of order in {} quantifier
    regex_constants::error_badbrace,
    //   5  number too big in {} quantifier
    regex_constants::error_badbrace,
    //   6  missing terminating  for character class
    regex_constants::error_brack,
    //   7  invalid escape sequence in character class
    regex_constants::error_escape,
    //   8  range out of order in character class
    regex_constants::error_range,
    //   9  nothing to repeat
    regex_constants::error_badrepeat,
    //  10  [this code is not in use
    regex_constants::error_type::_enum_error_last,
    //  11  internal error: unexpected repeat
    regex_constants::error_badrepeat,
    //  12  unrecognized character after (? or (?-
    regex_constants::error_backref,
    //  13  POSIX named classes are supported only within a class
    regex_constants::error_range,
    //  14  missing )
    regex_constants::error_paren,
    //  15  reference to non-existent subpattern
    regex_constants::error_backref,
    //  16  erroffset passed as NULL
    regex_constants::error_type::_enum_error_last,
    //  17  unknown option bit(s) set
    regex_constants::error_type::_enum_error_last,
    //  18  missing ) after comment
    regex_constants::error_paren,
    //  19  [this code is not in use
    regex_constants::error_type::_enum_error_last,
    //  20  regular expression is too large
    regex_constants::error_space,
    //  21  failed to get memory
    regex_constants::error_stack,
    //  22  unmatched parentheses
    regex_constants::error_paren,
    //  23  internal error: code overflow
    regex_constants::error_stack,
    //  24  unrecognized character after (?<
    regex_constants::error_backref,
    //  25  lookbehind assertion is not fixed length
    regex_constants::error_backref,
    //  26  malformed number or name after (?(
    regex_constants::error_backref,
    //  27  conditional group contains more than two branches
    regex_constants::error_backref,
    //  28  assertion expected after (?(
    regex_constants::error_backref,
    //  29  (?R or (?[+-digits must be followed by )
    regex_constants::error_backref,
    //  30  unknown POSIX class name
    regex_constants::error_ctype,
    //  31  POSIX collating elements are not supported
    regex_constants::error_collate,
    //  32  this version of PCRE is compiled without UTF support
    regex_constants::error_collate,
    //  33  [this code is not in use
    regex_constants::error_type::_enum_error_last,
    //  34  character value in \x{} or \o{} is too large
    regex_constants::error_escape,
    //  35  invalid condition (?(0)
    regex_constants::error_backref,
    //  36  \C not allowed in lookbehind assertion
    regex_constants::error_escape,
    //  37  PCRE does not support \L, \l, \N{name}, \U, or \u
    regex_constants::error_escape,
    //  38  number after (?C is > 255
    regex_constants::error_backref,
    //  39  closing ) for (?C expected
    regex_constants::error_paren,
    //  40  recursive call could loop indefinitely
    regex_constants::error_complexity,
    //  41  unrecognized character after (?P
    regex_constants::error_backref,
    //  42  syntax error in subpattern name (missing terminator)
    regex_constants::error_paren,
    //  43  two named subpatterns have the same name
    regex_constants::error_backref,
    //  44  invalid UTF-8 string (specifically UTF-8)
    regex_constants::error_collate,
    //  45  support for \P, \p, and \X has not been compiled
    regex_constants::error_escape,
    //  46  malformed \P or \p sequence
    regex_constants::error_escape,
    //  47  unknown property name after \P or \p
    regex_constants::error_escape,
    //  48  subpattern name is too long (maximum 32 characters)
    regex_constants::error_backref,
    //  49  too many named subpatterns (maximum 10000)
    regex_constants::error_complexity,
    //  50  [this code is not in use
    regex_constants::error_type::_enum_error_last,
    //  51  octal value is greater than \377 in 8-bit non-UTF-8 mode
    regex_constants::error_escape,
    //  52  internal error: overran compiling workspace
    regex_constants::error_type::_enum_error_last,
    //  53  internal error: previously-checked referenced subpattern not found
    regex_constants::error_type::_enum_error_last,
    //  54  DEFINE group contains more than one branch
    regex_constants::error_backref,
    //  55  repeating a DEFINE group is not allowed
    regex_constants::error_backref,
    //  56  inconsistent NEWLINE options
    regex_constants::error_escape,
    //  57  \g is not followed by a braced, angle-bracketed, or quoted name/number or by a plain
    //  number
    regex_constants::error_backref,
    //  58  a numbered reference must not be zero
    regex_constants::error_backref,
    //  59  an argument is not allowed for (*ACCEPT), (*FAIL), or (*COMMIT)
    regex_constants::error_complexity,
    //  60  (*VERB) not recognized or malformed
    regex_constants::error_complexity,
    //  61  number is too big
    regex_constants::error_complexity,
    //  62  subpattern name expected
    regex_constants::error_backref,
    //  63  digit expected after (?+
    regex_constants::error_backref,
    //  64   is an invalid data character in JavaScript compatibility mode
    regex_constants::error_escape,
    //  65  different names for subpatterns of the same number are not allowed
    regex_constants::error_backref,
    //  66  (*MARK) must have an argument
    regex_constants::error_complexity,
    //  67  this version of PCRE is not compiled with Unicode property support
    regex_constants::error_collate,
    //  68  \c must be followed by an ASCII character
    regex_constants::error_escape,
    //  69  \k is not followed by a braced, angle-bracketed, or quoted name
    regex_constants::error_backref,
    //  70  internal error: unknown opcode in find_fixedlength()
    regex_constants::error_type::_enum_error_last,
    //  71  \N is not supported in a class
    regex_constants::error_ctype,
    //  72  too many forward references
    regex_constants::error_backref,
    //  73  disallowed Unicode code point (>= 0xd800 && <= 0xdfff)
    regex_constants::error_escape,
    //  74  invalid UTF-16 string (specifically UTF-16)
    regex_constants::error_collate,
    //  75  name is too long in (*MARK), (*PRUNE), (*SKIP), or (*THEN)
    regex_constants::error_complexity,
    //  76  character value in \u.... sequence is too large
    regex_constants::error_escape,
    //  77  invalid UTF-32 string (specifically UTF-32)
    regex_constants::error_collate,
    //  78  setting UTF is disabled by the application
    regex_constants::error_collate,
    //  79  non-hex character in \x{} (closing brace missing?)
    regex_constants::error_escape,
    //  80  non-octal character in \o{} (closing brace missing?)
    regex_constants::error_escape,
    //  81  missing opening brace after \o
    regex_constants::error_brace,
    //  82  parentheses are too deeply nested
    regex_constants::error_complexity,
    //  83  invalid range in character class
    regex_constants::error_range,
    //  84  group name must start with a non-digit
    regex_constants::error_backref,
    //  85  parentheses are too deeply nested (stack check)
    regex_constants::error_stack};

}  // namespace rgx

#endif
