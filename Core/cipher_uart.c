#include "main.h"
#include <mbedTLS/gcm.h>
// #include <mbedTLS/sha256.h>

#include "cipher_uart.h"

#define COAP_MAX_PACKET_SIZE 256

#define AES_KEY "helloFriend2025!helloFriend2025!"
#define NONCE_LEN 12
#define TAG_LEN 16

int encrypt_payload(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len)
{
    mbedtls_gcm_context gcm;
    uint8_t nonce[NONCE_LEN];
    uint8_t tag[TAG_LEN];
    int ret;

    // Проверка входных параметров
    if (!input || !output || !output_len) {
        return -1;
    }

    // Проверка достаточности размера выходного буфера
    // if (*output_len < NONCE_LEN + input_len + TAG_LEN) {
    //     return -1;
    // }

    // Инициализация контекста GCM
    mbedtls_gcm_init(&gcm);

    // Генерация случайного nonce
    for (size_t i = 0; i < NONCE_LEN; i++) {
        nonce[i] = (uint8_t)(rand() & 0xFF); // Предполагается, что rand() инициализирован
    }

    // Установка ключа (AES_KEY должен быть определен, 256 бит = 32 байта)
    ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, (const unsigned char *)AES_KEY, 256);
    if (ret != 0) {
        mbedtls_gcm_free(&gcm);
        return -1;
    }

    // Шифрование данных
    ret = mbedtls_gcm_crypt_and_tag(
        &gcm,                      // Контекст GCM
        MBEDTLS_GCM_ENCRYPT,       // Режим шифрования
        input_len,                 // Длина входных данных
        nonce, NONCE_LEN,          // Nonce и его длина
        NULL, 0,                   // Нет дополнительных данных (AAD)
        input,                     // Входные данные
        output + NONCE_LEN,        // Выходной буфер (после nonce)
        TAG_LEN,                   // Длина тега
        tag                        // Буфер для тега
    );

    if (ret != 0) {
        mbedtls_gcm_free(&gcm);
        return -1;
    }

    // Формирование выходного пакета: nonce + ciphertext + tag
    memcpy(output, nonce, NONCE_LEN);
    if (input_len > 0) {
        // Копируем зашифрованный текст, если он есть
        // (уже записан в output + NONCE_LEN во время шифрования)
    }
    memcpy(output + NONCE_LEN + input_len, tag, TAG_LEN);

    // Установка итоговой длины пакета
    *output_len = NONCE_LEN + input_len + TAG_LEN;

    // Очистка контекста
    mbedtls_gcm_free(&gcm);
    return 0;
}

int decrypt_payload(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len)
{
    // if (input_len < NONCE_LEN + TAG_LEN)
    //     return -1;

    mbedtls_gcm_context gcm;
    size_t ciphertext_len = input_len - NONCE_LEN - TAG_LEN;

    if (*output_len < ciphertext_len)
        return -1;

    const uint8_t *nonce = input;
    const uint8_t *ciphertext = input + NONCE_LEN;
    const uint8_t *tag = input + NONCE_LEN + ciphertext_len;

    mbedtls_gcm_init(&gcm);
    int ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, (const unsigned char *)AES_KEY, 256);
    if (ret != 0)
    {
        mbedtls_gcm_free(&gcm);
        return -1;
    }

    // Decrypt and verify
    ret = mbedtls_gcm_auth_decrypt(&gcm, ciphertext_len,
                                   nonce, NONCE_LEN, NULL, 0, // No additional data
                                   tag, TAG_LEN, ciphertext, output);
    mbedtls_gcm_free(&gcm);

    // if (ret != 0)
    //     return -1;
    *output_len = ciphertext_len;
    return 0;
}

// int compute_hmac(const uint8_t *data, size_t data_len, uint8_t *hmac, size_t *hmac_len)
// {
//     if (*hmac_len < 32)
//         return -1;

//     mbedtls_sha256_context sha;
//     mbedtls_sha256_init(&sha);
//     mbedtls_sha256_starts(&sha, 0); // 0 for SHA-256
//     // mbedtls_sha256_update(&sha, (const unsigned char *)HMAC_KEY, strlen(HMAC_KEY));
//     mbedtls_sha256_update(&sha, data, data_len);
//     mbedtls_sha256_finish(&sha, hmac);
//     mbedtls_sha256_free(&sha);

//     *hmac_len = 32;
//     return 0;
// }

// int verify_hmac(const uint8_t *data, size_t data_len, const uint8_t *expected_hmac)
// {
//     uint8_t computed_hmac[32];
//     size_t hmac_len = 32;
//     if (compute_hmac(data, data_len, computed_hmac, &hmac_len) != 0)
//         return -1;
//     return memcmp(computed_hmac, expected_hmac, 32) == 0 ? 0 : -1;
// }