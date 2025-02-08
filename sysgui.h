#ifndef SYSGUI_H
#define SYSGUI_H

#include <stdint.h>

// String and pointer helper functions
void firm_skipstr(uint8_t **dptr);
uint8_t firm_read_ba_from_dptr(uint8_t **dptr);
uint16_t firm_read_dptr_from_dptr(uint8_t **dptr);

// OSD/Menu drawing and prompt routines
void sysgui_wrchr(char c);
void sysgui_wrstr(const char *str);
void sysgui_space_pad(void);

void sysgui_draw_item_name(void);
void sysgui_draw_item_state(void);
void sysgui_get_menu_ptr(void);

uint8_t verify_loaded_settings(void);
uint8_t sysgui_check_null_item(void);
uint8_t sysgui_count_num_options_r0(void);

uint8_t firm_check_signal(void);
void sysgui_prompt_panel_type(void);
void sysgui_select_menu(void);
void sysgui_change_option(void);
void sysgui_standby(void);
void sysgui_idle(void);

// Window positioning routines
void set_window_xloc(int8_t xloc, uint8_t *windowBase);
void set_window_yloc(int8_t yloc, uint8_t *windowBase);
uint8_t get_xflip_flag(void);
uint8_t get_yflip_flag(void);
uint16_t firm_get_screen_width_ba(void);
uint16_t firm_get_screen_height_ba(void);
void sysgui_apply_window_positions(void);

#endif // SYSGUI_H
