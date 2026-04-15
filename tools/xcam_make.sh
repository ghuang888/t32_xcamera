#!/bin/bash


while read line; do
    name=`echo $line | awk -F '=' '{print $1}'`
    value=`echo $line | awk -F '=' '{print $2}'`
    case $name in
        "march")
            march=$value
            if [ ${march} == "t31" ];then
                kernel_config=isvp_swan_defconfig
                echo "SET(CONFIG_DPLATFORM \"T31\")" > ../app/Config.cmake
                echo "SET(CONFIG_ENCODE \"H265\")" >> ../app/Config.cmake
            elif [ ${march} == "t21" ];then
                kernel_config=isvp_turkey_defconfig
                echo "SET(CONFIG_DPLATFORM \"T21\")" > ../app/Config.cmake
                echo "SET(CONFIG_ENCODE \"H264\")" >> ../app/Config.cmake
            elif [ ${march} == "t40" ];then
                kernel_config=isvp_shark_defconfig
                echo "SET(CONFIG_DPLATFORM \"T40\")" > ../app/Config.cmake
                echo "SET(CONFIG_ENCODE \"H265\")" >> ../app/Config.cmake
            elif [ ${march} == "t41" ];then
                kernel_config=isvp_marmot_defconfig
                echo "SET(CONFIG_DPLATFORM \"T41\")" > ../app/Config.cmake
                echo "SET(CONFIG_ENCODE \"H265\")" >> ../app/Config.cmake
            else
                echo "error: unsupport chip!!!"
            fi
            ;;
        "model")
            model=$value
            ;;
        "libc")
            libc=$value
            ;;
        "toolchain")
            toolchain=$value
            ;;
        "sensor")
            sensor=$value
            ;;
        "motor_enable")
            motor_enable=$value
            ;;
        "dl_enable")
			dl_enable=$value
            if [ ${dl_enable} == "1" ];then
                echo "SET(CONFIG_DL_ENABLE \"DL_ENABLE\")" >> ../app/Config.cmake
            elif [ ${dl_enable} == "0" ];then
                echo "SET(CONFIG_DL_ENABLE \"DL_UNABLE\")" >> ../app/Config.cmake
			else
                echo "dl_enable error!!!"
            fi
            ;;
        "GB_VERSION")
            GB_VERSION=$value
            echo "SET(CONFIG_GB_VERSION \"GB28181\")" >> ../app/Config.cmake
            ;;
        "XCAM_VER_MAJOR")
            XCAM_VER_MAJOR=$value
            ;;
        "XCAM_VER_MINOR")
            XCAM_VER_MINOR=$value
            echo "SET(XCAM_VER \"XCAM_VER_MAJOR=${XCAM_VER_MAJOR}\" \"XCAM_VER_MINOR=${XCAM_VER_MINOR}\")" >> ../app/Config.cmake
            ;;
        "ONVIF_VER_MAJOR")
            ONVIF_VER_MAJOR=$value
            ;;
        "ONVIF_VER_MINOR")
            ONVIF_VER_MINOR=$value
            echo "SET(ONVIF_VER \"ONVIF_VER_MAJOR=${ONVIF_VER_MAJOR}\" \"ONVIF_VER_MINOR=${ONVIF_VER_MINOR}\")" >> ../app/Config.cmake
            ;;
        *)
            ;;
    esac
done < xcam_config.txt

ROOTFS_TOOLCHAIN=toolchain7.2.0
TOOLCHAIN472=/opt/mips-gcc472-glibc216-64bit/bin
TOOLCHAIN540=/opt/mips-gcc540-glibc222-64bit-r3.3.0/bin
TOOLCHAIN720=/opt/mips-gcc720-glibc226/bin
TOOLCHAIN720_r514=/opt/mips-gcc720-glibc229-r5.1.4/bin

