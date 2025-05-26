#ifndef TEE_H_
#define TEE_H_

#ifdef __cplusplus
extern "C" {
#endif

void TEE_Init(void);

#define FIRMWARE_ADDR       (0x08004000)
#define FIRMWARE_SIZE       (0x10000)
#define SIGNATURE_SIZE      64

#ifdef __cplusplus
}
#endif
#endif