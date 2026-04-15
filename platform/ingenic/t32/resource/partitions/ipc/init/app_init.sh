udhcpc -i eth0

sleep 3

insmod /system/lib/modules/tx-isp-PRJ007.ko clk_name="vpll" clks_name="vpll" clka_name="vpll"   clkv_name="vpll"  isp_clk=300000000 isp_clka=600000000 isp_clks=600000000     isp_clkv=600000000

insmod /system/lib/modules/sensor_gc5613_PRJ007.ko
insmod /system/lib/modules/audio.ko
insmod /system/lib/modules/tnpu.ko
cp /system/etc/sensor/* /etc/sensor
cp /system/etc/setir /tmp
/system/bin/xcamera &
# ./carrier-server --st=gc8613 --i2c=0x31 -w 3840 -h 2160 --sboot=0 --fps=25 --nrvbs=1