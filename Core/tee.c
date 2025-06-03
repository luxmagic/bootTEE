#include "main.h"
#include "tee.h"
#include "flash.h"

#include "boot.h"
#include "cipher_uart.h"

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/pk.h>
#include <mbedtls/ecp.h>
#include <mbedtls/sha256.h>
#include <mbedtls/platform_util.h>

mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ecdsa_context ecdsa;

__attribute__((section(".sec_str"))) volatile uint8_t sec_arr[1024];
uint8_t * rnd_bytes = NULL;
size_t rnd_bytes_len = 0;

void init_ecdsa(void);
int sign_data(const uint8_t *data, size_t data_len, uint8_t *sig, size_t *sig_len, uint8_t *pubkey_der, size_t *pubkey_len, mbedtls_ecdsa_context *ecdsa, mbedtls_ctr_drbg_context *ctr_drbg);
int verify_signature(const uint8_t *data, size_t data_len, const uint8_t *sig, size_t sig_len, const uint8_t *pub_key_buf, size_t pub_key_len);

static int stm32_rng(void *rng_state, unsigned char *output, size_t len);
int generate_encryption_key(uint8_t *key, size_t key_len);

static int hex_char_to_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1; // ошибка
}

static int hexstr_to_bytes(const char *hexstr, uint8_t *bytes, size_t max_bytes)
{
    size_t len = strlen(hexstr);
    if (len % 2 != 0)
        return -1;

    size_t byte_count = len / 2;
    if (byte_count > max_bytes)
        return -1;

    for (size_t i = 0; i < byte_count; i++)
    {
        int high = hex_char_to_nibble(hexstr[2 * i]);
        int low = hex_char_to_nibble(hexstr[2 * i + 1]);

        if (high < 0 || low < 0)
            return -1; // некорректный символ

        bytes[i] = (high << 4) | low;
    }

    return byte_count;
}

int parse_init_packet(const uint8_t *input, uint16_t input_len, const uint8_t *ecrypted_tx, uint16_t ecrypted_tx_len)
{
    uint16_t rnd_bytes_len = 0;
    uint8_t *rnd_bytes = NULL;
    /**
     * При первом запуске
     * 1 - проверить статус (посмотреть что не нули)
     * 2 - если не инит то сгенерить все и отправить pub и privat
     * 3 - посчитать hash у user app
     * 4 - зашифровать хэш
     * 5 - записать его
     * 6 - заблокировать этот участок flash
     * 7 - отправить статус что все ок и app run 
     * 8 - запустить приложение
    */

    /**
     * При старте
     * 1 - проверить статус
     * 2 - отправить сигнатуру
     * 3 - проверить ключ
     * 4 - расшифровать хэш
     * 5 - посчитать хэш на прошивке
     * 6 - сравнить с эталонным
     * 7 - отправить статус что все ок и app run
     * 8 - запустить приложение
    */

    uint8_t *tokens[10];
    uint16_t token_count = 0;

    uint8_t *token = strtok(input, ":");
    while (token && token_count < 10)
    {
        tokens[token_count++] = token;
        token = strtok(NULL, ":");
    }

    uint32_t cmd = atoi(tokens[0]);
    rnd_bytes_len = atoi(tokens[1]);
    rnd_bytes = tokens[2];

    uint8_t rnd_arr[rnd_bytes_len];

    int rand_bytes = hexstr_to_bytes(rnd_bytes, rnd_arr, sizeof(rnd_arr));


    rnd_bytes = malloc(rand_bytes * sizeof(uint8_t));
    memcpy(rnd_bytes, rnd_arr, rand_bytes);
    
    uint8_t rot_key[32] = {0,};
    size_t rot_key_len = 0;

    uint8_t firm_hash[32] = {0,};
    uint8_t enc_firm_hash[32] = {0,};
    size_t enc_firm_hash_len = 0;

    uint8_t signature[80];
    size_t sig_len = 0;
    uint8_t pubkey_der[100];
    size_t pubkey_der_len = 0;

    int ret;
    ret = generate_encryption_key(rot_key, rot_key_len);

    ret = first_calc_hash(firm_hash); //1.

    ret = enc_hash(firm_hash, 32, enc_firm_hash, enc_firm_hash_len); //2.

    init_ecdsa();

    ret = sign_data(enc_firm_hash, enc_firm_hash_len, signature, sig_len, pubkey_der, pubkey_der_len, &ecdsa, &ctr_drbg); //.3

    uint8_t status = 1;
    char separate = ':';
    memcpy(sec_arr, status, 1);
    memcpy((sec_arr+1), separate, 1);
    memcpy((sec_arr+2), rot_key, rot_key_len);
    memcpy((sec_arr+2+rot_key_len), separate, 1);
    memcpy((sec_arr+3+rot_key_len), pubkey_der, pubkey_der_len);
    memcpy((sec_arr+3+rot_key_len+pubkey_der_len), separate, 1);
    memcpy((sec_arr+6+rot_key_len+pubkey_der_len), enc_firm_hash, enc_firm_hash_len); //.4

    memcpy(ecrypted_tx+ecrypted_tx_len, signature, sig_len); //.5

    return ret;
    //1. считать хэш пользовательской прошивки
    //2. вернуть сюда этот хэш и зашифровать его
    //3. подписать этот хэш
    //4. уложить новый статус, ключ, публичный ключ и зашифрованный хэш в sec_arr
    //5. вернуть сообщение с сигнатурой
    //6. запустить приложение
}

