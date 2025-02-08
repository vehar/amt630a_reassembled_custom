#include "firmware.h"

//====================================================================
// Global Variables
//====================================================================

volatile unsigned long systemTick = 0;
volatile unsigned int frameRateCounter = 0;

unsigned char ir_command = 0xFF; // No IR command received by default.
unsigned char keypad_key = 0;
unsigned char key_input = 0;
unsigned char lcd_status = 0; // 0 = off, 1 = on.
unsigned char current_menu_option = 0;

// Dummy variables to simulate ADC and other flag registers:
sbit ADC_IF = P1 ^ 0;   // Not a real 8051 register—placeholder.
sbit OTHER_IF = P1 ^ 1; // Placeholder for “other IRQ flag.”

// Dummy variable for LCD status (used in OSD functions)
unsigned char lcd_status_register = 0;

void system_init(void)
{
    init_timer_hardware();
    init_io();
    watchdog_disable();
    // Initialize additional peripherals here (ADC, IR, OSD, flash, etc.)
}

void watchdog_enable()
{
    write_io(SFR_IO_watchdog_unlock, 0x55);
    write_io(SFR_IO_watchdog_config1, 0x03);
    write_io(SFR_IO_watchdog_config3, 0x8F);
    write_io(SFR_IO_watchdog_config2, 0xFF);
    write_io(SFR_IO_watchdog_enable, 0x01);
    write_io(SFR_IO_watchdog_unlock, 0xAA);
}

void watchdog_disable()
{
    write_io(SFR_IO_watchdog_unlock, 0x55);
    write_io(SFR_IO_watchdog_enable, 0x00);
    write_io(SFR_IO_watchdog_unlock, 0xAA);
}

void watchdog_reload()
{
    write_io(SFR_IO_watchdog_unlock, 0x55);
    write_io(SFR_IO_watchdog_reload, 0xBB);
    write_io(SFR_IO_watchdog_reload, read_io(SFR_IO_watchdog_reload) ^ 0xFF);
    write_io(SFR_IO_watchdog_reload, 0x00);
    write_io(SFR_IO_watchdog_unlock, 0xAA);
}

//====================================================================
// Timer and Interrupt Helper Functions
//====================================================================

void timer0_reload(void)
{
    TH0 = 0xFB;
    TL0 = 0x9F;
    TR0 = 1;
}

void timer1_overflow(void)
{
    TR1 = 0;
    TH1 = 0xD8;
    TL1 = 0xEF;
    TR1 = 1;
    systemTick++; // Update global tick counter.
}

void process_ir_interrupt(void)
{
    // Read IR_DATA; if valid (assume 0xFF means “no command”), store it.
    unsigned char command = IR_DATA;
    if (command != 0xFF)
    {
        ir_command = command;
        // For demonstration, toggle an LED (e.g., P1.0).
        P1 ^= 0x01;
    }
    // Clear an IR flag bit (for example, bit0)
    IR_FLAGS &= ~0x01;
}

void adc_keypad_handler(void)
{
    // Read ADC conversion result and interpret it as a keypad value.
    unsigned int result = ADC_RESULT;
    if (result < 100)
    {
        keypad_key = 1; // For example, key code 1 pressed.
    }
    else
    {
        keypad_key = 0;
    }
    // Clear ADC interrupt flag.
    ADC_IF = 0;
}

void acknowledge_adc(void)
{
    // Clear ADC interrupt flag.
    ADC_IF = 0;
}

void acknowledge_other_irq(void)
{
    // Clear the “other” IRQ flag (e.g., clear bit 6 of OTHER_IF).
    OTHER_IF &= ~0x40;
}

void update_framerate(void)
{
    frameRateCounter++;
    static unsigned long lastTick = 0;
    if (systemTick - lastTick >= 1000)
    {
        unsigned int fps = frameRateCounter;
        // (Optionally store or display the FPS.)
        frameRateCounter = 0;
        lastTick = systemTick;
    }
}

void timer2_compare_handler(void)
{
    // For demonstration, do nothing or toggle a bit.
    // P1 ^= 0x02; // Example: toggle P1.1.
}

void init_timer_hardware()
{
    disable_timer0();
    disable_timer1();

    write_io(SFR_IO_TMOD, 0x11);
    write_io(SFR_IO_timer0_msb, 0xFB);
    write_io(SFR_IO_timer0_lsb, 0x9F);
    write_io(SFR_IO_timer1_msb, 0xD8);
    write_io(SFR_IO_timer1_lsb, 0xEF);
    write_io(SFR_IO_timer3_lsb, 0x00);
    write_io(SFR_IO_timer3_msb, 0x00);
    write_io(SFR_IO_timer4_lsb, 0x00);
    write_io(SFR_IO_timer4_msb, 0x00);

    disable_timer0_interrupt();
    enable_timer1_interrupt();
    disable_timer0();
    enable_timer1();
}

//====================================================================
// Flash and Settings Functions
//====================================================================

//====================================================================
// OSD and UI Functions
//====================================================================

void firm_set_bgmap_addr(unsigned int addr)
{
    IO_OSD_BG_ADDR_LSB = (unsigned char)(addr & 0xFF);
    IO_OSD_BG_ADDR_MSB = (unsigned char)((addr >> 8) & 0xFF);
}

