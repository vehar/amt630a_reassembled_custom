#include "lcd_spi.h"
#include "hardware.h"
#include <stdint.h>

// Dummy definitions for SPI–related registers/pins.
#define IO_SPI_DAT (*(volatile uint8_t *)0xD4)
#define IO_SPI_CLK (*(volatile uint8_t *)0xD5)
#define IO_SPI_CS (*(volatile uint8_t *)0xD6)
#define IO_SPI_DAT_DIR (*(volatile uint8_t *)0xD7)

extern void firm_wait_a_milliseconds(uint16_t ms);

// In these routines we assume that functions such as firm_set_p07_spi_cs_to_cy(),
// firm_set_p06_spi_res_to_cy(), etc. have been implemented elsewhere.
// Also assume that delay routines (firm_wait_a_milliseconds) are available.

// Read NV3035C register
uint8_t read_NV3035C_register(uint8_t reg)
{
    uint8_t data = reg; // Index
    data <<= 1;         // Shift left to prepare for R/W bit
    data &= 0xFE;       // Set R/W bit to 0 (read)

    // Send register index
    lcd_spi_send_r0_bits_from_a(data, 7);

    // Receive register data
    uint8_t result = lcd_spi_recv_r0_bits_to_a(9);

    // Release SPI bus
    lcd_spi_release();

    return result;
}

void display_spi_reset(void)
{
    // Set non-PWM mode on SPI-related pins.
    firm_set_p07_spi_cs_to_cy();
    firm_set_p06_spi_res_to_cy();
    firm_set_p37_spi_clk_to_cy();
    firm_set_p36_spi_dta_to_cy();
    // Assert RESET low, wait, then release:
    REG8(IO_SPI_RESET) = 0;
    firm_wait_a_milliseconds(1);
    REG8(IO_SPI_RESET) = 1;
    firm_set_p06_spi_res_to_cy();
    firm_wait_a_milliseconds(1);
}
void display_spi_reset(void)
{
    // Configure SPI pins to non-PWM mode and perform display reset sequence.
    firm_set_p07_spi_cs_to_cy();
    firm_set_p06_spi_res_to_cy();
    firm_set_p37_spi_clk_to_cy();
    firm_set_p36_spi_dta_to_cy();
    // Assert RESET low, wait, then release:
    *(volatile unsigned char *)IO_SPI_RESET = 0;
    firm_wait_a_milliseconds(1);
    *(volatile unsigned char *)IO_SPI_RESET = 1;
    firm_wait_a_milliseconds(1);
}

void display_spi_detect(void)
{
    uint8_t id;
    id = read_NV3035C_register_r0(2);
    if (id != 0x13)
    {
        id = read_NV3035C_register_r0(4);
        if (id != 0x46)
        {
            REG8(xram_lcdspi_type) = lcdspi_UNKNOWN;
            return;
        }
    }
    REG8(xram_lcdspi_type) = lcdspi_NV3035C;
}
unsigned char display_spi_detect(void)
{
    unsigned char id;
    id = read_NV3035C_register_r0(0x02);
    if (id == 0x13)
        return 1; // NV3035C detected
    else
        return 0;
}

// Placeholder for memory location
volatile uint8_t xram_initial_spi_regs[0x40] = {0};

void display_spi_dump_initial()
{
    for (uint8_t i = 0; i < 0x40; i++)
    {
        xram_initial_spi_regs[i] = read_NV3035C_register(i);
    }
}
void display_spi_dump_initial(void)
{
    uint8_t i;
    for (i = 0; i < 0x40; i++)
    {
        uint8_t val = read_NV3035C_register_r0(i);
        *((volatile uint8_t *)(xram_initial_spi_regs_base + i)) = val;
    }
}
void display_spi_dump_initial(void)
{
    // Read and store initial SPI registers into XRAM.
    unsigned char i;
    unsigned char *ptr = (unsigned char *)xram_initial_spi_regs; // defined elsewhere
    for (i = 0; i < 0x40; i++)
    {
        *ptr++ = read_NV3035C_register_r0(i);
    }
}

void display_spi_init(void)
{
    // The output list for the HX8238-D chip.
    uint8_t *p = hx8238_out_list;
    while (*p != 0xFF)
    {
        uint8_t index = *p++;
        uint8_t data_msb = *p++;
        uint8_t data_lsb = *p++;
        write_HX8238_index_r2_data_r1r0(index, data_msb, data_lsb);
    }
}
void display_spi_init(void)
{
    // For an HX8238-D display, send a series of configuration commands.
    extern unsigned char hx8238_out_list[]; // Defined in assembler or C array
    unsigned char *dptr = hx8238_out_list;
    while (*dptr != 0xFF)
    { // End marker
        unsigned char index = *dptr++;
        unsigned char msb = *dptr++;
        unsigned char lsb = *dptr++;
        write_HX8238_index_r2_data_r1r0(index, msb, lsb);
    }
}

