#ifndef FLASH_FUNCTIONS_H
#define FLASH_FUNCTIONS_H

#include <stdint.h>

// These addresses are defined elsewhere (for example, in a central SFR header).
// For example:
#define IO_SPI_status_ready 0xF100 // Dummy address
#define IO_SPI_ready_flags 0xF101
#define IO_SPI_kick_stop_reset 0xF102
#define IO_SPI_flash_addr_lsb 0xF110
#define IO_SPI_flash_addr_mid 0xF111
#define IO_SPI_flash_addr_msb 0xF112
#define IO_SPI_command_write 0xF113
#define IO_SPI_transfer_mode 0xF114
#define IO_SPI_manual_data_read 0xF115
#define IO_SPI_chip_id_read_lsb 0xF116
#define IO_SPI_chip_id_read_mid 0xF117
#define IO_SPI_chip_id_read_msb 0xF118
#define IO_SPI_wprot_stat_write 0xF119

// Flash command definitions (example values)
#define FLASH_CMD_WREN 0x06  // Write enable
#define FLASH_CMD_ERASE 0x20 // Erase block
#define FLASH_CMD_WRITE 0x80 // Write (DMA mode)
#define FLASH_CMD_WRDI 0x04  // Write disable
#define FLASH_CMD_RDSR 0x02  // Read status register
#define FLASH_CMD_RDID 0x01  // Read chip ID

void firm_flash_wait(void);
void firm_flash_clear(void);
void firm_flash_set_addr(uint8_t r0, uint8_t r1, uint8_t r2);
void firm_flash_kick(uint8_t cmd);
void flash_write_enable(void);
void flash_set_wprot_to(uint8_t param);
void flash_write_disable(void);
uint8_t flash_read_write_protect_status(void);
void flash_erase_block(uint8_t r2, uint8_t r1, uint8_t r0);
void flash_write_dptr_to_flash(uint8_t *src, uint16_t flashAddr, uint16_t len);
void flash_dma_upload_font(uint8_t *dst, uint16_t len, uint8_t r4, uint8_t r3);
void flash_read_to_dptr(uint8_t *dst, uint16_t len);
uint32_t flash_get_chip_id(void);

//============================================================
// Flash and Settings Functions
//============================================================
void firm_flash_wait(void);
void firm_flash_clear(void);
void firm_flash_set_addr(unsigned char r0, unsigned char r1, unsigned char r2);
void firm_flash_kick_r0(unsigned char cmd);
void flash_write_enable(void);
void flash_set_wprot_to_r0(unsigned char param);
void flash_write_disable(void);
unsigned char flash_read_write_protect_status_r0(void);
void flash_erase_block_r2r1r0(void);
void flash_write_dptr_to_r2r1r0_len_r3(unsigned char *src, unsigned int len);
void flash_write_dptr_to_r2r1r0_len_r3_without_erase(unsigned char *src, unsigned int len);
void flash_dma_upload_font_from_r2r1r0_to_dptr_len_r4r3(unsigned int len, unsigned char r4, unsigned char r3);
void flash_read_r2r1r0_to_dptr_len_r3(unsigned int len);
void flash_get_chip_id_r2r1r0(void);

#endif // FLASH_FUNCTIONS_H
