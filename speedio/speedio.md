# speedio
Basic I/O optimisation for linux &amp; windows to get maximum performance

# How to use
## io.hpp
`io.hpp` is a basic header file to include for basic abstraction into C++. It's thread-safe and lightweight.

There are 4 functions in io.hpp:
* `print_f`: if you already know the size of your string, you can call it faster with `print_f` (meaning print fast), the function abstracts OS specifics between linux and windows and runs based on system defines
* `print`: does not require a length nor conversion to `char*`, does not add much overhead
* `print_lx`: direct path to the print function for linux, written in `io.asm`
* `print_wn`: direct path to the print function for windows, written in `io.asm`

`io.hpp` doesn't need to be compiled, just included.

## io.asm
`io.asm` is where the actual outputting happens, here's what happens:
* `print_lx`: `print_lx` means "print linux", it's one of the fastest ways to output with I/O. It is not very complex compared to windows' function.
* `print_wn`: `print_wn` means "print windows", it uses externals for 2 windows call functions required to output, allocates space in `rsp` (required with windows API) and saves pointers in the 64 bytes. It then gets the windows output handle, the console pointer, string pointer, and length, then runs a syscall-like syntax, `WriteConsoleA`.

`print_wn` is not fully optimised, windows' `WriteConsoleA` still has many steps internally.

`io.asm` needs to be compiled seperately with `nasm`. A pre-compiled version is above (`io.o`) for anyone who doesn't have nasm.

## io.o
`io.o` is a pre-compiled version of `io.asm`, it's for easier compilation.

# Compilation
When using this library, you just need to include `io.o` into the folder your workspace is in and add it to your compile command. You also need to include `io.hpp` into your workspace, and don't need to add it to your compilation command.

For example (pre-library):
``` file structure
workspace /
  main.cpp
  header.h
  other.cpp
```
is paired with `g++ main.cpp other.cpp -o main.exe`

Including this library (post-library):
``` file structure
workspace /
  main.cpp
  header.h
  io.hpp
  io.o
  other.h
```
is paired with `g++ main.cpp other.cpp io.o -o main.exe`
and `main.cpp` adds `#include "io.hpp"` preferably with the other includes.

# Notes
This library is designed for `cmd.exe` and `shell`, not file I/O speed.
The main overheads are introduced in windows when transferring to files, try using `WriteFile()` instead of this library for maximum performance.

# Summary
`speedio` gives you:
* Minimal, direct, cross-platform, fast outputs
* Zero buffering or formatting overhead (possible `WriteConsoleA` overhead)
* Easy integration, drag and drop 2 files, add 1 line of code and one argument to your compilation

Use `print_f` for max performance, `print` for convenience. Use `std::cout` for more safety (and slower times).
