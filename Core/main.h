#ifndef MAIN_H_
#define MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f4xx.h>
#include <mbedtls/sha256.h>
#include <mbedtls/ecdsa.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>	

void blink_init(void);
void blink_run(uint32_t ms);

#ifdef __cplusplus
}
#endif
#endif