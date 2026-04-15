all:
	gcc upgrade.c ../../socket/socket_client_tcp.c -lrt -lpthread -o upgrade
	mips-linux-gnu-gcc file_unpack.c -muclibc -lrt -o file_unpack

.PHONY : clean
clean:
	-rm -f file_unpack upgrade

