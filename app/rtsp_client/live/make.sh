make distclean
./genMakefiles mips-linux-gnu-uclibc
make V=1 -j32
make install
