#include "main.h"
#include "boot.h"
#include "flash.h"
#include <mbedTLS/sha256.h>

void configure_mpu(void)
{
    MPU->RNR = 0;                                                                                // Регион 0
    MPU->RBAR = 0x08000000;                                                                      // Начало загрузчика
    MPU->RASR = (0x3 << MPU_RASR_AP_Pos) | (0x1 << MPU_RASR_XN_Pos) | (48 << MPU_RASR_SIZE_Pos); // Только чтение, 16 КБ
    MPU->CTRL |= MPU_CTRL_ENABLE_Msk;                                                            // Включение MPU
}

// void configure_flash_protection(void)
// {
    // FLASH_OBProgramInitTypeDef ob_init;
    // HAL_FLASH_Unlock();
    // HAL_FLASH_OB_Unlock();
    // HAL_FLASHEx_OBGetConfig(&ob_init);
    // ob_init.WRPSector = OB_WRP_Sector_0 | OB_WRP_Sector_1 | OB_WRP_Sector_2;
    // HAL_FLASHEx_OBProgram(&ob_init);
    // HAL_FLASH_OB_Lock();
    // HAL_FLASH_Unlock();
// }

int validate_app(const uint8_t * restore_firm_hash)
{
    unsigned char hash[32];

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 = SHA-256

    for (uint32_t addr = APP_START; addr < (APP_START + APP_SIZE); addr += 512)
    {
        mbedtls_sha256_update(&ctx, (uint8_t *)addr, 512);
    }

    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    return memcmp(hash, restore_firm_hash, 32) == 0;
}

int first_calc_hash(const uint8_t *firm_hash)
{
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 = SHA-256

    for (uint32_t addr = APP_START; addr < (APP_START + APP_SIZE); addr += 512)
    {
        mbedtls_sha256_update(&ctx, (uint8_t *)addr, 512);
    }

    mbedtls_sha256_finish(&ctx, firm_hash);
    mbedtls_sha256_free(&ctx);

    return 0;
}

void jump_to_application(void)
{
    // Адрес начала приложения
    uint32_t app_address = APP_START;

    // Проверка вектора прерываний приложения
    if ((*(volatile uint32_t *)(app_address + 4)) != 0xFFFFFFFF)
    {
        // Отключение всех прерываний
        __disable_irq();

        // Установка указателя стека приложения
        __set_MSP(*(volatile uint32_t *)app_address);

        // Переход к функции Reset_Handler приложения
        void (*app_reset_handler)(void) = (void *)(*(volatile uint32_t *)(app_address + 4));
        app_reset_handler();
    }
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
