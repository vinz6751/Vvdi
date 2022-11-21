Vvdi is an open source graphics library, similar to the VDI used in the operating system of the Atari ST range of computers.

It is written in C, with little 68000 assembly. The C compiler is Vincent Rivière GCC 4.6.4 port, which can be installed on Linux and Windows (cygwin or WSL).

It has the following design goals:
1. Be clear and simple to understand
2. Be ultimately usable on the Foenix Retro System 68k-based computers
3. Be somewhat performant

It has not the following design goals:
1. 100% Atari VDI/NVDI/GDOS compatibility. This is a nice to have.

The code is organized like if Vvdi was really a library, so function like e.g. vs_color() are defined according to the documentation in TOS.hyp. There is a trap interface (in trap.c) that is in charge of providing the trap interface (heh....) and adapting the VDI parameter block provided to it to the VDI functions.


Coding conventions:
I don't claim these are the best ever, but they are what they are and are not very opinionated so should be easy to adapt to.
1. Use stdint
2. Types are called type_t
3. Use stdbool for booleans
4. Avoid magic numbers
5. Test for exit conditions early then exit early rather than nesting if()s.
6. People read from top to bottom. So put the important functions at the top of the file, and the helpers or less important ones at the bottom. Make forward declarations.
7. Prefix public function names of a module (.c file) with some short word indicating the module name. I konw it's annoying but it helps new comers figure out what is where, and also for reviewers to check if things belong where they should belong. Sadly, C has no concept of namespace.
