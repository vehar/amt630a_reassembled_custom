#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <reg51.h>
#include <intrins.h>

// Resolution IO list for 480x272
#define IO_PLL_DOTCLK_MULTIPLIER 0xA8
#define IO_PLL_DOTCLK_DIVIDER_1 0x09
#define IO_PLL_16H_CFG 0x0A
#define IO_PLL_19H_USED 0xC3
#define IO_AV_CTRL_WHATEVER_2 0x40
#define IO_LCD_SNOW_ENABLE_MISC 0x23
#define IO_LCD_CONFIG_FFB2_FFB4 0x10
#define IO_60HZ_BOLDNESS_CONTRAST 0x00

#define IO_60HZ_15KHZ_LSB 0x358
#define IO_60HZ_15KHZ_DIV2_LSB 0x22D
#define IO_60HZ_XLOC_AV_OSD_LSB 0x01
#define IO_60HZ_XLOC_OSD_LSB 0x06
#define IO_60HZ_XLOC_AV_LSB 0x27
#define IO_60HZ_XCROP_END_AV_LSB 0x207
#define IO_60HZ_YLOC_AV_OSD_LSB 0x00
#define IO_60HZ_YLOC_OSD_LSB 0x0A
#define IO_60HZ_YCROP_UPPER_AV_LSB 0x18
#define IO_60HZ_HEAVY_FLIMMER_LSB 0x131

#define IO_50HZ_15KHZ_LSB 0x3F6
#define IO_50HZ_15KHZ_DIV2_LSB 0x298
#define IO_50HZ_XLOC_AV_OSD_LSB 0x01
#define IO_50HZ_XLOC_OSD_LSB 0x06
#define IO_50HZ_XLOC_AV_LSB 0x27
#define IO_50HZ_XCROP_END_AV_LSB 0x207
#define IO_50HZ_YLOC_AV_OSD_LSB 0x04
#define IO_50HZ_YLOC_OSD_LSB 0x0A
#define IO_50HZ_YCROP_UPPER_AV_LSB 0x18
#define IO_50HZ_HEAVY_FLIMMER_LSB 0x131

// Mode ratio settings
#define MODE_RATIO_320x240_WIDE_SCALE60 0x861
#define MODE_RATIO_320x240_WIDE_SCALE50 0x85C
#define MODE_RATIO_320x240_WIDE_X60 0x10
#define MODE_RATIO_320x240_WIDE_X50 0x06

#define MODE_RATIO_480x272_WIDE_SCALE60 0x5AF
#define MODE_RATIO_480x272_WIDE_SCALE50 0x5A7
#define MODE_RATIO_480x272_WIDE_X60 0x16
#define MODE_RATIO_480x272_WIDE_X50 0x0B

#define MODE_RATIO_480x272_NARROW_SCALE60 0x762
#define MODE_RATIO_480x272_NARROW_SCALE50 0x770
#define MODE_RATIO_480x272_NARROW_X60 0x16
#define MODE_RATIO_480x272_NARROW_X50 0x0B
#define MODE_RATIO_480x272_NARROW_WHAT 0x3C

//====================================================================
// Hardware Register Definitions (dummy values; adjust as needed)
//====================================================================

// 8051 SFR bit definitions for timers and interrupts:
sbit TR0 = P3 ^ 0; // Timer0 run control
sbit TR1 = P3 ^ 1; // Timer1 run control
sbit ET0 = IE ^ 1; // Timer0 interrupt enable
sbit ET1 = IE ^ 3; // Timer1 interrupt enable
sbit EX0 = IE ^ 0; // External Interrupt 0 enable (IR)
sbit EA = IE ^ 7;  // Global interrupt enable

// Dummy watchdog registers (replace with your actual addresses)
#define WDT_UNLOCK (*(volatile unsigned char *)0xF0)
#define WDT_CON (*(volatile unsigned char *)0xF1)
#define WDT_RELOAD (*(volatile unsigned char *)0xF2)

// Dummy ADC/IR registers
#define ADC_RESULT (*(volatile unsigned int *)0xE0)
#define IR_DATA (*(volatile unsigned char *)0xE1)
#define IR_FLAGS (*(volatile unsigned char *)0xE2)

// Dummy OSD registers (for background and font memory addresses)
#define IO_OSD_BG_ADDR_LSB (*(volatile unsigned char *)0xD0)
#define IO_OSD_BG_ADDR_MSB (*(volatile unsigned char *)0xD1)
#define IO_OSD_FONT_ADDR_LSB (*(volatile unsigned char *)0xD2)
#define IO_OSD_FONT_ADDR_MSB (*(volatile unsigned char *)0xD3)

//====================================================================
// Global Variables (extern declarations)
//====================================================================

extern volatile unsigned long systemTick;      // System tick (updated in Timer1 ISR)
extern volatile unsigned int frameRateCounter; // Frame counter

extern unsigned char ir_command;          // Last IR command received (0xFF means none)
extern unsigned char keypad_key;          // Last keypad key code
extern unsigned char key_input;           // Key pressed from user (polled)
extern unsigned char lcd_status;          // 0 = off, 1 = on
extern unsigned char current_menu_option; // Current menu option index

