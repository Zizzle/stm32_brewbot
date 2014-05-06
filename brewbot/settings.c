///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, licensed under the GPL3.
//
// Authors: Matthew Pratt
//
// Date: 13 Feb 2011
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"
#include "lcd.h"
#include "menu.h"
#include "flash.h"

#define SETTINGS_FLASH_ADDR 0
#define SETTINGS_DISPLAY_SIZE 9

struct settings g_settings;

#define TEMP   1
#define UINT8  2

struct settings_display
{
	const char * fmt;
	int type;
	void  *value;
	const char * name;
};

static struct settings_display settings_display_list[] = 
{
		{" Mash target %s C",   TEMP,  &g_settings.mash_target_temp, "mash_target_temp"},
		{" Mash time   %d min", UINT8, &g_settings.mash_time,        "mash_time"},
		{" Mash duty   %d %%",  UINT8, &g_settings.mash_duty_cycle,  "mash_duty_cycle"},
		{" Mash out time %d" ,  UINT8, &g_settings.mash_out_time,    "mash_out_time"},
		{" Boil time   %d min", UINT8, &g_settings.boil_time,        "boil_time"},
		{" Boil duty   %d %%",  UINT8, &g_settings.boil_duty_cycle,  "boil_duty_cycle"},
		{" Hop time 1  %d min", UINT8, &g_settings.hop_addition[0],  "hop_addition_1"},
		{" Hop time 2  %d min", UINT8, &g_settings.hop_addition[1],  "hop_addition_2"},
		{" Hop time 3  %d min", UINT8, &g_settings.hop_addition[2],  "hop_addition_3"},
		{NULL, 0, NULL, NULL}
};

#define SETTINGS_MENU_LEN 9

int settings_offset = 0;
int settings_cursor = 0;

void settings_load()
{
//    printf("\r\nflash 0x%x val %x\r\n", flash_read(), *flash_read());

	memcpy(&g_settings, flash_read(), sizeof(g_settings));

	printf("MAGIC 0x%x\r\n", g_settings.magic);

	if (g_settings.magic != SETTINGS_MAGIC)
	{
		g_settings.magic             = SETTINGS_MAGIC;
		g_settings.mash_time         = 60;
		g_settings.mash_target_temp  = 6600;
		g_settings.mash_out_time     = 5;
		g_settings.boil_time         = 60;
		g_settings.mash_duty_cycle   = 10;
		g_settings.boil_duty_cycle   = 50;
		g_settings.hop_addition[0]   = 60;
		g_settings.hop_addition[1]   = 40;
		g_settings.hop_addition[2]   = 0;
		g_settings.hop_addition[3]   = 0;
		g_settings.hop_addition[4]   = 0;
	}	
	printf("ok\r\n");
}

void settings_save()
{
	flash_write((uint8_t *)&g_settings, sizeof(g_settings));
}

static void settings_display_menu()
{
	lcd_background(0);
	int ii;
	for (ii = 0;
	ii < SETTINGS_DISPLAY_SIZE && settings_display_list[ii + settings_offset].fmt != NULL;
	ii++)
	{
		struct settings_display *disp = &settings_display_list[ii + settings_offset];
		if (disp->type == TEMP)
		{
			lcd_printf(0, 2 + ii, 19, disp->fmt, heat_as_str(*((uint16_t *)disp->value)));
		}
		else if (disp->type == UINT8)
		{
			lcd_printf(0, 2 + ii, 19, disp->fmt, *((uint8_t *)disp->value));
		}
		if (ii + settings_offset == settings_cursor)
		{
			lcd_text(0, 2 + ii, ">");
		}
	}
}

void button(const char *label, int xx, int yy, int ww, int hh, int bgCol, int fgCol)
{
	lcd_fill(xx, yy, ww, hh, bgCol); 
	lcd_text_xy(xx + ww / 2 - (strlen(label)/2) * 8, yy + hh / 2 - 8, label, fgCol, bgCol);
}

void settings_display(int init)
{
#define BUTTON_W 100
#define BUTTON_H 50
#define BG_COL 0x2441
#define FG_COL 0x7665
	
	if (init)
	{
		lcd_text(0, 0, "  Settings");
	    lcd_fill(0, CRUMB_H, LCD_W, LCD_H - CRUMB_H, 0x0);
	    button("/\\",  LCD_W - BUTTON_W, 40 + 0 * BUTTON_H, BUTTON_W, BUTTON_H - 1, BG_COL, FG_COL);
	    button("\\/",  LCD_W - BUTTON_W, 40 + 1 * BUTTON_H, BUTTON_W, BUTTON_H - 1, BG_COL, FG_COL);
	    button("-",    LCD_W - BUTTON_W, 40 + 2 * BUTTON_H, BUTTON_W, BUTTON_H - 1, BG_COL, FG_COL);
	    button("+",    LCD_W - BUTTON_W, 40 + 3 * BUTTON_H, BUTTON_W, BUTTON_H - 1, BG_COL, FG_COL);
	    button("Back", 0, LCD_H - BUTTON_H, BUTTON_W, BUTTON_H - 1, BG_COL, FG_COL);
	    settings_display_menu();
	}
	else
	{
		settings_save();
	}
}

#define IN_BUTTON(nr, xx, yy) xx > LCD_W - BUTTON_W && yy > 40 + nr * BUTTON_H && yy < 40 + (nr + 1) * BUTTON_H

int settings_touch(int xx, int yy)
{
	printf("TOUCH %d %d\r\n", xx, yy);
	if (xx < BUTTON_W && yy >= LCD_H - BUTTON_H)
		return 1;
	
	if (IN_BUTTON(0, xx, yy))
	{
		if (settings_offset != 0 && settings_offset == settings_cursor)
			settings_offset--;
		if (settings_cursor > 0)
			settings_cursor--;		
		settings_display_menu();
	}
	else if (IN_BUTTON(1, xx, yy))
	{
		if (settings_offset < SETTINGS_MENU_LEN - SETTINGS_DISPLAY_SIZE &&
				settings_cursor == settings_offset + SETTINGS_DISPLAY_SIZE - 1)
			settings_offset++;
		if (settings_cursor < SETTINGS_MENU_LEN)
			settings_cursor++;
		settings_display_menu();
	}
	else if (IN_BUTTON(2, xx, yy))
	{
		struct settings_display *disp = &settings_display_list[settings_cursor];
		if (disp->type == TEMP)
		{
			*((uint16_t *)disp->value) -= 50;
		}
		else if (disp->type == UINT8)
		{
			*((uint8_t *)disp->value) -= 1;
		}
		settings_display_menu();
	}
	else if (IN_BUTTON(3, xx, yy))
	{
		struct settings_display *disp = &settings_display_list[settings_cursor];
		if (disp->type == TEMP)
		{
			*((uint16_t *)disp->value) += 50;
		}
		else if (disp->type == UINT8)
		{
			*((uint8_t *)disp->value) += 1;
		}
		settings_display_menu();
	}
		
	return 0;
}