void firm_raise_font_addr(void)
{
    unsigned char temp = IO_OSD_FONT_ADDR_LSB;
    temp++;
    IO_OSD_FONT_ADDR_LSB = temp;
    if (temp == 0)
        IO_OSD_FONT_ADDR_MSB++;
}

void firm_raise_bgmap_addr(void)
{
    unsigned char temp = IO_OSD_BG_ADDR_LSB;
    temp++;
    IO_OSD_BG_ADDR_LSB = temp;
    if (temp == 0)
        IO_OSD_BG_ADDR_MSB++;
}

void sysgui_make_font(void)
{
    // Fill OSD font memory with a dummy pattern.
    unsigned int count = 256 * 8; // 256 characters, 8 rows each.
    firm_set_font_addr(0);
    while (count--)
    {
        unsigned int fontAddr = ((unsigned int)IO_OSD_FONT_ADDR_MSB << 8) | IO_OSD_FONT_ADDR_LSB;
        *((volatile unsigned char *)fontAddr) = 0xAA; // Dummy pattern.
        firm_raise_font_addr();
    }
}

unsigned char getkey_with_keyrepeat(void)
{
    unsigned char key = key_input;
    key_input = 0; // Clear after reading.
    return key;
}

unsigned char getkey_with_fast_keyrepeat(void)
{
    unsigned char key = key_input;
    key_input = 0;
    return key;
}

//====================================================================
// Apply Settings Functions
//====================================================================

void apply_tint()
{
    if (read_io(IO_AV_stat_framerate_flag) & 0x04 || read_xram(xram_sett_pal_ntsc))
    {
        write_io(IO_LCD_basic_tint, 0x00);
        return;
    }

    int8_t tint = read_xram(xram_sett_tint) - 0x50;
    tint = (tint != 0) ? (tint ^ 0x80) : 0;
    write_io(IO_LCD_basic_tint, tint);
}
void apply_tint(void)
{
    *((volatile unsigned char *)0xC2) = 0x44; // Dummy tint value.
}

void force_pal60()
{
    write_io(IO_VIDEO_control, read_io(IO_VIDEO_control) | 0x01);
    write_io(IO_AV_ctrl_whatever_1, read_io(IO_AV_ctrl_whatever_1) | 0x01);

    if (!read_xram(xram_sett_panel_type))
        write_io(IO_60HZ_15khz_lsb, 0xC0);
}

void apply_allow_c64()
{
    uint8_t value = read_xram(xram_sett_allow_c64) ? 0x20 : 0x00;
    write_io(IO_AV_secret_control, value);
}
void apply_allow_c64(void)
{
    *((volatile unsigned char *)0xC4) = 0x20;
}

void apply_tianma_r0f()
{
    uint8_t value = read_xram(xram_sett_tianma_r0f) ? 0xA4 : 0x24;
    write_NV3035C_index_r0_data(0x0F, value);
}
void apply_tianma_r0f(void)
{
    *((volatile unsigned char *)0xC5) = 0x0F;
}

// Simulated memory locations
volatile uint8_t xram_sett_yuv_consts = 0; // Determines which YUV constants to use

// Placeholder for LCD configuration registers
volatile uint8_t IO_LCD_config[12] = {0};

void apply_yuv_constants()
{
    const uint8_t *src;

    // Select source based on xram_sett_yuv_consts
    if (xram_sett_yuv_consts == 0)
        src = new_yuv_constants;
    else
        src = old_yuv_constants;

    // Apply the YUV constants to the LCD config
    for (uint8_t i = 0; i < 12; i++)
        IO_LCD_config[i] = src[i];
}

// Apply YUV constants into the LCD configuration registers.
// Assume IO_LCD_config_FFF0h is the start of a 12–byte block.
void apply_yuv_constants(void)
{
    volatile unsigned char *dst = (volatile unsigned char *)IO_LCD_config_FFF0h;
    int i;
    if (xram_sett_yuv_consts != 0)
    {
        for (i = 0; i < 12; i++)
            dst[i] = new_yuv_constants[i];
    }
    else
    {
        for (i = 0; i < 12; i++)
            dst[i] = old_yuv_constants[i];
    }
}
void apply_yuv_constants()
{
    const uint8_t *src = read_xram(xram_sett_yuv_consts) ? old_yuv_constants : new_yuv_constants;
    write_io_multiple(IO_LCD_config_FFF0h, src, 12);
}

// Simulated memory-mapped registers
volatile uint8_t xram_sett_backlight = 50; // Backlight setting (0-100)

volatile uint8_t IO_PWM0_DUTY_TOTAL_LSB;
volatile uint8_t IO_PWM0_DUTY_TOTAL_MSB;
volatile uint8_t IO_PWM0_DUTY_HIGH_LSB;
volatile uint8_t IO_PWM0_DUTY_HIGH_MSB;
volatile uint8_t IO_PIN_P35_P36_PWM;
volatile uint8_t IO_PWM_ENABLE_FLAGS;

// Constants for PWM calculations
#define DUTY_HIGH 2000
#define TOTAL_PWM (DUTY_HIGH * 2) // Must be 2x for smooth transition

