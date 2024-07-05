%{
#include "AST/BinaryOperator.hpp"
#include "AST/CompoundStatement.hpp"
#include "AST/ConstantValue.hpp"
#include "AST/FunctionInvocation.hpp"
#include "AST/UnaryOperator.hpp"
#include "AST/VariableReference.hpp"
#include "AST/assignment.hpp"
#include "AST/ast.hpp"
#include "AST/decl.hpp"
#include "AST/expression.hpp"
#include "AST/for.hpp"
#include "AST/function.hpp"
#include "AST/if.hpp"
#include "AST/print.hpp"
#include "AST/program.hpp"
#include "AST/read.hpp"
#include "AST/return.hpp"
#include "AST/variable.hpp"
#include "AST/while.hpp"
#include "AST/AstDumper.hpp"
#include "AST/operator.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define YYLTYPE yyltype

typedef struct YYLTYPE {
    uint32_t first_line;
    uint32_t first_column;
    uint32_t last_line;
    uint32_t last_column;
} yyltype;

extern uint32_t line_num;   /* declared in scanner.l */
extern char current_line[]; /* declared in scanner.l */
extern FILE *yyin;          /* declared by lex */
extern char *yytext;        /* declared by lex */

static AstNode *root;

extern "C" int yylex(void);
static void yyerror(const char *msg);
extern int yylex_destroy(void);
%}

// This guarantees that headers do not conflict when included together.
%define api.token.prefix {TOK_}

%code requires {
    #include <vector>
    #include <memory>

    class AstNode;
    class CompoundStatementNode;
    class ConstantValueNode;
    class DeclNode;
    class FunctionNode;
    class IdNode;
    class Type;
    class ExpressionNode;
    class AssignmentNode;
}

    /* For yylval */
%union {
    /* basic semantic value */
    char *identifier;
    int integer;
    double real;
    char *string;
    bool boolean;

    AstNode *node;
    Type *type;
    DeclNode *decl;
    FunctionNode *func;
    CompoundStatementNode *comp_stmt;
    ConstantValueNode *const_val;
    ExpressionNode *expr;

    std::vector<std::unique_ptr<AstNode>> *nodes;
    std::vector<std::unique_ptr<DeclNode>> *decls;
    std::vector<std::unique_ptr<FunctionNode>> *funcs;
    std::vector<std::unique_ptr<ExpressionNode>> *exprs;
    std::vector<IdNode> *ids;
    std::vector<int> *dims;
};

%type <identifier> ProgramName ID FunctionName
%type <integer> INT_LITERAL NegOrNot
%type <real> REAL_LITERAL
%type <string> STRING_LITERAL
%type <boolean> TRUE FALSE

%type <node> Statement Simple Condition While For Return FunctionCall
%type <decl> Declaration FormalArg
%type <func> Function FunctionDeclaration FunctionDefinition
%type <comp_stmt> CompoundStatement ElseOrNot
%type <decls> DeclarationList Declarations FormalArgList FormalArgs
%type <funcs> FunctionList Functions
%type <type> Type ScalarType ArrType ReturnType
%type <ids> IdList
%type <const_val> LiteralConstant StringAndBoolean
%type <expr> Expression IntegerAndReal VariableReference FunctionInvocation
%type <dims> ArrDecl
%type <nodes> StatementList Statements
%type <exprs> ExpressionList Expressions ArrRefList ArrRefs

    /* Follow the order in scanner.l */

    /* Delimiter */
%token COMMA SEMICOLON COLON
%token L_PARENTHESIS R_PARENTHESIS
%token L_BRACKET R_BRACKET

    /* Operator */
%token ASSIGN
%left  OR AND
%right NOT
%left  LESS LESS_OR_EQUAL EQUAL GREATER GREATER_OR_EQUAL NOT_EQUAL
%left  PLUS MINUS
%left  MULTIPLY DIVIDE MOD
%right UNARY_MINUS

    /* Keyword */
