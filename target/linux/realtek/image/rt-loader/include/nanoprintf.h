/* nanoprintf v0.5.5: a tiny embeddable printf replacement written in C.
   https://github.com/charlesnicholson/nanoprintf
   charles.nicholson+nanoprintf@gmail.com
   dual-licensed under 0bsd and unlicense, take your pick. see eof for details. */

#ifndef NPF_H_INCLUDED
#define NPF_H_INCLUDED

#include <stdarg.h>
#include <stddef.h>

// Define this to fully sandbox nanoprintf inside of a translation unit.
#ifdef NANOPRINTF_VISIBILITY_STATIC
  #define NPF_VISIBILITY static
#else
  #define NPF_VISIBILITY extern
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
  #define NPF_PRINTF_ATTR(FORMAT_INDEX, VARGS_INDEX) \
    __attribute__((format(printf, FORMAT_INDEX, VARGS_INDEX)))
#else
  #define NPF_PRINTF_ATTR(FORMAT_INDEX, VARGS_INDEX)
#endif

// Public API

#ifdef __cplusplus
#define NPF_RESTRICT
extern "C" {
#else
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define NPF_RESTRICT restrict
#else
#define NPF_RESTRICT
#endif
#endif

// The npf_ functions all return the number of bytes required to express the
// fully-formatted string, not including the null terminator character.
// The npf_ functions do not return negative values, since the lack of 'l' length
// modifier support makes encoding errors impossible.

NPF_VISIBILITY int npf_snprintf(char * NPF_RESTRICT buffer,
                                size_t bufsz,
                                const char * NPF_RESTRICT format,
                                ...) NPF_PRINTF_ATTR(3, 4);

NPF_VISIBILITY int npf_vsnprintf(char * NPF_RESTRICT buffer,
                                 size_t bufsz,
                                 char const * NPF_RESTRICT format,
                                 va_list vlist)   NPF_PRINTF_ATTR(3, 0);

typedef void (*npf_putc)(int c, void *ctx);
NPF_VISIBILITY int npf_pprintf(npf_putc pc,
                               void * NPF_RESTRICT pc_ctx,
                               char const * NPF_RESTRICT format,
                               ...) NPF_PRINTF_ATTR(3, 4);

NPF_VISIBILITY int npf_vpprintf(npf_putc pc,
                                void * NPF_RESTRICT pc_ctx,
                                char const * NPF_RESTRICT format,
                                va_list vlist) NPF_PRINTF_ATTR(3, 0);

#ifdef __cplusplus
}
#endif

#endif // NPF_H_INCLUDED

/* The implementation of nanoprintf begins here, to be compiled only if
   NANOPRINTF_IMPLEMENTATION is defined. In a multi-file library what follows would
   be nanoprintf.c. */

#ifdef NANOPRINTF_IMPLEMENTATION

#ifndef NPF_IMPLEMENTATION_INCLUDED
#define NPF_IMPLEMENTATION_INCLUDED

#include <limits.h>
#include <stdint.h>

// The conversion buffer must fit at least UINT64_MAX in octal format with the leading '0'.
#ifndef NANOPRINTF_CONVERSION_BUFFER_SIZE
  #define NANOPRINTF_CONVERSION_BUFFER_SIZE    23
#endif
#if NANOPRINTF_CONVERSION_BUFFER_SIZE < 23
  #error The size of the conversion buffer must be at least 23 bytes.
#endif

// Pick reasonable defaults if nothing's been configured.
#if !defined(NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS) && \
    !defined(NANOPRINTF_USE_ALT_FORM_FLAG)
  #define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
  #define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
  #define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
  #define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
  #define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
  #define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
  #define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
  #define NANOPRINTF_USE_ALT_FORM_FLAG 1
#endif

// If anything's been configured, everything must be configured.
#ifndef NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif
#ifndef NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif
#ifndef NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif
#ifndef NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif
#ifndef NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif
#ifndef NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif
#ifndef NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS
  #error NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS must be #defined to 0 or 1
#endif

// Ensure flags are compatible.
#if (NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1) && \
    (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 0)
  #error Precision format specifiers must be enabled if float support is enabled.
#endif

// intmax_t / uintmax_t require stdint from c99 / c++11
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  #ifndef _MSC_VER
    #ifdef __cplusplus
      #if __cplusplus < 201103L
        #error large format specifier support requires C++11 or later.
      #endif
    #else
      #if __STDC_VERSION__ < 199409L
        #error nanoprintf requires C99 or later.
      #endif
    #endif
  #endif
#endif

// Figure out if we can disable warnings with pragmas.
#ifdef __clang__
  #define NPF_CLANG 1
  #define NPF_GCC_PAST_4_6 0
#else
  #define NPF_CLANG 0
  #if defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 6)))
    #define NPF_GCC_PAST_4_6 1
  #else
    #define NPF_GCC_PAST_4_6 0
  #endif
#endif

#if NPF_CLANG || NPF_GCC_PAST_4_6
  #define NPF_HAVE_GCC_WARNING_PRAGMAS 1
#else
  #define NPF_HAVE_GCC_WARNING_PRAGMAS 0
#endif

#if NPF_HAVE_GCC_WARNING_PRAGMAS
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-function"
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  #ifdef __cplusplus
    #pragma GCC diagnostic ignored "-Wold-style-cast"
  #endif
  #pragma GCC diagnostic ignored "-Wpadded"
  #pragma GCC diagnostic ignored "-Wfloat-equal"
  #if NPF_CLANG
    #pragma GCC diagnostic ignored "-Wc++98-compat-pedantic"
    #pragma GCC diagnostic ignored "-Wcovered-switch-default"
    #pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
    #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    #ifndef __APPLE__
      #pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
    #endif
  #elif NPF_GCC_PAST_4_6
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  #endif
#endif

