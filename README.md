# Practical: Semantic Analyzer
This is the repository for the semantic analyzer library for the Practical programming language.

In order to make tooling easier, as well as allowing different back end implementations easy, the Practical compiler is split into
a semantic analyzer library, and the actual glue that generates the code. The semantic analyzer is built to be generic, allowing
implementing compilers over LLVM, GCC and Visual C backend with little or no need to port, or even adapt, the semantic analysis
part of the compiler in any way.

# What is Practical?
Practical is a new programming language designed to give the safety and features one would expect from a modern programming
language without sacrificing speed in any way. Practical aims to be as fast, if not faster, than C.

Practical takes its inspiration for design and syntax from C++, Rust and D, while trying not to fall into any of the pitfalls
that those languages encountered.

Practical is aimed for programmers in the gaming, storage, video and, eventually, embedded markets, where high throughput and low
latency are everything.

# Status
Practical is currently in preliminary the implementation effort stage. It does not, yet, have any working compiler.

# What's Planned?
For specifics on the plans for the language, syntax and features, please check out the
[language's wiki](https://github.com/Practical/practical-sa/wiki).
