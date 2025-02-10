#include "init.h"
#include "hardware.h"
#include "flash.h"
#include "lcd_spi.h"
#include "gpio.h"
#include "sysgui.h"
#include "delay.h"
#include "io_lists.h"
#include "baudrate.h"

// For example, these global variables simulate XRAM/IRAM.
uint8_t xram_sett_panel_type = 0; // 0 = 320x240, nonzero = 480x272
// (Other settings variables should be defined similarly.)

void main_init(void)
{
    main_init_part_1();
    main_init_part_2();
    main_init_part_3();
    main_init_part_4();
    main_init_part_5();
    EA = 1; // Enable global interrupts.
}

void main_init_part_1(void)
{
    REG8(SFR_IO_memory_system) |= 0x80;
    IO_PLL_0Eh_used = 0x20;
}

/*
set_baudrate_57600bps:   ;init 80C52-style Timer2 for baudrate
 computes the baudrate reload value as so:
  -27000000/(57600*32) = -14.6484375 = rounded = -14 = FFF2h
 due to the rounding, this will produce:
  -27000000/-14/32 = 60267.85714 bauds = does that work well?
 alternately, a bit closer would be:
  -27000000/-15/32 = 56250 bauds
 best would be to avoid/reduce the div32 effect if possible
 (at least standard 80C51's allow to select div16 in PCON)
 - - -
@@rate57600 equ 0FFF2h
 ;- - -
 */
void set_baudrate_57600bps()
{
    write_io(SFR_IO_baudrate_control, 0x30);
    write_io(SFR_IO_sio_scon, 0x50);
    write_io(SFR_IO_baudrate_msb, 0xFF);
    write_io(SFR_IO_baudrate_lsb, 0xF2);
    set_bit(SFR_IO_baudrate_control, 2);
    set_bit(es1);
    clear_bit(ti);
    clear_bit(ri);
    clear_bit(es0);
}
void set_baudrate_57600bps(void)
{
    // Calculate reload value: -27000000/(57600*32) ≈ -14 = 0xFFF2
    uint16_t reload = 0xFFF2;
    SFR_IO_baudrate_control &= ~(1 << 2);
    SFR_IO_baudrate_control = 0x30;
    SFR_IO_sio_scon = 0x50; // UART mode: 8-bit, no parity.
    SFR_IO_baudrate_msb = reload >> 8;
    SFR_IO_baudrate_lsb = reload & 0xFF;
    SFR_IO_baudrate_control |= (1 << 2);
    ES1 = 1; // Enable Timer2 interrupt (if applicable)
    TI = 0;
    RI = 0;
    ES0 = 0; // Disable UART interrupt.
}

void init_timer_hardware(void)
{
    TMOD = 0x11; // Timer0: mode 1; Timer1: mode 1.
    // Set reload values (example values)
    TH0 = 0xFB;
    TL0 = 0x9F;
    TH1 = 0xD8;
    TL1 = 0xEF;
    ET0 = 0; // Disable Timer0 interrupt.
    ET1 = 1; // Enable Timer1 interrupt.
    TR0 = 1; // Start Timer0.
    TR1 = 1; // Start Timer1.
}

void main_init_part_2(void)
{
    watchdog_disable();
    init_lcd_pins_and_force_display_off();
    init_timer_hardware();
    set_baudrate_57600bps();
    watchdog_enable();
    init_ack_irqs();
}

void main_init_part_3(void)
{
    init_variables();
    detect_flash_chiptype();
    init_load_settings();
}

