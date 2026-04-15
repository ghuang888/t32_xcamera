#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>

#include "xcam_daemon_protocol.h"

#define MEMGETINFO	_IOR('M', 1, struct mtd_info_user)
#define MEMERASE    _IOW('M', 2, struct erase_info_user)
#define MAX_MTD_COUNT 10	//actually 9 block, 1 for all block together
#define H_LENGHT 20

static int pack_num = 0;
static unsigned char* p_mem = NULL;

typedef struct pack_handle_info {
	int32_t	product_type;
	int16_t	version_major;
	int16_t	version_minor;
	int32_t	lenght;				//include header list and content
	int32_t	item_number;
	int32_t	crc32;			//include header(except crc32),list and content
	int32_t	list[MAX_MTD_COUNT][4];	//the max sfc part is 10
} pack_handle_info_t;

typedef struct update_handle {
	pack_handle_info_t pack_info;
	int mtd_size[MAX_MTD_COUNT][2];
	int mtd_index;		//mtd index of what we are now operating
	int item_index;		//file index of what we are now operating
	int mtd_count;		//number of all mtd
	unsigned char *mem_addr;
} update_handle_t;

typedef struct mtd_info_user {
	u_int8_t	type;
	u_int32_t	flags;
	u_int32_t	size;	/* Total size of the MTD */
	u_int32_t	erasesize;
	u_int32_t	writesize;
	u_int32_t	oobsize;	/* Amount of OOB data per block (e.g. 16) */
	u_int64_t	padding;	/* Old obsolete field; do not use */
} mtd_info_user_t;

typedef struct erase_info_user {
	u_int32_t	start;
	u_int32_t	lenght;
} erase_info_user_t;

int erase_norflash(char *mtd, u_int32_t offset, u_int32_t bytes) {
	int fd;
	if ((fd = open(mtd, O_SYNC | O_RDWR)) < 0) {
		printf("%s:open %s failed!\n", __func__, mtd);
		return -1;
	}
	if (offset != lseek(fd, offset, SEEK_SET)) {
		printf("%s:lseek size not right!\n", __func__);
		return -1;
	}
	erase_info_user_t mtd_erase;
	mtd_erase.start = offset;
	mtd_erase.lenght = bytes; //bytes must be divisible by 0x1000
	if (ioctl(fd, MEMERASE, &mtd_erase) < 0) {
		printf("%s:erase ioctl failed!\n", __func__);
		return -1;
	}
	printf("%s:Erased 0x%x bytes from address 0x%x in flash %s\n", __func__, bytes, offset, mtd);
	close(fd);
	return 0;
}

int write_norflash(char *mtd, u_int32_t offset, u_int32_t bytes, u_int8_t *data) {
	int fd;
	if ((fd = open(mtd, O_SYNC | O_RDWR)) < 0) {
		printf("%s:open %s failed!\n", __func__, mtd);
		return -1;
	}
	if (offset != lseek(fd, offset, SEEK_SET)) {
		printf("%s:lseek size not right!\n", __func__);
		return -1;
	}
	if (write(fd, data, bytes) < 0) {
		printf("%s:write norflash %s offset:%d bytes:%d failed!\n", __func__, mtd, offset, bytes);
		close(fd);
		return -1;
	}
	printf("%s:Copied 0x%x bytes to address 0x%x in flash %s\n", __func__, bytes, offset, mtd);
	close(fd);
	return 0;
}

int get_all_mtd_size(update_handle_t *update) {
	int fd, i, j;
	char mtd_name[10] = "/dev/mtd0";
	mtd_info_user_t mtd_info;
	int (*mtd_size)[2] = update->mtd_size;
	memset(mtd_size, 0, sizeof(int) * 2 * MAX_MTD_COUNT);

	for (i = 0; (fd = open(mtd_name, O_SYNC | O_RDONLY)) > 0; i ++) {
		printf("%s\n", mtd_name);
		if (ioctl(fd, MEMGETINFO, &mtd_info) < 0) {
			printf("%s:ioctl MEMGETINFO failed\n", __func__);
			return -1;
		}
		mtd_size[i][1] = mtd_info.size;
		if (i == 0)
			mtd_size[i][0] = 0;
		else
			mtd_size[i][0] = mtd_size[i - 1][0] + mtd_size[i - 1][1];
		mtd_name[8] += 1;
		printf("start addr: 0x%x\n", mtd_size[i][0]);
		printf("size: 0x%x\n", mtd_size[i][1]);
		close(fd);
	}
	for (j = 0; j < i; j ++)
		mtd_size[i][0] += mtd_size[j][1];
	return i;
}