#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable:4619) // there is no warning number 'number'
  // C4619 has to be disabled first!
  #pragma warning(disable:4127) // conditional expression is constant
  #pragma warning(disable:4505) // unreferenced local function has been removed
  #pragma warning(disable:4514) // unreferenced inline function has been removed
  #pragma warning(disable:4701) // potentially uninitialized local variable used
  #pragma warning(disable:4706) // assignment within conditional expression
  #pragma warning(disable:4710) // function not inlined
  #pragma warning(disable:4711) // function selected for inline expansion
  #pragma warning(disable:4820) // padding added after struct member
  #pragma warning(disable:5039) // potentially throwing function passed to extern C function
  #pragma warning(disable:5045) // compiler will insert Spectre mitigation for memory load
  #pragma warning(disable:5262) // implicit switch fall-through
  #pragma warning(disable:26812) // enum type is unscoped
#endif

#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
  #define NPF_NOINLINE __attribute__((noinline))
  #define NPF_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
  #define NPF_NOINLINE __declspec(noinline)
  #define NPF_FORCE_INLINE inline __forceinline
#else
  #define NPF_NOINLINE
  #define NPF_FORCE_INLINE
#endif

#if (NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1) || \
    (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1)
enum {
  NPF_FMT_SPEC_OPT_NONE,
  NPF_FMT_SPEC_OPT_LITERAL,
  NPF_FMT_SPEC_OPT_STAR,
};
#endif

enum {
  NPF_FMT_SPEC_LEN_MOD_NONE,
#if NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS == 1
  NPF_FMT_SPEC_LEN_MOD_SHORT,       // 'h'
  NPF_FMT_SPEC_LEN_MOD_CHAR,        // 'hh'
#endif
  NPF_FMT_SPEC_LEN_MOD_LONG,        // 'l'
  NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE, // 'L'
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG, // 'll'
  NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX,    // 'j'
  NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET,     // 'z'
  NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT,  // 't'
#endif
};

enum {
  NPF_FMT_SPEC_CONV_NONE,
  NPF_FMT_SPEC_CONV_PERCENT,      // '%'
  NPF_FMT_SPEC_CONV_CHAR,         // 'c'
  NPF_FMT_SPEC_CONV_STRING,       // 's'
  NPF_FMT_SPEC_CONV_SIGNED_INT,   // 'i', 'd'
#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
  NPF_FMT_SPEC_CONV_BINARY,       // 'b'
#endif
  NPF_FMT_SPEC_CONV_OCTAL,        // 'o'
  NPF_FMT_SPEC_CONV_HEX_INT,      // 'x', 'X'
  NPF_FMT_SPEC_CONV_UNSIGNED_INT, // 'u'
  NPF_FMT_SPEC_CONV_POINTER,      // 'p'
#if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
  NPF_FMT_SPEC_CONV_WRITEBACK,    // 'n'
#endif
#if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
  NPF_FMT_SPEC_CONV_FLOAT_DEC,      // 'f', 'F'
  NPF_FMT_SPEC_CONV_FLOAT_SCI,      // 'e', 'E'
  NPF_FMT_SPEC_CONV_FLOAT_SHORTEST, // 'g', 'G'
  NPF_FMT_SPEC_CONV_FLOAT_HEX,      // 'a', 'A'
#endif
};

typedef struct npf_format_spec {
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  int field_width;
#endif
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
  int prec;
  uint8_t prec_opt;
#endif
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  uint8_t field_width_opt;
  char left_justified;   // '-'
  char leading_zero_pad; // '0'
#endif
  char prepend;          // ' ' or '+'
#if NANOPRINTF_USE_ALT_FORM_FLAG == 1
  char alt_form;         // '#'
#endif
  char case_adjust;      // 'a' - 'A' , or 0 (must be non-negative to work)
  uint8_t length_modifier;
  uint8_t conv_spec;
} npf_format_spec_t;

#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 0
  typedef long npf_int_t;
  typedef unsigned long npf_uint_t;
#else
  typedef intmax_t npf_int_t;
  typedef uintmax_t npf_uint_t;
#endif

typedef struct npf_bufputc_ctx {
  char *dst;
  size_t len;
  size_t cur;
} npf_bufputc_ctx_t;

#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
  typedef char npf_size_is_ptrdiff[(sizeof(size_t) == sizeof(ptrdiff_t)) ? 1 : -1];
  typedef ptrdiff_t npf_ssize_t;
  typedef size_t npf_uptrdiff_t;
#endif

#ifdef _MSC_VER
  #include <intrin.h>
#endif

#define NPF_MIN(x, y)    ((x) <= (y) ? (x) : (y))
#define NPF_MAX(x, y)    ((x) >= (y) ? (x) : (y))

