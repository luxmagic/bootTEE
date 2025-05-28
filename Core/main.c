#include "main.h"
#include "clock.h"

#include "uart.h"

#include "boot.h"
#include "tee.h"

uint16_t calc_len(const uint8_t * input, uint16_t input_len)
{
    uint16_t out = 0;
    for(uint16_t i = 0; i < input_len; i++)
    {
        if (input[i] == '\n')
        {
            return out;
        }
        out++;
    }
}

const uint8_t * msg = "hello";
uint8_t *ptr_buff = NULL;

uint8_t rx_buff[MAX_BUFF_SIZE] = {0,};
static uint8_t tx_buff[MAX_BUFF_SIZE] = {0,};


uint16_t size = 0;
volatile bool flag_recv = false;
volatile bool flag_trns = false;

int main(void) 
{
    ptr_buff = &rx_buff[0];
    uint32_t check_cmd = 0;
    
    uint8_t decrypted_rx[MAX_BUFF_SIZE] = {0,};
    size_t decrypted_rx_len = 0;
    size_t size_after_check = 0;

    SysTick_Config(SystemCoreClock/1000);
    blink_init();
    uart_init(115200);

    blink_run(50);
   

    while (1) 
    {
        if (flag_recv)
        {
            
            flag_recv = false;

            if (crypto_handle_recv(rx_buff, size-1, decrypted_rx, &decrypted_rx_len) == 0)
            {
                blink_run(50);
            }
            check_cmd = atoi(decrypted_rx);
            // size_after_check = calc_len(decrypted_rx, decrypted_rx_len);
            if (crypto_handle_trns(decrypted_rx, decrypted_rx_len) == 0)
            {
                blink_run(50);
            }
            check_cmd = 0;
            // size_after_check = 0;
            memset(decrypted_rx, 0, decrypted_rx_len);
            decrypted_rx_len = sizeof(decrypted_rx);
            flag_trns = true;
            ptr_buff -= size;
            size = 0;
            memset(rx_buff, 0, size);
        }
    }
}


////////////////////////////////////////////////////////////
//Handlers
////////////////////////////////////////////////////////////
void USART1_IRQHandler(void)
{
	if (USART1->SR & USART_SR_RXNE) 
	{
        CLEAR_BIT(USART1->SR, USART_SR_RXNE);
        if (USART1->DR == 0x1f)
        {
            flag_recv = true;
        }
        *(ptr_buff++) = USART1->DR;
        size++;
	}
}
