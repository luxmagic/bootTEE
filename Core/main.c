#include "main.h"
#include "clock.h"
#include "uart.h"

const char * msg = "hello";

int main(void) 
{
    SysTick_Config(SystemCoreClock/1000);
    
    RCC->AHB1ENR	|= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER	|= GPIO_MODER_MODER13_0;    
    GPIOC->OTYPER   &= ~GPIO_OTYPER_OT13;
    GPIOC->PUPDR    |= GPIO_PUPDR_PUPD13_1;

    uart_init(115200);

    uint8_t msg_rx[50] = {0,};
    uint8_t msg_tx[50] = {0,};

    while (1) 
    {
        SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS13);
        // GPIOC->BSRR = GPIO_BSRR_BS13;
        delay_ms(1000);
        uart_receive(msg_rx, 50);
        memcpy(msg_tx, msg_rx, sizeof(msg_rx));
        uart_send(msg_tx, sizeof(msg_tx));
        SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR13);
        // GPIOC->BSRR = GPIO_BSRR_BR13;
        delay_ms(1000);
    }
}