#define NUM_MENU_OPTIONS 5 // Example: five options in the main menu

#include <reg51.h>
#include <intrins.h>

/*===========================================================================
  Dummy Register and Memory Definitions (adjust for your hardware)
===========================================================================*/
volatile unsigned char IO_AV_stat_signal_detect; // AV status register (only lower nibble used)
volatile unsigned char IO_AV_stat_detect_2;      // AV second status register
volatile unsigned char IO_AV_stat_detect_1;      // For boldness adjustment

volatile unsigned char IO_AV_ctrl_artifacts;     // AV artifacts control register
volatile unsigned char IO_AV_ctrl_whatever_1;    // Another AV control register (bit1 used)
volatile unsigned char IO_AV_ctrl_sensitivity_0; // Sensitivity control registers
volatile unsigned char IO_AV_ctrl_sensitivity_1;
volatile unsigned char IO_AV_stat_sensitivity_msb; // AV sensitivity MSB
volatile unsigned char IO_AV_stat_sensitivity_lsb; // AV sensitivity LSB

volatile unsigned char IO_50HZ_control_lsb; // Video timing registers (50Hz)
volatile unsigned char IO_60HZ_control_mid;
volatile unsigned char IO_60HZ_boldness_contrast;

volatile unsigned char IO_PLL_12h_used; // PLL used flag

// For flash, SPI, OSD, etc. we assume these are defined elsewhere:
#define IO_LCD_sharpness_or_so (*(volatile unsigned char *)0xD4)
#define IO_AV_video_on_off (*(volatile unsigned char *)0xC0)
#define IO_AV_input_select_reg_0 (*(volatile unsigned char *)0xC1)
#define IO_AV_input_select_reg_1 (*(volatile unsigned char *)0xC2)

// Dummy PWM registers for backlight and volume:
volatile unsigned int IO_PWM0_duty_total;   // Backlight total duty value
volatile unsigned int IO_PWM0_duty_high;    // Backlight high duty value
volatile unsigned int IO_PWM1_duty_total;   // Volume total duty value
volatile unsigned int IO_PWM1_duty_high;    // Volume high duty value
volatile unsigned char IO_PWM_enable_flags; // PWM enable bits

// Dummy I/O pin registers for SPI/backlight:
volatile unsigned char IO_PIN_P35_P36_pwm;
// (Also dummy registers for backlight, volume, etc.)

// For OSD, we assume these addresses:
#define IO_OSD_bgmap_addr_lsb (*(volatile unsigned char *)0xD0)
#define IO_OSD_bgmap_addr_msb (*(volatile unsigned char *)0xD1)
#define IO_OSD_font_addr_lsb (*(volatile unsigned char *)0xD2)
#define IO_OSD_font_addr_msb (*(volatile unsigned char *)0xD3)

// Some XRAM–variables (simulate XRAM storage):
volatile unsigned char xram_tmp__23h;
volatile unsigned char xram_tmp__24h;
volatile unsigned char xram_old_AV_stat_signal_detect = 0xFF; // Initially FFh
volatile unsigned char xram_irq_artifacts_offhold;
volatile unsigned char xram_irq_sharpness_offhold;
volatile unsigned char xram_irq_sensitivity_offhold;
volatile unsigned char xram_sett_backlight;  // backlight setting (0–100)
volatile unsigned char xram_sett_volume;     // volume setting (0–100)
volatile unsigned char xram_sett_input;      // input selection (bit0 used)
volatile unsigned char xram_curr_input;      // current input value
volatile unsigned char xram_sett_yuv_consts; // nonzero means “new” YUV constants

// For the OSD/menu system (dummy addresses in XRAM)
volatile unsigned char xram_sett_id_5Ah;
volatile unsigned char xram_sett_id_A5h;
volatile unsigned char xram_sett_chksum;
volatile unsigned char xram_palntsc_same_counter;
volatile unsigned char iram_sysgui_menu_index;
volatile unsigned char iram_sysgui_menu_param_msb;
volatile unsigned char iram_sysgui_menu_param_lsb;
volatile unsigned char iram_sysgui_menu_text_msb;
volatile unsigned char iram_sysgui_menu_text_lsb;
// (Other menu–related XRAM variables omitted for brevity)

// Dummy key input variable:
volatile unsigned char key_input;

// Dummy watchdog registers (already defined in previous file)
#define SFR_IO_watchdog_unlock (*(volatile unsigned char *)0xF0)
#define SFR_IO_watchdog_config1 (*(volatile unsigned char *)0xF1)
#define SFR_IO_watchdog_config2 (*(volatile unsigned char *)0xF2)
#define SFR_IO_watchdog_config3 (*(volatile unsigned char *)0xF3)
#define SFR_IO_watchdog_enable (*(volatile unsigned char *)0xF4)
#define SFR_IO_watchdog_reload (*(volatile unsigned char *)0xF5)

