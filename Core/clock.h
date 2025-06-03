#ifndef CLOCK_H_
#define CLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void  SysTick_Handler(void);
void delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif
#endif