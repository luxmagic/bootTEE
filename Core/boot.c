#include "main.h"
#include "boot.h"

void configure_mpu(void)
{
    MPU->RNR = 0;                                                                                // Регион 0
    MPU->RBAR = 0x08000000;                                                                      // Начало загрузчика
    MPU->RASR = (0x3 << MPU_RASR_AP_Pos) | (0x1 << MPU_RASR_XN_Pos) | (16 << MPU_RASR_SIZE_Pos); // Только чтение, 16 КБ
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk;                                                            // Включение MPU
}

__attribute__((section(".boot"))) void Bootloader_Main(void)
{
    uint32_t app_start_address = 0x08010000;
    uint32_t app_reset_vector = *(volatile uint32_t *)(app_start_address + 4);

    if (validate_app(app_start_address))
    {
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
    for (uint32_t *p = (uint32_t *)base_addr; p < (uint32_t *)(base_addr + 0x1000 - 4); ++p)
    {
        calc_crc ^= *p; // Простейшая проверка — XOR
    }
    return calc_crc == expected_crc;
}

void jump_to_application(void)
{
    // // Адрес начала приложения
    // uint32_t app_address = 0x08004000;

    // // Проверка вектора прерываний приложения
    // if ((*(volatile uint32_t *)(app_address + 4)) != 0xFFFFFFFF)
    // {
    //     // Отключение всех прерываний
    //     __disable_irq();

    //     // Установка указателя стека приложения
    //     __set_MSP(*(volatile uint32_t *)app_address);

    //     // Переход к функции Reset_Handler приложения
    //     void (*app_reset_handler)(void) = (void *)(*(volatile uint32_t *)(app_address + 4));
    //     app_reset_handler();
    // }
}

void check_flash_protection(void)
{
    // if (FLASH->OPTCR1 & FLASH_OPTCR1_nWRP_0)
    // {
        // Сектор 0 защищён
        blink_run(50);
    // }
    // else
    // {
    //     // Ошибка: защита не активна
        
    //     blink_run(1000);
    //     while(1);
    // }
}
