#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MEMGETINFO	_IOR('M', 1, struct mtd_info_user)
#define MEMERASE    _IOW('M', 2, struct erase_info_user)
#define MAX_MTD_COUNT 10	//actually 9 block, 1 for all block together
#define H_LENGHT 20

int get_all_mtd_size();
int update_type_judge(int mtd_count, FILE *fp);
int file_packet_update(char *filename);
int mem_packet_update(char *mem_addr);
int erase_norflash(char *mtd, unsigned int offset, unsigned int bytes);
int write_norflash(char *mtd, unsigned int offset, unsigned int bytes, unsigned char *data);
int write_direct_block(int mtd_index, int file_index, FILE *fp);
int write_cross_block(int mtd_index, int file_index, int mtd_count, FILE *fp);
int mesg_deal();

typedef struct handle_info {
	u_int32_t	product_type;
	u_int16_t	version_major;
	u_int16_t	version_minor;
	u_int32_t	lenght;				//include header list and content
	u_int32_t	item_number;
	u_int32_t	crc32;			//include header(except crc32),list and content
	u_int32_t	list[MAX_MTD_COUNT][4];	//the max sfc part is 10
} handle_info_t;

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

static handle_info_t packet_handle_info;
static unsigned int mtd_size[MAX_MTD_COUNT][2];

int get_all_mtd_size() {
	int fd, i, j;
	char mtd_name[10] = "/dev/mtd0";
	mtd_info_user_t mtd_info;
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

int update_type_judge(int mtd_count, FILE *fp) {
	int i, j;
	int item_count = packet_handle_info.item_number;

	for (i = 0; i < item_count; i ++) {
		for (j = 0; j < mtd_count; j ++) {
			if (packet_handle_info.list[i][0] >= mtd_size[j][0] && packet_handle_info.list[i][0] < mtd_size[j + 1][0]) {
				if (packet_handle_info.list[i][0] + packet_handle_info.list[i][2] <= mtd_size[j + 1][0]){
					printf("file_index:%d doing first\n", i);
					write_direct_block(j, i, fp);
					printf("\n");
					break;
				}
				else {
					printf("file_index:%d doing second\n", i);
					write_cross_block(j, i, mtd_count, fp);
					printf("\n");
					break;
				}
			}
		}
	}
	return 0;
}

int file_packet_update(char *filename) {
	FILE *fp;
	int mtd_count;
	if ((mtd_count = get_all_mtd_size()) <= 0) {
		printf("%s:no mtd block can oprate!\n", __func__);
		return -1;
	}
	printf("\n\n\n\n");
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("%s:fopen update file %s failed!\n", __func__, filename);
		return -1;
	}
	fread(&packet_handle_info, 1, H_LENGHT, fp);
	fread(packet_handle_info.list, 1, 16 * packet_handle_info.item_number, fp);
	printf("product:%d\nver_major:%d\nver_minor:%d\nlenght:%d\nitem_number:%d\ncrc32:%8x\n", packet_handle_info.product_type, packet_handle_info.version_major, packet_handle_info.version_minor, packet_handle_info.lenght, packet_handle_info.item_number, packet_handle_info.crc32);
	int x,y;
	for(x = 0; x < packet_handle_info.item_number; x ++)
	{
		for(y = 0; y < 4; y++)
		{
			printf("\tlist[%d][%d]:%x ", x, y, packet_handle_info.list[x][y]);
			if (y == 3) printf("\n");
		}
	}
	if (update_type_judge(mtd_count, fp) < 0) return -1;

	fclose(fp);

	return 0;
}

int mem_packet_update(char *mem_addr) {
	return 0;
}

