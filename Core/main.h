#ifndef MAIN_H_
#define MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f4xx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>	

void blink_init(void);
void blink_run(uint32_t ms);

#define CMD_COM     568300
#define CMD_INIT    568301
#define CMD_STATUS  568302 //568302:0 - not init status;  568302:1 - init status [568302:-] - статус не подтвержден  
#define CMD_DATA    568303
#define CMD_INFO    568304
#define CMD_START   568305

#define INFO "STM32F401CCU6:VID-PID 0483-3748:Flash 256KB:RAM 64KB:OSC 84MHz:Perif 11TIM 1ADC 3I2C 3USART 4SPI USB 2.0:Temperture Range -40C to 85C:Package UFQFPN"
#define INFO_SIZE   149

#define MAX_BUFF_SIZE 512

#ifdef __cplusplus
}
#endif
#endif