static int npf_parse_format_spec(char const *format, npf_format_spec_t *out_spec) {
  char const *cur = format;

#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  out_spec->left_justified = 0;
  out_spec->leading_zero_pad = 0;
#endif
  out_spec->case_adjust = 'a' - 'A'; // lowercase
  out_spec->prepend = 0;
#if NANOPRINTF_USE_ALT_FORM_FLAG == 1
  out_spec->alt_form = 0;
#endif

  while (*++cur) { // cur points at the leading '%' character
    switch (*cur) { // Optional flags
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
      case '-': out_spec->left_justified = '-'; out_spec->leading_zero_pad = 0; continue;
      case '0': out_spec->leading_zero_pad = !out_spec->left_justified; continue;
#endif
      case '+': out_spec->prepend = '+'; continue;
      case ' ': if (out_spec->prepend == 0) { out_spec->prepend = ' '; } continue;
#if NANOPRINTF_USE_ALT_FORM_FLAG == 1
      case '#': out_spec->alt_form = '#'; continue;
#endif
      default: break;
    }
    break;
  }

#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
  out_spec->field_width = 0;
  out_spec->field_width_opt = NPF_FMT_SPEC_OPT_NONE;
  if (*cur == '*') {
    out_spec->field_width_opt = NPF_FMT_SPEC_OPT_STAR;
    ++cur;
  } else {
    while ((*cur >= '0') && (*cur <= '9')) {
      out_spec->field_width_opt = NPF_FMT_SPEC_OPT_LITERAL;
      out_spec->field_width = (out_spec->field_width * 10) + (*cur++ - '0');
    }
  }
#endif

#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
  out_spec->prec = 0;
  out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
  if (*cur == '.') {
    ++cur;
    if (*cur == '*') {
      out_spec->prec_opt = NPF_FMT_SPEC_OPT_STAR;
      ++cur;
    } else {
      if (*cur == '-') {
        ++cur;
      } else {
        out_spec->prec_opt = NPF_FMT_SPEC_OPT_LITERAL;
      }
      while ((*cur >= '0') && (*cur <= '9')) {
        out_spec->prec = (out_spec->prec * 10) + (*cur++ - '0');
      }
    }
  }
#endif

  uint_fast8_t tmp_conv = NPF_FMT_SPEC_CONV_NONE;
  out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_NONE;
  switch (*cur++) { // Length modifier
#if NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS == 1
    case 'h':
      out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_SHORT;
      if (*cur == 'h') {
        out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_CHAR;
        ++cur;
      }
      break;
#endif
    case 'l':
      out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LONG;
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
      if (*cur == 'l') {
        out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_LONG_LONG;
        ++cur;
      }
#endif
      break;
#if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
    case 'L': out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE; break;
#endif
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
    case 'j': out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_INTMAX; break;
    case 'z': out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_SIZET; break;
    case 't': out_spec->length_modifier = NPF_FMT_SPEC_LEN_MOD_LARGE_PTRDIFFT; break;
#endif
    default: --cur; break;
  }

  switch (*cur++) { // Conversion specifier
    case '%': out_spec->conv_spec = NPF_FMT_SPEC_CONV_PERCENT;
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
      out_spec->prec = 0;
#endif
      break;

    case 'c': out_spec->conv_spec = NPF_FMT_SPEC_CONV_CHAR;
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
      out_spec->prec = 0;
#endif
      break;

    case 's': out_spec->conv_spec = NPF_FMT_SPEC_CONV_STRING;
      break;

    case 'i':
    case 'd': tmp_conv = NPF_FMT_SPEC_CONV_SIGNED_INT; goto finish;
    case 'o': tmp_conv = NPF_FMT_SPEC_CONV_OCTAL; goto finish;
    case 'u': tmp_conv = NPF_FMT_SPEC_CONV_UNSIGNED_INT; goto finish;
    case 'X': out_spec->case_adjust = 0;
    case 'x': tmp_conv = NPF_FMT_SPEC_CONV_HEX_INT; goto finish;
    finish:
      out_spec->conv_spec = (uint8_t)tmp_conv;
#if (NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1) && \
    (NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1)
      if (out_spec->prec_opt != NPF_FMT_SPEC_OPT_NONE) { out_spec->leading_zero_pad = 0; }
#endif
      break;

#if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
    case 'F': out_spec->case_adjust = 0;
    case 'f':
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_FLOAT_DEC;
      if (out_spec->prec_opt == NPF_FMT_SPEC_OPT_NONE) { out_spec->prec = 6; }
      break;

    case 'E': out_spec->case_adjust = 0;
    case 'e':
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_FLOAT_SCI;
      if (out_spec->prec_opt == NPF_FMT_SPEC_OPT_NONE) { out_spec->prec = 6; }
      break;

    case 'G': out_spec->case_adjust = 0;
    case 'g':
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_FLOAT_SHORTEST;
      if (out_spec->prec_opt == NPF_FMT_SPEC_OPT_NONE) { out_spec->prec = 6; }
      break;

    case 'A': out_spec->case_adjust = 0;
    case 'a':
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_FLOAT_HEX;
      if (out_spec->prec_opt == NPF_FMT_SPEC_OPT_NONE) { out_spec->prec = 6; }
      break;
#endif

#if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
    case 'n':
      // todo: reject string if flags or width or precision exist
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_WRITEBACK;
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
#endif
      break;
#endif

    case 'p':
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_POINTER;
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      out_spec->prec_opt = NPF_FMT_SPEC_OPT_NONE;
#endif
      break;

#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
    case 'B':
      out_spec->case_adjust = 0;
    case 'b':
      out_spec->conv_spec = NPF_FMT_SPEC_CONV_BINARY;
      break;
#endif

    default: return 0;
  }

  return (int)(cur - format);
}

static NPF_NOINLINE int npf_utoa_rev(
    npf_uint_t val, char *buf, uint_fast8_t base, char case_adj) {
  uint_fast8_t n = 0;
  do {
    int_fast8_t const d = (int_fast8_t)(val % base);
    *buf++ = (char)(((d < 10) ? '0' : ('A' - 10 + case_adj)) + d);
    ++n;
    val /= base;
  } while (val);
  return (int)n;
}

