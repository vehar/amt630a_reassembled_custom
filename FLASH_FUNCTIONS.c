#include "flash.h"
#include "hardware.h" // This header must define your flash–related registers.
#include "delay.h"    // For firm_wait_a_milliseconds()

// Dummy definitions for flash–related registers (adjust addresses as needed)
#define IO_SPI_status_ready (*(volatile uint8_t *)0xC0)
#define IO_SPI_ready_flags (*(volatile uint8_t *)0xC1)
#define IO_SPI_kick_stop_reset (*(volatile uint8_t *)0xC2)
#define IO_SPI_flash_addr_lsb (*(volatile uint8_t *)0xC3)
#define IO_SPI_flash_addr_mid (*(volatile uint8_t *)0xC4)
#define IO_SPI_flash_addr_msb (*(volatile uint8_t *)0xC5)
#define IO_SPI_command_write (*(volatile uint8_t *)0xC6)
#define IO_SPI_transfer_mode (*(volatile uint8_t *)0xC7)
#define IO_SPI_manual_data_read (*(volatile uint8_t *)0xC8)
#define IO_SPI_chip_id_read_lsb (*(volatile uint8_t *)0xC9)
#define IO_SPI_chip_id_read_mid (*(volatile uint8_t *)0xCA)
#define IO_SPI_chip_id_read_msb (*(volatile uint8_t *)0xCB)
// (Other registers as needed)

// For simulation we assume an external flash memory array.
extern uint8_t flashMemory[];

void firm_flash_wait(void)
{
    // Poll until the status register’s bit7 is set.
    while (!(IO_SPI_status_ready & 0x80))
        ;
}

void firm_flash_clear(void)
{
    firm_flash_wait();
    // Clear flash ready flags.
    *(volatile unsigned char *)IO_SPI_ready_flags = 0;
    // Issue a kick/stop reset command.
    *(volatile unsigned char *)IO_SPI_kick_stop_reset = 0x80;
}

void firm_flash_kick_r0(unsigned char cmd)
{
    *(volatile unsigned char *)IO_SPI_transfer_mode = cmd;
    // Wait until the ready flags show the command has been processed.
    while (!((*(volatile unsigned char *)IO_SPI_ready_flags) & cmd))
        ;
}

void flash_set_wprot_to_r0(unsigned char param)
{
    firm_flash_clear();
    *(volatile unsigned char *)IO_SPI_wprot_stat_write = param;
    *(volatile unsigned char *)IO_SPI_command_write = 0x04; // Write-protect command
    firm_flash_kick_r0(0x08);
    firm_flash_wait();
}

unsigned char flash_read_write_protect_status_r0(void)
{
    firm_flash_clear();
    *(volatile unsigned char *)IO_SPI_command_write = 0x02; // Read status register
    firm_flash_kick_r0(0x08);
    return *(volatile unsigned char *)IO_SPI_manual_data_read;
}
uint8_t flash_read_write_protect_status(void)
{
    firm_flash_clear();
    IO_SPI_command_write = FLASH_CMD_RDSR;
    firm_flash_kick(0x08);
    uint8_t data = IO_SPI_manual_data_read;
    firm_flash_wait();
    return data;
}

void flash_erase_block(uint8_t r2, uint8_t r1, uint8_t r0)
{
    firm_flash_clear();
    firm_flash_set_addr(r0, r1, r2);
    IO_SPI_command_write = FLASH_CMD_ERASE;
    firm_flash_kick(0x08);
    firm_flash_wait();
}

void flash_write_dptr_to_r2r1r0_len_r3(unsigned char *src, unsigned int len)
{
    unsigned char dummy;
    firm_flash_write_enable(); // Assume this calls flash_write_enable()
    flash_erase_block_r2r1r0();
    while (len--)
    {
        dummy = *src++;
        // Write byte via SPI:
        *(volatile unsigned char *)IO_SPI_dma_ram_addr_lsb = dummy;
        // (Fill in proper DMA or SPI transfer code)
        firm_flash_kick_r0(0x80); // Example command
    }
}

