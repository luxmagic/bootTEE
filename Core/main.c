#include "main.h"
#include "clock.h"

#include "uart.h"
#include "w5500_run.h"
#include <coap_port.h>

#include "boot.h"
#include "tee.h"

uint8_t response_msg[COAP_MAX_PACKET_SIZE] = {0,};
uint16_t response_len = 0;
volatile bool flag_recv = false;

int main(void) 
{
    
    SysTick_Config(SystemCoreClock/1000);
    blink_init();
    // uart_init(115200);
    w5500_network_init();

    coap_init();
    coap_send_get("iot", "cmd");
    // response_len = sizeof(response_msg);
    // if (response_len > 0)
    // {
    //     coap_send_put("iot", "cmd", response_msg, response_len);
    // }
    // uart_send(response_msg, response_len);

    while (1) 
    {
        if (flag_recv)
        {
            flag_recv = false;
            coap_send_put("iot", "cmd", response_msg, response_len);
            response_len = 0;
        }
    }
}