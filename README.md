libmcp23s17
===========
A simple library for accessing the MCP23S17 port expander through SPI. See
`example.c` for ideas on how to use.

Use
---
Build the library:

    $ make

This creates the library `libmcp23s17.a`. Build the example (using PiFace Digital):

    $ make example

Include the library in your project with:

    $ gcc -o example example.c -Isrc/ -L. -lmcp23s17

`-Isrc/` is for including the header file.

Todo
----
Feel free to contribute!

- Debian install
- Interrupts (using epoll?)
