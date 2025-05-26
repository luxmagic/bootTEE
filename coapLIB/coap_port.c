#include "main.h"
#include "coap.h"
#include <socket.h>
#include <w5500_run.h>

#define COAP_PORT 5683
#define SERVER_IP {192, 168, 1, 101}
#define SERVER_PORT 5683

static uint8_t socket_id = 0;
static uint8_t rx_buffer[COAP_MAX_PACKET_SIZE];
static uint8_t tx_buffer[COAP_MAX_PACKET_SIZE];

void coap_init(void)
{
    udp_init(socket_id, COAP_PORT);
}

int coap_send(const uint8_t *data, size_t len, uint8_t *dest_ip, uint16_t dest_port)
{
    int32_t ret = sendto(socket_id, (uint8_t *)data, len, dest_ip, dest_port);
    return ret > 0 ? 0 : -1;
}

int coap_receive(uint8_t *data, size_t *len, uint8_t *src_ip, uint16_t *src_port)
{
    int32_t ret = recvfrom(socket_id, data, *len, src_ip, src_port);
    if (ret > 0)
    {
        *len = ret;
        return 0;
    }
    return -1;
}

int coap_send_get(const char *path)
{
    coap_packet_t packet;
    uint8_t buffer[COAP_MAX_PACKET_SIZE];
    size_t len = sizeof(buffer);

    // Инициализация пакета
    memset(&packet, 0, sizeof(coap_packet_t));
    packet.hdr.ver = COAP_VERSION;
    packet.hdr.t = COAP_TYPE_CON;
    packet.hdr.tkl = 0; // Без токена
    packet.hdr.code = COAP_METHOD_GET;
    packet.hdr.id[0] = rand() & 0xFF;
    packet.hdr.id[1] = rand() & 0xFF;

    // Опция Uri-Path
    packet.numopts = 1;
    packet.opts[0].num = COAP_OPTION_URI_PATH;
    packet.opts[0].buf.p = (const uint8_t *)path;
    packet.opts[0].buf.len = strlen(path);

    // Сериализация
    if (coap_build(buffer, &len, &packet) != 0)
    {
        return -1;
    }

    // // Шифрование
    // uint8_t encrypted[COAP_MAX_PACKET_SIZE];
    // uint8_t hmac[32];
    // size_t encrypted_len, hmac_len;
    // if (encrypt_message(buffer, len, encrypted, &encrypted_len) != 0)
    //     return -1;
    // if (compute_hmac(encrypted, encrypted_len, hmac, &hmac_len) != 0)
    //     return -1;

    // // Формирование буфера: длина + шифрованное + HMAC
    // uint8_t send_buf[COAP_MAX_PACKET_SIZE];
    // send_buf[0] = (uint8_t)encrypted_len;
    // memcpy(send_buf + 1, encrypted, encrypted_len);
    // memcpy(send_buf + 1 + encrypted_len, hmac, hmac_len);

    // Отправка
    uint8_t dest_ip[4] = SERVER_IP;
    return coap_send(buffer, len, dest_ip, SERVER_PORT);
}

int coap_send_put(const char *path, const uint8_t *payload, size_t payload_len)
{
    coap_packet_t packet;
    uint8_t buffer[COAP_MAX_PACKET_SIZE];
    size_t len = sizeof(buffer);

    // Проверка размера payload
    if (payload_len > COAP_MAX_PACKET_SIZE - 128)
        return -1; // Ограничение для безопасности

    // Инициализация пакета
    memset(&packet, 0, sizeof(coap_packet_t));
    packet.hdr.ver = COAP_VERSION;
    packet.hdr.t = COAP_TYPE_CON;
    packet.hdr.tkl = 0;
    packet.hdr.code = COAP_METHOD_PUT;
    packet.hdr.id[0] = rand() & 0xFF;
    packet.hdr.id[1] = rand() & 0xFF;

    // Опция Uri-Path
    packet.numopts = 1;
    packet.opts[0].num = COAP_OPTION_URI_PATH;
    packet.opts[0].buf.p = (const uint8_t *)path;
    packet.opts[0].buf.len = strlen(path);

    // Payload
    packet.payload.p = payload;
    packet.payload.len = payload_len;

    // Сериализация
    if (coap_build(buffer, &len, &packet) != 0)
    {
        return -1;
    }

    // // Шифрование
    // uint8_t encrypted[COAP_MAX_PACKET_SIZE];
    // uint8_t hmac[32];
    // size_t encrypted_len, hmac_len;
    // if (encrypt_message(buffer, len, encrypted, &encrypted_len) != 0)
    //     return -1;
    // if (compute_hmac(encrypted, encrypted_len, hmac, &hmac_len) != 0)
    //     return -1;

    // // Формирование буфера
    // uint8_t send_buf[COAP_MAX_PACKET_SIZE];
    // send_buf[0] = (uint8_t)encrypted_len;
    // memcpy(send_buf + 1, encrypted, encrypted_len);
    // memcpy(send_buf + 1 + encrypted_len, hmac, hmac_len);

    // Отправка
    uint8_t dest_ip[4] = SERVER_IP;
    return coap_send(buffer, len, dest_ip, SERVER_PORT);
}

int coap_handle_response(uint8_t *response, size_t *response_len)
{
    uint8_t data[COAP_MAX_PACKET_SIZE];
    size_t len = sizeof(data);
    uint8_t src_ip[4];
    uint16_t src_port;

    if (coap_receive(data, &len, src_ip, &src_port) != 0)
    {
        return -1;
    }

    // Проверка HMAC
    // uint8_t encrypted[COAP_MAX_PACKET_SIZE];
    // uint8_t hmac[32];
    // size_t encrypted_len = data[0];
    // if (encrypted_len > len - 33)
    //     return -1;
    // memcpy(encrypted, data + 1, encrypted_len);
    // memcpy(hmac, data + 1 + encrypted_len, 32);
    // if (verify_hmac(encrypted, encrypted_len, hmac) != 0)
    //     return -1;

    // // Дешифрование
    // uint8_t decrypted[COAP_MAX_PACKET_SIZE];
    // size_t decrypted_len;
    // if (decrypt_message(encrypted, encrypted_len, decrypted, &decrypted_len) != 0)
    //     return -1;

    // Парсинг CoAP
    coap_packet_t packet;
    if (coap_parse(&packet, data, len) != 0)
    {
        return -1;
    }

    // Копирование payload
    if (packet.payload.len > 0 && packet.payload.len <= *response_len)
    {
        memcpy(response, packet.payload.p, packet.payload.len);
        *response_len = packet.payload.len;
        return 0;
    }

    return -1;
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR1)
    {
        EXTI->PR = EXTI_PR_PR1;
        uint8_t socket = 0;
        uint8_t ir = getSn_IR(socket);

        if (ir & Sn_IR_RECV)
        {
            setSn_IR(socket, Sn_IR_RECV);
            uint8_t response[COAP_MAX_PACKET_SIZE];
            uint16_t response_len = sizeof(response);
            if (coap_handle_response(response, &response_len) == 0)
            {
                if (strncmp((char *)response, "CHECK_FIRMWARE_OK", response_len) == 0)
                {
                    const uint8_t payload[] = "1";
                    coap_send_put("check_firmware", payload, strlen((char *)payload));
                }
            }
        }

        if (ir & Sn_IR_TIMEOUT)
        {
            setSn_IR(socket, Sn_IR_TIMEOUT);
            coap_send_get("check/firmware");
        }
    }
}