void apply_backlight()
{
    uint8_t duty = xram_sett_backlight;

    if (duty == 0 || duty > 49)
    { // Use FIXED TOTAL when duty=0 or duty>49
        // Set PWM total to a fixed value
        IO_PWM0_DUTY_TOTAL_LSB = (uint8_t)(TOTAL_PWM & 0xFF);
        IO_PWM0_DUTY_TOTAL_MSB = (uint8_t)(TOTAL_PWM >> 8);

        // Compute duty cycle (total / 100 * duty)
        uint16_t duty_high = (TOTAL_PWM / 100) * duty;
        IO_PWM0_DUTY_HIGH_LSB = (uint8_t)(duty_high & 0xFF);
        IO_PWM0_DUTY_HIGH_MSB = (uint8_t)(duty_high >> 8);
    }
    else
    {                                                                 // Use FIXED HIGH when duty=1..49
        uint16_t total = div_dptr_by_r0(DUTY_HIGH * (100 / 4), duty); // total = DUTY_HIGH * 100 / duty
        total *= 4;                                                   // Restore correct scale

        // Clamp to 16-bit max if overflow
        if (total > 0xFFFF)
            total = 0xFFFF;

        // Set PWM total
        IO_PWM0_DUTY_TOTAL_LSB = (uint8_t)(total & 0xFF);
        IO_PWM0_DUTY_TOTAL_MSB = (uint8_t)(total >> 8);

        // Set fixed duty cycle
        IO_PWM0_DUTY_HIGH_LSB = (uint8_t)(DUTY_HIGH & 0xFF);
        IO_PWM0_DUTY_HIGH_MSB = (uint8_t)(DUTY_HIGH >> 8);
    }

    // Enable PWM output
    IO_PIN_P35_P36_PWM |= 3;  // Enable PWM mode
    IO_PWM_ENABLE_FLAGS |= 1; // Enable PWM #0
}

void apply_backlight()
{
    uint8_t duty = read_xram(xram_sett_backlight);
    uint16_t total = (duty == 0 || duty > 49) ? 4000 : (2000 * 100 / duty);
    write_io(IO_PWM0_duty_total_lsb, total & 0xFF);
    write_io(IO_PWM0_duty_total_msb, (total >> 8) & 0xFF);
    write_io(IO_PWM0_duty_high_lsb, 0x00);
    write_io(IO_PWM0_duty_high_msb, 0x20);
    write_io(IO_PIN_P35_P36_pwm, read_io(IO_PIN_P35_P36_pwm) | 0x03);
    write_io(IO_PWM_enable_flags, read_io(IO_PWM_enable_flags) | 0x01);
}

// Adjust backlight PWM based on the xram_sett_backlight value.
// This implementation uses a simplified calculation.
void apply_backlight(void)
{
    unsigned char duty = xram_sett_backlight; // 0 to 100
    unsigned int total, high;
    if (duty == 0 || duty > 50)
    {
        total = 2000 * 2; // fixed total value (example)
        high = (total * duty) / 100;
    }
    else
    {
        // For low duty values, use fixed high value.
        high = 2000;
        total = (high * 100) / duty;
    }
    IO_PWM0_duty_total = total;
    IO_PWM0_duty_high = high;
    // Switch backlight pin to PWM mode and enable PWM.
    IO_PIN_P35_P36_pwm |= 0x03;
    IO_PWM_enable_flags |= 0x01;
}

void apply_volume()
{
    uint8_t volume = read_xram(xram_sett_volume);
    uint16_t total = 100;
    uint16_t duty = (total * volume) / 100;
    write_io(IO_PWM1_duty_total_lsb, total & 0xFF);
    write_io(IO_PWM1_duty_total_msb, (total >> 8) & 0xFF);
    write_io(IO_PWM1_duty_high_lsb, duty & 0xFF);
    write_io(IO_PWM1_duty_high_msb, (duty >> 8) & 0xFF);
    write_io(IO_PIN_P35_P36_pwm, read_io(IO_PIN_P35_P36_pwm) | 0x30);
    write_io(IO_PWM_enable_flags, read_io(IO_PWM_enable_flags) | 0x02);
}
// Adjust volume PWM based on the xram_sett_volume value.
void apply_volume(void)
{
    unsigned char volume = xram_sett_volume; // 0 to 100
    unsigned int total = 100;                // fixed total (example)
    unsigned int high = (total * volume) / 100;
    IO_PWM1_duty_total = total;
    IO_PWM1_duty_high = high;
    // Set PWM1 mode on the corresponding pin and enable PWM.
    IO_PIN_P35_P36_pwm |= 0x30;
    IO_PWM_enable_flags |= 0x02;
}

void apply_xyflip()
{
    uint8_t xyflip = read_xram(xram_sett_xyflip) ^ 0x03;
    write_NV3035C_index_r0_data(0x02, xyflip);
    write_io(IO_OSD_xyflip, (xyflip << 4));
}
void apply_xyflip(void)
{
    unsigned char flip = 0;
    flip |= getkey_with_keyrepeat() & 0x01;      // Dummy: use key input as flag
    flip |= (getkey_with_fast_keyrepeat() << 1); // Dummy: use fast key repeat for y–flip flag
    *((volatile unsigned char *)0xC1) = flip;
}

