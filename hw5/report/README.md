# hw5 report

|Field|Value|
|-:|:-|
|Name|林嘉軒|
|ID|312551014|

## How much time did you spend on this project

3 days.

## Project overview

This project is about implementing the RISC-V assembly code generator for a program written in the P language.

The main steps involved are:
1.  Modify the symbol table for storing local variable offsets
2.  Generate the `.bss` and `.data` sections for global variables
3.  Generate the corresponding assembly using the stack machine model

### The Conditional Branches

The approach I implement for the conditional branches is as follows:

```c++
void CodeGenerator::visit(IfNode &p_if) {
    int l1 = m_symbol_manager.getNewLabel();
    int l2 = m_symbol_manager.getNewLabel();
    int l3 = m_symbol_manager.getNewLabel();

    const_cast<ExpressionNode &>(p_if.getCondition()).accept(*this);
    dumpInstructions(m_output_file.get(), "L%d\nL%d:\n", l2, l1);
    const_cast<CompoundStatementNode &>(p_if.getBody()).accept(*this);
    dumpInstructions(m_output_file.get(), "    j L%d\nL%d:\n", l3, l2);
    p_if.visitElseBodyChildNodes(*this);
    dumpInstructions(m_output_file.get(), "L%d:\n", l3);
}
```

### The ReturnNode Bug

There's a bug (maybe more in the other places) in the `ReturnNode` that I have no time to solve.
I only saved the function result into the register `a0` but did not jump to the function epilogue.
So that when leaving a function with the return statement which not at the end of the function will cause an error.
A solution is to generate a label at the end of the function and jump to that label when a return statement is encountered.

```c++
void CodeGenerator::visit(ReturnNode &p_return) {
    p_return.visitChildNodes(*this);
    dumpInstructions(m_output_file.get(),
                     "    lw t0, 0(sp)     # pop the value from the stack\n"
                     "    addi sp, sp, 4\n"
                     "    mv a0, t0        # load the value to ret register\n");
}
```

## What is the hardest you think in this project

Conditional branches, for loops, anything that uses labels.

## Feedback to T.A.s

I hope the hidden test cases will not be more difficult than the public ones.
