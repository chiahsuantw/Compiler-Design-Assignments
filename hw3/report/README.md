# hw3 report

|Field|Value|
|-:|:-|
|Name|林嘉軒|
|ID|312551014|

## How much time did you spend on this project

1 week.

## Project overview

This project is about extending the parser from the previous assignment to construct an abstract syntax tree (AST) for a given program written in P language. The main steps involved are:

1. Returning yylval from the scanner.
2. Designing and constructing the AST in C++.
3. **Using the visitor pattern** to dump the AST in a designated format.

### How visitor pattern works

The visitor pattern is a design pattern used in object-oriented programming to separate algorithms from the objects on which they operate. It allows for adding new operations to existing object structures without modifying those structures. The pattern involves defining a visitor interface with methods corresponding to each object type in the structure. Each object in the structure then accepts a visitor, which invokes the appropriate method based on the object's type. This enables flexible and extensible behavior while encapsulating related operations within the visitor objects.

The root node calls the `accept` method, and the vistor will call the corresponding `visit` method to print the AST information of this node and also visit its child nodes by invoking the `visitChildNodes` method.

## What is the hardest you think in this project

The first two testcases.

## Feedback to T.A.s

Provide a `.clang-format` file for automatic formatting the codebase.