#if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1

#include <float.h>

#if (DBL_MANT_DIG <= 11) && (DBL_MAX_EXP <= 16)
  typedef uint_fast16_t npf_double_bin_t;
  typedef int_fast8_t npf_ftoa_exp_t;
#elif (DBL_MANT_DIG <= 24) && (DBL_MAX_EXP <= 128)
  typedef uint_fast32_t npf_double_bin_t;
  typedef int_fast8_t npf_ftoa_exp_t;
#elif (DBL_MANT_DIG <= 53) && (DBL_MAX_EXP <= 1024)
  typedef uint_fast64_t npf_double_bin_t;
  typedef int_fast16_t npf_ftoa_exp_t;
#else
  #error Unsupported width of the double type.
#endif

// The floating point conversion code works with an unsigned integer type of any size.
#ifndef NANOPRINTF_CONVERSION_FLOAT_TYPE
  #define NANOPRINTF_CONVERSION_FLOAT_TYPE unsigned int
#endif
typedef NANOPRINTF_CONVERSION_FLOAT_TYPE npf_ftoa_man_t;

#if (NANOPRINTF_CONVERSION_BUFFER_SIZE <= UINT_FAST8_MAX) && (UINT_FAST8_MAX <= INT_MAX)
  typedef uint_fast8_t npf_ftoa_dec_t;
#else
  typedef int npf_ftoa_dec_t;
#endif

enum {
  NPF_DOUBLE_EXP_MASK = DBL_MAX_EXP * 2 - 1,
  NPF_DOUBLE_EXP_BIAS = DBL_MAX_EXP - 1,
  NPF_DOUBLE_MAN_BITS = DBL_MANT_DIG - 1,
  NPF_DOUBLE_BIN_BITS = sizeof(npf_double_bin_t) * CHAR_BIT,
  NPF_DOUBLE_SIGN_POS = sizeof(double) * CHAR_BIT - 1,
  NPF_FTOA_MAN_BITS   = sizeof(npf_ftoa_man_t) * CHAR_BIT,
  NPF_FTOA_SHIFT_BITS =
    ((NPF_FTOA_MAN_BITS < DBL_MANT_DIG) ? NPF_FTOA_MAN_BITS : DBL_MANT_DIG) - 1
};

/* Generally, floating-point conversion implementations use
   grisu2 (https://bit.ly/2JgMggX) and ryu (https://bit.ly/2RLXSg0) algorithms,
   which are mathematically exact and fast, but require large lookup tables.

   This implementation was inspired by Wojciech Muła's (zdjęcia@garnek.pl)
   algorithm (http://0x80.pl/notesen/2015-12-29-float-to-string.html) and
   extended further by adding dynamic scaling and configurable integer width by
   Oskars Rubenis (https://github.com/Okarss). */

static NPF_FORCE_INLINE npf_double_bin_t npf_double_to_int_rep(double f) {
  // Union-cast is UB pre-C11 and in all C++; the compiler optimizes the code below.
  npf_double_bin_t bin;
  char const *src = (char const *)&f;
  char *dst = (char *)&bin;
  for (uint_fast8_t i = 0; i < sizeof(f); ++i) { dst[i] = src[i]; }
  return bin;
}