int parse_status_packet(const uint8_t *input, uint16_t input_len, const uint8_t *ecrypted_tx, uint16_t ecrypted_tx_len)
{
    uint16_t checked_status = 0;

    uint8_t *tokens[10];
    uint16_t token_count = 0;

    uint8_t *token = strtok(input, ":");
    while (token && token_count < 10)
    {
        tokens[token_count++] = token;
        token = strtok(NULL, ":");
    }

    uint32_t cmd = atoi(tokens[0]);
    checked_status = atoi(tokens[1]);

    if (checked_status == 0)
    {
        if (sec_arr[0] == 1)
        {
            //уже инициализирован
            char *status = "1";
            memcpy(ecrypted_tx+ecrypted_tx_len, status, strlen(status));
            return 1;
        }
        if (sec_arr[0] == 0xff)
        {
            //не инициализирован
            char *status = "0";
            memcpy(ecrypted_tx+ecrypted_tx_len, status, strlen(status));
            return 0;
        }
    }
}

int parse_secure(const uint8_t *input, uint16_t input_len, const uint8_t *ecrypted_tx, uint16_t ecrypted_tx_len)
{
    uint8_t *tokens[10];
    uint16_t token_count = 0;

    uint8_t *rot_key = NULL;
    uint8_t *sign = NULL;
    uint8_t *pub_key = NULL;
    uint8_t *enc_hash= NULL;

    size_t rot_key_len = 32;
    size_t sign_len = 80;
    size_t pub_key_len = 100;
    size_t enc_hash_len = 32;

    uint8_t *token = strtok(input, ":");
    while (token && token_count < 10)
    {
        tokens[token_count++] = token;
        token = strtok(NULL, ":");
    }

    uint32_t cmd = atoi(tokens[0]);
    sign = tokens[1];

    token_count = 0;
    token = NULL;

    token = strtok(sec_arr, ":");
    while (token && token_count < 10)
    {
        tokens[token_count++] = token;
        token = strtok(NULL, ":");
    }

    uint8_t status = atoi(tokens[0]);
    rot_key = tokens[1];
    pub_key = tokens[2];
    enc_hash = tokens[3];

    uint8_t dec_firm_hash[32] = {0,};
    size_t dec_firm_hash_len = 0;


    int ret;

    ret = verify_signature(enc_hash, enc_hash_len, sign, sign_len, pub_key, pub_key_len);

    ret = dec_hash(enc_hash, enc_hash_len, dec_firm_hash_len, dec_firm_hash_len);

    ret = validate_app(dec_firm_hash);

    char *out = "0";
    memcpy(ecrypted_tx+ecrypted_tx_len, out, strlen(out));

    return ret;
}

int enc_hash(const uint8_t *input, uint16_t input_len, uint8_t *request, size_t *request_len)
{
    uint8_t ecrypted[MAX_BUFF_SIZE] = {0,};
    size_t ecrypted_len = 0;
    if (encrypt_payload(input, input_len, ecrypted, &ecrypted_len) != 0)
        return -1;
    
    memcpy(request, ecrypted, ecrypted_len);
    *request_len = ecrypted_len;
    return 0;
}

int dec_hash(const uint8_t *input, uint16_t input_len, uint8_t *response, size_t *response_len)
{
    uint8_t decrypted[MAX_BUFF_SIZE];
    size_t decrypted_len = sizeof(decrypted);
    if (decrypt_payload(input, input_len, decrypted, &decrypted_len) != 0)
        return -1;

    memcpy(response, decrypted, decrypted_len);
    *response_len = decrypted_len;
    return 0;
}

int verify_hash(void)
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

    // return 0;
}

static int stm32_rng(void *rng_state, unsigned char *output, size_t len)
{
    (void)rng_state; // Unused
    if (len > 0)
    {
        // получить данные из uart -> дешифровать -> закинуть сюда
        memcpy(output, rnd_bytes, rnd_bytes_len);
    }
    return 0;
}

int generate_encryption_key(uint8_t *key, size_t key_len)
{
    // if (key_len != 32)
    // {
    //     return -1;
    // }

    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    const char *personalization = "STM32F401_TEE_KEYGEN";
    int ret;

    // Initialize entropy and CTR-DRBG
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    // Seed the RNG with STM32 hardware RNG
    ret = mbedtls_ctr_drbg_seed(&ctr_drbg, stm32_rng, NULL,
                                (const unsigned char *)personalization,
                                strlen(personalization));
    if (ret != 0)
    {
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        return ret;
    }

    // Generate 32-byte key
    ret = mbedtls_ctr_drbg_random(&ctr_drbg, key, key_len);

    // Clean up
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret;
}

