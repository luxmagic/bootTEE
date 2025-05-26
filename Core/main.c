#include "main.h"
#include "clock.h"

#include "uart.h"
#include "w5500_run.h"
#include <coap_port.h>

#include "boot.h"
#include "tee.h"

const uint8_t * msg = "hello";
uint8_t *ptr_buff = NULL;
bool flag_receive = false;
bool flag_transmit = false;
volatile uint8_t  size = 0;

int main(void) 
{
    ptr_buff = malloc(20*sizeof(uint8_t));
    
    SysTick_Config(SystemCoreClock/1000);
    blink_init();
    // uart_init(115200);
    w5500_network_init();

    coap_init();
    coap_send_get("check_firmware");
    
    while (1) 
    {
        

        if (flag_receive)
        {
            // compltReceive(size, ptr_buff);
            flag_receive = false;
            blink_run(100);

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
        if (USART1->DR == 0x1f)
        {
            flag_receive = true;
        }
        *(ptr_buff++) = USART1->DR;
        size++;
	}
    if (USART1->SR & USART_SR_TXE)
    {
        flag_transmit = true;
    }
}