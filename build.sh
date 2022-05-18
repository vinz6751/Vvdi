#!/bin/sh
m68k-atari-mint-gcc vdi.c trap_S.s vicky.c shifter.c linea.S debug.c fill_patterns.c utils_S.s line.c test.c trap.c -o vdi.tos -std=c99 -fomit-frame-pointer -O2

