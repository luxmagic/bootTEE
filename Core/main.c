#include "main.h"
#include "clock.h"
#include "flash.h"

#include "uart.h"

#include "boot.h"
#include "tee.h"

uint8_t rx_buff[MAX_BUFF_SIZE];
uint8_t *ptr_buff = &rx_buff[0];

uint8_t decrypted_rx[MAX_BUFF_SIZE];
size_t decrypted_rx_len = 0;

uint16_t size = 0;
volatile bool flag_recv_1 = false;
volatile bool flag_recv_2 = false;

int main(void) 
{
    SysTick_Config(SystemCoreClock/1000);
    blink_init();
    uart_init(115200);

    blink_run(50);

    uint8_t get_magic[4] = {0,};
    const uint8_t magic[4] = {0x33, 0x64, 0x01, 0x10};
    // write_to_secure_flash(SECURE, msg, strlen(msg));
    read_from_secure_flash(MAGIC, get_magic, 4);
    if (strcmp(get_magic, magic) != 0)
    {
        //сообщение о том что устройство скомпроментировано
        while(1);
    }
    else 
    {
        //все ок, продолжаем дальнейшие проверки
        while (1) 
        {
            if (flag_recv_2)
            {
                flag_recv_2 = false;
                flag_recv_1 = false;

                if (crypto_handle_recv(rx_buff, size-2, decrypted_rx, &decrypted_rx_len) == 0)
                {
                    blink_run(50);
                    if (handle_cmd(decrypted_rx, decrypted_rx_len))
                    {
                        blink_run(50);
                    }
                }
                memset(decrypted_rx, 0, decrypted_rx_len);
                decrypted_rx_len = sizeof(decrypted_rx);

                ptr_buff -= size;
                size = 0;
                memset(rx_buff, 0, size);
            }
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
        // CLEAR_BIT(USART1->SR, USART_SR_RXNE);
        if (flag_recv_1 & USART1->DR == '\n')
        {
            flag_recv_2 = true;
        }
        if (USART1->DR == '\r')
        {
            flag_recv_1 = true;
        }
        *(ptr_buff++) = USART1->DR;
        size++;
	}
}
