#include "io_registers.h"
#include "xram.h"

//====================================================================
// Interrupt Service Routines (ISRs)
//====================================================================

// Reset vector (address 0000h)
// void reset_entrypoint(void) __interrupt 0
// {
//     system_init();
//     main(); // Jump into the main program.
// }

// External Interrupt 0 (IR remote) – vector 1 (address 0003h)
void ext_int0_ir(void) __interrupt 1
{
#ifdef WITH_INFRARED
    process_ir_interrupt();
#endif
    // RETI is automatically generated.
}

// Timer 0 overflow – vector 2 (address 000Bh)
void timer0_overflow_isr(void) __interrupt 2
{
    timer0_reload();
}

// Timer 1 overflow – vector 4 (address 001Bh)
void timer1_overflow_isr(void) __interrupt 4
{
    timer1_overflow();
}

// Serial Port 0 – vector 5 (unused in this example)
void serial0_isr(void) __interrupt 5
{
    // Implement serial I/O if needed.
}

// Serial Port 1 – vector 6 (unused)
void serial1_isr(void) __interrupt 6
{
    // Not used.
}

// External Interrupt 2 (CT0) – vector 7 (unused)
void ct0_isr(void) __interrupt 7
{
    // Not used.
}

// External Interrupt 3 (CT1) – vector 8 (unused)
void ct1_isr(void) __interrupt 8
{
    // Not used.
}

// ADC/Keypad – vector 9 (address 0043h)
void adc_keypad_isr(void) __interrupt 9
{
#ifdef WITH_KEYPAD_IRQ
    adc_keypad_handler();
#endif
}

// “Whatever 91h 5 ACK” – vector 10 (address 004Bh)
void whatever_91h5_ack_isr(void) __interrupt 10
{
    acknowledge_adc();
}

// “Whatever 91h 6 ACK” – vector 11 (address 0053h)
void whatever_91h6_ack_isr(void) __interrupt 11
{
    acknowledge_other_irq();
}

// Framerate – vector 12 (address 005Bh)
void framerate_isr(void) __interrupt 12
{
    update_framerate();
}

// Timer2 Compare – vector 13 (address 0063h)
void timer2_compare_isr(void) __interrupt 13
{
    timer2_compare_handler();
}

/// @brief //////////////////////////////////////////

void reset_entrypoint()
{
    // Disable IRQs (for warm boot)
    __disable_irq();

    // ** Zero-fill IRAM (0x01 to 0x7F) **
    for (uint8_t i = 0x7F; i > 0; i--)
        *((volatile uint8_t *)i) = 0;

    // ** Zero-fill XRAM (0x0000 to 0x07FF) **
    for (volatile uint16_t *ptr = (volatile uint16_t *)0x0000; ptr < (volatile uint16_t *)0x0800; ptr++)
        *ptr = 0;

    // ** Initialize Stack Pointer **
    __set_MSP(0x7F); // Assuming MSP usage (adjust if different)

    // ** Set Initial Hardware States **
    firm_set_p35_backlight_off(); // Turn off backlight
    firm_set_p36_volume_off();    // Turn off volume

    // ** Apply Initial Settings **
    main_init(); // Initialize system components

    // ** Initialize OSD Font **
    sysgui_make_font();

    // ** Set Initial GUI Menu Index **
    iram_sysgui_menu_index = SYS_GUI_MAIN_MENU_INDEX;

    // ** Check for First Boot or Forced Panel Type Selection **
    if (!(getkey() & 0x02))
    { // Check if key A.1 is pressed
        if (xram_sett_id_5Ah != 0x5A)
        {
            if (xram_lcdspi_type == 0xNV3035C)
            {
                xram_sett_panel_type = 0x00; // Assume 320x240 Tianma LCD
            }
            else
            {
                switch_screen_and_backlight_on();
                sysgui_prompt_panel_type();
                firm_do_save_settings();
                sysgui_select_menu();
                return;
            }
        }
    }

    // ** Boot into Normal Standby Mode **
    sysgui_standby();
}

