#include "main.h"
#include "w5500_run.h"

void w5500_network_init(void)
{
    uint8_t mac[6] = {0x00, 0x08, 0xDC, 0xAB, 0xCD, 0xEF}; // Уникальный MAC
    uint8_t ip[4] = {192, 168, 1, 100};                    // Статический IP
    uint8_t subnet[4] = {255, 255, 255, 0};
    uint8_t gateway[4] = {192, 168, 1, 1};
    uint8_t dns[4] = {8, 8, 8, 8};

    w5500_init();
    
    wiz_NetInfo netInfo = {
        .mac = {mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]},
        .ip = {ip[0], ip[1], ip[2], ip[3]},
        .sn = {subnet[0], subnet[1], subnet[2], subnet[3]},
        .gw = {gateway[0], gateway[1], gateway[2], gateway[3]},
        .dns = {dns[0], dns[1], dns[2], dns[3]}
    };
    
    static_host_configuration(netInfo.mac, netInfo.ip, netInfo.sn, netInfo.gw, netInfo.dns);
    // dynamic_host_configuration(netInfo.mac);

    check_cable_presence();

    check_phy_status();

    blink_run(50);
    // print_current_host_configuration();

}
void udp_init(uint8_t sct, uint16_t local_port)
{
    socket(sct, Sn_MR_UDP, local_port, 0);
}