static int npf_ftoa_rev(char *buf, npf_format_spec_t const *spec, double f) {
  char const *ret = NULL;
  npf_double_bin_t bin = npf_double_to_int_rep(f);

  // Unsigned -> signed int casting is IB and can raise a signal but generally doesn't.
  npf_ftoa_exp_t exp =
    (npf_ftoa_exp_t)((npf_ftoa_exp_t)(bin >> NPF_DOUBLE_MAN_BITS) & NPF_DOUBLE_EXP_MASK);

  bin &= ((npf_double_bin_t)0x1 << NPF_DOUBLE_MAN_BITS) - 1;
  if (exp == (npf_ftoa_exp_t)NPF_DOUBLE_EXP_MASK) { // special value
    ret = (bin) ? "NAN" : "FNI";
    goto exit;
  }
  if (spec->prec > (NANOPRINTF_CONVERSION_BUFFER_SIZE - 2)) { goto exit; }
  if (exp) { // normal number
    bin |= (npf_double_bin_t)0x1 << NPF_DOUBLE_MAN_BITS;
  } else { // subnormal number
    ++exp;
  }
  exp = (npf_ftoa_exp_t)(exp - NPF_DOUBLE_EXP_BIAS);

  uint_fast8_t carry; carry = 0;
  npf_ftoa_dec_t end, dec; dec = (npf_ftoa_dec_t)spec->prec;
  if (dec
#if NANOPRINTF_USE_ALT_FORM_FLAG == 1
      || spec->alt_form
#endif
  ) {
    buf[dec++] = '.';
  }

  { // Integer part
    npf_ftoa_man_t man_i;

    if (exp >= 0) {
      int_fast8_t shift_i =
        (int_fast8_t)((exp > NPF_FTOA_SHIFT_BITS) ? (int)NPF_FTOA_SHIFT_BITS : exp);
      npf_ftoa_exp_t exp_i = (npf_ftoa_exp_t)(exp - shift_i);
      shift_i = (int_fast8_t)(NPF_DOUBLE_MAN_BITS - shift_i);
      man_i = (npf_ftoa_man_t)(bin >> shift_i);

      if (exp_i) {
        if (shift_i) {
          carry = (bin >> (shift_i - 1)) & 0x1;
        }
        exp = NPF_DOUBLE_MAN_BITS; // invalidate the fraction part
      }

      // Scale the exponent from base-2 to base-10.
      for (; exp_i; --exp_i) {
        if (!(man_i & ((npf_ftoa_man_t)0x1 << (NPF_FTOA_MAN_BITS - 1)))) {
          man_i = (npf_ftoa_man_t)(man_i << 1);
          man_i = (npf_ftoa_man_t)(man_i | carry); carry = 0;
        } else {
          if (dec >= NANOPRINTF_CONVERSION_BUFFER_SIZE) { goto exit; }
          buf[dec++] = '0';
          carry = (((uint_fast8_t)(man_i % 5) + carry) > 2);
          man_i /= 5;
        }
      }
    } else {
      man_i = 0;
    }
    end = dec;

    do { // Print the integer
      if (end >= NANOPRINTF_CONVERSION_BUFFER_SIZE) { goto exit; }
      buf[end++] = (char)('0' + (char)(man_i % 10));
      man_i /= 10;
    } while (man_i);
  }

  { // Fraction part
    npf_ftoa_man_t man_f;
    npf_ftoa_dec_t dec_f = (npf_ftoa_dec_t)spec->prec;

    if (exp < NPF_DOUBLE_MAN_BITS) {
      int_fast8_t shift_f = (int_fast8_t)((exp < 0) ? -1 : exp);
      npf_ftoa_exp_t exp_f = (npf_ftoa_exp_t)(exp - shift_f);
      npf_double_bin_t bin_f =
        bin << ((NPF_DOUBLE_BIN_BITS - NPF_DOUBLE_MAN_BITS) + shift_f);

      // This if-else statement can be completely optimized at compile time.
      if (NPF_DOUBLE_BIN_BITS > NPF_FTOA_MAN_BITS) {
        man_f = (npf_ftoa_man_t)(bin_f >> ((unsigned)(NPF_DOUBLE_BIN_BITS -
                                                      NPF_FTOA_MAN_BITS) %
                                           NPF_DOUBLE_BIN_BITS));
        carry = (uint_fast8_t)((bin_f >> ((unsigned)(NPF_DOUBLE_BIN_BITS -
                                                     NPF_FTOA_MAN_BITS - 1) %
                                          NPF_DOUBLE_BIN_BITS)) & 0x1);
      } else {
        man_f = (npf_ftoa_man_t)((npf_ftoa_man_t)bin_f
                                 << ((unsigned)(NPF_FTOA_MAN_BITS -
                                                NPF_DOUBLE_BIN_BITS) % NPF_FTOA_MAN_BITS));
        carry = 0;
      }

      // Scale the exponent from base-2 to base-10 and prepare the first digit.
      for (uint_fast8_t digit = 0; dec_f && (exp_f < 4); ++exp_f) {
        if ((man_f > ((npf_ftoa_man_t)-4 / 5)) || digit) {
          carry = (uint_fast8_t)(man_f & 0x1);
          man_f = (npf_ftoa_man_t)(man_f >> 1);
        } else {
          man_f = (npf_ftoa_man_t)(man_f * 5);
          if (carry) { man_f = (npf_ftoa_man_t)(man_f + 3); carry = 0; }
          if (exp_f < 0) {
            buf[--dec_f] = '0';
          } else {
            ++digit;
          }
        }
      }
      man_f = (npf_ftoa_man_t)(man_f + carry);
      carry = (exp_f >= 0);
      dec = 0;
    } else {
      man_f = 0;
    }

    if (dec_f) {
      // Print the fraction
      for (;;) {
        buf[--dec_f] = (char)('0' + (char)(man_f >> (NPF_FTOA_MAN_BITS - 4)));
        man_f = (npf_ftoa_man_t)(man_f & ~((npf_ftoa_man_t)0xF << (NPF_FTOA_MAN_BITS - 4)));
        if (!dec_f) { break; }
        man_f = (npf_ftoa_man_t)(man_f * 10);
      }
      man_f = (npf_ftoa_man_t)(man_f << 4);
    }
    if (exp < NPF_DOUBLE_MAN_BITS) {
      carry &= (uint_fast8_t)(man_f >> (NPF_FTOA_MAN_BITS - 1));
    }
  }

  // Round the number
  for (; carry; ++dec) {
    if (dec >= NANOPRINTF_CONVERSION_BUFFER_SIZE) { goto exit; }
    if (dec >= end) { buf[end++] = '0'; }
    if (buf[dec] == '.') { continue; }
    carry = (buf[dec] == '9');
    buf[dec] = (char)(carry ? '0' : (buf[dec] + 1));
  }

  return (int)end;
exit:
  if (!ret) { ret = "RRE"; }
  uint_fast8_t i;
  for (i = 0; ret[i]; ++i) { buf[i] = (char)(ret[i] + spec->case_adjust); }
  return -(int)i;
}

#endif // NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS

#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
static int npf_bin_len(npf_uint_t u) {
  // Return the length of the binary string format of 'u', preferring intrinsics.
  if (!u) { return 1; }

#ifdef _MSC_VER // Win64, use _BSR64 for everything. If x86, use _BSR when non-large.
  #ifdef _M_X64
    #define NPF_HAVE_BUILTIN_CLZ
    #define NPF_CLZ _BitScanReverse64
  #elif NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 0
    #define NPF_HAVE_BUILTIN_CLZ
    #define NPF_CLZ _BitScanReverse
  #endif
  #ifdef NPF_HAVE_BUILTIN_CLZ
    unsigned long idx;
    NPF_CLZ(&idx, u);
    return (int)(idx + 1);
  #endif
#elif NPF_CLANG || NPF_GCC_PAST_4_6
  #define NPF_HAVE_BUILTIN_CLZ
  #if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
    #define NPF_CLZ(X) ((sizeof(long long) * CHAR_BIT) - (size_t)__builtin_clzll(X))
  #else
    #define NPF_CLZ(X) ((sizeof(long) * CHAR_BIT) - (size_t)__builtin_clzl(X))
  #endif
  return (int)NPF_CLZ(u);
#endif

#ifndef NPF_HAVE_BUILTIN_CLZ
  int n;
  for (n = 0; u; ++n, u >>= 1); // slow but small software fallback
  return n;
#else
  #undef NPF_HAVE_BUILTIN_CLZ
  #undef NPF_CLZ
#endif
}
#endif

