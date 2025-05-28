#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(uint32_t baudrate);
void uart_send(const char *data, size_t len);
int uart_receive(uint8_t *data, uint32_t len);

int crypto_handle_trns(const uint8_t *input, uint16_t input_len);
int crypto_handle_recv(const uint8_t *  input, uint16_t input_len, uint8_t *response, size_t *response_len);
uint32_t buff_to_number(const uint8_t * input, uint16_t input_len);

#define MAX_BUFF_SIZE 256

#ifdef __cplusplus
}
#endif
#endif