void apply_panel_type()
{
    uint8_t panel_type = read_xram(xram_sett_panel_type);
    write_io_list(panel_type ? lcd_pin_variant_io_list_innolux : lcd_pin_variant_io_list_tianma);
    write_io_list(panel_type ? resolution_io_list_for_480x272 : resolution_io_list_for_320x240);
}

// Simulated memory-mapped I/O registers
volatile uint8_t IO_VIDEO_something_4 = 0x45; // Example: Video control register
volatile uint8_t IO_VIDEO_something_5 = 0x00; // Unused/read-only in assembly

// Simulated configuration settings
volatile uint8_t xram_sett_panel_type = 0; // 0 = 320x240, 1 = 480x272
volatile uint8_t xram_sett_ratio = 0;      // 0 = narrow (4:3), 1 = wide (16:9)

// Predefined mode ratio configuration lists
const uint8_t mode_ratio_io_list_320x240_narrow[] = {/* Data here */, 0xFF};
const uint8_t mode_ratio_io_list_320x240_wide[] = {/* Data here */, 0xFF};
const uint8_t mode_ratio_io_list_480x272_narrow[] = {/* Data here */, 0xFF};
const uint8_t mode_ratio_io_list_480x272_wide[] = {/* Data here */, 0xFF};

void apply_mode_ratio()
{
    // Modify IO_VIDEO_something_4 (pause display?)
    IO_VIDEO_something_4 |= 0x40;

    // Select the correct mode ratio list based on panel type
    const uint8_t *ratio_list;
    if (xram_sett_panel_type == 0)
    {                                                 // 320x240
        ratio_list = mode_ratio_io_list_320x240_wide; // Default to wide
    }
    else
    { // 480x272
        ratio_list = (xram_sett_ratio == 0) ? mode_ratio_io_list_480x272_narrow
                                            : mode_ratio_io_list_480x272_wide;
    }

    // Apply the selected I/O configuration list
    init_io_via_io_list(ratio_list);

    // Modify IO_VIDEO_something_5 (resume display?)
    IO_VIDEO_something_5 = 0x00; // This register seems unused/read-only
}

void apply_pal_ntsc(void)
{
    // For demonstration, assume NTSC (0) is selected.
    unsigned char pal_ntsc = 0;
    if (pal_ntsc)
        *((volatile unsigned char *)0xC3) = 0x01;
    else
        *((volatile unsigned char *)0xC3) = 0x00;
}
void apply_pal_ntsc()
{
    if (read_xram(xram_sett_pal_ntsc))
    {
        force_pal60();
        return;
    }

    write_io(IO_VIDEO_control, read_io(IO_VIDEO_control) & ~0x01);
    write_io(IO_AV_ctrl_whatever_1, read_io(IO_AV_ctrl_whatever_1) & ~0x01);

    if (!read_xram(xram_sett_panel_type))
    {
        write_io(IO_60HZ_15khz_lsb, 0xC8);
    }
}

void apply_settings_to_IO_ports()
{
    apply_backlight();
    apply_volume();
    apply_settings_to_IO_ports_except_backlight();
}

void apply_settings_to_IO_ports_except_backlight()
{
    apply_panel_type();
    apply_backdrop();
    apply_mode_ratio();
    apply_xyflip();
    apply_rgb_gamma_ramps();
    apply_yuv_constants();
    apply_pal_ntsc();
    apply_allow_c64();
    apply_tianma_r0f();

    write_io(IO_LCD_basic_brightness, read_xram(xram_sett_brightness) + (0x80 - 0x50));
    write_io(IO_LCD_basic_contrast, read_xram(xram_sett_contrast) + (0x80 - 0x50));
    write_io(IO_LCD_basic_saturation, read_xram(xram_sett_saturation) + (0x36 - 0x50));

    apply_tint();

    uint8_t av_video_on = 0x00;
    xlat_r7_to_forced_blank_color(av_video_on);
}

void apply_backdrop()
{
    uint8_t config = read_xram(xram_sett_no_signal);
    uint8_t backdrop_color = (config & 0x01) ? 0x13 : 0x00;
    uint8_t backdrop_cb = (config & 0x01) ? 0xDD : 0x80;
    uint8_t backdrop_cr = (config & 0x01) ? 0x72 : 0x80;

    write_io(IO_LCD_snow_enable_and_misc, (read_io(IO_LCD_snow_enable_and_misc) & ~0x80) | ((config & 0x02) << 6));
    write_io(IO_backdrop_snow_level, 0x6C);
    write_io(IO_LCD_backdrop_color_Y, backdrop_color);
    write_io(IO_LCD_backdrop_color_Cb, backdrop_cb);
    write_io(IO_LCD_backdrop_color_Cr, backdrop_cr);
}

void init_load_settings()
{
    if (!flash_read_settings(xram_settings_start, flash_settings_1_addr_div_100h))
    {
        if (!flash_read_settings(xram_settings_start, flash_settings_2_addr_div_100h))
            firm_reset_all_settings();
    }
}

