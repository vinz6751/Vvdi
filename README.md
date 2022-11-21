# Vvdi
Atari TOS-compatible Virtual Device Interface for the Foenix 68k computer
The [Virtual Device Interface](https://freemint.github.io/tos.hyp/en/vdi_main.html) is the graphics librabry that is part of the Graphics Environment Manager, and is used a foundation for drawing windows, boxes, text etc. in a desktop environment.
It does not aim to support the Line A (which is a hack), I would like it to be a rather "clean, simple to understand, by the book" implementation.

It's very, very early and not really functional yet..

To compile, you need Vincent Rivière's Atari cross-MiNT [GCC](http://vincent.riviere.free.fr/soft/m68k-atari-mint/) and the [libcmini](https://github.com/freemint/libcmini)

Just run "make" and it will build vvdi.prg. This includes both the VDI and a test program (in test.c), that you can run on an emulated Atari ST. It will just install the VDI on the trap #2, then run some functions, then uninstall and exit.

Display drivers:
There are 2 drivers:
* shifter.c, which is the driver for the Atari ST frame buffer. That's the one I use for development of the VDI because it's easy to run an Atari ST emulator, e.g. STEEM SSE or hatari.
* vicky.c, which is the driver for VICKY II. This is probably not functional yet.
The interface the drivers must implement is vdi_driver_t. It's not frozen yet, I add stuff to it as a go along.

The file Status.ods is a LibreOffice spreadsheet the implementation status of mandatory VDI functions.
The reference information for implementing this VDI is [tos.hyp](https://freemint.github.io/tos.hyp/en/vdi_main.html)

It produces a vdi.tos file that can be executed on the A2560U running [EmuTOS](https://github.com/vinz6751/genxtos).
