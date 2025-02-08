#ifndef LCD_SPI_H
#define LCD_SPI_H

#include <stdint.h>

// High–level LCD SPI routines
void display_spi_reset(void);
void display_spi_detect(void);
void display_spi_dump_initial(void);
void display_spi_init(void);

// Low–level LCD SPI routines
void lcd_spi_send_8_bits_from_a(uint8_t data);
void lcd_spi_send_r0_bits_from_a(uint8_t data, uint8_t numBits);
uint8_t lcd_spi_recv_r0_bits_to_a(uint8_t numBits);
void lcd_spi_release(void);

// Higher–level display commands
void write_HX8238_index_r2_data_r1r0(uint8_t index, uint8_t data_msb, uint8_t data_lsb);
void write_NV3035C_index_r0_data_a(uint8_t index, uint8_t data);
uint8_t read_NV3035C_register_r0(uint8_t index);

#endif // LCD_SPI_H
