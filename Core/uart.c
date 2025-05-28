#include "main.h"
#include "uart.h"

#include "cipher_uart.h"

int crypto_handle_recv(
    const uint8_t *input, uint16_t input_len,
    uint8_t *response, size_t *response_len  
)
{
    // Проверка HMAC
    // uint8_t encrypted[COAP_MAX_PACKET_SIZE];
    // uint8_t hmac[32];
    // size_t encrypted_len = data[0];
    // if (encrypted_len > len - 33)
    //     return -1;
    // memcpy(encrypted, data + 1, encrypted_len);
    // memcpy(hmac, data + 1 + encrypted_len, 32);
    // if (verify_hmac(encrypted, encrypted_len, hmac) != 0)
    //     return -1;

    // Дешифрование
    uint8_t decrypted[MAX_BUFF_SIZE];
    size_t decrypted_len = sizeof(decrypted);
    if (decrypt_payload(input, input_len, decrypted, &decrypted_len) != 0)
        return -1;

    memcpy(response, decrypted, decrypted_len);
    *response_len = decrypted_len;
    return 0;
}

int crypto_handle_trns(const uint8_t *input, uint16_t input_len)
{
    //Шифрование
    uint8_t ecrypted[MAX_BUFF_SIZE] = {0,};
    size_t ecrypted_len = 0;
    if (encrypt_payload(input, input_len, ecrypted, &ecrypted_len) != 0)
        return -1;

    ecrypted[ecrypted_len] = 0x1f;

    uart_send(ecrypted, ecrypted_len+1);
    return 0;
}


void uart_init(uint32_t baudrate)
{
    // Enable RCC for  UART1 и GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Settings PA9 (TX) and PA10 (RX) how alternative function
    GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);
    GPIOA->AFR[1] |= (7 << 4) | (7 << 8); // AF7 для UART1

    USART1->BRR = SystemCoreClock / baudrate;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    SET_BIT(USART1->CR1, USART_CR1_RXNEIE);

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);
}

void uart_send(const char *data, size_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        while (!(USART1->SR & USART_SR_TXE));
        USART1->DR = data[i];
    }
}

int uart_receive(uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        while (!(USART1->SR & USART_SR_RXNE));
        data[i] = USART1->DR;
    }
    return 0;
}