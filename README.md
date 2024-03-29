# Refactoring Example

A C++ project for the first refactoring example of Fowler (2018).

## Features

* Based on the "theatrical players" (rather than "video rental store") exercise
  from the first chapter
  (freely available from [here](https://www.thoughtworks.com/books/refactoring2))
  of Fowler (2018)
* C++20 (rather than JavaScript) with some tests
  (using [GoogleTest](https://github.com/google/googletest))
* Two branches: `master` (the starting point before refactoring) vs.
  `refactored` (the fully refactored state)
  with some [refactoring memo](doc/memo.md) describing the key refactoring steps

## How to configure, build, and test

```bash
$ cmake --preset ninja -D CMAKE_BUILD_TYPE=Debug
$ cmake --build build
$ ctest --test-dir build --output-on-failure
```

## Reference(s)
* Fowler, M. (2018). _Refactoring: Improving the Design of Existing Code_ (2nd ed.). Addison-Wesley.
