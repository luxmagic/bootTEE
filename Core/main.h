#ifndef MAIN_H_
#define MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f4xx.h>
// #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>	

void blink_init(void);
void blink_run(uint32_t ms);

#define CMD_REQUEST_DATA    568301
#define CMD_RESPONSE_DATA   568302
#define CMD_RESPONSE_STATUS 568303
#define CMD_FOR_DEVICE      568304
#define CMD_FOR_WEB         568305

#ifdef __cplusplus
}
#endif
#endif