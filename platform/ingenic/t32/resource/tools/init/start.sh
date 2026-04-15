#!/bin/sh

START_CMD="xcamera"

SINFO_KO_PATH=/lib/modules
SENSOR_DRV_PATH=/lib/modules
ISP_DRV_PATH=/lib/modules
CFGS_DEFAULT=/system/cfgs_default
CONFIGS=/opt/configs

check_return()
{
	if [ $? -ne 0 ] ;then
		echo err: $1
		echo exit
		exit
	fi
}

for cfgs in `ls $CFGS_DEFAULT`
do
	find $CONFIGS/$cfgs > /dev/zero 2>&1
	if [ $? -ne 0 ]; then
		cp $CFGS_DEFAULT/$cfgs $CONFIGS
	fi
done

lsmod | grep "sinfo" > /dev/null
if [ $? -ne 0 ] ;then
	insmod ${SINFO_KO_PATH/%\//}/sinfo.ko
	check_return "insmod sinfo"
fi

echo 1 >/proc/jz/sinfo/info
check_return "start sinfo"

SENSOR_INFO=`cat /proc/jz/sinfo/info`
check_return "get sensor type"
echo ${SENSOR_INFO}

PARAM_PATH=/tmp/start_param
find /tmp/start_param > /dev/zero 2>&1
if [ $? -eq 0 ]; then
	PARAM_PATH=/tmp/start_param
else
	PARAM_PATH=start_param
fi
echo ${PARAM_PATH}

SENSOR=${SENSOR_INFO#*:}
ISP_PARAM="isp_clk=125000000"
SENSOR_PARAM=
CARRIER_SERVER_PARAM="--nrvbs 2"
START=0
while read str
do
	if [ "$str" = "" ];then
		continue
	fi
	name=${str%:*}
	value=${str#*:}
	if [ ${START} = 0 ];then
		if [ "$value" = "$SENSOR" ];then
			echo export SENSOR_NAME=${value} > /tmp/delay_env.sh
			#export SENSOR_NAME=${value}
			START=1
		fi
	else
		case ${name} in
			"sensor_i2c")
				echo export SENSOR_I2C=${value} >> /tmp/delay_env.sh
				#export SENSOR_I2C=${value}
				;;
			"sensor_width")
				echo export SENSOR_WIDTH=${value} >> /tmp/delay_env.sh
				#export SENSOR_WIDTH=${value}
				;;
			"sensor_height")
				echo export SENSOR_HEIGHT=${value} >> /tmp/delay_env.sh
				#export SENSOR_HEIGHT=${value}
				;;
			"isp_param")
				ISP_PARAM=${value}
				;;
			"sensor_param")
				SENSOR_PARAM=${value}
				;;
			"carrier_server_param")
				CARRIER_SERVER_PARAM=${value}
				;;
			*)
				break;
				;;
		esac
	fi
done<${PARAM_PATH}
echo --------------------
echo ${ISP_PARAM}
echo ${SENSOR_PARAM}
echo ${CARRIER_SERVER_PARAM}

lsmod | grep "tx_isp" > /dev/null
if [ $? -ne 0 ] ;then
	insmod ${ISP_DRV_PATH/%\//}/tx-isp-t21.ko  ${ISP_PARAM}
	check_return "insmod isp drv"
fi

lsmod | grep "audio" > /dev/null
if [ $? -ne 0 ] ;then
        insmod ${ISP_DRV_PATH/%\//}/audio.ko
        check_return "insmod audio"
fi



lsmod | grep ${SENSOR} > /dev/null
if [ $? -ne 0 ] ;then
	insmod ${SENSOR_DRV_PATH/%\//}/sensor_${SENSOR}_t21.ko ${SENSOR_PARAM}
	check_return "insmod sensor drv"
fi

source /tmp/delay_env.sh
echo ${START_CMD##*/} start
/system/bin/xcamera &
echo ${START_CMD##*/} exit

