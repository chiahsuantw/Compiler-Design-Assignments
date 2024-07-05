# hw2 report

|Field|Value|
|-:|:-|
|Name|林嘉軒|
|ID|312551014|

## How much time did you spend on this project

3 hours.

## Project overview

This project is about creating a parser for the P language. The main steps involved are (1) returning tokens from the scanner and (2) defining the grammar of the P language within the parser. The project specification provides a clear definition of the grammar, which I can readily follow to construct the parser. Here are some tips I used for converting EBNF to BNF.

* Convert `{A}` to a fresh non-terminal `X` and define `X := ε|XA`.
* Convert `[A]` to a fresh non-terminal `X` and define `X := ε|A`.

It's quite surprising that no shift/reduce or reduce/reduce conflicts were encountered. In my previous experiences building compilers, they typically arose frequently.

## What is the hardest you think in this project

To come up with good token names.

## Feedback to T.A.s

None.
