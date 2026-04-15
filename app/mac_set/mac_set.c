#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

typedef unsigned int	uint32_t;

//#define TEST
#define OTP_BASE 0x13540200
#define CHIP_BASE 0x1300002C

int get_value(unsigned int addr, unsigned int *value, int is_write)
{
	void *map_base, *virt_addr;
	off_t target;
	unsigned page_size, mapped_size, offset_in_page;
	int fd, width = 32, ret = 0;

	target = addr;
	fd = open("/dev/mem", is_write ? (O_RDWR | O_SYNC) : (O_RDONLY | O_SYNC));
	if (fd < 0) {
		printf("open /dev/mem failed\n");
		return -1;
	}

	mapped_size = page_size = getpagesize();
	offset_in_page = (unsigned)target & (page_size - 1);
	if (offset_in_page + width > page_size) {
		/* This access spans pages.
		 * Must map two pages to make it possible: */
		mapped_size *= 2;
	}
	map_base = mmap(NULL,
			mapped_size,
			is_write ? (PROT_READ | PROT_WRITE) : PROT_READ,
			MAP_SHARED,
			fd,
			target & ~(off_t)(page_size - 1));
	printf("#####################################\n");
	if (map_base == MAP_FAILED) {
		printf("mmap failed\n");
		ret = -1;
		goto close_fd;
	}

	virt_addr = (char*)map_base + offset_in_page;

	if (!is_write)
		*value = *(volatile uint32_t*)virt_addr;
	else
		*(volatile uint32_t*)virt_addr = *value;

	if (munmap(map_base, mapped_size) == -1)
		printf("munmap failed\n");

close_fd:
	close(fd);

	return ret;
}

int otp_get_mac(uint32_t value, uint32_t *otp_mac)
{
	int i;
	for(i = 0; i < 4; i++)
	{
		if(i > 0)
			value >>= 8;
		otp_mac[i] = value & 0xff;
	}
	return 0;
}

int chip_get_mac(uint32_t value, uint32_t *chip_mac)
{
	uint32_t value1, value2;
	value1 = (value >> 12) & 0xff;
	value2 = (value >> 20) & 0xff;
	chip_mac[0] = (value1 + value2) & 0xff;
	return 0;
}

int set_mac_cmd(uint32_t *otp_mac, uint32_t *chip_mac)
{
	int i;
	char mac_cmd[200] = "ifconfig eth0 hw ether 00:";
	sprintf(mac_cmd + strlen(mac_cmd), "%x", chip_mac[0]);
	for(i = 0; i < 4; i++)
	{
		strcat(mac_cmd, ":");
		sprintf(mac_cmd + strlen(mac_cmd), "%x", otp_mac[i]);
	}
#ifdef TEST
	system("ifconfig eth0 down");
	sleep(1);
#endif
	system(mac_cmd);
#ifdef TEST
	sleep(1);
	system("ifconfig eth0 up");
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	uint32_t otp_value, chip_value;
	uint32_t otp_mac[4], chip_mac[1];
	int ret = 0;
	ret = get_value(OTP_BASE, &otp_value, 0);
	if(ret)
	{
		printf("get value error!\n");
		return -1;
	}
	otp_get_mac(otp_value, otp_mac);

	ret = get_value(CHIP_BASE, &chip_value, 0);
	if(ret)
	{
		printf("get value error!\n");
		return -1;
	}
	chip_get_mac(chip_value, chip_mac);

	set_mac_cmd(otp_mac, chip_mac);
	return 0;
}
