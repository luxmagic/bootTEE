#include "main.h"
#include "tee.h"

typedef struct
{
    uint32_t magic;
    uint8_t cert[32];
} TEE_TrustRoot;

__attribute__((section(".kms"))) static const TEE_TrustRoot tee_root =
{
    .magic = 0x54524545,
    .cert = {0x01, 0x02, 0x03}
};

__attribute__((section(".kms")))
const uint8_t root_of_trust_key[32] = {};

void TEE_Init(void)
{
    if (tee_root.magic != 0x54524545)
    {
        while (1); // повреждённая область TEE
    }
}

int verify_firmware(void)
{
    // uint8_t hash[32];
    // uint8_t signature[SIGNATURE_SIZE] = {/* Подпись из UART или Flash */};
    // mbedtls_sha256_context sha_ctx;
    // mbedtls_ecdsa_context ecdsa_ctx;

    // // Инициализация
    // mbedtls_sha256_init(&sha_ctx);
    // mbedtls_ecdsa_init(&ecdsa_ctx);

    // // Вычисление SHA-256 хэша прошивки
    // mbedtls_sha256_starts(&sha_ctx, 0); // 0 = SHA-256
    // mbedtls_sha256_update(&sha_ctx, (const uint8_t *)FIRMWARE_ADDR, FIRMWARE_SIZE);
    // mbedtls_sha256_finish(&sha_ctx, hash);

    // // Загрузка публичного ключа (из RoT)
    // const uint8_t *public_key = root_of_trust_key; // Укажите ваш ключ
    // mbedtls_ecdsa_read_pubkey(&ecdsa_ctx, public_key, sizeof(root_of_trust_key));

    // // Проверка подписи
    // int ret = mbedtls_ecdsa_verify(&ecdsa_ctx, hash, sizeof(hash), signature, &SIGNATURE_SIZE);
    // if (ret != 0)
    // {
    //     return -1; // Проверка не пройдена
    // }

    // // Создание новой подписи (если требуется)
    // uint8_t new_signature[SIGNATURE_SIZE];
    // ret = mbedtls_ecdsa_write_signature(&ecdsa_ctx, hash, sizeof(hash), new_signature, sizeof(new_signature), &SIGNATURE_SIZE, NULL, NULL);
    // if (ret != 0)
    // {
    //     return -1;
    // }

    // return 0; // Успех
}


void check_sign_firmware(void)
{
    if (verify_firmware() == 0) 
    {
        uart_send("Environment secure. Starting application.\n", 40);
        jump_to_application();
    } 
    else
    {
        uart_send("Verification failed!\n", 20);
        while (1);
    }
}