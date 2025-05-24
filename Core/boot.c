#include "main.h"

__attribute__((section(".boot")))
void Bootloader_Main(void) 
{
    uint32_t app_start_address = 0x08010000;
    uint32_t app_reset_vector = *(volatile uint32_t *)(app_start_address + 4);

    if (validate_app(app_start_address)) {
        void (*app_entry)(void) = (void (*)(void))app_reset_vector;
        __disable_irq();
        __set_MSP(*(volatile uint32_t *)app_start_address);
        app_entry();
    }

    while (1); // Ожидание отладки или перезапуска
}

int validate_app(uint32_t base_addr) 
{
    uint32_t expected_crc = *(uint32_t *)(base_addr + 0x1000 - 4); // CRC в конце
    uint32_t calc_crc = 0;
    for (uint32_t *p = (uint32_t *)base_addr; p < (uint32_t *)(base_addr + 0x1000 - 4); ++p) {
        calc_crc ^= *p; // Простейшая проверка — XOR
    }
    return calc_crc == expected_crc;
}