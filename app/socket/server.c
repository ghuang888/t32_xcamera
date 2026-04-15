#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "socket_tcp_common.h"

int msg_process_func(socket_tcp_link_t *plink)
{
#if 0
	printf("msg recv: cmd = 0x%08x, len = 0x%08x, action = 0x%08x\n",
			msg_header->cmd, msg_header->len, msg_header->action);
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	void *skt;
	skt = socket_server_tcp_alloc(11910, 5);
	assert(NULL != skt);
	ret = socket_server_tcp_set_msg_process_cb(skt, msg_process_func);
	assert(0 == ret);
	ret = socket_server_tcp_start(skt);
	assert(0 == ret);
	while (1)
		sleep(1);
	return 0;
}