int write_block_across(update_handle_t *update) {
	int i, j;
	char mtd_name[10] = "/dev/mtd0";
	int (*list)[4] = update->pack_info.list;
	int (*mtd_size)[2] = update->mtd_size;
	int item_index = update->item_index;
	int mtd_index = update->mtd_index;
	int mtd_count = update->mtd_count;
	unsigned char *mem_addr = update->mem_addr;
	int item_number = update->pack_info.item_number;
	u_int32_t temp = list[item_index][0] + list[item_index][2];
	u_int32_t lenght, off_in_mtd;

	//erase flash
	for (i = mtd_index + 1; i < mtd_count; i ++)
		if (temp > mtd_size[i][0] && temp <= mtd_size[i + 1][0])
			break;
	printf("erase tail in mtd %d\n", i);
	for (j = mtd_index; j <= i; j ++) {
		if (j == mtd_index) {
			mtd_name[8] = 48 + j;
			off_in_mtd = list[item_index][0] - mtd_size[j][0];
			lenght = mtd_size[j + 1][0] - list[item_index][0];
			if (erase_norflash(mtd_name, off_in_mtd, lenght) < 0) return -1;
		}
		else if (j == i){
			mtd_name[8] = 48 + j;
			lenght = list[item_index][0] + list[item_index][2] - mtd_size[j][0];
			if (erase_norflash(mtd_name, 0, lenght) < 0) return -1;
		} else {
			mtd_name[8] = 48 + j;
			lenght = mtd_size[j + 1][0] - mtd_size[j][0];
			if (erase_norflash(mtd_name, 0, lenght) < 0) return -1;
		}
	}

	//write flash
	temp = list[item_index][0] + list[item_index][3];
	mem_addr += (list[item_index][1] + H_LENGHT + item_number * 16);

	if (list[item_index][3] <= mtd_size[mtd_index + 1][0] - list[item_index][0]) {
		mtd_name[8] = 48 + mtd_index;
		off_in_mtd = list[item_index][0] - mtd_size[mtd_index][0];
		lenght = list[item_index][3];
		if (write_norflash(mtd_name, off_in_mtd, lenght, mem_addr) < 0) return -1;
	} else {
		for (i = mtd_index + 1; i < mtd_count; i ++)
			if (temp > mtd_size[i][0] && temp <= mtd_size[i + 1][0])
				break;
		printf("write tail in mtd %d\n", i);
		for (j = mtd_index; j <= i; j ++) {
			if (j == mtd_index) {
				mtd_name[8] = 48 + j;
				off_in_mtd = list[item_index][0] - mtd_size[j][0];
				lenght = mtd_size[j + 1][0] - list[item_index][0];
				if (write_norflash(mtd_name, off_in_mtd, lenght, mem_addr) < 0) return -1;
				mem_addr += lenght;
			}
			else if (j == i){
				mtd_name[8] = 48 + j;
				lenght = list[item_index][0] + list[item_index][3] - mtd_size[j][0];
				if (write_norflash(mtd_name, 0, lenght, mem_addr) < 0) return -1;
				mem_addr += lenght;
			} else {
				mtd_name[8] = 48 + j;
				lenght = mtd_size[j + 1][0] - mtd_size[j][0];
				if (write_norflash(mtd_name, 0, lenght, mem_addr) < 0) return -1;
				mem_addr += lenght;
			}
		}
	}
	return 0;
}

int write_block_direct(update_handle_t *update) {
	char mtd_name[10] = "/dev/mtd0";
	int (*list)[4] = update->pack_info.list;
	int (*mtd_size)[2] = update->mtd_size;
	int item_index = update->item_index;
	int mtd_index = update->mtd_index;
	int item_number = update->pack_info.item_number;
	unsigned char *mem_addr = update->mem_addr;

	u_int32_t off_in_mtd = list[item_index][0] - mtd_size[mtd_index][0];
	printf("item:%d  mtd:%d\n", item_index, mtd_index);
	mtd_name[8] += mtd_index;
	if (erase_norflash(mtd_name, off_in_mtd, list[item_index][2]) < 0) return -1;
	mem_addr += (list[item_index][1] + H_LENGHT + item_number * 16);
	if (write_norflash(mtd_name, off_in_mtd, list[item_index][3], mem_addr) < 0) return -1;
	return 0;
}

