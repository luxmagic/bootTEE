#include "main.h"
#include "flash.h"

void flash_unlock(void)
{
    if (FLASH->CR & FLASH_CR_LOCK)
    {
        FLASH->KEYR = 0x45670123;
        FLASH->KEYR = 0xCDEF89AB;
    }
}

void flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

void flash_erase_sector(void)
{
    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR &= ~FLASH_CR_SNB;
    FLASH->CR |= FLASH_CR_SER | (5 << FLASH_CR_SNB_Pos); // Sector 5 (0x08020000–0x0803FFFF)
    FLASH->CR |= FLASH_CR_STRT;
    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR &= ~FLASH_CR_SER; 
}

void flash_write(uint32_t addr, const uint8_t *data, size_t len)
{
    FLASH->CR |= FLASH_CR_PG;
    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_CR_PSIZE_0;

    if (len % 2 != 0) len++;

    for (size_t i = 0; i < len; i+=2)
    {
      
        uint16_t hw = data[i];
        if (i + 1 < len) 
            hw |= (data[i + 1] << 8);
        else 
            hw |= 0xFF00;

        while (FLASH->SR & FLASH_SR_BSY);
        *(volatile uint16_t *)(addr + i) = hw;
        while (FLASH->SR & FLASH_SR_BSY);
    }
    FLASH->CR &= ~FLASH_CR_PG;
}

void flash_read(uint32_t addr, uint32_t *addr_read)
{
    uint32_t data;
	uint32_t counter = 0;
  
	while (1)
	{
        data = *(volatile uint32_t *)addr;
        if (data == 0xffffffff)
		{
            if(counter == 0) return 0;
            else
			{
                addr -= 4*counter;
                return counter;
            }
        }  
        else
		{                         
            *addr_read = data;    
            counter++;
            addr += 4;
            addr_read++;               
		}
	}
}

void write_to_secure_flash(uint32_t addr, const uint8_t *data, size_t len)
{
    flash_unlock();
    flash_erase_sector();
    flash_write(addr, data, len);
    flash_lock();
}

void read_from_secure_flash(uint32_t addr, uint8_t *buffer, size_t length)
{
    flash_read(addr, buffer);
}

// void read_from_secure_flash(uint32_t addr, uint8_t *buffer, size_t length)
// {
//     const uint8_t *flash_ptr = (const uint8_t *)addr;
//     memcpy(buffer, flash_ptr, length);
// }