%token ARRAY BOOLEAN INTEGER REAL STRING
%token END BEGIN
%token DO ELSE FOR IF THEN WHILE
%token DEF OF TO RETURN VAR
%token FALSE TRUE
%token PRINT READ

    /* Identifier */
%token ID

    /* Literal */
%token INT_LITERAL
%token REAL_LITERAL
%token STRING_LITERAL

%%

ProgramUnit:
    Program
    |
    Function
;

Program:
    ProgramName SEMICOLON
    DeclarationList FunctionList CompoundStatement
    END { root = new ProgramNode(@1.first_line, @1.first_column, $1, new Type(Type::TypeEnum::VOID), *$3, *$4, $5); }
;

ProgramName:
    ID
;

DeclarationList:
    Epsilon { $$ = new std::vector<std::unique_ptr<DeclNode>>(); }
    |
    Declarations
;

Declarations:
    Declaration {
        $$ = new std::vector<std::unique_ptr<DeclNode>>();
        $$->emplace_back($1);
    }
    |
    Declarations Declaration {
        $1->emplace_back($2);
        $$ = $1;
    }
;

FunctionList:
    Epsilon { $$ = new std::vector<std::unique_ptr<FunctionNode>>(); }
    |
    Functions
;

Functions:
    Function {
        $$ = new std::vector<std::unique_ptr<FunctionNode>>();
        $$->emplace_back($1);
    }
    |
    Functions Function {
        $1->emplace_back($2);
        $$ = $1;
    }
;

Function:
    FunctionDeclaration
    |
    FunctionDefinition
;

FunctionDeclaration:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType SEMICOLON
    { $$ = new FunctionNode(@1.first_line, @1.first_column, $1, *$3, $5, nullptr); }
;

FunctionDefinition:
    FunctionName L_PARENTHESIS FormalArgList R_PARENTHESIS ReturnType
    CompoundStatement
    END { $$ = new FunctionNode(@1.first_line, @1.first_column, $1, *$3, $5, $6); }
;

FunctionName:
    ID
;

FormalArgList:
    Epsilon { $$ = new std::vector<std::unique_ptr<DeclNode>>(); }
    |
    FormalArgs
;

FormalArgs:
    FormalArg {
        $$ = new std::vector<std::unique_ptr<DeclNode>>();
        $$->emplace_back($1);
    }
    |
    FormalArgs SEMICOLON FormalArg {
        $1->emplace_back($3);
        $$ = $1;
    }
;

FormalArg:
    IdList COLON Type { $$ = new DeclNode(@1.first_line, @1.first_column, $1, $3); }
;

IdList:
    ID {
        $$ = new std::vector<IdNode>();
        $$->emplace_back(@1.first_line, @1.first_column, $1);
    }
    |
    IdList COMMA ID {
        $1->emplace_back(@3.first_line, @3.first_column, $3);
        $$ = $1;
    }
;

ReturnType:
    COLON ScalarType { $$ = $2; }
    |
    Epsilon { $$ = new Type(Type::TypeEnum::VOID); }
;

    /*
       Data Types and Declarations
                                   */

Declaration:
    VAR IdList COLON Type SEMICOLON { $$ = new DeclNode(@1.first_line, @1.first_column, $2, $4); }
    |
    VAR IdList COLON LiteralConstant SEMICOLON { $$ = new DeclNode(@1.first_line, @1.first_column, $2, $4); }
;

Type:
    ScalarType
    |
    ArrType
;

ScalarType:
    INTEGER { $$ = new Type(Type::TypeEnum::INTEGER); }
    |
    REAL { $$ = new Type(Type::TypeEnum::REAL); }
    |
    STRING { $$ = new Type(Type::TypeEnum::STRING); }
    |
    BOOLEAN { $$ = new Type(Type::TypeEnum::BOOLEAN); }
;

ArrType:
    ArrDecl ScalarType {
        $2->setDimensions(*$1);
        $$ = $2;
    }
;

ArrDecl:
    ARRAY INT_LITERAL OF { $$ = new std::vector<int>{$2}; }
    |
    ArrDecl ARRAY INT_LITERAL OF {
        $1->emplace_back($3);
        $$ = $1;
    }
