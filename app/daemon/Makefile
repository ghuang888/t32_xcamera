CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CFLAGS = -O2 -Wall -march=mips32r2 -muclibc -pthread -lrt -static
STRIP = $(CROSS_COMPILE)strip
SRC = ../socket/socket_server_tcp.c xcam_daemon.c xcam_daemon_upgrade.c
PROG = xcam_daemon

all:
	$(CC) $(CFLAGS) $(SRC) -o $(PROG)
	$(STRIP) $(PROG)

.PHONY : clean
clean:
	-rm -f xcam_daemon