int erase_norflash(char *mtd, unsigned int offset, unsigned int bytes) {
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

int write_norflash(char *mtd, unsigned int offset, unsigned int bytes, unsigned char *data) {
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

int write_cross_block(int mtd_index, int file_index, int mtd_count, FILE *fp) {
	int i, j;
	char mtd_name[10] = "/dev/mtd0";
	unsigned int temp = packet_handle_info.list[file_index][0] + packet_handle_info.list[file_index][2];
	unsigned int lenght, off_in_mtd;

	//erase flash
	for (i = mtd_index + 1; i < mtd_count; i ++)
		if (temp > mtd_size[i][0] && temp <= mtd_size[i + 1][0])
			break;
	printf("erase tail in mtd %d\n", i);
	for (j = mtd_index; j <= i; j ++) {
		if (j == mtd_index) {
			mtd_name[8] = 48 + j;
			off_in_mtd = packet_handle_info.list[file_index][0] - mtd_size[j][0];
			lenght = mtd_size[j + 1][0] - packet_handle_info.list[file_index][0];
			if (erase_norflash(mtd_name, off_in_mtd, lenght) < 0) return -1;
		}
		else if (j == i){
			mtd_name[8] = 48 + j;
			lenght = packet_handle_info.list[file_index][0] + packet_handle_info.list[file_index][2] - mtd_size[j][0];
			if (erase_norflash(mtd_name, 0, lenght) < 0) return -1;
		} else {
			mtd_name[8] = 48 + j;
			lenght = mtd_size[j + 1][0] - mtd_size[j][0];
			if (erase_norflash(mtd_name, 0, lenght) < 0) return -1;
		}
	}

	//write flash

	char file_name[20] = "/mnt/file_test0";
	file_name[14] += file_index;
	FILE *tt = fopen(file_name, "w+");

	temp = packet_handle_info.list[file_index][0] + packet_handle_info.list[file_index][3];
	fseek(fp, packet_handle_info.list[file_index][1] + H_LENGHT + packet_handle_info.item_number * 16, SEEK_SET);
	unsigned char *buf = (unsigned char *)malloc(packet_handle_info.list[file_index][3]);
	fread(buf, 1, packet_handle_info.list[file_index][3], fp);

	if (packet_handle_info.list[file_index][3] <= mtd_size[mtd_index + 1][0] - packet_handle_info.list[file_index][0]) {
		mtd_name[8] = 48 + mtd_index;
		off_in_mtd = packet_handle_info.list[file_index][0] - mtd_size[mtd_index][0];
		lenght = packet_handle_info.list[file_index][3];
		if (write_norflash(mtd_name, off_in_mtd, lenght, buf) < 0) return -1;
		fwrite(buf, 1, lenght, tt); //test
		free(buf);
	} else {
		for (i = mtd_index + 1; i < mtd_count; i ++)
			if (temp > mtd_size[i][0] && temp <= mtd_size[i + 1][0])
				break;
		printf("write tail in mtd %d\n", i);
		for (j = mtd_index; j <= i; j ++) {
			if (j == mtd_index) {
				mtd_name[8] = 48 + j;
				off_in_mtd = packet_handle_info.list[file_index][0] - mtd_size[j][0];
				lenght = mtd_size[j + 1][0] - packet_handle_info.list[file_index][0];
				if (write_norflash(mtd_name, off_in_mtd, lenght, buf) < 0) return -1;
				fwrite(buf, 1, lenght, tt);
				buf += lenght;
			}
			else if (j == i){
				mtd_name[8] = 48 + j;
				lenght = packet_handle_info.list[file_index][0] + packet_handle_info.list[file_index][3] - mtd_size[j][0];
				if (write_norflash(mtd_name, 0, lenght, buf) < 0) return -1;
				fwrite(buf, 1, lenght, tt);
				buf += lenght;
			} else {
				mtd_name[8] = 48 + j;
				lenght = mtd_size[j + 1][0] - mtd_size[j][0];
				if (write_norflash(mtd_name, 0, lenght, buf) < 0) return -1;
				fwrite(buf, 1, lenght, tt);
				buf += lenght;
			}
		}
		free(buf - packet_handle_info.list[file_index][3]);
	}
	fclose(tt);
	return 0;
}

int write_direct_block(int mtd_index, int file_index, FILE *fp) {
	char mtd_name[10] = "/dev/mtd0";
	unsigned int off_in_mtd = packet_handle_info.list[file_index][0] - mtd_size[mtd_index][0];
	mtd_name[8] += mtd_index;
	if (erase_norflash(mtd_name, off_in_mtd, packet_handle_info.list[file_index][2]) < 0) return -1;
	fseek(fp, packet_handle_info.list[file_index][1] + H_LENGHT + packet_handle_info.item_number * 16, SEEK_SET);
	unsigned char *buf = (unsigned char *)malloc(packet_handle_info.list[file_index][3]);
	fread(buf, 1, packet_handle_info.list[file_index][3], fp);
	if (write_norflash(mtd_name, off_in_mtd, packet_handle_info.list[file_index][3], buf) < 0) return -1;

	char file_name[20] = "/mnt/file_test0";
	file_name[14] += file_index;
	FILE *tt = fopen(file_name, "w+");
	fwrite(buf, 1, packet_handle_info.list[file_index][3], tt);
	fclose(tt);

	free(buf);
	return 0;
}

int mesg_deal() {
	char buf[64];
	key_t key = ftok("/bin", 1);
	int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0664);
	if (msgid < 0) {
		printf("%s:create mesg fifo failed!\n", __func__);
		return -1;
	}

	while(1) {
		msgrcv(msgid, buf, sizeof(buf), 100, 0);
		if (file_packet_update(buf) < 0 ) {
			printf("file_packet_update failed! please check reason!\n");
			return -1;
		}
	}
}
int main(int argc, char *argv[]) {
	file_packet_update(argv[1]);
	//mesg_deal();
	return 0;
}
