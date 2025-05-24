#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(uint32_t baudrate);
void uart_send(const uint8_t *data, uint32_t len);
int uart_receive(uint8_t *data, uint32_t len);


#ifdef __cplusplus
}
#endif
#endif