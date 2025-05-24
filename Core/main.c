#include "main.h"

int main(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;     // Включить тактирование GPIOA
    GPIOC->MODER |= (1 << 10);               // PA5 — выход

    while (1) {
        GPIOC->ODR ^= (1 << 5);              // Переключить светодиод
        for (volatile int i = 0; i < 100000; i++); // Задержка
    }
}