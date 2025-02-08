#include "io_lists.h"
#include "hardware.h"

// All lists are defined as constant arrays terminated by 0xFF.

// Placeholder for memory-mapped I/O addresses
#define IO_LIST_ENDCODE 0xFF

typedef struct
{
    volatile uint16_t *address;
    uint8_t value;
} IORegisterSetting;

// Define the large I/O initialization table
const IORegisterSetting huge_fixed_io_list[] = {
    {(volatile uint16_t *)0x000B, 0x40},           // IO_PLL_0Bh_used
    {(volatile uint16_t *)0x000D, 0xF0},           // IO_PLL_0Dh_cfg
    {(volatile uint16_t *)0x0010, 0x04},           // IO_PLL_10h_cfg
    {(volatile uint16_t *)0x0012, 0x03},           // IO_PLL_dotclk_divider_2
    {(volatile uint16_t *)0x0013, 0x02},           // IO_PLL_dotclk_divider_3
    {(volatile uint16_t *)0x001A, 0x08},           // IO_PLL_1Ah_cfg
    {(volatile uint16_t *)0xFE83, 0xFE},           // IO_AV_config_FE83h
    {(volatile uint16_t *)0xFE84, 0xB1},           // IO_AV_ctrl_sensitivity_1
    {(volatile uint16_t *)0xFFB7, 0x90},           // IO_LCD_config_FFB7h
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE} // End marker
};

// Function to apply I/O register settings
void apply_io_list(const IORegisterSetting *io_list)
{
    while (io_list->address != (volatile uint16_t *)0xFFFF)
    {
        *(io_list->address) = io_list->value;
        io_list++;
    }
}

void init_io_via_io_list(unsigned char *list)
{
    unsigned char msb, lsb, data;
    while (1)
    {
        msb = *list++;
        if (msb == 0xFF)
            break; // End-of-list marker
        lsb = *list++;
        data = *list++;
        unsigned int addr = (msb << 8) | lsb;
        *((volatile unsigned char *)addr) = data;
    }
}
void init_io_via_io_list(const IORegisterSetting *io_list)
{
    while (io_list->address != (volatile uint16_t *)0xFFFF)
    {
        *(io_list->address) = io_list->value; // Write value to memory-mapped register
        io_list++;                            // Move to the next entry
    }
}

// Function to apply the extra fixed I/O list twice
void output_initial_data_from_extra_fixed_io_list()
{
    extern const IORegisterSetting extra_fixed_io_list[];

    init_io_via_io_list(extra_fixed_io_list);
    init_io_via_io_list(extra_fixed_io_list); // Apply twice for stability
}

// Mode ratio lists
const IORegisterSetting mode_ratio_io_list_320x240_wide[] = {
    {(volatile uint16_t *)0x0000, 0x61},           // @@scale60 = 0861h
    {(volatile uint16_t *)0x0001, 0x85},           // @@scale50 = 085Ch
    {(volatile uint16_t *)0x0002, 0x10},           // @@x60 = 10h
    {(volatile uint16_t *)0x0003, 0x06},           // @@x50 = 06h
    {(volatile uint16_t *)0x0004, 0x00},           // @@what = 00h
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE} // End marker
};

const IORegisterSetting mode_ratio_io_list_320x240_narrow[] = {
    {(volatile uint16_t *)0x0000, 0x7E}, // @@scale60 = 087Eh
    {(volatile uint16_t *)0x0001, 0x84}, // @@scale50 = 0884h
    {(volatile uint16_t *)0x0002, 0x10}, // @@x60 = 10h
    {(volatile uint16_t *)0x0003, 0x06}, // @@x50 = 06h
    {(volatile uint16_t *)0x0004, 0x00}, // @@what = 00h
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE}};

const IORegisterSetting mode_ratio_io_list_480x272_wide[] = {
    {(volatile uint16_t *)0x0000, 0xAF}, // @@scale60 = 05AFh
    {(volatile uint16_t *)0x0001, 0xA7}, // @@scale50 = 05A7h
    {(volatile uint16_t *)0x0002, 0x16}, // @@x60 = 16h
    {(volatile uint16_t *)0x0003, 0x0B}, // @@x50 = 0Bh
    {(volatile uint16_t *)0x0004, 0x00}, // @@what = 00h
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE}};