void firm_reset_all_settings()
{
    memset_xram(xram_settings_start, 0, xram_settings_size);
    uint8_t default_value = 0x32;
    write_xram(xram_sett_backlight, default_value);
    write_xram(xram_sett_brightness, default_value);
    write_xram(xram_sett_saturation, default_value);
    write_xram(xram_sett_contrast, default_value);
    write_xram(xram_sett_tint, default_value);
    write_xram(xram_sett_volume, default_value);
}
void firm_reset_all_settings(void)
{
    unsigned char *settings = (unsigned char *)0x8000;
    unsigned int len = 64;
    unsigned int i;
    for (i = 0; i < len; i++)
        settings[i] = 0;
    // Now set default values:
    settings[0] = 0x32; // Default backlight (dummy)
    settings[1] = 0x32; // Default brightness (dummy)
    // etc.
}

void firm_do_save_settings()
{
    force_valid_settings();
    write_xram(xram_sett_id_5Ah, 0x5A);
    write_xram(xram_sett_id_A5h, 0xA5);
    write_xram(xram_sett_chksum, firm_calc_settings_chksum());

    flash_write_settings(xram_settings_start, flash_settings_1_addr_div_100h);
    firm_wait_a_milliseconds(50);
    flash_write_settings(xram_settings_start, flash_settings_2_addr_div_100h);
    firm_wait_a_milliseconds(50);
}
void firm_do_save_settings(void)
{
    unsigned char chk = firm_calc_settings_chksum();
    *((volatile unsigned char *)0x8040) = chk; // Store checksum in RAM.
    // Now write the settings area to flash memory.
    flash_write_dptr_to_flash((unsigned char *)0x8000, 0x1000, 64);
}

uint8_t firm_calc_settings_chksum()
{
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < xram_settings_size; i++)
        checksum ^= read_xram(xram_settings_start + i);
    return checksum;
}

unsigned char firm_calc_settings_chksum(void)
{
    unsigned char chk = 0;
    unsigned char *settings = (unsigned char *)0x8000; // Example settings area in RAM.
    unsigned int len = 64;                             // Assume 64 bytes of settings.
    unsigned int i;
    for (i = 0; i < len; i++)
        chk ^= settings[i];
    return chk;
}
//====================================================================
// Utility Functions
//====================================================================
void firm_wait_a_milliseconds(uint8_t a)
{
    uint8_t r0;
    while (a--)
    {
        r0 = 0xD0;
        while (r0--)
        {
            __asm__("nop"); // Delay loop
        }
    }
}
void firm_wait_a_milliseconds(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 200; j++)
            _nop_();
    }
}

void xlat_r7_to_forced_blank_color(uint8_t r7)
{
    if (r7 > 6)
        return;
    static const uint8_t blank_color_list[] = {0x4F, 0x50, 0x51, 0x52, 0x53, 0x55, 0x54};
    write_io(IO_LCD_forced_blank_color, blank_color_list[r7]);
}

//====================================================================
// LCD/Display Functions
//====================================================================

void switch_lcd_screen_on(void)
{
    lcd_status = 1;
    sysgui_wrstr("\r\nLCD Screen ON");
}

void switch_lcd_screen_off(void)
{
    lcd_status = 0;
    sysgui_wrstr("\r\nLCD Screen OFF");
}

void switch_screen_and_backlight_on(void)
{
    switch_lcd_screen_on();
    P2 |= 0x01; // For example, set bit0 of Port2 to turn on backlight.
    sysgui_wrstr("\r\nBacklight ON");
}

void switch_screen_and_backlight_off(void)
{
    P2 &= ~0x01; // Turn off backlight.
    switch_lcd_screen_off();
    sysgui_wrstr("\r\nBacklight OFF");
}

//============================================================
// OSD GAMMA RAMP FUNCTIONS
//============================================================
void apply_rgb_gamma_ramps()
{
    uint8_t ramp_mode = read_xram(xram_sett_rgb_ramps);

    if (ramp_mode == 0)
    {
        set_gamma_ramp(IO_LCD_gamma_ramp_red, RAMP_NORMAL_NEW);
        set_gamma_ramp(IO_LCD_gamma_ramp_green, RAMP_NORMAL_NEW);
        set_gamma_ramp(IO_LCD_gamma_ramp_blue, RAMP_NORMAL_NEW);
    }
    else if (ramp_mode == 1)
    {
        set_gamma_ramp(IO_LCD_gamma_ramp_red, RAMP_NORMAL_OLD);
        set_gamma_ramp(IO_LCD_gamma_ramp_green, RAMP_NORMAL_OLD);
        set_gamma_ramp(IO_LCD_gamma_ramp_blue, RAMP_NORMAL_OLD);
    }
    else
    {
        set_gamma_ramp(IO_LCD_gamma_ramp_red, RAMP_LINEAR);
        set_gamma_ramp(IO_LCD_gamma_ramp_green, RAMP_LINEAR);
        set_gamma_ramp(IO_LCD_gamma_ramp_blue, RAMP_LINEAR);
    }
}

// The old_yuv_constants() and new_yuv_constants() functions would similarly copy
// constant tables into the proper registers. (Omitted for brevity.)
void firm_swap_dptr_vs_r1r0(uint16_t *dptr, uint16_t *r1r0)
{
    uint16_t temp = *dptr;
    *dptr = *r1r0;
    *r1r0 = temp;
}