XCAM_PATH=$(pwd)/../
UBOOT_PATH=$(pwd)/../platform/ingenic/${march}/uboot
KERNEL_T41_PATH=$(pwd)/../platform/ingenic/${march}/kernel-4.4.94
DRIVERS_PATH=$(pwd)/../platform/ingenic/${march}/resource/drivers
ISP_PATH=$(pwd)/../platform/ingenic/${march}/resource/drivers/isp-${march}/tx-isp-${march}
SENSOR_PATH=$(pwd)/../platform/ingenic/${march}/resource/drivers/sensors-t41
AVPU_PATH=$(pwd)/../platform/ingenic/${march}/resource/drivers/avpu
SENSOR_INFO_PATH=$(pwd)/../platform/ingenic/${march}/resource/drivers/sensor_info
APP_PATH=$(pwd)/../app
XCAM_RESOURCE=$(pwd)/../_out
SETTINGS_PATH=$(pwd)/../platform/ingenic/${march}/resource/sensor_settings
ROOTFS_PATH=$(pwd)/../platform/ingenic/${march}/resource/partitions/rootfs
CGI_PATH=$(pwd)/../app/web_server/webServer/web_cgi
CGI_DST_PATH=$(pwd)/../platform/ingenic/t40/resource/partitions/ipc/etc/webServer/cgi-bin

show_help(){
	echo "Useage:"
	echo " $0 -toolchain -model -rootfs_libc -flash_size"
	echo " -toolchain: gcc472; gcc540"
	echo " -model: N; L; X; A"
	echo " -rootfs_libc: uclibc; glibc"
	echo " -flash_size: 8M; 16M"
	echo "Exp:"
	echo " $0 -gcc472 -N -uclibc -8M"
	echo "Remark:"
	echo " If no parameters, defaut: $0 -gcc472 -N -uclibc -16M"
}

checkreturn(){
	if [ $? -ne 0 ]
	then
		echo error $1
		exit 1;
	fi
}

set_gcc720(){
	export PATH=${TOOLCHAIN720_r514}:$PATH
	GCC_VERSION=`mips-linux-gnu-gcc -v 2>&1 | sed -r 's/\s+$//g' | sed  -n '/gcc version/p'`
	WANT_VERSION="gcc version 7.2.0 (Ingenic Linux-Release5.1.4.1-Default(xburst2(fp64)+glibc2.29+Go language) 2022.08-08 10:51:21)"
	echo "$WANT_VERSION"
	if [ "${GCC_VERSION}" != "${WANT_VERSION}" ];then
		echo toolchain error
		echo want:${WANT_VERSION}
		echo real:${GCC_VERSION}
		exit 1;
	fi
}

make_uboot(){
	echo make uboot
	cd ${UBOOT_PATH}
	make distclean
	if [ "$1" = "n" ]; then
		make isvp_${march}$1_sfc_nor
	elif [ "$1" = "l" ];then
		make isvp_${march}$1_sfc_nor
	else
		echo make uboot error
		exit 1;
	fi
	checkreturn "make uboot"
    cp u-boot-with-spl.bin $XCAM_RESOURCE
	echo make uboot success
}

make_kernel(){
	echo make kernel
	cd ${KERNEL_T41_PATH}
	make ${kernel_config}
	checkreturn "make kernel"
	make uImage -j32
	checkreturn "make kernel"
	cp arch/mips/boot/uImage $XCAM_RESOURCE
	echo make kernel success
}

make_driver(){
	echo make isp driver
    cd ${ISP_PATH}
    make -j
    cp tx-isp-${march}.ko $XCAM_RESOURCE
    checkreturn "complier isp driver"

	echo make sensor driver
	cd ${SENSOR_PATH}
    cd $sensor
    make -j
    cp sensor_${sensor}_${march}.ko $XCAM_RESOURCE
    checkreturn "complier sensor driver"

	echo make sinfo driver
    cd ${SENSOR_INFO_PATH}
	make -j
    cp sinfo.ko $XCAM_RESOURCE
	checkreturn "complier sinfo driver"

	echo make avpu driver
	cd ${AVPU_PATH}
	make -j
	cp avpu.ko $XCAM_RESOURCE
	checkreturn "make avpu driver"

	echo cp sensor_setting
    cd ${SETTINGS_PATH}
    cp ${sensor}-${march}.bin ${XCAM_RESOURCE}
	checkreturn "cp sensor_setting"

	echo dirivers make success
}

