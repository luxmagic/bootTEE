#ifndef FLASH_H_
#define FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SECURE  ((uint32_t)0x0803F000)
#define MAGIC   ((uint32_t)0x1FFF7800)

#define APP_START ((uint32_t)0x0803F004)
#define APP_SIZE  0xC000


void flash_unlock(void);
void flash_lock(void);
void flash_erase_sector(void);
void flash_write(uint32_t address, const uint8_t *data, size_t len);
void flash_read(uint32_t addr, uint32_t *addr_read);

void write_to_secure_flash(uint32_t addr, const uint8_t *data, size_t len);
void read_from_secure_flash(uint32_t addr, uint8_t *buffer, size_t length);


#ifdef __cplusplus
}
#endif
#endif