//------------------------------------------------------------
// Vertical Blank Wait Function
//------------------------------------------------------------
void firm_wait_vblank(void)
{
    /* In the original assembly the routine checks that:
       - The PLL/AV/dotclk is on,
       - Master IRQ and framerate IRQ are enabled,
       then it polls the sys_vblank_flag.
       Here we simulate this behavior.
    */
    if ((*(volatile unsigned char *)IO_PLL_12h_used) == 0)
    {
        // PLL is off; skip waiting.
        return;
    }

    if (((*(volatile unsigned char *)SFR_IO_iec) & 0x80) == 0 ||
        ((*(volatile unsigned char *)SFR_IO_IEC2) & 0x08) == 0)
    {
        // IRQ not enabled: do a “light” wait.
        while (((*(volatile unsigned char *)SFR_IO_xxx91h) & 0x80) == 0)
            ; // busy‐wait until bit7 is set
        *(volatile unsigned char *)SFR_IO_xxx91h &= ~0x80;
        return;
    }
    while (*(volatile unsigned char *)sys_vblank_flag == 0)
    {
        // Optionally, execute a halt/idle instruction here.
        _nop_();
    }
    // Clear vblank flag for next cycle.
    *(volatile unsigned char *)sys_vblank_flag = 0;
}

void firm_timer_coarse_enter(void)
{
    // Enter coarse mode: clear the coarse offhold counter and set a bit.
    *(volatile unsigned char *)xram_irq_coarse_offhold = 0;
    unsigned char ctrl = *(volatile unsigned char *)IO_AV_ctrl_whatever_2;
    ctrl |= 0x40;
    *(volatile unsigned char *)IO_AV_ctrl_whatever_2 = ctrl;
}

//====================================================================
// Main Program
//====================================================================

//
// Gamma ramp arrays:
//
const unsigned char old_gamma_ramp[32] = {
    0x03, 0x06, 0x0A, 0x0E, 0x14, 0x1A, 0x21,
    0x29, 0x34, 0x40, 0x4D, 0x59, 0x66, 0x73, 0x81,
    0x8E, 0x9C, 0xA7, 0xB1, 0xBA, 0xC2, 0xCA, 0xD0,
    0xD7, 0xDD, 0xE2, 0xE7, 0xEC, 0xF1, 0xF6, 0xFA};

const unsigned char new_gamma_ramp[32] = {
    0x03, 0x07, 0x0B, 0x10, 0x15, 0x1B, 0x22,
    0x2A, 0x34, 0x3F, 0x4B, 0x58, 0x65, 0x72, 0x7E,
    0x89, 0x96, 0xA2, 0xAE, 0xBA, 0xC4, 0xCD, 0xD5,
    0xDC, 0xE2, 0xE7, 0xEC, 0xF0, 0xF4, 0xF8, 0xFB};

//
// YUV constant arrays:
//
const unsigned char old_yuv_constants[12] = {0x1A, 0x06, 0xD4, 0xD2, 0xF1, 0x0E, 0x15, 0xE4, 0xF6, 0xF1, 0x1B, 0x81};
const unsigned char new_yuv_constants[12] = {0x11, 0x00, 0x00, 0xE9, 0xE1, 0x0E, 0x09, 0xEE, 0xF4, 0xF1, 0x23, 0x81};

// Adjust “sharpness” based on AV_stat_detect_2.
void firm_timer_adjust_sharpness(void)
{
    unsigned char current_sharp = IO_AV_stat_detect_2;
    // If the sharpness value has changed:
    if (current_sharp != xram_tmp__24h)
    {
        // Update stored value:
        xram_old_AV_stat_detect_2 = current_sharp;
        // Reset sharpness offhold:
        xram_irq_sharpness_offhold = 0;
    }
    else
    {
        // If stable, optionally adjust LCD sharpness.
        if (xram_irq_sharpness_offhold < 10)
        { // threshold of 10, for example
            xram_irq_sharpness_offhold++;
        }
        else
        {
            // If stable and reached threshold, adjust the LCD sharpness control.
            // For demonstration, if bit4 of current_sharp is clear, clear bit1;
            // otherwise set bit1.
            if (current_sharp & 0x10)
                IO_LCD_sharpness_or_so |= 0x02;
            else
                IO_LCD_sharpness_or_so &= ~0x02;
        }
    }
}

void firm_timer_adjust_sharpness(void)
{
    // Compare the new detect_2 value with stored old value.
    unsigned char new_val = *(volatile unsigned char *)IO_AV_stat_detect_2;
    unsigned char old_val = *(volatile unsigned char *)xram_old_AV_stat_detect_2;
    if (new_val != old_val)
    {
        *(volatile unsigned char *)xram_old_AV_stat_detect_2 = new_val;
        *(volatile unsigned char *)xram_irq_sharpness_offhold = 0;
        return;
    }
    // Otherwise, if stable, increase the offhold counter (up to 0x0A).
    unsigned char offhold = *(volatile unsigned char *)xram_irq_sharpness_offhold;
    if (offhold < 0x0A)
    {
        offhold++;
        *(volatile unsigned char *)xram_irq_sharpness_offhold = offhold;
    }
}