;

LiteralConstant:
    NegOrNot INT_LITERAL {
        Constant::ConstantValue value;
        value.integer = $1 * $2;
        auto *location = $1 == 1 ? &@2 : &@1;
        $$ = new ConstantValueNode(location->first_line, location->first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::INTEGER)), value));
    }
    |
    NegOrNot REAL_LITERAL {
        Constant::ConstantValue value;
        value.real = $1 * $2;
        auto *location = $1 == 1 ? &@2 : &@1;
        $$ = new ConstantValueNode(location->first_line, location->first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::REAL)), value));
    }
    |
    StringAndBoolean
;

NegOrNot:
    Epsilon { $$ = 1; }
    |
    MINUS %prec UNARY_MINUS { $$ = -1; }
;

StringAndBoolean:
    STRING_LITERAL {
        Constant::ConstantValue value;
        value.string = $1;
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::STRING)), value));
    }
    |
    TRUE {
        Constant::ConstantValue value;
        value.boolean = $1;
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::BOOLEAN)), value));
    }
    |
    FALSE {
        Constant::ConstantValue value;
        value.boolean = $1;
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::BOOLEAN)), value));
    }
;

IntegerAndReal:
    INT_LITERAL {
        Constant::ConstantValue value;
        value.integer = $1;
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::INTEGER)), value));
    }
    |
    REAL_LITERAL {
        Constant::ConstantValue value;
        value.real = $1;
        $$ = new ConstantValueNode(@1.first_line, @1.first_column,
            new Constant(std::make_shared<Type>(Type(Type::TypeEnum::REAL)), value));
    }
;

    /*
       Statements
                  */

Statement:
    CompoundStatement { $$ = static_cast<AstNode *>($1); }
    |
    Simple
    |
    Condition
    |
    While
    |
    For
    |
    Return
    |
    FunctionCall
;

CompoundStatement:
    BEGIN
    DeclarationList
    StatementList
    END { $$ = new CompoundStatementNode(@1.first_line, @1.first_column, *$2, *$3); }
;

Simple:
    VariableReference ASSIGN Expression SEMICOLON { $$ = new AssignmentNode(@2.first_line, @2.first_column, dynamic_cast<VariableReferenceNode *>($1), $3); }
    |
    PRINT Expression SEMICOLON { $$ = new PrintNode(@1.first_line, @1.first_column, $2); }
    |
    READ VariableReference SEMICOLON { $$ = new ReadNode(@1.first_line, @1.first_column, dynamic_cast<VariableReferenceNode *>($2)); }
;

VariableReference:
    ID ArrRefList { $$ = new VariableReferenceNode(@1.first_line, @1.first_column, $1, *$2); }
;

ArrRefList:
    Epsilon { $$ = new std::vector<std::unique_ptr<ExpressionNode>>(); }
    |
    ArrRefs
;

ArrRefs:
    L_BRACKET Expression R_BRACKET {
        $$ = new std::vector<std::unique_ptr<ExpressionNode>>();
        $$->emplace_back($2);
    }
    |
    ArrRefs L_BRACKET Expression R_BRACKET {
        $1->emplace_back($3);
        $$ = $1;
    }
;

Condition:
    IF Expression THEN
    CompoundStatement
    ElseOrNot
    END IF { $$ = new IfNode(@1.first_line, @1.first_column, $2, $4, $5); }
;

ElseOrNot:
    ELSE
    CompoundStatement { $$ = $2; }
    |
    Epsilon { $$ = nullptr; }
;

While:
    WHILE Expression DO
    CompoundStatement
    END DO { $$ = new WhileNode(@1.first_line, @1.first_column, $2, $4); }
;

