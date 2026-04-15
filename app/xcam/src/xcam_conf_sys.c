#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <getopt.h>
#include "xcam_conf.h"
#include "xcam_conf_sys.h"
#include "xcam_general.h"
#include "xcam_log.h"

#define LOG_TAG "CONFIGSYS"
#define CONFIG_SYS_FILE "/system/sys.ini"

unsigned char sensor_name[10];
unsigned int  sensor_i2c;
unsigned int  sensor_width;
unsigned int  sensor_height;

int xcam_load_sys_config(void)
{
    int ret = 0;
    char tmp[50];

    ret = xcam_read_profile(CONFIG_SYS_FILE, "sensor", "sensor_name", tmp);
    if (ret < 0) {
        printf("%s,%d: error\n", __func__, __LINE__);
        return -1;
    }
    strcpy((char*)sensor_name, tmp);

    ret = xcam_read_profile(CONFIG_SYS_FILE, "sensor", "sensor_i2c", tmp);
    if (ret < 0) {
        printf("%s,%d: error\n", __func__, __LINE__);
        return -1;
    }
    sensor_i2c = strtol(tmp, NULL, 0);

    ret = xcam_read_profile(CONFIG_SYS_FILE, "sensor", "sensor_width", tmp);
    if (ret < 0) {
        printf("%s,%d: error\n", __func__, __LINE__);
        return -1;
    }
    sensor_width = strtol(tmp, NULL, 0);

    ret = xcam_read_profile(CONFIG_SYS_FILE, "sensor", "sensor_height", tmp);
    if (ret < 0) {
        printf("%s,%d: error\n", __func__, __LINE__);
        return -1;
    }
    sensor_height = strtol(tmp, NULL, 0);

    return 0;
}