void init_OSD_hardware()
{
    init_io_via_io_list(init_osd_io_list);

    // Clear background map
    write_io(IO_OSD_bgmap_addr_lsb, 0x00);
    uint8_t r0 = 0x00; // Inner loop count (256 iterations)
    uint8_t r1 = 0x00; // Address MSB

    do
    {
        write_io(IO_OSD_bgmap_addr_msb, r1);
        write_io(IO_OSD_bgmap_data_msb, 0x01C0 >> 8);
        write_io(IO_OSD_bgmap_data_lsb, 0x01C0 & 0xFF);

        for (uint8_t i = 0; i < 0x100; i++)
        {
            write_io(IO_OSD_bgmap_data_lsb, 0x01C0 & 0xFF);
        }

        write_io(IO_OSD_bgmap_data_attr, 0x12);
        for (uint8_t i = 0; i < 0x100; i++)
        {
            write_io(IO_OSD_bgmap_data_attr, 0x12);
        }

        r1++;
    } while (r1 < 2);

    write_io(IO_OSD_font_addr_lsb, 0x00);
    write_io(IO_OSD_font_data_lsb, 0x00);
    write_io(IO_OSD_font_data_msb, 0x00);
    write_io(IO_OSD_font_addr_msb, 0x00);

    // TODO: Initialize bitmap colors
}

void init_AV_stuff()
{
    write_io(IO_AV_video_on_off, read_io(IO_AV_video_on_off) | 0x1C);
    write_io(IO_AV_input_select_reg_0, (read_io(IO_AV_input_select_reg_0) | 0x1B) & 0xDB);
    write_io(IO_AV_config_FED9h_bits, (read_io(IO_AV_config_FED9h_bits) & 0xCF) | 0x40);
    write_io(IO_AV_config_FEDBh_bit, read_io(IO_AV_config_FEDBh_bit) & 0x7F);
    write_io(IO_AV_ctrl_whatever_1, read_io(IO_AV_ctrl_whatever_1) | 0x10);
    write_io(IO_AV_config_FE04h, 0x30);
    write_io(IO_AV_config_FE05h, 0x40);
}

void main_init_part_4()
{
    // Initialize various hardware components
    init_part_4_subfunc(); // Initialize I/O
    init_OSD_hardware();   // Initialize OSD
#ifdef WITH_INFRARED
    init_IR_hardware(); // Initialize IR (if enabled)
#endif
    init_ADC_analog_hardware(); // Initialize ADC
    init_AV_stuff();            // Initialize AV settings
    apply_settings_to_IO_ports_except_backlight();
}

void main_init_part_5(void)
{
    display_spi_reset();
    display_spi_detect();
    display_spi_dump_initial();
    display_spi_init();
}

// Simulated memory-mapped variables
volatile uint8_t xram_new_detected_video = 0;
volatile uint8_t iram_ir_msb_device = 0;
volatile uint8_t iram_ir_lsb_cmd = 0;
volatile uint8_t xram_palntsc_same_counter = 0;

void init_variables()
{
    // Set xram_new_detected_video to 0xFF
    xram_new_detected_video = 0xFF;

    // Set IR-related variables to 0xFF
    iram_ir_msb_device = 0xFF;
    iram_ir_lsb_cmd = 0xFF;

    // Clear xram_palntsc_same_counter
    xram_palntsc_same_counter = 0x00;
}

void init_lcd_pins_and_force_display_off()
{
    output_initial_data_from_extra_fixed_io_list();
    xlat_r7_to_forced_blank_color(0x06);
    firm_set_p35_backlight_off();
    firm_set_p36_volume_off();
}

void init_ack_irqs()
{
    disable_global_interrupts();

    set_irq_vector(0x03, firm_irq_0003h_ext_int0_infrared);
    set_irq_vector(0x1B, firm_irq_001Bh_timer_1_overflow);
    set_irq_vector(0x43, firm_irq_0043h_adc_keypad);
    set_irq_vector(0x5B, firm_irq_005Bh_framerate);

    write_io(SFR_IO_ipc, 0x00);
    set_bit(it0);
    clear_bit(ie0);
    clear_bits(SFR_IO_xxx91h, 0xF0);
    set_bit(SFR_IO_xxxF8h, 1);
    clear_bits(SFR_IO_xxxF8h, 0x1D);

#ifdef WITH_INFRARED
    enable_external_interrupt0();
#else
    disable_external_interrupt0();
#endif

#ifdef WITH_KEYPAD_IRQ
    set_bit(SFR_IO_IEC2, 0);
#else
    clear_bit(SFR_IO_IEC2, 0);
#endif

    clear_bits(SFR_IO_IEC2, 0x06);
    set_bit(SFR_IO_IEC2, 3);
}