static void npf_bufputc(int c, void *ctx) {
  npf_bufputc_ctx_t *bpc = (npf_bufputc_ctx_t *)ctx;
  if (bpc->cur < bpc->len) { bpc->dst[bpc->cur++] = (char)c; }
}

static void npf_bufputc_nop(int c, void *ctx) { (void)c; (void)ctx; }

typedef struct npf_cnt_putc_ctx {
  npf_putc pc;
  void *ctx;
  int n;
} npf_cnt_putc_ctx_t;

static void npf_putc_cnt(int c, void *ctx) {
  npf_cnt_putc_ctx_t *pc_cnt = (npf_cnt_putc_ctx_t *)ctx;
  ++pc_cnt->n;
  pc_cnt->pc(c, pc_cnt->ctx); // sibling-call optimization
}

#define NPF_PUTC(VAL) do { npf_putc_cnt((int)(VAL), &pc_cnt); } while (0)

#define NPF_EXTRACT(MOD, CAST_TO, EXTRACT_AS) \
  case NPF_FMT_SPEC_LEN_MOD_##MOD: val = (CAST_TO)va_arg(args, EXTRACT_AS); break

#define NPF_WRITEBACK(MOD, TYPE) \
  case NPF_FMT_SPEC_LEN_MOD_##MOD: *(va_arg(args, TYPE *)) = (TYPE)pc_cnt.n; break

