#include "main.h"
#include <mbedTLS/gcm.h>

#include "cipher_uart.h"
#include "key.h"

#define NONCE_LEN 12
#define TAG_LEN 16

int encrypt_payload(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len)
{
    mbedtls_gcm_context gcm;
    uint8_t nonce[NONCE_LEN];
    uint8_t tag[TAG_LEN];
    int ret;

    if (!input || !output || !output_len) {
        return -1;
    }

    mbedtls_gcm_init(&gcm);

    for (size_t i = 0; i < NONCE_LEN; i++) {
        nonce[i] = (uint8_t)(rand() & 0xFF);
    }

    ret = mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, (const unsigned char *)AES_KEY, 256);
    if (ret != 0) {
        mbedtls_gcm_free(&gcm);
        return -1;
    }

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

    memcpy(output, nonce, NONCE_LEN);
    memcpy(output + NONCE_LEN + input_len, tag, TAG_LEN);
    *output_len = NONCE_LEN + input_len + TAG_LEN;

    mbedtls_gcm_free(&gcm);
    return 0;
}

int decrypt_payload(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len)
{
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

    ret = mbedtls_gcm_auth_decrypt(&gcm, ciphertext_len,
                                   nonce, NONCE_LEN, NULL, 0,
                                   tag, TAG_LEN, ciphertext, output);
    mbedtls_gcm_free(&gcm);

    if (ret != 0)
        return -1;
    *output_len = ciphertext_len;
    return 0;
}