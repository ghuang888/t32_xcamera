#ifndef _XCAM_NETWORK_H_
#define _XCAM_NETWORK_H_

#ifdef __cplusplus
extern "C" {
#endif

int xcam_network_get_device_ip(char* interface, char* addr);
int xcam_network_set_device_static_ip(char* interface, char *addr);
void xcam_network_stop_dhcp();
void xcam_network_start_dhcp();
void xcam_network_dhcp_status_and_pid(bool *pIsDhcpEnable, pid_t *pid);
int xcam_network_set_device_ip_gateway(char *gateway);
int xcam_network_get_device_ip_gateway(char *interface, char *gateway);
int xcam_network_get_device_ip_mask(char *interface, char *mask);
int xcam_network_get_device_DNS_server_addr(char *DNSaddr);
int xcam_network_set_device_DNS_server_addr(char *DNSaddr);
void xcam_reboot(void);
int xcam_network_get_net_interface(char *pInterface);
int xcam_network_get_device_mac(char *interface, unsigned char *pMac);
void xcam_network_dhcp_status_and_pid(bool *pIsDhcpEnable, pid_t *pid);

#ifdef __cplusplus
}
#endif
#endif