// Memory-mapped SFR (Special Function Registers)
#define SFR_IO_IPC ((volatile uint8_t *)0x90)   // Interrupt Priority Register
#define SFR_IO_XX91H ((volatile uint8_t *)0x91) // Unknown IRQ Ack Register
#define SFR_IO_XXF8H ((volatile uint8_t *)0xF8) // Interrupt Enable Control
#define SFR_IO_IEC2 ((volatile uint8_t *)0xB0)  // Interrupt Enable Control 2
#define ENABLE_INTERRUPTS() __asm__("setb ea")  // Enable global interrupts
#define DISABLE_INTERRUPTS() __asm__("clr ea")  // Disable global interrupts

// **Set IRQ Vector Table**
void set_irq_vector(uint8_t irq_num, void *isr_addr)
{
    volatile uint16_t *irq_vector = (volatile uint16_t *)(0x200 + irq_num);
    *irq_vector = (uint16_t)isr_addr;
}

// **Initialize and Acknowledge IRQs**
void init_ack_irqs()
{
    // Disable global interrupts
    DISABLE_INTERRUPTS();

    // Set Interrupt Vectors
    set_irq_vector(0x03, (void *)firm_irq_0003h_ext_int0_infrared);
    set_irq_vector(0x1B, (void *)firm_irq_001Bh_timer_1_overflow);
    set_irq_vector(0x43, (void *)firm_irq_0043h_adc_keypad);
    set_irq_vector(0x5B, (void *)firm_irq_005Bh_framerate);

    // Set Interrupt Priorities (IPC Register)
    *SFR_IO_IPC = 0x00;

    // Configure External Interrupt 0 (Infrared)
    *SFR_IO_XX91H &= ~(0x10 | 0x20 | 0x40 | 0x80); // Clear IRQ Flags (Bit4-7)
    *SFR_IO_XXF8H = 0x02;                          // Enable Interrupt #1

// Enable/Disable Infrared IRQ (External Interrupt 0)
#ifdef WITH_INFRARED
    __asm__("setb ex0"); // Enable External IRQ 0
#else
    __asm__("clr ex0");        // Disable External IRQ 0
#endif

// Enable ADC Keypad IRQ if required
#ifdef WITH_KEYPAD_IRQ
    *SFR_IO_IEC2 |= (1 << 0); // Enable ADC Interrupt
#else
    *SFR_IO_IEC2 &= ~(1 << 0); // Disable ADC Interrupt
#endif

    *SFR_IO_IEC2 &= ~(1 << 1); // Disable Unused IRQ
    *SFR_IO_IEC2 &= ~(1 << 2); // Disable Unused IRQ
    *SFR_IO_IEC2 |= (1 << 3);  // Enable VBlank IRQ (Framerate Sync)

    // Re-enable interrupts
    ENABLE_INTERRUPTS();
}

void init_part_4_subsubfunc()
{
    write_io(SFR_IO_whatever_config, read_io(SFR_IO_whatever_config) | 0x07);
    init_io_via_io_list(io_list_1);

    write_io(IO_PLL_19h_used, read_io(IO_PLL_19h_used) | 0x81);
    init_io_via_io_list(io_list_2);

    write_io(IO_AV_ctrl_whatever_1, read_io(IO_AV_ctrl_whatever_1) | 0x02);
    init_io_via_io_list(io_list_3);
}

void init_part_4_subfunc()
{
    init_part_4_subsubfunc();
    output_initial_data_from_extra_fixed_io_list();
    init_io_via_huge_fixed_io_list();

    write_xram(xram_curr_input, 0x00);
    apply_av_input(0x00);

    write_io(IO_PLL_0Eh_used, 0x2C);
}
