#ifndef W5500_RUN_H_
#define W5500_RUN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <socket.h>
#include <w5500_spi.h>
#include <w5500_phy.h>
#include <w5500_host_config.h>

void w5500_network_init(void);
void udp_init(uint8_t sct, uint16_t local_port);

#ifdef __cplusplus
}
#endif
#endif