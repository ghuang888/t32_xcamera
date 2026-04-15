CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CFLAGS = -O2 -Wall -march=mips32r2 -muclibc
STRIP = $(CROSS_COMPILE)strip
SRC = ./mac_set.c
PROG = mac_set

all:
	$(CC) $(CFLAGS) $(SRC) -o $(PROG)
	$(STRIP) $(PROG)

.PHONY : clean
clean:
	-rm -rf mac_set
