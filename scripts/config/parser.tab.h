/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_HELPTEXT = 258,
    T_WORD = 259,
    T_WORD_QUOTE = 260,
    T_ALLNOCONFIG_Y = 261,
    T_BOOL = 262,
    T_CHOICE = 263,
    T_CLOSE_PAREN = 264,
    T_COLON_EQUAL = 265,
    T_COMMENT = 266,
    T_CONFIG = 267,
    T_DEFAULT = 268,
    T_DEFCONFIG_LIST = 269,
    T_DEF_BOOL = 270,
    T_DEF_TRISTATE = 271,
    T_DEPENDS = 272,
    T_ENDCHOICE = 273,
    T_ENDIF = 274,
    T_ENDMENU = 275,
    T_HELP = 276,
    T_HEX = 277,
    T_IF = 278,
    T_IMPLY = 279,
    T_INT = 280,
    T_MAINMENU = 281,
    T_MENU = 282,
    T_MENUCONFIG = 283,
    T_MODULES = 284,
    T_ON = 285,
    T_OPEN_PAREN = 286,
    T_OPTION = 287,
    T_OPTIONAL = 288,
    T_PLUS_EQUAL = 289,
    T_PROMPT = 290,
    T_RANGE = 291,
    T_RESET = 292,
    T_SELECT = 293,
    T_SOURCE = 294,
    T_STRING = 295,
    T_TRISTATE = 296,
    T_VISIBLE = 297,
    T_EOL = 298,
    T_ASSIGN_VAL = 299,
    T_OR = 300,
    T_AND = 301,
    T_EQUAL = 302,
    T_UNEQUAL = 303,
    T_LESS = 304,
    T_LESS_EQUAL = 305,
    T_GREATER = 306,
    T_GREATER_EQUAL = 307,
    T_NOT = 308
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


	char *string;
	struct symbol *symbol;
	struct expr *expr;
	struct menu *menu;
	enum symbol_type type;
	enum variable_flavor flavor;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