//
// Adjust “sensitivity” based on AV_stat_detect_0.
void firm_timer_adjust_sensitivity(void)
{
    // Check if bit1 of IO_AV_stat_detect_0 is set:
    if (!(IO_AV_stat_detect_0 & 0x02))
    {
        return; // skip adjustment if bit not set
    }
    // Increment sensitivity offhold counter:
    if (++xram_irq_sensitivity_offhold >= 100)
    {
        // Read sensitivity from hypothetical registers (simulate with constants)
        unsigned char sens_msb = 0x02; // example value
        unsigned char sens_lsb = 0xA0; // example value: 0x02A0 = 672 decimal
        unsigned int sensitivity = ((unsigned int)sens_msb << 8) | sens_lsb;
        // If sensitivity is too small (for example, < 416) then adjust:
        if (sensitivity < 416)
        {
            IO_AV_ctrl_sensitivity_1 = 0xB6;
            IO_AV_ctrl_sensitivity_0 = 9;
        }
        else if (sensitivity > 4095)
        {
            IO_AV_ctrl_sensitivity_0 = 0;
        }
        xram_irq_sensitivity_offhold = 0;
    }
}
void firm_timer_adjust_sensitivity(void)
{
    // Check if sensitivity adjustment should occur.
    unsigned char stat0 = *(volatile unsigned char *)IO_AV_stat_detect_0;
    if (!(stat0 & 0x02))
        return; // Skip if bit1 not set.

    // Increment sensitivity offhold counter.
    unsigned char offhold = *(volatile unsigned char *)xram_irq_sensitivity_offhold;
    offhold++;
    *(volatile unsigned char *)xram_irq_sensitivity_offhold = offhold;
    // Divide offhold by 100 (simulate division by 64h=100 decimal).
    if (offhold < 100)
        return; // Not reached threshold yet.

    // Read sensitivity (MSB and LSB).
    unsigned char sens_msb = *(volatile unsigned char *)IO_AV_stat_sensitivity_msb;
    unsigned char sens_lsb = *(volatile unsigned char *)IO_AV_stat_sensitivity_lsb;
    // Compare against threshold values (example: 0x01A0 for LSB, 0x01 for MSB).
    if (sens_lsb < 0xA0 || sens_msb < 0x01)
    {
        // Too small sensitivity: adjust control registers.
        *(volatile unsigned char *)IO_AV_ctrl_sensitivity_1 = 0xB6;
        *(volatile unsigned char *)IO_AV_ctrl_sensitivity_0 = 0x09;
    }
}

//
void firm_timer_adjust_boldness(void)
{
    // Get AV_stat_detect_1 value.
    unsigned char detect = *(volatile unsigned char *)IO_AV_stat_detect_1;
    // If bit0 is zero, then use boldness adjustments.
    if ((detect & 0x01) == 0)
    {
        unsigned char ctrl = *(volatile unsigned char *)IO_50HZ_control_lsb;
        ctrl |= 0x10;
        *(volatile unsigned char *)IO_50HZ_control_lsb = ctrl;
        ctrl = *(volatile unsigned char *)IO_60HZ_control_mid;
        ctrl |= 0x10;
        *(volatile unsigned char *)IO_60HZ_control_mid = ctrl;
        *(volatile unsigned char *)IO_60HZ_boldness_contrast = 0x00;
    }
    else
    {
        unsigned char ctrl = *(volatile unsigned char *)IO_50HZ_control_lsb;
        ctrl &= 0xEF;
        *(volatile unsigned char *)IO_50HZ_control_lsb = ctrl;
        ctrl = *(volatile unsigned char *)IO_60HZ_control_mid;
        ctrl &= 0xEF;
        *(volatile unsigned char *)IO_60HZ_control_mid = ctrl;
        *(volatile unsigned char *)IO_60HZ_boldness_contrast = 0x02;
    }
}
// Adjust “boldness” based on AV_stat_detect_1 and 50Hz control.
void firm_timer_adjust_boldness(void)
{
    unsigned char stat1 = IO_AV_stat_detect_1;
    unsigned char ctrl50 = IO_50HZ_control_lsb;
    if (!(ctrl50 & 0x01))
    {
        // If bit0 of IO_50HZ_control_lsb is 0, set boldness.
        IO_50HZ_control_lsb |= 0x10; // set bit4
        IO_60HZ_control_mid |= 0x10; // set bit4
        // Assume boldness contrast set to 0:
        IO_60HZ_boldness_contrast = 0;
    }
    else
    {
        // Otherwise clear bit4 and set contrast to 2.
        IO_50HZ_control_lsb &= ~0x10;
        IO_60HZ_control_mid &= ~0x10;
        IO_60HZ_boldness_contrast = 2;
    }
}

