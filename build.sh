#!/bin/sh
m68k-atari-mint-gcc vdi.c vdi_S.s vicky.c linea.S debug.c -o vdi.tos -std=c99
