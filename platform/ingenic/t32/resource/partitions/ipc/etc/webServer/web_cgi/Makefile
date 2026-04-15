CC=mips-linux-gnu-gcc
CFLAGS=-muclibc -Wall -g -O2
TARGET=webTransData.cgi
SRC=$(wildcard ./*.c)
OBJ=$(patsubst %.c,%.o,$(SRC))

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm $(OBJ) $(TARGET) -f
