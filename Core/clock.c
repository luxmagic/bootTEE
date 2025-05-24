#include <stdint.h>
#include "main.h"
#include "clock.h"

extern uint32_t SystemCoreClock;
volatile uint32_t  ticks = 0;

void  SysTick_Handler(void)
{
	ticks++;
}

void delay_ms(uint32_t ms)
{
	uint32_t started = ticks;
	while((ticks-started)<=ms);
}