const IORegisterSetting mode_ratio_io_list_480x272_narrow[] = {
    {(volatile uint16_t *)0x0000, 0x62}, // @@scale60 = 0762h
    {(volatile uint16_t *)0x0001, 0x70}, // @@scale50 = 0770h
    {(volatile uint16_t *)0x0002, 0x16}, // @@x60 = 16h
    {(volatile uint16_t *)0x0003, 0x0B}, // @@x50 = 0Bh
    {(volatile uint16_t *)0x0004, 0x3C}, // @@what = 3Ch
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE}};

const IORegisterSetting extra_fixed_io_list[] = {
    {(volatile uint16_t *)0x0010, 0x11}, // IO_PIN_P10_P11_spi_flash
    {(volatile uint16_t *)0x0012, 0x11}, // IO_PIN_P12_P13_spi_flash
    {(volatile uint16_t *)0x0020, 0x01}, // IO_PIN_maybe1_01h
    {(volatile uint16_t *)0x0021, 0x00}, // IO_PIN_maybe2_zero
    {(volatile uint16_t *)0x0022, 0x00}, // IO_PIN_maybe3_zero
    {(volatile uint16_t *)0x0023, 0x00}, // IO_PIN_maybe4_zero
    {(volatile uint16_t *)0x0024, 0x00}, // IO_PIN_maybe5_zero
    {(volatile uint16_t *)0x0025, 0x00}, // IO_PIN_maybe6_zero
    {(volatile uint16_t *)0x0026, 0x00}, // IO_PIN_maybe7_zero
    {(volatile uint16_t *)0x0027, 0x00}, // IO_PIN_maybe8_zero
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE}};

const uint8_t io_list_1[] = {
    IO_PLL_0Eh_used, 0x20,
    IO_PLL_11h_used, 0xFF,
    IO_PLL_12h_used, 0xFF,
    IO_PLL_13h_used, 0xFF,
    IO_PLL_0Bh_used, 0x40,
    IO_PLL_10h_cfg, 0x04,
    0xFF};

// IO List 2: Various LCD and zero-filled configurations
const IORegisterSetting io_list_2[] = {
    {(volatile uint16_t *)0xFD50, 0x00},           // IO_whatever_FD50h (set to 00h but 0Bh elsewhere)
    {(volatile uint16_t *)0x0000, 0x00},           // IO_whatever_zerofilled+0
    {(volatile uint16_t *)0x0001, 0x00},           // IO_whatever_zerofilled+1
    {(volatile uint16_t *)0x0002, 0x00},           // IO_whatever_zerofilled+2
    {(volatile uint16_t *)0x0003, 0x00},           // IO_whatever_zerofilled+3
    {(volatile uint16_t *)0x0004, 0x00},           // IO_whatever_zerofilled+4
    {(volatile uint16_t *)0x0005, 0x00},           // IO_whatever_zerofilled+5
    {(volatile uint16_t *)0x0006, 0x00},           // IO_whatever_zerofilled+6
    {(volatile uint16_t *)0x0007, 0x00},           // IO_whatever_zerofilled+7
    {(volatile uint16_t *)0x0008, 0x00},           // IO_whatever_zerofilled+8
    {(volatile uint16_t *)0x0009, 0x00},           // IO_whatever_zerofilled+9
    {(volatile uint16_t *)0xFFCB, 0x80},           // IO_LCD_whatever_FFCBh (set to 2Ah elsewhere)
    {(volatile uint16_t *)0xFFCC, 0x80},           // IO_LCD_config_FFCCh
    {(volatile uint16_t *)0xFFCD, 0x2D},           // IO_LCD_config_FFCDh
    {(volatile uint16_t *)0xFFB7, 0x90},           // IO_LCD_config_FFB7h
    {(volatile uint16_t *)0xFFD8, 0x80},           // IO_LCD_whatever_FFD8h
    {(volatile uint16_t *)0xFFD7, 0x10},           // IO_LCD_config_FFD7h
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE} // End marker
};

