#include "stm32f4xx.h"
#include "tee.h"

typedef struct {
    uint32_t magic;
    uint8_t cert[32];
} TEE_TrustRoot;

__attribute__((section(".tee"))) static const TEE_TrustRoot tee_root = 
{
    .magic = 0x54524545,            // "TREE"
    .cert = {0x01, 0x02, 0x03}      // и т.д.
};

void TEE_Init(void) 
{
    if (tee_root.magic != 0x54524545) {
        while (1); // повреждённая область TEE
    }
}