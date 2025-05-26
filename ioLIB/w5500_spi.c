#include "stm32f4xx.h"
#include "wizchip_conf.h"
#include "w5500_spi.h"
#include "stdio.h"

/**
 *@brief Helper driver to send and receive one byte through SPI
 *@param data   : 1 byte data to transmit
 *@return		: 1 byte data which is received
 */
uint8_t SPIReadWrite(uint8_t data)
{

	/*
	 * The transmit sequence begins when a byte is written in the Tx Buffer
	 * The data byte is parallel-loaded into the shift register
	 * (from the internal bus) during the first
	 * bit transmission and then shifted out serially
	 * to the MOSI pin MSB first or LSB first
	 * depending on the LSBFIRST bit in the SPI_CR1 register.
	 * The TXE flag is set on the transfer
	 * of data from the Tx Buffer to the shift register and an interrupt is generated if the TXEIE bit in
	 * the SPI_CR2 register is set.
	 */

	// check TXE(tx empty buffer) bit of the status register(SR),
	// and wait until the tx buffer becomes empty
	while(!(SPI1->SR & SPI_SR_TXE));



	// write the data to the data register to be sent
	*(__IO uint8_t*)&SPI1->DR = data;



	/*
	 * 	"Receive sequence" Page 569 of STM32F401xC reference manual
	 *
	 *	 * For the receiver,
	 *	 * when data transfer is complete:
	 *	 * The data in the shift register is transferred to the RX Buffer
	 *	 * and the RXNE flag is set
	 */

	// wait until the data from the slave is received,
	// and the RXNE flag is set
	while(!(SPI1->SR & SPI_SR_RXNE));


	// return the received byte
	return 	(*(__IO uint8_t*)&SPI1->DR);
}

/**
 *@brief Helper driver to deselect wizchip
 */
void wizchip_deselect(void)
{
	GPIOA->BSRR = GPIO_BSRR_BS4;
}

/**
 *@brief Helper driver to select wizchip
 */
void wizchip_select(void)
{
	GPIOA->BSRR = GPIO_BSRR_BR4;
}

/**
 *@brief Helper driver to read one byte through SPI
 *@return		: 1 byte data which is received
 */
uint8_t wizchip_read()
{
	// pass a dummy variable and read from the spi
	return SPIReadWrite(0);
}

/**
 *@brief Helper driver to send one byte through SPI
 *@param data   : 1 byte data to transmit
 */
void wizchip_write(uint8_t data)
{
	SPIReadWrite(data);
}

/**
 *@brief Helper driver to read burst of bytes through SPI
 *@param buff   : buffer to put the received data
 *@param len    : number of bytes to read
 */
void wizchip_read_burst(uint8_t* buff, uint16_t len)
{
	for (int i = 0; i < len; i++)
	{
		buff[i] = SPIReadWrite(0);
	}

}


/**
 *@brief Helper driver to read burst of bytes through SPI
 *@param data   : array containing data to be sent
 *@param len    : number of bytes to send
 */
void wizchip_write_burst(uint8_t* data, uint16_t len)
{
	for (int i = 0; i < len; i++)
	{
		wizchip_write(data[i]);
	}
}


/**
 *@brief initializer of PA0(RESET) and SCS(PA1)
 */

void w5500_pins_init()
{
    // Включение тактирования SPI1 и GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Настройка пинов PA5 (SCK), PA6 (MISO), PA7 (MOSI) как AF5 (SPI1)
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1);                  // Alternate function
    GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28);                                                 // AF5 для PA5, PA6, PA7
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR5_1 | GPIO_OSPEEDER_OSPEEDR6_1 | GPIO_OSPEEDER_OSPEEDR7_1); // High speed

    // Настройка PA4 как CS (GPIO Output)
    GPIOA->MODER |= GPIO_MODER_MODER4_0; // Output
    GPIOA->BSRR = GPIO_BSRR_BS4;         // CS высокий (неактивный)

    // Настройка PB0 как RST (GPIO Output)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    GPIOB->MODER |= GPIO_MODER_MODER0_0; // Output
    GPIOB->BSRR = GPIO_BSRR_BS0;         // RST высокий

    // Настройка SPI1
    SPI1->CR1 = 0;                            // Сброс настроек
    SPI1->CR1 |= SPI_CR1_MSTR |               // Мастер
                 (3 << SPI_CR1_BR_Pos) |      // Делитель 16 (80 МГц / 16 = 5 МГц, безопасно для W5500)
                 SPI_CR1_SSM | SPI_CR1_SSI |  // Программное управление CS
                 SPI_CR1_CPOL | SPI_CR1_CPHA; // Mode 3 (CPOL=1, CPHA=1)
    SPI1->CR1 |= SPI_CR1_SPE;                 // Включение SPI

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// Настройка PB1 как вход с подтяжкой вверх
	GPIOB->MODER &= ~GPIO_MODER_MODER1; // Input
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR1_0; // Pull-up

	// Настройка EXTI1 для PB1
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB; // PB1 для EXTI1
	EXTI->IMR |= EXTI_IMR_MR1; // Разрешить прерывание
	EXTI->FTSR |= EXTI_FTSR_TR1; // Падающий фронт

	// Включение прерывания в NVIC
	NVIC_SetPriority(EXTI1_IRQn, 0);
	NVIC_EnableIRQ(EXTI1_IRQn);
}

void w5500_init()
{
	// the available size of w5500 is 32KB
	// w5500 supports 8 independents sockets simultaneously
	// here 2KB is allocated for
	// each of the transmit and receive buffer of each socket
	uint8_t memory_for_each_socket[2][8] =
	{
			{2, 2, 2, 2, 2, 2, 2, 2},		// receive buffer size
			{2, 2, 2, 2, 2, 2, 2, 2}		// transmit buffer size
	};

	uint8_t tmp = 0xFF;


	// initialize RESET and SCS pins
	w5500_pins_init();

	// first deselect the chip(by setting SCS pin)
	wizchip_deselect();


	// hard reset the wiznet chip by clearing RESET
	GPIOB->BSRR = GPIO_BSRR_BR0;

	// busy wait for a while and keep the RESET pin 0
	while(tmp--);

	GPIOB->BSRR = GPIO_BSRR_BS0;
	// assign the helper drivers to complete the driver

	// callback functions for selecting and deselecting the chip
	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);

	// callback functions for reading and writing bytes of data
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);

	// callback functions for reading and writing burst of data
	reg_wizchip_spiburst_cbfunc(wizchip_read_burst, wizchip_write_burst);


	// allocate the specified memory for each socket
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*)memory_for_each_socket) == -1)
	{
		printf("WIZCHIP initialization failed\r\n");
		while(1);
	}

	wizchip_init(memory_for_each_socket[0], memory_for_each_socket[1]);

	// Включение прерываний для сокета 0
	setIMR(0xFF); // Разрешить все прерывания (CON, DISCON, RECV, TIMEOUT, SENDOK)
	setSn_IMR(0, Sn_IR_CON | Sn_IR_RECV); // Прерывания для соединения и получения данных
	setSn_IR(0, 0xFF);

	printf("WIZCHIP initialization done successfully\r\n");
}