void flash_write_dptr_to_r2r1r0_len_r3_without_erase(unsigned char *src, unsigned int len)
{
    firm_flash_clear();
    firm_flash_set_addr(0x00, 0x00, 0x00); // Destination address (dummy)
    // Setup DMA parameters (source address, length) – details omitted.
    *(volatile unsigned char *)IO_SPI_dma_length_lsb = (unsigned char)(len - 1);
    *(volatile unsigned char *)IO_SPI_dma_length_msb = 0;
    *(volatile unsigned char *)IO_SPI_command_write = 0x80; // Program command
    firm_flash_kick_r0(0x80);
}

void flash_dma_upload_font_from_r2r1r0_to_dptr_len_r4r3(unsigned int len, unsigned char r4, unsigned char r3)
{
    // Disable interrupts if needed.
    EA = 0;
    firm_flash_clear();
    *(volatile unsigned char *)IO_SPI_dma_ram_addr_lsb = 0; // Destination VRAM address (dummy)
    *(volatile unsigned char *)IO_SPI_dma_ram_addr_msb = 0;
    *(volatile unsigned char *)IO_SPI_dma_length_lsb = (unsigned char)(len - 1);
    *(volatile unsigned char *)IO_SPI_dma_length_msb = 0;
    *(volatile unsigned char *)IO_SPI_command_write = 0x40; // DMA upload command
    firm_flash_kick_r0(0x80);
    firm_flash_wait();
    // Resume interrupts.
    EA = 1;
}

void flash_read_r2r1r0_to_dptr_len_r3(unsigned int len)
{
    unsigned char dummy;
    firm_flash_wait();
    *(volatile unsigned char *)IO_SPI_kick_stop_reset = 0x80;
    while (len--)
    {
        *(volatile unsigned char *)IO_SPI_ready_flags = 0;
        firm_flash_set_addr(0x00, 0x00, 0x00); // Set flash address appropriately.
        dummy = *(volatile unsigned char *)IO_SPI_manual_data_read;
        // Write dummy to XRAM at pointer dptr (not shown – update pointer as needed)
    }
}

uint32_t flash_get_chip_id(void)
{
    firm_flash_clear();
    IO_SPI_command_write = FLASH_CMD_RDID;
    firm_flash_kick(0x08);
    uint8_t lsb = IO_SPI_chip_id_read_lsb;
    uint8_t mid = IO_SPI_chip_id_read_mid;
    uint8_t msb = IO_SPI_chip_id_read_msb;
    firm_flash_wait();
    return ((uint32_t)msb << 16) | ((uint32_t)mid << 8) | lsb;
}

void firm_flash_clear(void)
{
    firm_flash_wait();
    IO_SPI_ready_flags = 0;
    IO_SPI_kick_stop_reset = 0x80;
}

void firm_flash_set_addr(uint8_t r0, uint8_t r1, uint8_t r2)
{
    IO_SPI_flash_addr_lsb = r0;
    IO_SPI_flash_addr_mid = r1;
    IO_SPI_flash_addr_msb = r2;
}

void firm_flash_kick(uint8_t cmd)
{
    IO_SPI_transfer_mode = cmd;
    // Wait until the ready flag indicates command completion.
    while (!(IO_SPI_ready_flags & cmd))
    {
    }
}

void flash_write_enable(void)
{
    firm_flash_clear();
    firm_flash_set_addr(0xFF, 0xFF, 0xFF); // Dummy flash address FFFFFF
    IO_SPI_command_write = FLASH_CMD_WREN;
    firm_flash_kick(0x08); // Use a dummy command value (e.g., FlashCmd08h)
    firm_flash_wait();
}