int npf_vpprintf(npf_putc pc, void *pc_ctx, char const *format, va_list args) {
  npf_format_spec_t fs;
  char const *cur = format;
  npf_cnt_putc_ctx_t pc_cnt;
  pc_cnt.pc = pc;
  pc_cnt.ctx = pc_ctx;
  pc_cnt.n = 0;

  while (*cur) {
    int const fs_len = (*cur != '%') ? 0 : npf_parse_format_spec(cur, &fs);
    if (!fs_len) { NPF_PUTC(*cur++); continue; }
    cur += fs_len;

    // Extract star-args immediately
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    if (fs.field_width_opt == NPF_FMT_SPEC_OPT_STAR) {
      fs.field_width = va_arg(args, int);
      if (fs.field_width < 0) {
        fs.field_width = -fs.field_width;
        fs.left_justified = 1;
      }
    }
#endif
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    if (fs.prec_opt == NPF_FMT_SPEC_OPT_STAR) {
      fs.prec = va_arg(args, int);
      if (fs.prec < 0) { fs.prec_opt = NPF_FMT_SPEC_OPT_NONE; }
    }
#endif

    union { char cbuf_mem[NANOPRINTF_CONVERSION_BUFFER_SIZE]; npf_uint_t binval; } u;
    char *cbuf = u.cbuf_mem, sign_c = 0;
    int cbuf_len = 0;
    char need_0x = 0;
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    int field_pad = 0;
    char pad_c = 0;
#endif
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    int prec_pad = 0;
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    uint_fast8_t zero = 0;
#endif
#endif

    // Extract and convert the argument to string, point cbuf at the text.
    switch (fs.conv_spec) {
      case NPF_FMT_SPEC_CONV_PERCENT:
        *cbuf = '%';
        cbuf_len = 1;
        break;

      case NPF_FMT_SPEC_CONV_CHAR:
        *cbuf = (char)va_arg(args, int);
        cbuf_len = (*cbuf) ? 1 : 0;
        break;

      case NPF_FMT_SPEC_CONV_STRING: {
        cbuf = va_arg(args, char *);
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
        for (char const *s = cbuf;
             ((fs.prec_opt == NPF_FMT_SPEC_OPT_NONE) || (cbuf_len < fs.prec)) && cbuf && *s;
             ++s, ++cbuf_len);
#else
        for (char const *s = cbuf; cbuf && *s; ++s, ++cbuf_len); // strlen
#endif
      } break;

      case NPF_FMT_SPEC_CONV_SIGNED_INT: {
        npf_int_t val = 0;
        switch (fs.length_modifier) {
          NPF_EXTRACT(NONE, int, int);
#if NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS == 1
          NPF_EXTRACT(SHORT, short, int);
          NPF_EXTRACT(CHAR, signed char, int);
#endif
          NPF_EXTRACT(LONG, long, long);
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
          NPF_EXTRACT(LARGE_LONG_LONG, long long, long long);
          NPF_EXTRACT(LARGE_INTMAX, intmax_t, intmax_t);
          NPF_EXTRACT(LARGE_SIZET, npf_ssize_t, npf_ssize_t);
          NPF_EXTRACT(LARGE_PTRDIFFT, ptrdiff_t, ptrdiff_t);
#endif
          default: break;
        }

        sign_c = (val < 0) ? '-' : fs.prepend;

#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
        zero = !val;
#endif
        // special case, if prec and value are 0, skip
        if (!val && (fs.prec_opt != NPF_FMT_SPEC_OPT_NONE) && !fs.prec) {
          cbuf_len = 0;
        } else
#endif
        {
          npf_uint_t uval = (npf_uint_t)val;
          if (val < 0) { uval = 0 - uval; }
          cbuf_len = npf_utoa_rev(uval, cbuf, 10, fs.case_adjust);
        }
      } break;

#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
      case NPF_FMT_SPEC_CONV_BINARY:
#endif
      case NPF_FMT_SPEC_CONV_OCTAL:
      case NPF_FMT_SPEC_CONV_HEX_INT:
      case NPF_FMT_SPEC_CONV_UNSIGNED_INT:
      case NPF_FMT_SPEC_CONV_POINTER: {
        npf_uint_t val = 0;

        if (fs.conv_spec == NPF_FMT_SPEC_CONV_POINTER) {
          val = (npf_uint_t)(uintptr_t)va_arg(args, void *);
        } else {
          switch (fs.length_modifier) {
            NPF_EXTRACT(NONE, unsigned, unsigned);
#if NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS == 1
            NPF_EXTRACT(SHORT, unsigned short, unsigned);
            NPF_EXTRACT(CHAR, unsigned char, unsigned);
#endif
            NPF_EXTRACT(LONG, unsigned long, unsigned long);
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
            NPF_EXTRACT(LARGE_LONG_LONG, unsigned long long, unsigned long long);
            NPF_EXTRACT(LARGE_INTMAX, uintmax_t, uintmax_t);
            NPF_EXTRACT(LARGE_SIZET, size_t, size_t);
            NPF_EXTRACT(LARGE_PTRDIFFT, npf_uptrdiff_t, npf_uptrdiff_t);
#endif
            default: break;
          }
        }

#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
        zero = !val;
#endif
        if (!val && (fs.prec_opt != NPF_FMT_SPEC_OPT_NONE) && !fs.prec) {
          // Zero value and explicitly-requested zero precision means "print nothing".
#if NANOPRINTF_USE_ALT_FORM_FLAG == 1
          if ((fs.conv_spec == NPF_FMT_SPEC_CONV_OCTAL) && fs.alt_form) {
            fs.prec = 1; // octal special case, print a single '0'
          }
#endif
        } else
#endif
#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
        if (fs.conv_spec == NPF_FMT_SPEC_CONV_BINARY) {
          cbuf_len = npf_bin_len(val); u.binval = val;
        } else
#endif
        {
          uint_fast8_t const base = (fs.conv_spec == NPF_FMT_SPEC_CONV_OCTAL) ?
            8u : ((fs.conv_spec == NPF_FMT_SPEC_CONV_UNSIGNED_INT) ? 10u : 16u);
          cbuf_len = npf_utoa_rev(val, cbuf, base, fs.case_adjust);
        }

#if NANOPRINTF_USE_ALT_FORM_FLAG == 1
        if (val && fs.alt_form && (fs.conv_spec == NPF_FMT_SPEC_CONV_OCTAL)) {
          cbuf[cbuf_len++] = '0'; // OK to add leading octal '0' immediately.
        }

        if (val && fs.alt_form) { // 0x or 0b but can't write it yet.
          if ((fs.conv_spec == NPF_FMT_SPEC_CONV_HEX_INT) ||
              (fs.conv_spec == NPF_FMT_SPEC_CONV_POINTER)) { need_0x = 'X'; }
#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
          else if (fs.conv_spec == NPF_FMT_SPEC_CONV_BINARY) { need_0x = 'B'; }
#endif
          if (need_0x) { need_0x = (char)(need_0x + fs.case_adjust); }
        }
#endif
      } break;

#if NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS == 1
      case NPF_FMT_SPEC_CONV_WRITEBACK:
        switch (fs.length_modifier) {
          NPF_WRITEBACK(NONE, int);
#if NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS == 1
          NPF_WRITEBACK(SHORT, short);
          NPF_WRITEBACK(CHAR, signed char);
#endif
          NPF_WRITEBACK(LONG, long);
#if NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS == 1
          NPF_WRITEBACK(LARGE_LONG_LONG, long long);
          NPF_WRITEBACK(LARGE_INTMAX, intmax_t);
          NPF_WRITEBACK(LARGE_SIZET, npf_ssize_t);
          NPF_WRITEBACK(LARGE_PTRDIFFT, ptrdiff_t);
#endif
          default: break;
        } break;
#endif

#if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
      case NPF_FMT_SPEC_CONV_FLOAT_DEC:
      case NPF_FMT_SPEC_CONV_FLOAT_SCI:
      case NPF_FMT_SPEC_CONV_FLOAT_SHORTEST:
      case NPF_FMT_SPEC_CONV_FLOAT_HEX: {
        double val;
        if (fs.length_modifier == NPF_FMT_SPEC_LEN_MOD_LONG_DOUBLE) {
          val = (double)va_arg(args, long double);
        } else {
          val = va_arg(args, double);
        }

        sign_c = (npf_double_to_int_rep(val) >> NPF_DOUBLE_SIGN_POS) ? '-' : fs.prepend;
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
        zero = (val == 0.);
#endif
        cbuf_len = npf_ftoa_rev(cbuf, &fs, val);
        if (cbuf_len < 0) { // negative means text (not number), so ignore the '0' flag
           cbuf_len = -cbuf_len;
#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
           fs.leading_zero_pad = 0;
#endif
        }
      } break;
#endif
      default: break;
    }

#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    // Compute the field width pad character
    if (fs.field_width_opt != NPF_FMT_SPEC_OPT_NONE) {
      if (fs.leading_zero_pad) {
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
        if ((fs.prec_opt != NPF_FMT_SPEC_OPT_NONE) && !fs.prec && zero) {
          pad_c = ' ';
        } else
#endif
        { pad_c = '0'; }
      } else { pad_c = ' '; }
    }
#endif

    // Compute the number of bytes to truncate or '0'-pad.
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    if (fs.conv_spec != NPF_FMT_SPEC_CONV_STRING) {
#if NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS == 1
      // float precision is after the decimal point
      if ((fs.conv_spec != NPF_FMT_SPEC_CONV_FLOAT_DEC) &&
          (fs.conv_spec != NPF_FMT_SPEC_CONV_FLOAT_SCI) &&
          (fs.conv_spec != NPF_FMT_SPEC_CONV_FLOAT_SHORTEST) &&
          (fs.conv_spec != NPF_FMT_SPEC_CONV_FLOAT_HEX))
#endif
      { prec_pad = NPF_MAX(0, fs.prec - cbuf_len); }
    }
#endif

#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    // Given the full converted length, how many pad bytes?
    field_pad = fs.field_width - cbuf_len - !!sign_c;
    if (need_0x) { field_pad -= 2; }
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
    field_pad -= prec_pad;
#endif
    field_pad = NPF_MAX(0, field_pad);

    // Apply right-justified field width if requested
    if (!fs.left_justified && pad_c) { // If leading zeros pad, sign goes first.
      if (pad_c == '0') {
        if (sign_c) { NPF_PUTC(sign_c); sign_c = 0; }
        // Pad byte is '0', write '0x' before '0' pad chars.
        if (need_0x) { NPF_PUTC('0'); NPF_PUTC(need_0x); }
      }
      while (field_pad-- > 0) { NPF_PUTC(pad_c); }
      // Pad byte is ' ', write '0x' after ' ' pad chars but before number.
      if ((pad_c != '0') && need_0x) { NPF_PUTC('0'); NPF_PUTC(need_0x); }
    } else
#endif
    { if (need_0x) { NPF_PUTC('0'); NPF_PUTC(need_0x); } } // no pad, '0x' requested.

    // Write the converted payload
    if (fs.conv_spec == NPF_FMT_SPEC_CONV_STRING) {
      for (int i = 0; cbuf && (i < cbuf_len); ++i) { NPF_PUTC(cbuf[i]); }
    } else {
      if (sign_c) { NPF_PUTC(sign_c); }
#if NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS == 1
      while (prec_pad-- > 0) { NPF_PUTC('0'); } // int precision leads.
#endif
#if NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS == 1
      if (fs.conv_spec == NPF_FMT_SPEC_CONV_BINARY) {
        while (cbuf_len) { NPF_PUTC('0' + ((u.binval >> --cbuf_len) & 1)); }
      } else
#endif
      { while (cbuf_len-- > 0) { NPF_PUTC(cbuf[cbuf_len]); } } // payload is reversed
    }

#if NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS == 1
    if (fs.left_justified && pad_c) { // Apply left-justified field width
      while (field_pad-- > 0) { NPF_PUTC(pad_c); }
    }
#endif
  }

  return pc_cnt.n;
}