For:
    FOR ID ASSIGN INT_LITERAL TO INT_LITERAL DO
    CompoundStatement
    END DO {
        Constant::ConstantValue value;

        auto *decl = new DeclNode(@2.first_line, @2.first_column,
            new std::vector<IdNode>{IdNode(@2.first_line, @2.first_column, $2)},
            new Type(Type::TypeEnum::INTEGER));

        value.integer = $4;
        auto *asgmt = new AssignmentNode(@3.first_line, @3.first_column,
            new VariableReferenceNode(@2.first_line, @2.first_column, $2),
            new ConstantValueNode(@4.first_line, @4.first_column,
                new Constant(std::make_shared<Type>(Type(Type::TypeEnum::INTEGER)), value)));

        value.integer = $6;
        $$ = new ForNode(@1.first_line, @1.first_column, decl, asgmt,
            new ConstantValueNode(@6.first_line, @6.first_column,
                new Constant(std::make_shared<Type>(Type(Type::TypeEnum::INTEGER)), value)), $8);
    }
;

Return:
    RETURN Expression SEMICOLON { $$ = new ReturnNode(@1.first_line, @1.first_column, $2); }
;

FunctionCall:
    FunctionInvocation SEMICOLON
;

FunctionInvocation:
    ID L_PARENTHESIS ExpressionList R_PARENTHESIS { $$ = new FunctionInvocationNode(@1.first_line, @1.first_column, $1, *$3); }
;

ExpressionList:
    Epsilon { $$ = new std::vector<std::unique_ptr<ExpressionNode>>(); }
    |
    Expressions
;

Expressions:
    Expression {
        $$ = new std::vector<std::unique_ptr<ExpressionNode>>();
        $$->emplace_back($1);
    }
    |
    Expressions COMMA Expression {
        $1->emplace_back($3);
        $$ = $1;
    }
;

StatementList:
    Epsilon { $$ = new std::vector<std::unique_ptr<AstNode>>(); }
    |
    Statements
;

Statements:
    Statement {
        $$ = new std::vector<std::unique_ptr<AstNode>>();
        $$->emplace_back($1);
    }
    |
    Statements Statement {
        $1->emplace_back($2);
        $$ = $1;
    }
;

Expression:
    L_PARENTHESIS Expression R_PARENTHESIS { $$ = $2; }
    |
    MINUS Expression %prec UNARY_MINUS { $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, Operator::UNARY_MINUS, $2); }
    |
    Expression MULTIPLY Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::MULTIPLY, $1, $3); }
    |
    Expression DIVIDE Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::DIVIDE, $1, $3); }
    |
    Expression MOD Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::MOD, $1, $3); }
    |
    Expression PLUS Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::PLUS, $1, $3); }
    |
    Expression MINUS Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::MINUS, $1, $3); }
    |
    Expression LESS Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::LESS, $1, $3); }
    |
    Expression LESS_OR_EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::LESS_OR_EQUAL, $1, $3); }
    |
    Expression GREATER Expression  { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::GREATER, $1, $3); }
    |
    Expression GREATER_OR_EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::GREATER_OR_EQUAL, $1, $3); }
    |
    Expression EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::EQUAL, $1, $3); }
    |
    Expression NOT_EQUAL Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::NOT_EQUAL, $1, $3); }
    |
    NOT Expression { $$ = new UnaryOperatorNode(@1.first_line, @1.first_column, Operator::NOT, $2); }
    |
    Expression AND Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::AND, $1, $3); }
    |
    Expression OR Expression { $$ = new BinaryOperatorNode(@2.first_line, @2.first_column, Operator::OR, $1, $3); }
    |
    IntegerAndReal
    |
    StringAndBoolean
    |
    VariableReference
    |
    FunctionInvocation
;

    /*
       misc
            */
Epsilon:
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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename> [--dump-ast]\n", argv[0]);
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        perror("fopen() failed");
        exit(-1);
    }

    yyparse();

    if (argc >= 3 && strcmp(argv[2], "--dump-ast") == 0) {
        AstDumper dumper;
        root->accept(dumper);
    }

    printf("\n"
           "|--------------------------------|\n"
           "|  There is no syntactic error!  |\n"
           "|--------------------------------|\n");

    delete root;
    fclose(yyin);
    yylex_destroy();
    return 0;
}
