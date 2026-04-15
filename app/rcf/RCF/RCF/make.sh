echo "compiler wait a little time"
mips-linux-gnu-g++ -muclibc -O2 RCF.cpp -c -I ../include/ -ldl
mips-linux-gnu-ar rcs libRCF.a RCF.o