// For SPI LCD registers (dummy addresses)
#define IO_SPI_transfer_mode (*(volatile unsigned char *)0xE0)
#define IO_SPI_ready_flags (*(volatile unsigned char *)0xE1)
#define IO_SPI_manual_data_read (*(volatile unsigned char *)0xE2)
#define IO_SPI_chip_id_read_lsb (*(volatile unsigned char *)0xE3)
#define IO_SPI_chip_id_read_mid (*(volatile unsigned char *)0xE4)
#define IO_SPI_chip_id_read_msb (*(volatile unsigned char *)0xE5)

// For brevity, many register definitions are omitted. Adjust these for your system.
//====================================================================
// Function Prototypes
//====================================================================

// --- System Initialization ---
void system_init(void);
void init_timer_hardware(void);
void init_io(void);
void watchdog_disable(void);
void watchdog_enable(void);
void watchdog_reload(void);

// --- Interrupt Service Routines (ISRs) ---
void reset_entrypoint(void) __interrupt 0;
void ext_int0_ir(void) __interrupt 1;
void timer0_overflow_isr(void) __interrupt 2;
void timer1_overflow_isr(void) __interrupt 4;
void serial0_isr(void) __interrupt 5;
void serial1_isr(void) __interrupt 6;
void ct0_isr(void) __interrupt 7;
void ct1_isr(void) __interrupt 8;
void adc_keypad_isr(void) __interrupt 9;
void whatever_91h5_ack_isr(void) __interrupt 10;
void whatever_91h6_ack_isr(void) __interrupt 11;
void framerate_isr(void) __interrupt 12;
void timer2_compare_isr(void) __interrupt 13;

// --- Low-Level Handlers Called by ISRs ---
void timer0_reload(void);
void timer1_overflow(void);
void process_ir_interrupt(void);
void adc_keypad_handler(void);
void acknowledge_adc(void);
void acknowledge_other_irq(void);
void update_framerate(void);
void timer2_compare_handler(void);

// --- Flash/Settings Functions ---
void init_load_settings(void);
unsigned char firm_calc_settings_chksum(void);
void firm_reset_all_settings(void);
void firm_do_save_settings(void);
void flash_write_dptr_to_flash(unsigned char *src, unsigned int flashAddr, unsigned int len);
unsigned char flash_read_byte(unsigned int flashAddr);

// --- I/O List Initialization ---
void init_io_via_io_list(unsigned char *list);

// --- OSD and UI Functions ---
void sysgui_wrchr(char c);
void sysgui_wrstr(char *str);
void firm_set_font_addr(unsigned int addr);
void firm_set_bgmap_addr(unsigned int addr);
void firm_raise_font_addr(void);
void firm_raise_bgmap_addr(void);
void sysgui_make_font(void);
unsigned char getkey_with_keyrepeat(void);
unsigned char getkey_with_fast_keyrepeat(void);

// --- Apply Settings Functions ---
void apply_settings_to_IO_ports(void);
void apply_settings_to_IO_ports_except_backlight(void);
void apply_mode_ratio(void);
void apply_xyflip(void);
void apply_tint(void);
void apply_pal_ntsc(void);
void apply_allow_c64(void);
void apply_tianma_r0f(void);

// --- OSD Panel Prompt and Menu Functions ---
void sysgui_prompt_panel_type(void);
void sysgui_select_menu(void);

// --- Additional Stub Functions ---
void init_load_settings_from_flash(void);

// --- LCD/Display Functions ---
void switch_lcd_screen_on(void);
void switch_lcd_screen_off(void);
void switch_screen_and_backlight_on(void);
void switch_screen_and_backlight_off(void);

//============================================================
// LCD SPI Functions
//============================================================
void write_HX8238_index_r2_data_r1r0(unsigned char index, unsigned char msb, unsigned char lsb);
void write_NV3035C_index_r0_data_a(unsigned char index, unsigned char data);
unsigned char read_NV3035C_register_r0(unsigned char index);
void lcd_spi_send_8_bits_from_a(unsigned char data);
void lcd_spi_send_r0_bits_from_a(unsigned char data, unsigned char numbits);
unsigned char lcd_spi_recv_r0_bits_to_a(unsigned char numbits);
void lcd_spi_release(void);

//============================================================
// ADC Initialization Functions
//============================================================
void init_ADC_analog_hardware(void);
void firm_adc_init_io_list(void);

//============================================================
// Display SPI Functions
//============================================================
void display_spi_reset(void);
unsigned char display_spi_detect(void);
void display_spi_dump_initial(void);
void display_spi_init(void);

//============================================================
// OSD Gamma Ramp and YUV Functions
//============================================================
void ramp_clear(void);
void ramp_linear(void);
void ramp_negative(void);
void ramp_normal_old(void);
void ramp_normal_new(void);
void old_gamma_ramp(void);
void new_gamma_ramp(void);
void old_yuv_constants(void);
void new_yuv_constants(void);
void firm_swap_dptr_vs_r1r0(void);
void apply_yuv_constants(void);
void apply_backdrop(void);

#endif // FIRMWARE_EXTRA_H

#endif // FIRMWARE_H
