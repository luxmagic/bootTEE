#ifndef BOOT_H_
#define BOOT_H_

#ifdef __cplusplus
extern "C" {
#endif

void check_flash_protection(void);

int validate_app(const uint8_t * restore_firm_hash);
void jump_to_application(void);
int first_calc_hash(const uint8_t *firm_hash);


#ifdef __cplusplus
}
#endif
#endif