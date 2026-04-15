#ifndef _XCAM_CONF_NETWORK_H_
#define _XCAM_CONF_NETWORK_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	XCAM_IPV4,
	XCAM_IPV6,
}network_flag_e;

typedef enum {
	NETWORK_STATIC,
	NETWORK_DHCP,
}network_mode_e;

union network_addr_u {
	unsigned long ipu_addr;
	unsigned long ipv6u_addr [4];
};

typedef struct network_config_s{
	int mode;
	int net_flag;
	union network_addr_u net_addr;
	unsigned long mask;
	unsigned long gateway;
	unsigned long DNSaddr;
}network_config_t;

#define ip_addr net_addr.ipu_addr
#define ipv6_addr net_addr.ipv6u_addr

void xcam_conf_network_init();
int xcam_conf_set_auto(bool *IsDhcpEnable);
int xcam_conf_set_gateway(char *gateway);
int xcam_conf_set_mask(char *mask);
int xcam_conf_set_ip_addr(char *addr);

#ifdef __cplusplus
}
#endif
#endif
