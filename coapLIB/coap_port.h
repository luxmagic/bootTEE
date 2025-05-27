#ifndef COAP_PORT_H_
#define COAP_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

void coap_init(void);
int coap_send(const uint8_t *data, size_t len, uint8_t *dest_ip, uint16_t dest_port);
int coap_receive(uint8_t *data, size_t *len, uint8_t *src_ip, uint16_t *src_port);
int coap_send_get(const char *path_1, const char *path_2);
int coap_send_put(const char *path_1, const char *path_2, const uint8_t *payload, size_t payload_len);
int coap_handle_response(uint8_t *response, size_t *response_len);

#define COAP_MAX_PACKET_SIZE 256

#ifdef __cplusplus
}
#endif
#endif