int update_type_judge(update_handle_t *update) {
	int i, j;
	int item_count = update->pack_info.item_number;
	int mtd_count = update->mtd_count;
	int (*list)[4] = update->pack_info.list;
	int (*mtd_size)[2] = update->mtd_size;

	for (i = 0; i < item_count; i ++) {
		for (j = 0; j < mtd_count; j ++) {
			if (list[i][0] >= mtd_size[j][0] && list[i][0] < mtd_size[j + 1][0]) {
				if (list[i][0] + list[i][2] <= mtd_size[j + 1][0]){
					printf("item_index:%d doing first\n", i);
					update->item_index = i;
					update->mtd_index = j;
					if (write_block_direct(update) < 0) {
						printf("%s:write_block_direct error, now operating fileblock %d, /dev/mtd%d\n", __func__, i, j);
						return -1;
					}
					printf("\n");
					break;
				}
				else {
					printf("item_index:%d doing second\n", i);
					update->item_index = i;
					update->mtd_index = j;
					if (write_block_across(update) < 0) {
						printf("%s:write_block_across error, now operating fileblock %d, /dev/mtd%d\n", __func__, i, j);
						return -1;
					}
					printf("\n");
					break;
				}
			}
		}
	}
	return 0;
}

int mem_packet_update(u_int8_t *mem_addr) {
	update_handle_t update;
	update.mem_addr = mem_addr;
	if ((update.mtd_count = get_all_mtd_size(&update)) <= 0) {
		printf("%s:no mtd block can oprate!\n", __func__);
		return -1;
	}
	memcpy(&update.pack_info, mem_addr, H_LENGHT);
	memcpy(update.pack_info.list, mem_addr + H_LENGHT, 16 * update.pack_info.item_number);
	printf("product:%d\nver_major:%d\nver_minor:%d\nlenght:%d\nitem_number:%d\ncrc32:%8x\n", update.pack_info.product_type, update.pack_info.version_major, update.pack_info.version_minor, update.pack_info.lenght, update.pack_info.item_number, update.pack_info.crc32);
	int x,y;
	for(x = 0; x < update.pack_info.item_number; x ++)
	{
		for(y = 0; y < 4; y++)
		{
			printf("\tlist[%d][%d]:%x ", x, y, update.pack_info.list[x][y]);
			if (y == 3) printf("\n");
		}
	}
	if (update_type_judge(&update) < 0) {
		printf("%s:update_type_judge failed!\n", __func__);
		return -1;
	}
	return 0;
}

int xcam_socket_upgrade_msg_process(socket_tcp_link_t *plink)
{
	skt_msg_header_t *msg_header = (skt_msg_header_t *)plink->msgbuf;
	char *data = plink->msgbuf->data;
	pack_handle_info_t *file_handle = (pack_handle_info_t *)data;

	if (!pack_num) {
		printf("%d\n", file_handle->lenght);
		p_mem = (unsigned char *)malloc(file_handle->lenght);
		FILE *fcmd = popen("ps | grep -v grep | grep xcamera | awk '{print $1}'", "r");
		char pcmd[20];
		strcpy(pcmd, "kill -9 ");
		fgets(pcmd + 8, 6, fcmd);
		printf("%s\n", pcmd);
		pclose(fcmd);
		system(pcmd);
	}

	if (msg_header->flags) {
		memcpy(p_mem, data, msg_header->len);
		p_mem = p_mem - pack_num * plink->skt->bufsize;
		if (mem_packet_update(p_mem) < 0) {
			printf("err(%s,%d): upgrade failed\n", __func__, __LINE__);
			return -1;
		}
		pack_num = 0;
		free(p_mem);

		skt_msg_upgrade_ack_t buf;
		buf.header.cmd = SET_SYS_FIRMWARE_ACK;
		buf.header.len = sizeof(skt_msg_upgrade_ack_t) - sizeof(skt_msg_header_t);
		buf.header.flags = 1;
		buf.status = CMD_ACK_STATUS_SUCCESS;
		if (socket_server_tcp_send(plink, (skt_msg_t *)&buf) < 0) {
			printf("err(%s,%d): server send error\n", __func__, __LINE__);
			return -1;
		}
		printf("system will reboot in 3s..\n");
		sleep(3);
		reboot(RB_AUTOBOOT);
	} else {
		memcpy(p_mem, data, msg_header->len);
		p_mem += msg_header->len;
		pack_num ++;
	}
	return 0;
}

