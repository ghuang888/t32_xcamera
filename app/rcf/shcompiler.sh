#!bash/bin
echo "compiling wait ..."
echo $1
str=mips 
if [ $1 = $str ]
then

    echo "mips-linux-gnu-g++   run"
   #mips-linux-gnu-g++ -o client rcf_client/Client.cpp -muclibc -O2 -I./inc -I ./RCF-2.2.0.0/include/  -L./lib/ -luuid  -lRCF -ldl  -lpthread
   #mips-linux-gnu-g++ -o server rcf_server/Server.cpp -muclibc -O2 ../conf/src/conf_process.c -I../conf/inc -I./inc -I ./RCF-2.2.0.0/include/   -L./lib/ -lcJson -luuid  -lRCF -ldl  -lpthread
    mips-linux-gnu-g++ -o client rcf_client/Client.cpp -muclibc -O2 -I./inc -I ./RCF/include/  -L./lib/ -ldl -lRCF -lpthread

else

    echo "g++ run"
    g++ -o client rcf_client/Client.cpp  -I./inc -I ./RCF-2.2.0.0/include/  -L./lib/ -luuid  -lRCF -ldl  -lpthread
    g++ -o server rcf_server/Server.cpp ../conf/conf_process.c -I../conf/ -I./inc -I ./RCF-2.2.0.0/include/   -L./lib/ -lcJson -luuid  -lRCF -ldl  -lpthread

fi
#g++ -o client Client.cpp MyService.hpp -I ../include/  -L./lib/ -luuid  -lRCF -ldl  -lpthread
#g++ -o client rcf_client/Client.cpp  -I./inc -I ./RCF-2.2.0.0/include/  -L./lib/ -luuid  -lRCF -ldl  -lpthread

#g++ -o server conf_process.h conf_process.c Server.cpp MyService.hpp  -I ../include/  -L./lib/ -lcJson -luuid  -lRCF  -ldl  -lpthread
#g++ -o server rcf_server/Server.cpp ../conf/conf_process.c -I../conf/ -I./inc -I ./RCF-2.2.0.0/include/   -L./lib/ -lcJson -luuid  -lRCF -ldl  -lpthread
mv client server ./bin/

echo "compiler finished !"
#使用动态库，若出现ld找不到动态库，则可以使用下面的一句，设置动态库临时搜索路径
#export LD_LIBRARY_PATH=/home_d/jwang_01/work/xcamera_device/app/rcf/lib 

