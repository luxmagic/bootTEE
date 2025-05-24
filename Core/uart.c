#include "main.h"
#include "uart.h"

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
}

void uart_send(const uint8_t *data, uint32_t len)
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