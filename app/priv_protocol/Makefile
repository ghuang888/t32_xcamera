all:
	gcc xconfig.c ../cJSON/cJSON.c ../socket/socket_client_tcp.c xcam_json_process.c -lrt -pthread -lm -o xconfig
	gcc server.c ../cJSON/cJSON.c ../socket/socket_server_tcp.c xcam_json_process.c -lrt -pthread -lm -o server

.PHONY : clean
clean:
	-rm -f xconfig server
