rm -rf build

mkdir build

cd build

cmake ..

make -j8

mips-linux-uclibc-gnu-strip out/xcamera

cp out/xcamera /home_b/nfsroot/fwang/bin/
