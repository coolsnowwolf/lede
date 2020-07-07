/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2002 Roman Zippel <zippel@linux-m68k.org>
 */

#ifndef LKC_H
#define LKC_H

#include "expr.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "lkc_proto.h"

#define SRCTREE "srctree"

#ifndef PACKAGE
#define PACKAGE "linux"
#endif

#ifndef CONFIG_
#define CONFIG_ "CONFIG_"
#endif
static inline const char *CONFIG_prefix(void)
{
	return getenv( "CONFIG_" ) ?: CONFIG_;
}
#undef CONFIG_
#define CONFIG_ CONFIG_prefix()

enum conf_def_mode {
	def_default,
	def_yes,
	def_mod,
	def_y2m,
	def_m2y,
	def_no,
	def_random
};

extern int yylineno;
void zconfdump(FILE *out);
void zconf_starthelp(void);
FILE *zconf_fopen(const char *name);
void zconf_initscan(const char *name);
void zconf_nextfile(const char *name);
int zconf_lineno(void);
const char *zconf_curname(void);
extern int recursive_is_error;

/* confdata.c */
const char *conf_get_configname(void);
void sym_set_change_count(int count);
void sym_add_change_count(int count);
bool conf_set_all_new_symbols(enum conf_def_mode mode);
void conf_rewrite_mod_or_yes(enum conf_def_mode mode);
void set_all_choice_values(struct symbol *csym);

/* confdata.c and expr.c */
static inline void xfwrite(const void *str, size_t len, size_t count, FILE *out)
{
	assert(len != 0);

	if (fwrite(str, len, count, out) != count)
		fprintf(stderr, "Error in writing or end of file.\n");
}

/* menu.c */
void _menu_init(void);
void menu_warn(struct menu *menu, const char *fmt, ...);
struct menu *menu_add_menu(void);
void menu_end_menu(void);
void menu_add_entry(struct symbol *sym);
void menu_add_dep(struct expr *dep);
void menu_add_visibility(struct expr *dep);
struct property *menu_add_prop(enum prop_type type, struct expr *expr, struct expr *dep);
struct property *menu_add_prompt(enum prop_type type, char *prompt, struct expr *dep);
void menu_add_expr(enum prop_type type, struct expr *expr, struct expr *dep);
void menu_add_symbol(enum prop_type type, struct symbol *sym, struct expr *dep);
void menu_add_option_modules(void);
void menu_add_option_defconfig_list(void);
void menu_add_option_allnoconfig_y(void);
void menu_finalize(struct menu *parent);
void menu_set_type(int type);

/* util.c */
struct file *file_lookup(const char *name);
void *xmalloc(size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xrealloc(void *p, size_t size);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);

/* lexer.l */
int yylex(void);

struct gstr {
	size_t len;
	char  *s;
	/*
	* when max_width is not zero long lines in string s (if any) get
	* wrapped not to exceed the max_width value
	*/
	int max_width;
};
struct gstr str_new(void);
void str_free(struct gstr *gs);
void str_append(struct gstr *gs, const char *s);
void str_printf(struct gstr *gs, const char *fmt, ...);
const char *str_get(struct gstr *gs);

/* symbol.c */
void sym_clear_all_valid(void);
struct symbol *sym_choice_default(struct symbol *sym);
struct property *sym_get_range_prop(struct symbol *sym);
const char *sym_get_string_default(struct symbol *sym);
struct symbol *sym_check_deps(struct symbol *sym);
struct symbol *prop_get_symbol(struct property *prop);

static inline tristate sym_get_tristate_value(struct symbol *sym)
{
	return sym->curr.tri;
}


static inline struct symbol *sym_get_choice_value(struct symbol *sym)
{
	return (struct symbol *)sym->curr.val;
}

static inline bool sym_set_choice_value(struct symbol *ch, struct symbol *chval)
{
	return sym_set_tristate_value(chval, yes);
}

static inline bool sym_is_choice(struct symbol *sym)
{
	return sym->flags & SYMBOL_CHOICE ? true : false;
}

static inline bool sym_is_choice_value(struct symbol *sym)
{
	return sym->flags & SYMBOL_CHOICEVAL ? true : false;
}

static inline bool sym_is_optional(struct symbol *sym)
{
	return sym->flags & SYMBOL_OPTIONAL ? true : false;
}

static inline bool sym_has_value(struct symbol *sym)
{
	return sym->flags & SYMBOL_DEF_USER ? true : false;
}

#ifdef __cplusplus
}
#endif

#endif /* LKC_H */