void init_ecdsa(void)
{
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ecdsa_init(&ecdsa);

    const char *pers = "ecdsa";
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                          (const unsigned char *)pers, strlen(pers));

    mbedtls_ecdsa_genkey(&ecdsa, MBEDTLS_ECP_DP_SECP256R1,
                         mbedtls_ctr_drbg_random, &ctr_drbg);
}

static int ecdsa_read_public_key_from_der(mbedtls_ecdsa_context *ecdsa,
    const uint8_t *pub_key_buf,
    size_t pub_key_len
)
{
    int ret;
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    // Считать ключ из DER
    ret = mbedtls_pk_parse_public_key(&pk, pub_key_buf, pub_key_len);
    if (ret != 0)
    {
        mbedtls_pk_free(&pk);
        return ret;
    }

    // Проверка, что это ECDSA ключ
    if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_ECDSA))
    {
        mbedtls_pk_free(&pk);
        return MBEDTLS_ERR_PK_TYPE_MISMATCH;
    }

    // Инициализировать ecdsa из pk
    ret = mbedtls_ecp_group_copy(&ecdsa->private_grp, &mbedtls_pk_ec(pk)->private_grp);
    if (ret != 0)
        goto cleanup;

    ret = mbedtls_ecp_copy(&ecdsa->private_Q, &mbedtls_pk_ec(pk)->private_Q);
    if (ret != 0)
        goto cleanup;

cleanup:
    mbedtls_pk_free(&pk);
    return ret;
}

static int write_public_key_der(mbedtls_ecdsa_context *ecdsa,
    uint8_t *out_buf,
    size_t out_buf_size
)
{
    int ret;
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    // Оборачиваем ecdsa в pk
    ret = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0)
        return ret;

    // Копируем ecdsa в pk.ec
    *(mbedtls_ecp_keypair *)pk.private_pk_ctx = *ecdsa;

    // DER будет записан в конец буфера, поэтому нужен сдвиг
    ret = mbedtls_pk_write_pubkey_der(&pk, out_buf, out_buf_size);

    // Восстановим владение памятью обратно, иначе произойдёт двойное освобождение
    *(mbedtls_ecp_keypair *)pk.private_pk_ctx = (mbedtls_ecp_keypair){0};

    mbedtls_pk_free(&pk);
    return ret; // Возвращается offset в буфере, т.е. сдвинутая позиция
}

int sign_data(const uint8_t *data, size_t data_len,
        uint8_t *sig, size_t *sig_len,
        uint8_t *pubkey_der, size_t *pubkey_len,
        mbedtls_ecdsa_context *ecdsa,
        mbedtls_ctr_drbg_context *ctr_drbg
)
{
    uint8_t hash[32];
    int ret;

    mbedtls_sha256(data, data_len, hash, 0);

    ret = mbedtls_ecdsa_write_signature(
        ecdsa, MBEDTLS_MD_SHA256,
        hash, sizeof(hash),
        sig, 64, sig_len,
        mbedtls_ctr_drbg_random, ctr_drbg);
    

    if (ret != 0) return ret;

    // 3. Экспорт публичного ключа в DER формате
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    // Связать существующий ecdsa с pk
    ret = mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    if (ret != 0) goto cleanup;

    // Копируем ключ в pk контекст
    ret = mbedtls_ecp_copy(&mbedtls_pk_ec(pk)->private_Q, &ecdsa->private_Q);
    if (ret != 0) goto cleanup;

    ret = mbedtls_ecp_group_copy(&mbedtls_pk_ec(pk)->private_grp, &ecdsa->private_grp);
    if (ret != 0) goto cleanup;

    // Сохраняем публичный ключ в DER формате
    unsigned char buf[120]; // Временный буфер
    unsigned char *p = buf + sizeof(buf);
    size_t len = 0;

    ret = mbedtls_pk_write_pubkey_der(&pk, buf, sizeof(buf));
    if (ret < 0) goto cleanup;

    len = ret;
    memcpy(pubkey_der, buf + sizeof(buf) - len, len);
    *pubkey_len = len;

cleanup:
    mbedtls_pk_free(&pk);
    mbedtls_platform_zeroize(hash, sizeof(hash));
    return ret;
}

int verify_signature(const uint8_t *data, size_t data_len,
    const uint8_t *sig, size_t sig_len,
    const uint8_t *pub_key_buf, size_t pub_key_len
)
{
    int ret;
    uint8_t hash[32];

    mbedtls_ecdsa_context ecdsa;
    mbedtls_ecdsa_init(&ecdsa);

    // Вычисляем хеш от данных
    mbedtls_sha256(data, data_len, hash, 0);

    ret = ecdsa_read_public_key_from_der(&ecdsa, pub_key_buf, pub_key_len);
    if (ret != 0)
        goto cleanup;

    // Проверка подписи
    ret = mbedtls_ecdsa_read_signature(&ecdsa, hash, sizeof(hash), sig, sig_len);

cleanup:
    mbedtls_ecdsa_free(&ecdsa);
    mbedtls_platform_zeroize(hash, sizeof(hash));
    return ret;
}