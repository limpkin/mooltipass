 /* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */


#ifndef GRAPHICS_H_
#define GRAPHICS_H_

void merge_1bit_buffer_to_1bit_picture(unsigned char picture, unsigned char* buffer, unsigned char x, unsigned char y, unsigned int width, unsigned char height);
void Fill_Block(unsigned char Data, unsigned char Data2, unsigned char column_start, unsigned char end_column, unsigned char row_start, unsigned char row_end);
void merge_1bit_buffer_to_picture(unsigned char picture, unsigned char* buffer, unsigned char x, unsigned char y, unsigned int width, unsigned char height);
void Show_BF_Nb_Chars(char* string, unsigned char reverse, unsigned char x, unsigned char y, unsigned char nb_chars, unsigned char line);
void display_1bit_picture(unsigned char picture, unsigned char x, unsigned char y, unsigned int width, unsigned char height);
void display_picture(unsigned char picture, unsigned char x, unsigned char y);
void Show_Big_Font(unsigned char data, unsigned char reverse, unsigned char x, unsigned char y, unsigned char line);
void Show_Nb_Chars(char* string, unsigned char reverse, unsigned char x, unsigned char y, unsigned char nb_chars);
void display_main_bandeau(unsigned char item, unsigned char type, unsigned char* jauges, unsigned char* values);
void Show_BF_String(char* string, unsigned char reverse, unsigned char x, unsigned char y, unsigned char line);
void draw_string_on_grid(char* string, unsigned char grid_x, unsigned char grid_y, unsigned char selected);
void select_jauge_item(unsigned char type, unsigned char item, unsigned char* jauges, unsigned char way);
void display_screen_configuration_menu(unsigned char type, unsigned char* jauges, unsigned char page);
void Show_Font57(unsigned char data, unsigned char reverse, unsigned char x, unsigned char y);
void Show_String(char* string, unsigned char reverse, unsigned char x, unsigned char y);
void deselect_jauge_item(unsigned char type, unsigned char item, unsigned char* jauges);
unsigned char display_opening_curtain_on_gauges_animation(unsigned char i);
unsigned char display_closing_curtain_on_gauges_animation(unsigned char i);
void display_cp_presence(unsigned char presence, unsigned char position);
void cursor_settings_menu_item(unsigned char item, unsigned char state);
void draw_log_menu(unsigned char item, unsigned char nb_log_eeproms);
void cursor_log_menu_item(unsigned char item, unsigned char state);
void display_map_type(unsigned char type, unsigned char position);
void display_tps_type(unsigned char type, unsigned char position);
void display_vdo_type(unsigned char type, unsigned char position);
void display_afr_type(unsigned char type, unsigned char position);
void update_custom_bitmap(unsigned char data, unsigned int index);
unsigned char display_opening_window_animation(unsigned char i);
unsigned char display_closing_window_animation(unsigned char i);
void display_set_brightness_screen(unsigned char level);
char* get_sensor_descript_string(unsigned char item);
void display_limit_values_info(unsigned char page);
void deselect_alarm_menu_item(unsigned char item);
void display_alarm_picture(unsigned char alarm);
void select_alarm_menu_item(unsigned char item);
void deselect_screen_item(unsigned char item);
void set_custom_bmp_act(unsigned char value);
void draw_settings_menu(unsigned char item);
void select_screen_item(unsigned char item);
void deselect_menu_item(unsigned char item);
void select_menu_item(unsigned char item);
void draw_alarm_menu(unsigned char item);
void draw_menu(unsigned char item);
unsigned char is_custom_bmp_act();
void display_debug_mode_screen();
void lcd_display_grayscale();
void display_custom_bmp();
void draw_screen_frame();
void draw_grid_canvas();
void init_graphics();
void clear_screen();
void show_screen();
void hide_screen();

#endif /* GRAPHICS_H_ */