// IO List 3: AV and Video settings
const IORegisterSetting io_list_3[] = {
    {(volatile uint16_t *)0xFEA0, 0xF7},           // IO_AV_video_on_off
    {(volatile uint16_t *)0xFEBA, 0xFF},           // IO_AV_config_FEBAh
    {(volatile uint16_t *)0xFEB5, 0x67},           // IO_AV_config_FEB5h
    {(volatile uint16_t *)0xFE84, 0xB1},           // IO_AV_ctrl_sensitivity_1 (uses 2mA)
    {(volatile uint16_t *)0xFE56, 0x00},           // IO_AV_config_FE56h
    {(volatile uint16_t *)0xFE13, 0x1E},           // IO_AV_config_FE13h
    {(volatile uint16_t *)0xFE20, 0x01},           // IO_VIDEO_something_3
    {(volatile uint16_t *)0xFE21, 0x45},           // IO_VIDEO_something_4
    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE} // End marker
};

// OSD Initialization Table
const IORegisterSetting init_osd_io_list[] = {
    {(volatile uint16_t *)0xFB60, 0x00}, // IO_OSD_window_enable_bits
    {(volatile uint16_t *)0xFB61, 0x00}, // IO_OSD_misc_transp_enable
    {(volatile uint16_t *)0xFB62, 0x80}, // IO_OSD_bright_transp_level
    {(volatile uint16_t *)0xFB63, 0x00}, // IO_OSD_bitmap_start_lsb
    {(volatile uint16_t *)0xFB64, 0x00}, // IO_OSD_bitmap_start_msb
    {(volatile uint16_t *)0xFB65, 0x00}, // IO_OSD_bitmap_transp_misc
    {(volatile uint16_t *)0xFB66, 0x00}, // IO_OSD_screen_position
    {(volatile uint16_t *)0xFB67, 0x00}, // IO_OSD_whatever_FB62h
    {(volatile uint16_t *)0xFB68, 0x08}, // IO_OSD_char_xsiz
    {(volatile uint16_t *)0xFB69, 0x08}, // IO_OSD_char_ysiz
    {(volatile uint16_t *)0xFB6A, 0x00}, // IO_OSD_xyflip

    // Window 0 Configuration
    {(volatile uint16_t *)0xFB70, 0x00}, // IO_OSD_window_0_size_x
    {(volatile uint16_t *)0xFB71, 0x00}, // IO_OSD_window_0_size_y
    {(volatile uint16_t *)0xFB72, 0x00}, // IO_OSD_window_0_xyloc_msb
    {(volatile uint16_t *)0xFB73, 0x00}, // IO_OSD_window_0_xloc_lsb
    {(volatile uint16_t *)0xFB74, 0x00}, // IO_OSD_window_0_yloc_lsb

    // OSD Color Definitions
    {(volatile uint16_t *)0xFC00, 0x00}, // IO_OSD_color_1_msb (Red)
    {(volatile uint16_t *)0xFC01, 0x0F}, // IO_OSD_color_1_lsb
    {(volatile uint16_t *)0xFC02, 0x00}, // IO_OSD_color_2_msb (Green)
    {(volatile uint16_t *)0xFC03, 0xF0}, // IO_OSD_color_2_lsb
    {(volatile uint16_t *)0xFC04, 0x0F}, // IO_OSD_color_3_msb (Blue)
    {(volatile uint16_t *)0xFC05, 0x00}, // IO_OSD_color_3_lsb
    {(volatile uint16_t *)0xFC06, 0x00}, // IO_OSD_color_4_msb (Yellow)
    {(volatile uint16_t *)0xFC07, 0xFF}, // IO_OSD_color_4_lsb
    {(volatile uint16_t *)0xFC08, 0x0F}, // IO_OSD_color_5_msb (Cyan)
    {(volatile uint16_t *)0xFC09, 0xF0}, // IO_OSD_color_5_lsb
    {(volatile uint16_t *)0xFC0A, 0x0F}, // IO_OSD_color_6_msb (White)
    {(volatile uint16_t *)0xFC0B, 0xFF}, // IO_OSD_color_6_lsb

    {(volatile uint16_t *)0xFFFF, IO_LIST_ENDCODE} // End marker
};