make_app(){
	echo make app
	cd $APP_PATH
	if [ -d $APP_PATH/build ]; then
		rm -rf $APP_PATH/build
		mkdir $APP_PATH/build
	else
		mkdir $APP_PATH/build
	fi
	checkreturn "make app"
	cd $APP_PATH/build
	cmake ..
	make -j8

    cp out/* $XCAM_RESOURCE
	checkreturn "cp app"

	echo make web_cgi
	cd $CGI_PATH
	./make.sh
	cp login.cgi webTransData.cgi $CGI_DST_PATH
}

cd ${XCAM_PATH}
source tools/env.sh
if [ ! -d $XCAM_RESOURCE ]; then
	mkdir -p $XCAM_RESOURCE
else
	rm -rf $XCAM_RESOURCE
	mkdir -p $XCAM_RESOURCE
fi
echo ${XCAM_RESOURCE}

checkreturn "enter xcamera_device dir"

if [ "$toolchain" = "gcc472" ]; then
	TOOLCHAIN="gcc472"
	ROOTFS_TOOLCHAIN="toolchain4.7.2"
	set_gcc472
elif [ "$toolchain" = "gcc540" ]; then
	TOOLCHAIN="gcc540"
	ROOTFS_TOOLCHAIN="toolchain5.4.0"
	set_gcc540
elif [ "$toolchain" = "gcc720" ]; then
	TOOLCHAIN="gcc720"
	ROOTFS_TOOLCHAIN="toolchain7.2.0"
	set_gcc720
else
	echo "unsupport the toolchain!"
	echo "DEFAULT_TOOL set error!"
	exit 1
fi

if [ "$model" = "N" ]; then
	MODEL="n"
	make_uboot "n"
elif [ "$model" = "L" ]; then
	MODEL="l"
	make_uboot "l"
else
	echo "unsupport the model!"
	echo "DEFAULT_MODEL set error!"
	exit 1
fi

if [ "$toolchain" = "gcc472" ]; then
	if [ "$libc" = "uclibc" ]; then
		cp $ROOTFS_PATH/root-uclibc-toolchain4.7.2-1.1.squashfs ${XCAM_RESOURCE}
		checkreturn "cp root-uclibc-toolchain4.7.2-1.1.squashfs"
		ROOTFS_LIBC="uclibc"
	elif [ "$libc" = "glibc" ]; then
		cp $ROOTFS_PATH/root-glibc-toolchain4.7.2-1.1.squashfs ${XCAM_RESOURCE}
		checkreturn "cp root-glibc-toolchain4.7.2-1.1.squashfs"
		ROOTFS_LIBC="glibc"
	else
		echo "unsupport 472 rootfs_libc!"
		show_help
		exit 1
	fi
elif [ "$toolchain" = "gcc540" ]; then
	if [ "$libc" = "uclibc" ]; then
		cp $ROOTFS_PATH/root-uclibc-toolchain5.4.0-1.1.squashfs ${XCAM_RESOURCE}
		checkreturn "cp root-uclibc-toolchain5.4.0-1.1.squashfs"
		ROOTFS_LIBC="uclibc"
	elif [ "$ROOTFS_LIBC" = "glibc" ]; then
		cp $ROOTFS_PATH/root-glibc-toolchain5.4.0-1.1.squashfs ${XCAM_RESOURCE}
		checkreturn "cp root-glibc-toolchain5.4.0-1.1.squashfs"
		ROOTFS_LIBC="glibc"
	else
		echo "unsupport 540 rootfs_libc!"
		show_help
		exit 1
	fi
elif [ "$toolchain" = "gcc720" ]; then
	if [ "$libc" = "uclibc" ]; then
		cp $ROOTFS_PATH/root-uclibc-toolchain-7.2.0-1.0.squashfs ${XCAM_RESOURCE}
		checkreturn "cp root-uclibc-toolchain720.squashfs"
		ROOTFS_LIBC="uclibc"
	elif [ "$ROOTFS_LIBC" = "glibc" ]; then
		cp $ROOTFS_PATH/root-glibc-toolchain5.4.0-1.1.squashfs ${XCAM_RESOURCE}
		checkreturn "cp root-glibc-toolchain720.squashfs"
		ROOTFS_LIBC="glibc"
	else
		echo "unsupport 720 rootfs_libc!"
		show_help
		exit 1
	fi
else
	echo "unsupport the toolchain!"
	exit 1
fi

make_kernel

make_driver

make_app
