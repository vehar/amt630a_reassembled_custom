#include "adc.h"
void firm_adc_init_io_list(void)
{
    // Process an I/O list for ADC initialization.
    extern unsigned char adc_init_io_list[];
    init_io_via_io_list(adc_init_io_list);
}

void init_ADC_analog_hardware()
{
    init_io_via_io_list(firm_adc_init_io_list);
}

void init_ADC_analog_hardware(void)
{
    firm_adc_init_io_list();
}