void lcd_spi_send_8_bits_from_a(uint8_t data)
{
    lcd_spi_send_r0_bits_from_a(data, 8);
}

// Simulated memory-mapped registers
volatile uint8_t SFR_IO_PORT3_MODE_A;
volatile uint8_t SFR_IO_PORT3_MODE_B;
volatile uint8_t SFR_IO_PORT3_DATA;
volatile uint8_t SFR_IO_PORT0_DATA;
volatile uint8_t IO_PIN_P35_P36_PWM;

// Fast delay factor
#define FASTFAC 1

void lcd_spi_send_r0_bits_from_a(uint8_t data, uint8_t num_bits)
{
    // Set data as output
    SFR_IO_PORT3_MODE_A &= 0xBF;
    SFR_IO_PORT3_MODE_B |= 0x40;

    // Set /CS = LOW (active)
    SFR_IO_PORT0_DATA &= ~(1 << 7);

    for (uint8_t i = 0; i < num_bits; i++)
    {
        // Set DAT = data MSB
        SFR_IO_PORT3_DATA = (SFR_IO_PORT3_DATA & ~(1 << 6)) | ((data & 0x80) >> 1);

        // Set CLK = LOW
        SFR_IO_PORT3_DATA &= ~(1 << 7);

        // Short delay
        for (volatile int d = 0; d < 5 * FASTFAC; d++)
            ;

        // Set CLK = HIGH
        SFR_IO_PORT3_DATA |= (1 << 7);

        // Short delay
        for (volatile int d = 0; d < 5 * FASTFAC; d++)
            ;

        // Shift data left
        data <<= 1;
    }
}
void lcd_spi_send_r0_bits_from_a(uint8_t data, uint8_t numBits)
{
    // Set the SPI chip select low.
    REG8(IO_CS) = 0;
    while (numBits--)
    {
        uint8_t bit = (data & 0x80) ? 1 : 0;
        if (bit)
            REG8(IO_SPI_DATA) |= 0x40; // Assume DATA bit mask 0x40.
        else
            REG8(IO_SPI_DATA) &= ~(0x40);
        REG8(IO_SPI_CLK) = 0;
        for (volatile int i = 0; i < 5; i++)
            ; // Small delay.
        REG8(IO_SPI_CLK) = 1;
        for (volatile int i = 0; i < 5; i++)
            ;
        data <<= 1;
    }
}

uint8_t lcd_spi_recv_r0_bits_to_a(uint8_t numBits)
{
    uint8_t data = 0;
    // Configure DATA pin as input.
    REG8(IO_SPI_DATA_DIR) = 0;
    while (numBits--)
    {
        REG8(IO_SPI_CLK) = 0;
        for (volatile int i = 0; i < 5; i++)
            ;
        uint8_t bit = (REG8(IO_SPI_DATA) & 0x40) ? 1 : 0;
        data = (data << 1) | bit;
        REG8(IO_SPI_CLK) = 1;
        for (volatile int i = 0; i < 5; i++)
            ;
    }
    return data;
}
uint8_t lcd_spi_recv_r0_bits_to_a(uint8_t num_bits)
{
    uint8_t data = 0;

    // Set data direction to input
    SFR_IO_PORT3_MODE_A |= 0x40;
    SFR_IO_PORT3_MODE_B &= ~0x40;

    for (uint8_t i = 0; i < num_bits; i++)
    {
        // Set CLK = LOW
        SFR_IO_PORT3_DATA &= ~(1 << 7);

        // Short delay
        for (volatile int d = 0; d < 5 * FASTFAC; d++)
            ;

        // Read data bit
        data <<= 1;
        if (SFR_IO_PORT3_DATA & (1 << 6))
            data |= 1;

        // Set CLK = HIGH
        SFR_IO_PORT3_DATA |= (1 << 7);

        // Short delay
        for (volatile int d = 0; d < 5 * FASTFAC; d++)
            ;
    }

    return data;
}

void lcd_spi_release(void)
{
    // Set chip select high and delay.
    REG8(IO_CS) = 1;
    for (volatile int i = 0; i < 20 * 4; i++)
        ;
    // Restore pin configuration for PWM if needed.
    uint8_t tmp = REG8(IO_PIN_P35_P36_pwm);
    tmp |= 0x30;
    REG8(IO_PIN_P35_P36_pwm) = tmp;
}
void lcd_spi_release()
{
    // Set /CS = HIGH
    SFR_IO_PORT0_DATA |= (1 << 7);

    // Short delay (20 * FASTFAC)
    for (volatile int d = 0; d < 20 * FASTFAC; d++)
        ;

    IO_PIN_P35_P36_PWM |= 0x30; // Restore PWM mode
}

