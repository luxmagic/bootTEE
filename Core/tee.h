#ifndef TEE_H_
#define TEE_H_

#ifdef __cplusplus
extern "C" {
#endif

void chek_tee(void);

int parse_init_packet(const uint8_t *input, uint16_t input_len, const uint8_t *ecrypted_tx, uint16_t ecrypted_tx_len);
int parse_status_packet(const uint8_t *input, uint16_t input_len, const uint8_t *ecrypted_tx, uint16_t ecrypted_tx_len);
int parse_secure(const uint8_t *input, uint16_t input_len, const uint8_t *ecrypted_tx, uint16_t ecrypted_tx_len);

int enc_hash(const uint8_t *input, uint16_t input_len, uint8_t *request, size_t *request_len);
int dec_hash(const uint8_t *input, uint16_t input_len, uint8_t *response, size_t *response_len);

#ifdef __cplusplus
}
#endif
#endif