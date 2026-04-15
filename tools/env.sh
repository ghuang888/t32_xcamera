if [ -f tools/env.sh ]; then
	echo "Set cross complie path"
    export PATH=${PWD}/platform/ingenic/toolchain/mips-gcc472-glibc226/bin:$PATH
else
    echo "Please run source at xcamera_device"
fi