void write_HX8238_index_r2_data_r1r0(uint8_t index, uint8_t data_msb, uint8_t data_lsb)
{
    // Send command to set index.
    lcd_spi_send_r0_bits_from_a(0x70, 8);
    lcd_spi_send_r0_bits_from_a(0x00, 8); // Index MSB = 0.
    lcd_spi_send_r0_bits_from_a(index, 8);
    lcd_spi_release();
    // Send command to write data.
    lcd_spi_send_r0_bits_from_a(0x73, 8);
    lcd_spi_send_r0_bits_from_a(data_msb, 8);
    lcd_spi_send_r0_bits_from_a(data_lsb, 8);
    lcd_spi_release();
}

void write_NV3035C_index_r0_data_a(uint8_t index, uint8_t data)
{
    // For NV3035C, send index (with direction and dummy bits) then data.
    lcd_spi_send_8_bits_from_a((index << 2) | 0x01);
    lcd_spi_send_8_bits_from_a(data);
    lcd_spi_release();
}

void write_NV3035C_index_r0_data_a(unsigned char index, unsigned char data)
{
    unsigned char temp;
    // Prepare index: shift left with carry bits
    temp = index;
    // Set carry and dummy bits (simulate rcl instructions)
    temp = (temp << 1) | 1;
    lcd_spi_send_8_bits_from_a(temp);
    lcd_spi_send_8_bits_from_a(data);
    lcd_spi_release();
}

unsigned char read_NV3035C_register_r0(unsigned char index)
{
    unsigned char data;
    unsigned char dummy;
    dummy = index;
    dummy = (dummy << 2); // simulate rcl shifts
    lcd_spi_send_8_bits_from_a(dummy);
    data = lcd_spi_recv_r0_bits_to_a(9);
    lcd_spi_release();
    return data;
}

void lcd_spi_send_8_bits_from_a(unsigned char data)
{
    // Send 8 bits from register A via SPI (bit–banging)
    unsigned char i;
    for (i = 0; i < 8; i++)
    {
        // Set DATA line according to MSB:
        if (data & 0x80)
            *(volatile unsigned char *)SFR_IO_PORT3_DATA |= 0x40;
        else
            *(volatile unsigned char *)SFR_IO_PORT3_DATA &= ~0x40;
        // Toggle CLK low then high with delays:
        *(volatile unsigned char *)SFR_IO_PORT3_DATA &= ~0x80;
        firm_wait_a_milliseconds(1);
        *(volatile unsigned char *)SFR_IO_PORT3_DATA |= 0x80;
        firm_wait_a_milliseconds(1);
        data <<= 1;
    }
}

void lcd_spi_send_r0_bits_from_a(unsigned char data, unsigned char numbits)
{
    unsigned char i;
    // Set DATA as output, lower /CS:
    *(volatile unsigned char *)IO_SPI_CS = 0;
    for (i = 0; i < numbits; i++)
    {
        // Shift data left and output carry bit on DATA line:
        if (data & 0x80)
            *(volatile unsigned char *)SFR_IO_PORT3_DATA |= 0x40;
        else
            *(volatile unsigned char *)SFR_IO_PORT3_DATA &= ~0x40;
        // Toggle CLK:
        *(volatile unsigned char *)SFR_IO_PORT3_DATA &= ~0x80;
        firm_wait_a_milliseconds(1);
        *(volatile unsigned char *)SFR_IO_PORT3_DATA |= 0x80;
        firm_wait_a_milliseconds(1);
        data <<= 1;
    }
}

unsigned char lcd_spi_recv_r0_bits_to_a(unsigned char numbits)
{
    unsigned char data = 0;
    unsigned char i;
    // Set DATA as input:
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_A |= 0x40;
    for (i = 0; i < numbits; i++)
    {
        data <<= 1;
        *(volatile unsigned char *)SFR_IO_PORT3_DATA &= ~0x80;
        firm_wait_a_milliseconds(1);
        if (*(volatile unsigned char *)SFR_IO_PORT3_DATA & 0x40)
            data |= 1;
        *(volatile unsigned char *)SFR_IO_PORT3_DATA |= 0x80;
        firm_wait_a_milliseconds(1);
    }
    return data;
}

void lcd_spi_release(void)
{
    // Release /CS and add a delay:
    *(volatile unsigned char *)IO_SPI_CS = 1;
    firm_wait_a_milliseconds(2);
    // Optionally, restore PWM mode on the pin:
}

uint8_t read_NV3035C_register_r0(uint8_t index)
{
    // For NV3035C, send index with RW=0.
    lcd_spi_send_8_bits_from_a(index & ~0x01);
    uint8_t data = lcd_spi_recv_r0_bits_to_a(9);
    lcd_spi_release();
    return data;
}
