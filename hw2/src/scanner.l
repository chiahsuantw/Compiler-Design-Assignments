%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512

uint32_t line_num = 1;
char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char string_literal[MAX_LINE_LENG];
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);

%}

integer 0|[1-9][0-9]*
float {integer}\.(0|[0-9]*[1-9])
nonzero_integer [1-9][0-9]*
/* If the integral part is nonzero, then the fractional part can be either zero
or not. However, if the integral part is zero, the fractional part cannot be zero. */
nonzero_float ({nonzero_integer}\.(0|[0-9]*[1-9]))|(0\.([0-9]*[1-9]))

/* exclusive start condition */
%x CCOMMENT

%%
    /* Delimiter */
"," { LIST_TOKEN(","); return COMMA; }
";" { LIST_TOKEN(";"); return SEMI; }
":" { LIST_TOKEN(":"); return COLON; }
"(" { LIST_TOKEN("("); return LPAREN; }
")" { LIST_TOKEN(")"); return RPAREN; }
"[" { LIST_TOKEN("["); return LBRA; }
"]" { LIST_TOKEN("]"); return RBRA; }

    /* Operator */
"+"   { LIST_TOKEN("+"); return ADD; }
"-"   { LIST_TOKEN("-"); return SUB; }
"*"   { LIST_TOKEN("*"); return MUL; }
"/"   { LIST_TOKEN("/"); return DIV; }
"mod" { LIST_TOKEN("mod"); return MOD; }
":="  { LIST_TOKEN(":="); return ASSIGN; }
"<"   { LIST_TOKEN("<"); return LT; }
"<="  { LIST_TOKEN("<="); return LE; }
"<>"  { LIST_TOKEN("<>"); return NEQ; }
">="  { LIST_TOKEN(">="); return GE; }
">"   { LIST_TOKEN(">"); return GT; }
"="   { LIST_TOKEN("="); return EQ; }
"and" { LIST_TOKEN("and"); return AND; }
"or"  { LIST_TOKEN("or"); return OR; }
"not" { LIST_TOKEN("not"); return NOT; }

    /* Reserved Word */
"var"     { LIST_TOKEN("KWvar"); return KWvar; }
"array"   { LIST_TOKEN("KWarray"); return KWarray; }
"of"      { LIST_TOKEN("KWof"); return KWof; }
"boolean" { LIST_TOKEN("KWboolean"); return KWboolean; }
"integer" { LIST_TOKEN("KWinteger"); return KWinteger; }
"real"    { LIST_TOKEN("KWreal"); return KWreal; }
"string"  { LIST_TOKEN("KWstring"); return KWstring; }

"true"    { LIST_TOKEN("KWtrue"); return KWtrue; }
"false"   { LIST_TOKEN("KWfalse"); return KWfalse; }

"def"     { LIST_TOKEN("KWdef"); return KWdef; }
"return"  { LIST_TOKEN("KWreturn"); return KWreturn; }

"begin"   { LIST_TOKEN("KWbegin"); return KWbegin; }
"end"     { LIST_TOKEN("KWend"); return KWend; }

"while"   { LIST_TOKEN("KWwhile"); return KWwhile; }
"do"      { LIST_TOKEN("KWdo"); return KWdo; }

"if"      { LIST_TOKEN("KWif"); return KWif; }
"then"    { LIST_TOKEN("KWthen"); return KWthen; }
"else"    { LIST_TOKEN("KWelse"); return KWelse; }

"for"     { LIST_TOKEN("KWfor"); return KWfor; }
"to"      { LIST_TOKEN("KWto"); return KWto; }

"print"   { LIST_TOKEN("KWprint"); return KWprint; }
"read"    { LIST_TOKEN("KWread"); return KWread; }

    /* Identifier */
[a-zA-Z][a-zA-Z0-9]* { LIST_LITERAL("id", yytext); return ID; }

    /* Integer (decimal/octal) */
{integer} { LIST_LITERAL("integer", yytext); return INT_LITERAL; }
0[0-7]+   { LIST_LITERAL("oct_integer", yytext); return INT_LITERAL; }

    /* Floating-Point */
{float} { LIST_LITERAL("float", yytext); return REAL_LITERAL; }

    /* Scientific Notation [Ee][+-]?[0-9]+ */
({nonzero_integer}|{nonzero_float})[Ee][+-]?({integer}) { LIST_LITERAL("scientific", yytext); return REAL_LITERAL; }

    /* String */
\"([^"\n]|\"\")*\" {
    char *yyt_ptr = yytext + 1;  // +1 for skipping the first double quote "
    char *str_ptr = string_literal;

    while (*yyt_ptr) {
        if (*yyt_ptr == '"') {
            // Handle the situation of two double quotes "" in string literal
            if (*(yyt_ptr + 1) == '"') {
                *str_ptr = *yyt_ptr;
                ++str_ptr;
                yyt_ptr += 2; // move to the next character of ""
            } else {
                ++yyt_ptr;
            }
        } else {  // normal character
            *str_ptr = *yyt_ptr;
            ++str_ptr;
            ++yyt_ptr;
        }
    }
    *str_ptr = '\0';
    LIST_LITERAL("string", string_literal);
    return STRING_LITERAL;
}

    /* Whitespace */
[ \t]+ { LIST_SOURCE; }

    /* Pseudocomment */
"//&"[ST][+-].* {
    LIST_SOURCE;
    char option = yytext[3];
    switch (option) {
    case 'S':
        opt_src = (yytext[4] == '+') ? 1 : 0;
        break;
    case 'T':
        opt_tok = (yytext[4] == '+') ? 1 : 0;
        break;
    }
}

    /* C++ Style Comment */
"//".* { LIST_SOURCE; }

    /* C Style Comment */
"/*"           { LIST_SOURCE; BEGIN(CCOMMENT); }
<CCOMMENT>"*/" { LIST_SOURCE; BEGIN(INITIAL); }
<CCOMMENT>.    { LIST_SOURCE; }

    /* Newline */
<INITIAL,CCOMMENT>\n {
    if (opt_src) {
        printf("%d: %s\n", line_num, current_line);
    }
    ++line_num;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}

    /* Catch the character which is not accepted by all rules above */
. {
    printf("Error at line %d: bad character \"%s\"\n", line_num, yytext);
    exit(-1);
}

%%

static void appendToCurrentLine(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *current_line_ptr = *yytext_ptr;
        ++current_line_ptr;
        ++yytext_ptr;
    }
    *current_line_ptr = '\0';
}