void timer_0_overflow_irq()
{
    disable_timer0();

    // Reload Timer0 with -1121 (FB9Fh)
    write_io(SFR_IO_timer0_msb, 0xFB);
    write_io(SFR_IO_timer0_lsb, 0x9F);

    enable_timer0();
}

void timer_1_overflow_irq()
{
    disable_timer1();

    // Reload Timer1 with -10001 (D8EFh)
    write_io(SFR_IO_timer1_msb, 0xD8);
    write_io(SFR_IO_timer1_lsb, 0xEF);

    enable_timer1();

    // Increment timer interrupt counter
    iram_timer1_irq_counter++;

    // Check AV status and handle coarse mode if necessary
    if (read_io(IO_AV_stat_detect_0) & 0x02)
    {
        adjust_artifacts();
        release_coarse_mode();
        adjust_sharpness();
    }
    else
    {
        enter_coarse_mode();
    }

    adjust_sensitivity();
    adjust_boldness();

    // Clear a specific video register
    write_io(IO_VIDEO_something_5, 0x00);

    // Handle framerate adjustments every 4th IRQ
    if ((iram_timer1_irq_counter & 0x03) == 0)
    {
        detect_framerate();
        apply_tint();
    }

    // Handle input selection every 16th IRQ
    if ((iram_timer1_irq_counter & 0x0F) == 0)
        input_selector();
}

void firm_irq_0043h_adc_keypad()
{
    disable_interrupts();

    uint8_t adc_msb, adc_lsb;
    uint8_t index;

    // Read ADC value
    adc_msb = read_io(IO_ADC_input_0_msb);
    adc_lsb = read_io(IO_ADC_input_0_lsb);

    // Update keypad index
    index = read_xram(xram_analog_keypad_index);
    index++;
    if (index >= NUM_KEYPAD_READINGS)
        index = 0;

    write_xram(xram_analog_keypad_index, index);

    // Store ADC values at the indexed position
    uint16_t addr = xram_analog_keypad_values + (index * 2);
    write_xram(addr, adc_msb);
    write_xram(addr + 1, adc_lsb);

    // Acknowledge ADC to CPU
    clear_bit(SFR_IO_xxx91h, 4);

    // Acknowledge ADC to itself
    write_io(IO_ADC_status_lsb, 0x00);
    write_io(IO_ADC_status_msb, 0x00);

    enable_interrupts();
}

void irq_91h_5_ack()
{
    clear_bit(SFR_IO_xxx91h, 5);
}

void framerate_irq()
{
    sys_random[0]++;
    sys_vblank_flag = 1;
    clear_bit(SFR_IO_xxx91h, 7);
}

void firm_irq_001Bh_timer_1_overflow(void) // ISR ?
{
    // Simulate the saving of registers by using local variables.
    TR1 = 0;
    *(volatile unsigned char *)SFR_IO_timer1_msb = 0xD8;
    *(volatile unsigned char *)SFR_IO_timer1_lsb = 0xEF;
    TR1 = 1;
    (*(volatile unsigned char *)&iram_timer1_irq_counter)++;
    unsigned char detect0 = *(volatile unsigned char *)IO_AV_stat_detect_0;
    if (detect0 & 0x02)
    {
        firm_timer_coarse_enter();
        goto artifacts_sharpness_done;
    }

    firm_timer_adjust_artifacts();
    firm_timer_coarse_release();
    firm_timer_adjust_sharpness();
artifacts_sharpness_done:
    firm_timer_adjust_sensitivity();
    firm_timer_adjust_boldness();
    *(volatile unsigned char *)IO_VIDEO_something_5 = 0;
    unsigned char counter = *(volatile unsigned char *)&iram_timer1_irq_counter;
    if ((counter & 0x03) == 0)
    {
        firm_timer_detect_framerate();
        apply_tint();
    }
    if ((counter & 0x0F) == 0)
    {
        input_selector();
    }
}