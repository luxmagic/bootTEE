#ifndef CIPHER_COAP_H_
#define CIPHER_COAP_H_

#ifdef __cplusplus
extern "C" {
#endif

int encrypt_payload(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len);
int decrypt_payload(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len);
int compute_hmac(const uint8_t *data, size_t data_len, uint8_t *hmac, size_t *hmac_len);
int verify_hmac(const uint8_t *data, size_t data_len, const uint8_t *expected_hmac);

#ifdef __cplusplus
}
#endif
#endif