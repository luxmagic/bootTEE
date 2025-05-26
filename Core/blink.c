#include "main.h"
#include "clock.h"

void blink_init(void)
{
    RCC->AHB1ENR	|= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER	|= GPIO_MODER_MODER13_0;
    SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS13);
}

void blink_run(uint32_t ms)
{
    SET_BIT(GPIOC->BSRR, GPIO_BSRR_BR13);
    delay_ms(ms);
    SET_BIT(GPIOC->BSRR, GPIO_BSRR_BS13);
    delay_ms(ms);
}