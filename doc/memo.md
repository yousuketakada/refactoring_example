## Introduction

The first chapter of Fowler (2018) gives a concrete example of
how we refactor code with anti-patterns or "bad smells."
Although the example is in JavaScript,
the concepts of refactoring are language agnostic so that
basically we can employ any other (dynamically- or statically-typed) language equally well;
see, e.g.,

* https://github.com/ryo-utsunomiya/refactoring-2nd-ts
  for TypeScript
* https://github.com/emilybache/Theatrical-Players-Refactoring-Kata
  for a handful of other languages including C++, C#, Go, Java, Python, etc.

This repo is yet another example in C++20,
where the `master` branch corresponds to "the starting point" and
`refactored` to the fully refactored state.
As demonstrated in the text,
in order to minimize the risks of refactoring
(introducing subtle bugs, breaking the code for a long time, etc.),
I have tried to refactor with the "one small step at a time" principle in mind
and made as frequent commits as possible (without squashing).

## The starting point

The source files relevant to our refactoring are:

* `statement_test.cpp`:
  The source file containing a simple test case (`BigCo`) in which
  we input some `plays` and `invoice` to `statement()`;
  and compare the output with `expected_text` for equality.
* `statement.h`:
  The header file where we define types like `Play`, `Performance`, and `Invoice`;
  and declare `statement()`.
* `statement.cpp`: The source file where we define `statement()`.

TODO

## Extract functions

TODO

## Split phase

TODO

## Replace conditionals with polymorphism

TODO

## Discussions

TODO
