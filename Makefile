DEBUG=yes

# libcmini is a lightweight replacement of the MiNTlib, which produces much smaller executables.
# See https://github.com/freemint/libcmini
LIBCMINI=/cygdrive/c/Atari/Crossdev/libcmini

# GGC-based toolchain for building, this uses the GCC 4.6.4 from Vincent Rivière
# See http://vincent.riviere.free.fr/soft/m68k-atari-mint/
TOOLCHAIN=m68k-atari-mintelf-
CC=$(TOOLCHAIN)gcc
AS=$(TOOLCHAIN)as
STRIP=$(TOOLCHAIN)strip
ASSFLAGS=
CFLAGS=-I$(LIBCMINI)/include -DLIBCMINI -fomit-frame-pointer -Os -std=c99 -Wa,--register-prefix-optional
LDFLAGS=-L$(LIBCMINI)/build -lcmini -lgcc

# Put the path of the emulated hard drive of the Atari ST here
EXEC=C:\Atari\Disques\C_ST_TOS\vvdi.prg

# conic.c is not included yet because it doesn't compile, it's not fully adapted yet to work with
# vvdi as it relies on a Fgfb structure representing a colour that I don't understand why it even exists
SRC_C=assignsys.c attribute.c debug.c fill_patterns.c line.c math.c memory.c \
	shifter.c trap.c vdi.c vicky.c font.c text.c raster.c utils.c \
	shifter_raster.c workstation.c \
	test.c
SRC_S= trap_asm.S math_asm.S linea_asm.S
SRC=$(SRC_C) $(SRC_S)
OBJ_C=$(SRC_C:.c=.o)
OBJ_S=$(SRC_S:.S=.o)
OBJ=$(OBJ_C) $(OBJ_S)

$(EXEC): $(OBJ)
	$(CC) -o $@ -nostdlib $(LIBCMINI)/build/objs/crt0.o $^ -s $(LDFLAGS)
	$(STRIP) $@

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.S
	$(CC) -o $@ -c $< $(CFLAGS)


.PHONY: clean mrproper

clean:
	@rm -rf *.o *~

mrproper: clean
	@rm -rf $(EXEC)
