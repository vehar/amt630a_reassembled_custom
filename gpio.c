#include "gpio.h"
#include "hardware.h"

void firm_set_p07_spi_cs_to_cy(void)
{
    uint8_t temp = IO_PIN_P06_P07_pwm;
    temp &= ~0x70;
    IO_PIN_P06_P07_pwm = temp;
    SFR_IO_PORT0_MODE_A &= 0x7F;
    SFR_IO_PORT0_MODE_B |= 0x80;
    IO_PORT0_DATA |= 0x80;
}

void firm_set_p07_spi_cs_to_cy(void)
{
    unsigned char temp = *(volatile unsigned char *)IO_PIN_P06_P07_pwm;
    temp &= ~0x70;
    *(volatile unsigned char *)IO_PIN_P06_P07_pwm = temp;
    // Adjust port mode registers (dummy addresses):
    *(volatile unsigned char *)SFR_IO_PORT0_MODE_A &= 0x7F;
    *(volatile unsigned char *)SFR_IO_PORT0_MODE_B |= 0x80;
    // Set bit 7 of Port0 data to the value in carry (simulate c):
    if (temp & 0x01)
        *(volatile unsigned char *)IO_PIN_P06_P07_pwm |= 0x80;
    else
        *(volatile unsigned char *)IO_PIN_P06_P07_pwm &= ~0x80;
}

void firm_set_p06_spi_res_to_cy(void)
{
    uint8_t temp = IO_PIN_P06_P07_pwm;
    temp &= ~0x07;
    IO_PIN_P06_P07_pwm = temp;
    SFR_IO_PORT0_MODE_A &= 0xBF;
    SFR_IO_PORT0_MODE_B |= 0x40;
    IO_PORT0_DATA &= ~0x40;
}
void firm_set_p06_spi_res_to_cy(void)
{
    unsigned char temp = *(volatile unsigned char *)IO_PIN_P06_P07_pwm;
    temp &= ~0x07;
    *(volatile unsigned char *)IO_PIN_P06_P07_pwm = temp;
    *(volatile unsigned char *)SFR_IO_PORT0_MODE_A &= 0xBF;
    *(volatile unsigned char *)SFR_IO_PORT0_MODE_B |= 0x40;
    if (temp & 0x01)
        *(volatile unsigned char *)IO_PIN_P06_P07_pwm |= 0x40;
    else
        *(volatile unsigned char *)IO_PIN_P06_P07_pwm &= ~0x40;
}

void firm_set_p37_spi_clk_to_cy(void)
{
    uint8_t temp = IO_PIN_P37_xxx_pwm;
    temp &= ~0x07;
    IO_PIN_P37_xxx_pwm = temp;
    SFR_IO_PORT3_MODE_A &= 0x7F;
    SFR_IO_PORT3_MODE_B |= 0x80;
    IO_PORT3_DATA |= 0x80;
}
void firm_set_p37_spi_clk_to_cy(void)
{
    unsigned char temp = *(volatile unsigned char *)IO_PIN_P37_xxx_pwm;
    temp &= ~0x07;
    *(volatile unsigned char *)IO_PIN_P37_xxx_pwm = temp;
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_A &= 0x7F;
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_B |= 0x80;
    if (temp & 0x01)
        *(volatile unsigned char *)IO_PIN_P37_xxx_pwm |= 0x80;
    else
        *(volatile unsigned char *)IO_PIN_P37_xxx_pwm &= ~0x80;
}

void firm_set_p36_spi_dta_to_cy(void)
{
    uint8_t temp = IO_PIN_P35_P36_pwm;
    temp &= ~0x70;
    IO_PIN_P35_P36_pwm = temp;
    SFR_IO_PORT3_MODE_A &= 0xBF;
    SFR_IO_PORT3_MODE_B |= 0x40;
    IO_PORT3_DATA &= ~0x40;
}
void firm_set_p36_spi_dta_to_cy(void)
{
    unsigned char temp = *(volatile unsigned char *)IO_PIN_P35_P36_pwm;
    temp &= ~0x70;
    *(volatile unsigned char *)IO_PIN_P35_P36_pwm = temp;
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_A &= 0xBF;
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_B |= 0x40;
    if (temp & 0x01)
        *(volatile unsigned char *)IO_PIN_P35_P36_pwm |= 0x40;
    else
        *(volatile unsigned char *)IO_PIN_P35_P36_pwm &= ~0x40;
}

void firm_set_p35_backlight_off(void)
{
    uint8_t temp = IO_PIN_P35_P36_pwm;
    temp &= ~0x07;
    IO_PIN_P35_P36_pwm = temp;
}
void firm_set_p35_backlight_off(void)
{
    // Turn off backlight by clearing the appropriate bit in the backlight port.
    *(volatile unsigned char *)IO_PIN_P35_P36_pwm &= ~0x01;
}

void firm_set_p35_backlight_to_cy(void)
{
    uint8_t temp = IO_PIN_P35_P36_pwm;
    temp &= ~0x07;
    IO_PIN_P35_P36_pwm = temp;
    SFR_IO_PORT3_MODE_A &= 0xDF;
    SFR_IO_PORT3_MODE_B |= 0x20;
    IO_PORT3_DATA &= ~0x20;
}
void firm_set_p35_backlight_to_cy(void)
{
    unsigned char temp = *(volatile unsigned char *)IO_PIN_P35_P36_pwm;
    temp &= ~0x07;
    *(volatile unsigned char *)IO_PIN_P35_P36_pwm = temp;
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_A &= 0xDF;
    *(volatile unsigned char *)SFR_IO_PORT3_MODE_B |= 0x20;
    if (temp & 0x01)
        *(volatile unsigned char *)IO_PIN_P35_P36_pwm |= 0x20;
    else
        *(volatile unsigned char *)IO_PIN_P35_P36_pwm &= ~0x20;
}