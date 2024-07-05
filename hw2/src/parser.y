%{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t line_num;    /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

extern int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

%token COMMA SEMI COLON
%token LPAREN RPAREN
%token LBRA RBRA

%token ASSIGN
%left  AND OR
%right NOT
%left  LT LE NEQ GE GT EQ
%left  SUB
%left  ADD
%left  DIV MOD
%left  MUL
%right MINUS

%token KWvar KWarray KWof KWboolean KWinteger KWreal KWstring
%token KWtrue KWfalse
%token KWdef KWreturn
%token KWbegin KWend
%token KWwhile KWdo
%token KWif KWthen KWelse
%token KWfor KWto
%token KWprint KWread

%token ID
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%
/* Program Units */
Program
    : ID SEMI DeclarationList FunctionList CompoundStatement KWend
    ;
DeclarationList
    :
    | DeclarationList Declaration
    ;
FunctionList
    :
    | FunctionList Function
    ;
Function
    : FunctionDeclaration
    | FunctionDefinition
    ;
FunctionDeclaration
    : FunctionHeader SEMI
    ;
FunctionDefinition
    : FunctionHeader CompoundStatement KWend
    ;
FunctionHeader
    : ID LPAREN FormalArgumentList RPAREN COLON ScalarType
    | ID LPAREN FormalArgumentList RPAREN
    ;
FormalArgumentList
    :
    | FormalArguments
    ;
FormalArguments
    : FormalArgument
    | FormalArguments SEMI FormalArgument
    ;
FormalArgument
    : IdentifierList COLON Type
    ;

/* Declarations */
Declaration
    : VariableDeclaration
    | ConstantDeclaration
    ;
VariableDeclaration
    : KWvar IdentifierList COLON Type SEMI
    ;
ConstantDeclaration
    : KWvar IdentifierList COLON Sign IntegerLiteral SEMI
    | KWvar IdentifierList COLON Sign RealLiteral SEMI
    | KWvar IdentifierList COLON StringLiteral SEMI
    | KWvar IdentifierList COLON BooleanLiteral SEMI
    ;
Sign
    : 
    | SUB
    ;
IdentifierList
    : ID
    | IdentifierList COMMA ID
    ;

/* Types */
Type
    : ScalarType
    | ArrayType
    ;
ScalarType
    : KWinteger
    | KWreal
    | KWstring
    | KWboolean
    ;
ArrayType
    : KWarray IntegerLiteral KWof Type
    ;

/* Statements */
Statement
    : SimpleStatement
    | ConditionalStatement
    | FunctionCallStatement
    | LoopStatement
    | ReturnStatement
    | CompoundStatement
    ;
SimpleStatement
    : VariableReference ASSIGN Expression SEMI
    | KWprint Expression SEMI
    | KWread VariableReference SEMI
    ;
ConditionalStatement
    : KWif Expression KWthen CompoundStatement KWelse CompoundStatement KWend KWif
    | KWif Expression KWthen CompoundStatement KWend KWif
    ;
FunctionCallStatement
    : FunctionCall SEMI
    ;
LoopStatement
    : KWwhile Expression KWdo CompoundStatement KWend KWdo
    | KWfor ID ASSIGN IntegerLiteral KWto IntegerLiteral KWdo CompoundStatement KWend KWdo
    ;
ReturnStatement
    : KWreturn Expression SEMI
    ;
CompoundStatement
    : KWbegin DeclarationList StatementList KWend
    ;
StatementList
    :
    | StatementList Statement
    ;

/* Expressions */
Expression
    : LiteralConstant
    | VariableReference
    | FunctionCall
    | BinaryOperation
    | UnaryOperation
    | LPAREN Expression RPAREN
    ;
LiteralConstant
    : IntegerLiteral
    | RealLiteral
    | StringLiteral
    | BooleanLiteral
    ;
IntegerLiteral
    : INT_LITERAL
    ;
RealLiteral
    : REAL_LITERAL
    ;
StringLiteral
    : STRING_LITERAL
    ;
BooleanLiteral
    : KWtrue
    | KWfalse
    ;
VariableReference
    : ID ArrayDimensions
    ;
ArrayDimensions
    : 
    | ArrayDimensions LBRA Expression RBRA
    ;
FunctionCall
    : ID LPAREN ExpressionList RPAREN
    ;
ExpressionList
    : 
    | Expressions
    ;
Expressions
    : Expression
    | Expressions COMMA Expression
    ;
BinaryOperation
    : Expression ADD Expression
    | Expression SUB Expression
    | Expression MUL Expression
    | Expression DIV Expression
    | Expression MOD Expression
    | Expression LT  Expression
    | Expression LE  Expression
    | Expression NEQ Expression
    | Expression GE  Expression
    | Expression GT  Expression
    | Expression EQ  Expression
    | Expression AND Expression
    | Expression OR  Expression
    ;
UnaryOperation
    : SUB Expression %prec MINUS
    | NOT Expression
    ;
%%

void yyerror(const char *msg) {
    fprintf(stderr,
            "\n"
            "|-----------------------------------------------------------------"
            "---------\n"
            "| Error found in Line #%d: %s\n"
            "|\n"
            "| Unmatched token: %s\n"
            "|-----------------------------------------------------------------"
            "---------\n",
            line_num, current_line, yytext);
    exit(-1);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    fclose(yyin);
    yylex_destroy();

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");
    return 0;
}
