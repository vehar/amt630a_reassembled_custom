#include <stdint.h>

// Define Menu Items
typedef struct
{
    uint16_t param_addr;
    const char *label;
    const char **options;
} MenuItem;

// Memory-mapped I/O addresses (assuming placeholders)
volatile uint8_t *IO_OSD_WINDOW_ENABLE_BITS = (volatile uint8_t *)0xFB00;
volatile uint8_t *IO_OSD_WINDOW_SIZE_X = (volatile uint8_t *)0xFB01;
volatile uint8_t *IO_OSD_WINDOW_SIZE_Y = (volatile uint8_t *)0xFB02;

// Menu Text Strings
const char *options_backlight[] = {"%", NULL};
const char *options_brightness[] = {"%", NULL};
const char *options_contrast[] = {"%", NULL};
const char *options_saturation[] = {"%", NULL};
const char *options_tint[] = {"%", NULL};
const char *options_volume[] = {"%", NULL};

const char *options_input[] = {
    "AV1+AV2 (AV1 PREFERRED)",
    "AV2+AV1 (AV2 PREFERRED)",
    "AV1 ONLY",
    "AV2 ONLY",
    ".",
    NULL};

const char *options_no_signal[] = {
    "STANDBY",
    "BLUE SCREEN",
    "BLUE+SNOW",
    "BLACK+SNOW",
    ".",
    NULL};

const char *options_ratio[] = {"4:3", "16:9", ".", NULL};

const char *options_keypadtype[] = {
    "3KEY +MENU- (NORMAL)",
    "3KEY -MENU+ (SWAPPED)",
    ".",
    NULL};

const char *options_panel_type[] = {
    "TIANMA 320x240",
    "INNOLUX 480x272",
    ".",
    NULL};

const char *options_xyflip[] = {
    "NORMAL",
    "X-FLIP",
    "Y-FLIP",
    "Y/X-FLIP (ROTATE)",
    ".",
    NULL};

const char *options_rgb_ramps[] = {"NEW", "OLD", "LINEAR", ".", NULL};
const char *options_yuv_consts[] = {"NEW", "OLD", ".", NULL};
const char *options_pal_ntsc[] = {"PAL/NTSC", "PAL/PAL60", ".", NULL};
const char *options_allow_c64[] = {"ACCEPT", "REFUSE", ".", NULL};
const char *options_tianma_r0f[] = {"CORRECT/RECOMMENDED", "INCORRECT/DEFAULT", ".", NULL};
const char *options_about[] = {"ABOUT", "@", NULL};

// Define the menu structure
MenuItem sysgui_menu_list[] = {
    {0x1000, "BACKLIGHT", options_backlight},
    {0x1001, "BRIGHTNESS", options_brightness},
    {0x1002, "CONTRAST", options_contrast},
    {0x1003, "SATURATION", options_saturation},
    {0x1004, "TINT (NTSC)", options_tint},
    {0x1005, "VOLUME", options_volume},
    {0x1006, "INPUT", options_input},
    {0x1007, "NO SIGNAL", options_no_signal},
    {0x1008, "RATIO", options_ratio},
    {0x1009, "KEYPAD TYPE", options_keypadtype},
    {0x100A, "DISPLAY PANEL", options_panel_type},
    {0x100B, "FLIP/ROTATE", options_xyflip},
    {0x100C, "RGB GAMMA RAMPS", options_rgb_ramps},
    {0x100D, "YUV CONSTANTS", options_yuv_consts},
    {0x100E, "SIGNAL DECODING", options_pal_ntsc},
    {0x100F, "C64 VIDEO SIGNAL", options_allow_c64},
    {0x1010, "TIANMA VOLTAGES", options_tianma_r0f},
    {0x1011, "ABOUT", options_about},
    {0, NULL, NULL} // End of menu
};

/*
BACKLIGHT: [%]
BRIGHTNESS: [%]
CONTRAST: [%]
SATURATION: [%]
TINT (NTSC): [%]
VOLUME: [%]
INPUT: [AV1+AV2 (AV1 PREFERRED)] [AV2+AV1 (AV2 PREFERRED)] [AV1 ONLY] [AV2 ONLY] [.]
NO SIGNAL: [STANDBY] [BLUE SCREEN] [BLUE+SNOW] [BLACK+SNOW] [.]
...
You selected: CONTRAST

*/