#undef NPF_PUTC
#undef NPF_EXTRACT
#undef NPF_WRITEBACK

int npf_pprintf(npf_putc pc,
                void * NPF_RESTRICT pc_ctx,
                char const * NPF_RESTRICT format,
                ...) {
  va_list val;
  va_start(val, format);
  int const rv = npf_vpprintf(pc, pc_ctx, format, val);
  va_end(val);
  return rv;
}

int npf_snprintf(char * NPF_RESTRICT buffer,
                 size_t bufsz,
                 const char * NPF_RESTRICT format,
                 ...) {
  va_list val;
  va_start(val, format);
  int const rv = npf_vsnprintf(buffer, bufsz, format, val);
  va_end(val);
  return rv;
}

int npf_vsnprintf(char * NPF_RESTRICT buffer,
                  size_t bufsz,
                  char const * NPF_RESTRICT format,
                  va_list vlist) {
  npf_bufputc_ctx_t bufputc_ctx;
  bufputc_ctx.dst = buffer;
  bufputc_ctx.len = bufsz;
  bufputc_ctx.cur = 0;

  npf_putc const pc = buffer ? npf_bufputc : npf_bufputc_nop;
  int const n = npf_vpprintf(pc, &bufputc_ctx, format, vlist);

  if (buffer && bufsz) {
#ifdef NANOPRINTF_SNPRINTF_SAFE_EMPTY_STRING_ON_OVERFLOW
    buffer[(n < 0 || (unsigned)n >= bufsz) ? 0 : n] = '\0';
#else
    buffer[n < 0 ? 0 : NPF_MIN((unsigned)n, bufsz - 1)] = '\0';
#endif
  }

  return n;
}

#if NPF_HAVE_GCC_WARNING_PRAGMAS
  #pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#endif // NPF_IMPLEMENTATION_INCLUDED
#endif // NANOPRINTF_IMPLEMENTATION

/*
  nanoprintf is dual-licensed under both the "Unlicense" and the
  "Zero-Clause BSD" (0BSD) licenses. The intent of this dual-licensing
  structure is to make nanoprintf as consumable as possible in as many
  environments / countries / companies as possible without any
  encumberances.

  The text of the two licenses follows below:

  ============================== UNLICENSE ==============================

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org>

  ================================ 0BSD =================================

  Copyright (C) 2019- by Charles Nicholson <charles.nicholson+nanoprintf@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for
  any purpose with or without fee is hereby granted.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
