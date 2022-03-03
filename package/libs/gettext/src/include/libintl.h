/*
 * This code is lifted from http://permalink.gmane.org/gmane.linux.gentoo.embedded/3218
 */

#ifndef _LIBINTL_H
#define _LIBINTL_H      1

#include <locale.h>

/* Undef gettext macros, if any... */
#undef gettext
#undef dgettext
#undef dcgettext
#undef ngettext
#undef dngettext
#undef dcngettext
#undef textdomain
#undef bindtextdomain
#undef bind_textdomain_codeset

/* part of locale.h */
/* #undef setlocale */

#undef _
#undef N_

/* Define gettext stubs that map back to the original strings */
#define gettext(String) (String)
#define dgettext(Domain, String) (String)
#define dcgettext(Domain, String, Type) (String)

#define ngettext(Singular, Plural, Count) \
	((Count) == 1 ? (const char *) (Singular) : (const char *) (Plural))

#define dngettext(Domain, Singular, Plural, Count) \
	((Count) == 1 ? (const char *) (Singular) : (const char *) (Plural))

#define dcngettext(Domain, Singular, Plural, Count, Category) \
	((Count) == 1 ? (const char *) (Singular) : (const char *) (Plural))

#define _(String) (String)
#define N_(String) String

#ifndef _LOCALE_H
/* #define setlocale(Category, Locale) ((char *)NULL) */
#endif

/* No-ops for textdomain operations */
#define bindtextdomain(Domain, Directory) (Domain)
#define bind_textdomain_codeset(Domain, Codeset) (Codeset)
#define textdomain(String) (String) ?: "messages"

#endif  /* _LIBINTL_H */