void flash_set_wprot_to(uint8_t param)
{
    firm_flash_clear();
    // Write the protection parameter to the flash’s protection register.
    *((volatile uint8_t *)IO_SPI_wprot_stat_write) = param;
    IO_SPI_command_write = FLASH_CMD_WRDI; // Use write–disable command here as in the asm.
    firm_flash_kick(0x08);
    firm_flash_wait();
}

void flash_write_disable(void)
{
    firm_flash_clear();
    firm_flash_set_addr(0xFF, 0xFF, 0xFF);
    IO_SPI_command_write = FLASH_CMD_WRDI;
    firm_flash_kick(0x08);
    firm_flash_wait();
}

void flash_write_dptr_to_flash(uint8_t *src, uint16_t flashAddr, uint16_t len)
{
    uint16_t i;
    firm_flash_clear();
    // Set flash address.
    firm_flash_set_addr((uint8_t)(flashAddr & 0xFF),
                        (uint8_t)((flashAddr >> 8) & 0xFF),
                        (uint8_t)(flashAddr >> 16));
    flash_write_enable();
    flash_erase_block((uint8_t)(flashAddr >> 16),
                      (uint8_t)((flashAddr >> 8) & 0xFF),
                      (uint8_t)(flashAddr & 0xFF));
    for (i = 0; i < len; i++)
    {
        // In a real system you would initiate a DMA transfer here.
        IO_SPI_dma_ram_addr_lsb = src[i];
        IO_SPI_command_write = FLASH_CMD_WRITE;
        firm_flash_kick(0x08);
    }
    firm_flash_wait();
}

// Define memory-mapped registers (placeholders)
#define IO_SPI_DMA_RAM_ADDR_LSB ((volatile uint8_t *)0xFA00)
#define IO_SPI_DMA_RAM_ADDR_MSB ((volatile uint8_t *)0xFA01)
#define IO_SPI_DMA_LENGTH_LSB ((volatile uint8_t *)0xFA02)
#define IO_SPI_DMA_LENGTH_MSB ((volatile uint8_t *)0xFA03)
#define SFR_IO_MEMORY_SYSTEM ((volatile uint8_t *)0x80)
#define ENABLE_INTERRUPTS() __asm__("setb ea") // Enable global interrupts
#define DISABLE_INTERRUPTS() __asm__("clr ea") // Disable global interrupts

// **DMA Upload from Flash to VRAM**
void flash_dma_upload_font(uint32_t flash_addr, uint16_t vram_addr, uint16_t length)
{
    // Disable interrupts (avoid interference)
    DISABLE_INTERRUPTS();

    // Clear flash (ready for DMA)
    firm_flash_clear();

    // Pause font rendering during DMA upload (bit 3 of memory system control)
    *SFR_IO_MEMORY_SYSTEM |= 0x08;

    // Set flash source address
    firm_flash_set_addr(flash_addr);

    // Set VRAM destination address
    *IO_SPI_DMA_RAM_ADDR_LSB = (uint8_t)(vram_addr & 0xFF);
    *IO_SPI_DMA_RAM_ADDR_MSB = (uint8_t)((vram_addr >> 8) & 0xFF);

    // Set transfer length (length - 1)
    *IO_SPI_DMA_LENGTH_LSB = (uint8_t)((length - 1) & 0xFF);
    *IO_SPI_DMA_LENGTH_MSB = (uint8_t)(((length - 1) >> 8) & 0xFF);

    // Execute DMA transfer (Flash command `0x40`)
    firm_flash_kick(0x40);

    // Wait for DMA transfer completion
    firm_flash_wait();

    // Resume font rendering (clear bit 3)
    *SFR_IO_MEMORY_SYSTEM &= 0xF7;

    // Re-enable interrupts
    ENABLE_INTERRUPTS();
}

void flash_read_to_dptr(uint8_t *dst, uint16_t len)
{
    // For simulation, copy from flashMemory.
    uint16_t i;
    for (i = 0; i < len; i++)
        dst[i] = flashMemory[i];

    firm_flash_wait();
}
