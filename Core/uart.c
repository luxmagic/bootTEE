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

    SET_BIT(USART1->CR1, USART_CR1_RXNEIE);
    SET_BIT(USART1->CR1, USART_CR1_TXEIE);

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);
}

void compltReceive(uint8_t size, uint8_t * ptr_buff)
{
    while(USART1->SR & USART_SR_RXNE)
	{
		*ptr_buff++ = USART1->DR;
        size++;
        if (size > 50)
		{
            (void)USART1->DR;
		}
	}
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