DEBUG=yes
LIBCMINI=/home/vincent/Atari/Crossdev/libcmini
TOOLCHAIN=m68k-atari-mint-
CC=$(TOOLCHAIN)gcc
AS=$(TOOLCHAIN)as
STRIP=$(TOOLCHAIN)strip
CFLAGS=-I$(LIBCMINI)/include -DLIBCMINI -fomit-frame-pointer -Os -std=c99
LDFLAGS=-L$(LIBCMINI)/build -lcmini -lgcc

EXEC=C:\Atari\Disques\C_ST_TOS\vvdi.prg

SRC_C=debug.c fill_patterns.c line.c shifter.c test.c trap.c vdi.c vicky.c
SRC_S=linea.S trap_S.s utils_S.s
SRC=$(SRC_C) $(SRC_S)
OBJ_C=$(SRC_C:.c=.o)
OBJ_S=$(SRC_S:.s=.o)
OBJ=$(OBJ_C) $(OBJ_S)

# all: $(EXEC)
# ifeq ($(DEBUG),yes)
#     @echo "Génération en mode debug"
# else
#     @echo "Génération en mode release"
# endif

$(EXEC): $(OBJ)
	$(CC) -o $@ -nostdlib $(LIBCMINI)/build/crt0.o $^ -s $(LDFLAGS)   
	$(STRIP) $@

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.s
	$(CC) -o $@ -c $< $(CFLAGS)


.PHONY: clean mrproper

clean:
	@rm -rf *.o *~

mrproper: clean
	@rm -rf $(EXEC)
