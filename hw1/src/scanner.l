%option never-interactive
%option nounput
%option noinput

%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define LIST_SOURCE                 appendToCurrentLine(yytext)
#define LIST_TOKEN(name)            do { LIST_SOURCE; if(opt_tok) printf("<%s>\n", name); } while(0)
#define LIST_LITERAL(name, literal) do { LIST_SOURCE; if(opt_tok) printf("<%s: %s>\n", name, literal); } while(0)
#define MAX_LINE_LENG               512

uint32_t line_num = 1;
static char current_line[MAX_LINE_LENG];

static uint32_t opt_src = 1;
static uint32_t opt_tok = 1;
static char *current_line_ptr = current_line;

static void appendToCurrentLine(const char *yytext_ptr);
static void handleNewline();
static void handleString();
%}

%x BLOCK_COMMENT

nonZeroDecimal [1-9][0-9]*
nonZeroFloat {ltZeroFloat}|{gtZeroFloat}
ltZeroFloat "0."[0-9]*[1-9]
gtZeroFloat [1-9][0-9]*"."(0|[0-9]*[1-9])

%%
    /* Delimiters */
","   { LIST_TOKEN(",");   }
";"   { LIST_TOKEN(";");   }
":"   { LIST_TOKEN(":");   }
"("   { LIST_TOKEN("(");   }
")"   { LIST_TOKEN(")");   }
"["   { LIST_TOKEN("[");   }
"]"   { LIST_TOKEN("]");   }

    /* Operators */
"+"   { LIST_TOKEN("+");   }
"-"   { LIST_TOKEN("-");   }
"*"   { LIST_TOKEN("*");   }
"/"   { LIST_TOKEN("/");   }
">"   { LIST_TOKEN(">");   }
"<"   { LIST_TOKEN("<");   }
"="   { LIST_TOKEN("=");   }
":="  { LIST_TOKEN(":=");  }
"<="  { LIST_TOKEN("<=");  }
"<>"  { LIST_TOKEN("<>");  }
">="  { LIST_TOKEN(">=");  }
"mod" { LIST_TOKEN("mod"); }
"and" { LIST_TOKEN("and"); }
"or"  { LIST_TOKEN("or");  }
"not" { LIST_TOKEN("not"); }

    /* Reserved Words */
"var"     { LIST_TOKEN("KWvar");     }
"def"     { LIST_TOKEN("KWdef");     }
"array"   { LIST_TOKEN("KWarray");   }
"of"      { LIST_TOKEN("KWof");      }
"boolean" { LIST_TOKEN("KWboolean"); }
"integer" { LIST_TOKEN("KWinteger"); }
"real"    { LIST_TOKEN("KWreal");    }
"string"  { LIST_TOKEN("KWstring");  }
"true"    { LIST_TOKEN("KWtrue");    }
"false"   { LIST_TOKEN("KWfalse");   }
"while"   { LIST_TOKEN("KWwhile");   }
"do"      { LIST_TOKEN("KWdo");      }
"if"      { LIST_TOKEN("KWif");      }
"then"    { LIST_TOKEN("KWthen");    }
"else"    { LIST_TOKEN("KWelse");    }
"for"     { LIST_TOKEN("KWfor");     }
"to"      { LIST_TOKEN("KWto");      }
"begin"   { LIST_TOKEN("KWbegin");   }
"end"     { LIST_TOKEN("KWend");     }
"print"   { LIST_TOKEN("KWprint");   }
"read"    { LIST_TOKEN("KWread");    }
"return"  { LIST_TOKEN("KWreturn");  }

    /* Identifiers */
[A-Za-z][A-Za-z0-9]* { LIST_LITERAL("id", yytext); }

    /* Integer Constants */
0|[1-9][0-9]* { LIST_LITERAL("integer", yytext); }
0[0-7]+ { LIST_LITERAL("oct_integer", yytext); }

    /* Floating-Point Constants */
(0|[1-9][0-9]*)"."(0|[0-9]*[1-9]) { LIST_LITERAL("float", yytext); }

    /* Scientific Notations */
({nonZeroDecimal}|{nonZeroFloat})[Ee][+-]?(0|[1-9][0-9]*) { LIST_LITERAL("scientific", yytext); }

    /* String Constants */
\"([^"\n]|\"\")*\" { handleString(); }

    /* Whitespace */
[ \t]+ { LIST_SOURCE; }
\n { handleNewline(); }

    /* Pseudocomments */
"//&S+".* { opt_src = 1; LIST_SOURCE; }
"//&S-".* { opt_src = 0; LIST_SOURCE; }
"//&T+".* { opt_tok = 1; LIST_SOURCE; }
"//&T-".* { opt_tok = 0; LIST_SOURCE; }

    /* Comments */
"//".* { LIST_SOURCE; }
"/*" { BEGIN(BLOCK_COMMENT); LIST_SOURCE; }
<BLOCK_COMMENT>. { LIST_SOURCE; }
<BLOCK_COMMENT>\n { handleNewline(); }
<BLOCK_COMMENT>"*/" { BEGIN(INITIAL); LIST_SOURCE; }

    /* Bad Characters */
. { printf("Error at line %d: bad character \"%s\"\n", line_num, yytext); exit(-1); }
%%

static void appendToCurrentLine(const char *yytext_ptr) {
    while (*yytext_ptr) {
        *current_line_ptr = *yytext_ptr;
        current_line_ptr++;
        yytext_ptr++;
    }
    *current_line_ptr = '\0';
}

static void handleNewline() {
    if (opt_src) printf("%d: %s\n", line_num, current_line);
    line_num++;
    current_line[0] = '\0';
    current_line_ptr = current_line;
}

static void handleString() {
    LIST_SOURCE;
    char text[MAX_LINE_LENG] = {0};
    for (int i = 1, j = 0; i <= yyleng - 2; i++) {
        if (yytext[i] == '"') text[j++] = yytext[i++];
        else text[j++] = yytext[i];
    }
    if (opt_tok) printf("<string: %s>\n", text);
}

int main(int argc, const char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Open file error\n");
        exit(-1);
    }

    yylex();

    fclose(yyin);
    /* Free resources used by the scanner; otherwise, unused data structures
    will persist throughout the successive compiler phases. */
    yylex_destroy();

    return 0;
}