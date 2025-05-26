#include "main.h"

void ext_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// Настройка PB1 как вход с подтяжкой вверх
	GPIOB->MODER &= ~GPIO_MODER_MODER1; // Input
	GPIOB->PUPDR |= GPIO_PUPDR_PUPDR1_0; // Pull-up

	// Настройка EXTI1 для PB1
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI1_PB; // PB1 для EXTI1
	EXTI->IMR |= EXTI_IMR_MR1; // Разрешить прерывание
	EXTI->FTSR |= EXTI_FTSR_TR1; // Падающий фронт

	// Включение прерывания в NVIC
	NVIC_SetPriority(EXTI1_IRQn, 0);
	NVIC_EnableIRQ(EXTI1_IRQn);
}