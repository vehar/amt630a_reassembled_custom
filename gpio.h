#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

void firm_set_p07_spi_cs_to_cy(void);
void firm_set_p06_spi_res_to_cy(void);
void firm_set_p37_spi_clk_to_cy(void);
void firm_set_p36_spi_dta_to_cy(void);
void firm_set_p35_backlight_off(void);
void firm_set_p35_backlight_to_cy(void);

#endif // GPIO_H