void input_selector(void)
{
    // Read the PLL status:
    unsigned char pll = *(volatile unsigned char *)IO_PLL_12h_used;
    if (pll & 0x04)
    { // Check bit2: PLL is powered.
        goto have_av_pll;
    }
    {
        // When PLL is off, use user selection.
        unsigned char sel = *(volatile unsigned char *)xram_sett_input;
        if (sel & 0x02)
            goto force_non_primary;
        else
            goto toggle_to_other;
    }
have_av_pll:
{
    // When PLL is powered, read signal detect bits.
    unsigned char sig = *(volatile unsigned char *)IO_AV_stat_signal_detect;
    unsigned char detect0 = *(volatile unsigned char *)IO_AV_stat_detect_0;
    // Replace bit4 with a better bit from detect0 (bit1).
    if (detect0 & 0x02)
        sig |= 0x10;
    unsigned char sel = *(volatile unsigned char *)xram_sett_input;
    if (sel & 0x02)
        goto force_primary;
    if (!(sig & 0x40))
        goto stay_current;
    if (!(sig & 0x10))
        goto toggle_to_other;
    goto force_primary;
}
toggle_to_other:
{
    unsigned char cur = *(volatile unsigned char *)xram_curr_input;
    cur ^= 1;
    *(volatile unsigned char *)xram_curr_input = cur;
}
force_non_primary:
{
    unsigned char cur = *(volatile unsigned char *)xram_curr_input;
    cur ^= 1;
    *(volatile unsigned char *)xram_curr_input = cur;
}
force_primary:
{
    unsigned char cur = *(volatile unsigned char *)xram_curr_input;
    cur &= 0x01;
    *(volatile unsigned char *)xram_curr_input = cur;
    // Call the routine that applies the current input selection.
    apply_av_input_a();
}
stay_current:
    return;
}

// Select the AV input based on power status and user selection.
void input_selector(void)
{
    // In this simplified version we simply choose the input based on xram_sett_input.
    // (In the original code, additional tests of signal detect bits are performed.)
    xram_curr_input = xram_sett_input & 0x01;
    apply_av_input_a();
}

// Write the selected AV input to the video registers.
void apply_av_input_a(void)
{
    // In the original code, NV3035C and HX8238 registers are updated.
    // Here we call two stub functions.
    if (xram_curr_input == 0)
    {
        // For AV1, send a “wake” command.
        write_NV3035C_index_r0_data_a(0x00, 0x03);
        write_HX8238_index_r2_data_r1r0(0x01, 0x63, 0x20);
    }
    else
    {
        // For AV2, send a “standby” command.
        write_NV3035C_index_r0_data_a(0x00, 0x01);
        write_HX8238_index_r2_data_r1r0(0x01, 0x76, 0x30);
    }
}

void ramp_clear(void)
{
    unsigned char i;
    for (i = 0; i < 0x1F; i++)
    {
        *((volatile unsigned char *)dptr_global) = 0; // dptr_global is a pointer to the ramp area
        dptr_global++;
    }
}

void ramp_negative(void)
{
    unsigned char i, val = 0;
    for (i = 0; i < 0x1F; i++)
    {
        *((volatile unsigned char *)dptr_global) = val;
        val -= 8;
        dptr_global++;
    }
}

// Clear a 32–byte gamma ramp.
void ramp_clear(volatile unsigned char *dptr)
{
    for (int i = 0; i < 32; i++)
        dptr[i] = 0;
}

// Create a linear gamma ramp (step = 8).
void ramp_linear(volatile unsigned char *dptr)
{
    unsigned char a = 0;
    for (int i = 0; i < 32; i++)
    {
        dptr[i] = a;
        a += 8;
    }
}
void ramp_linear(void)
{
    unsigned char i, val = 0;
    for (i = 0; i < 0x1F; i++)
    {
        *((volatile unsigned char *)dptr_global) = val;
        val += 8;
        dptr_global++;
    }
}

// Copy the old gamma ramp into destination.
void ramp_normal_old(volatile unsigned char *dptr)
{
    for (int i = 0; i < 32; i++)
    {
        dptr[i] = old_gamma_ramp[i];
    }
}
void ramp_normal_old(void)
{
    // Copy the old gamma ramp from a fixed table into the ramp area.
    extern unsigned char old_gamma_ramp_table[];
    unsigned char i;
    for (i = 0; i < 0x20; i++)
    {
        *((volatile unsigned char *)dptr_global) = old_gamma_ramp_table[i];
        dptr_global++;
    }
}

// Copy the new gamma ramp into destination.
void ramp_normal_new(volatile unsigned char *dptr)
{
    for (int i = 0; i < 32; i++)
        dptr[i] = new_gamma_ramp[i];
}

void ramp_normal_new(void)
{
    extern unsigned char new_gamma_ramp_table[];
    unsigned char i;
    for (i = 0; i < 0x20; i++)
    {
        *((volatile unsigned char *)dptr_global) = new_gamma_ramp_table[i];
        dptr_global++;
    }
}

uint16_t div_dptr_by_r0(uint16_t dptr, uint8_t r0)
{
    if (r0 == 0)
        return 0xFFFF; // Handle division by zero (return max value as an error)

    uint16_t quotient = 0;
    uint16_t remainder = 0;

    for (int i = 16; i >= 0; i--)
    {
        remainder = (remainder << 1) | ((dptr >> i) & 1); // Shift in the next bit

        if (remainder >= r0)
        {
            remainder -= r0;
            quotient |= (1 << i);
        }
    }

    return quotient; // Only returning quotient; remainder is discarded
}

int main(void)
{
    system_init();              // Initialize clocks, I/O, timers, watchdog, etc.
    init_load_settings();       // Load settings from flash (or use defaults if absent)
    init_OSD_hardware();        // Initialize OSD I/O (calls init_osd_io_list() inside)
    sysgui_make_font();         // Generate/load OSD font
    sysgui_prompt_panel_type(); // (Optional) Prompt for panel type

    // Main loop
    while (1)
    {
        watchdog_reload();
        sysgui_select_menu();
    }
    return 0;
}
