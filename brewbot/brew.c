///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, licensed under the GPL3+.
//
// Authors: Matthew Pratt
//
// Date: 13 Feb 2011
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "stm32f10x.h"
#include "task.h"
#include "semphr.h"

#include "brewbot.h"

#include "lcd.h"
#include "menu.h"
#include "brew.h"
#include "settings.h"
#include "heat.h"
#include "brew_task.h"
#include "brewbot.h"
#include "button.h"
#include "hop_droppers.h"

#define TICKS_PER_MINUTE (60 * configTICK_RATE_HZ)
#define BREW_LOG_PATH "/brews"
#define HH 56 // Button height

brew_task_t brew_task;

struct brew_step
{
	const char *name;
	void (*method)(int);
	int timeout;
};
#define BREW_STEPS_TOTAL 10
static struct brew_step g_steps[BREW_STEPS_TOTAL];

static struct state
{
	int          brew_number;
	char         in_alarm;
	uint8_t      step;
	uint8_t      graphx;
	portTickType brew_start_tick;
	portTickType step_start_tick;
	long         total_runtime;  // seconds since the start of the brew
	long         step_runtime;   // seconds since the start of the current step
	uint8_t      hop_addition_done[MAX_HOP_ADDITIONS];
} g_state = { 0, 0, 0, 0};

static struct button brew_buttons[] = 
{
	{ 0,             LCD_H - 1 * HH,     LCD_W / 2,     HH, "Back", COL_BG_NORM, 0xFFFF, NULL },
	{ 0, 0, 0, 0, NULL}
};

static const char *brew_step_name(unsigned char step)
{
	if (step < BREW_STEPS_TOTAL)
		return g_steps[step].name;
	return "Unknown";
}

static void brew_run_step()
{
	g_state.step_start_tick = xTaskGetTickCount();

	lcd_lock();
    lcd_fill(0, 0, LCD_W, CRUMB_H, 0x0);
	lcd_background(0x0);
    lcd_text(0, 0, "Brewbot:Brewing");
	lcd_fill(0, CRUMB_H - 2, LCD_W, 2, 0xFFFF);
	lcd_fill(0, CRUMB_H, LCD_W, LCD_H - CRUMB_H, COL_BG_NORM);
	lcd_fill(0, LCD_H - 1 * HH - 1, LCD_W / 2, 1, 0xFFFF);
	lcd_fill(LCD_W / 2, LCD_H - 1 * HH - 1, 1, HH, 0xFFFF);
	
	button_paint(brew_buttons);		
	lcd_background(COL_BG_NORM);
	lcd_printf(0, 2, 20, "%d.%s", g_state.step, g_steps[g_state.step].name);
	lcd_release();

	//log_brew(&g_state.log_file, "%.2d:%.2d Run step %d.%s\n",
	//		g_state.total_runtime / 60, g_state.total_runtime % 60,
	//		g_state.step, g_steps[g_state.step].name);

	g_steps[g_state.step].method(1);
}

static void brew_next_step()
{
	if (g_state.step >= BREW_STEPS_TOTAL)
	{
		return;
	}

	//brewbotOutput(STIRRER, OFF);

	g_state.step++;
	brew_run_step();
}

void brew_next_step_if(int cond)
{
	if (cond)
		brew_next_step();
}

void brew_error_handler(brew_task_t *bt)
{
	lcd_printf(0, 2, 18, "%s task failed", bt->name);
	lcd_printf(0, 3, 18, "in step %d %s", g_state.step, brew_step_name(g_state.step));
	lcd_printf(0, 4, 18, "Error = %s", bt->error);
	brew_task.error = "Failed";

// FIXME	audio_beep(1000, 1000);
}

// STEP 2
void brew_fill_and_heat(int init)
{
	if (init)
	{
		heat_start(brew_error_handler, BREW_LOG_PATH, g_state.brew_number);
		heat_set_target_temperature(g_settings.mash_target_temp);
		heat_set_dutycycle(70);
//		fill_start(brew_error_handler);
	}
	else brew_next_step_if(heat_has_reached_target());
}

// STEP 5
void brew_mash(int init)
{
	long remain = g_settings.mash_time * 60 - g_state.step_runtime;
	if (init)
	{
//		brewbotOutput(STIRRER, ON);
		heat_start(brew_error_handler, BREW_LOG_PATH, g_state.brew_number);
		heat_set_target_temperature(g_settings.mash_target_temp);
		heat_set_dutycycle(g_settings.mash_duty_cycle);
	}

//	brewbotOutput(STIRRER, heat_is_heating());
	lcd_printf(0, 1, 19, "%.2d:%.2d Elapsed", g_state.step_runtime / 60,
			g_state.step_runtime % 60);
	lcd_printf(0, 2, 19, "%.2d:%.2d Remaining", remain / 60, remain % 60);

	brew_next_step_if (g_state.step_runtime > g_settings.mash_time * 60);
}

// STEP 6
void brew_mash_out(int init)
{
	long remain = g_settings.mash_out_time * 60 - g_state.step_runtime;

	if (init)
	{
		heat_start(brew_error_handler, BREW_LOG_PATH, g_state.brew_number);
		heat_set_target_temperature(90.0f);
		heat_set_dutycycle(g_settings.boil_duty_cycle);
	}
	else brew_next_step_if (g_state.step_runtime > g_settings.mash_out_time * 60);

	lcd_printf(0, 1, 19, "%.2d:%.2d Elapsed", g_state.step_runtime / 60,
			g_state.step_runtime % 60);
	lcd_printf(0, 2, 19, "%.2d:%.2d Remaining", remain / 60, remain % 60);

}

// STEP 7
void brew_to_boil(int init)
{
	if (init)
	{
		heat_start(brew_error_handler, BREW_LOG_PATH, g_state.brew_number);
		heat_set_target_temperature(90.0f);
		heat_set_dutycycle(g_settings.boil_duty_cycle);
	}
	else brew_next_step_if (heat_has_reached_target());
}

// STEP 9
void brew_boil_hops(int init)
{
	int ii;
	long remain = g_settings.boil_time * 60 - g_state.step_runtime;

	for (ii = 0; ii < HOP_DROPPER_NUM; ii++)
	{
		long drop_second = g_settings.hop_addition[ii] * 60; 

		if (g_state.hop_addition_done[ii] == 0)
			lcd_printf(0, 1 + ii, 19, "Hops %d in %.2d:%.2d", ii + 1,
					(remain - drop_second) / 60,
					(remain - drop_second) % 60);

		if (init)
		{
			g_state.hop_addition_done[ii] = 0;
		}
		else if (remain <= drop_second && !g_state.hop_addition_done[ii])
		{
			g_state.hop_addition_done[ii] = 1;
			hops_drop(ii, brew_error_handler);
		}
	}
	lcd_printf(0, 4, 19, "%.2d:%.2d Remaining", remain / 60, remain % 60);

	brew_next_step_if (remain <= 0);
}

// STEP 10
void brew_finish(int init)
{
	static int beep_freq = 100;

	if (init)
	{
		heat_stop();
	}
	else
	{
// FIXME		audio_beep(beep_freq, 400);
		if ((beep_freq += 10) > 3000)
			beep_freq = 100;
	}
}

static struct brew_step g_steps[BREW_STEPS_TOTAL] = 
{
		{"Fill & Heat",        brew_fill_and_heat,   0},
		{"Mash",               brew_mash,            0},
		{"Mash out",           brew_mash_out,        0},
		{"Bring to boil",      brew_to_boil,         0},
		{"Boil & Hops",        brew_boil_hops,       0},
		{"Finish",             brew_finish,          0},
};

void brew_start_cb(brew_task_t *bt)
{
	brew_run_step();
}

void brew_iterate_cb(brew_task_t *bt)
{
	// caclulate the runtimes
	g_state.total_runtime = (xTaskGetTickCount() - g_state.brew_start_tick) / configTICK_RATE_HZ;
	g_state.step_runtime  = (xTaskGetTickCount() - g_state.step_start_tick) / configTICK_RATE_HZ;

	// run the current step
	g_steps[g_state.step].method(0);
	vTaskDelay(100);

	// display the total run time
	lcd_background(COL_BG_NORM);
	if (g_state.step != BREW_STEPS_TOTAL - 1)
		lcd_printf(20, 2, 5, "%.2d:%.2d", g_state.total_runtime / 60, g_state.total_runtime % 60);
}

void brew_stop_cb(brew_task_t *bt)
{
	// all off
	heat_stop();
//	brewbotOutput(STIRRER, OFF);
//	log_close(&g_state.log_file);
}

static void do_brew_start(int resume)
{
//	g_state.brew_number = log_find_max_number(BREW_LOG_PATH) + (resume == 0);
//	log_open(BREW_LOG_PATH, g_state.brew_number, "brew_log.txt", &g_state.log_file);
	brewTaskStart(&brew_task, NULL);
}

void brew_start_task()
{
	startBrewTask(&brew_task,
			"Brew", 700, 3, portMAX_DELAY,
			brew_start_cb,
			brew_iterate_cb,
			brew_stop_cb);
}

void brew_start(int init)
{
	if (init)
	{
		g_state.brew_start_tick = xTaskGetTickCount();
		g_state.step = 0;
		do_brew_start(0);
	}
	else
	{
//		log_brew(&g_state.log_file, "%.2d:%.2d Brew stopped\n", g_state.total_runtime / 60, g_state.total_runtime % 60);
		brewTaskStop(&brew_task);	
	}  
}

int brew_touch(int xx, int yy)
{
	printf("TOUCH %d %d\r\n", xx, yy);
	return button_touch(brew_buttons, xx, yy);
}


void brew_resume_prev(char button_down)
{
	printf("PREV %d", button_down);
	
	if (button_down) return;

	if (g_state.step != 0)
		g_state.step--;

	lcd_background(COL_BG_NORM);
	lcd_printf(10, 4, 30, "%d. %s", g_state.step + 1, brew_step_name(g_state.step));
}

void brew_resume_next(char button_down)
{
	if (button_down) return;
	
    if (g_state.step < BREW_STEPS_TOTAL - 1)
        g_state.step++;

	lcd_background(COL_BG_NORM);
	lcd_printf(10, 4, 30, "%d. %s", g_state.step + 1, brew_step_name(g_state.step));
}

void brew_resume_go(char button_down)
{
	if (!button_down) do_brew_start(1);
}
		
static struct button resume_buttons[] = 
{
	{ 0,             LCD_H - 2 * HH - 1, LCD_W / 2,     HH, "<",    COL_BG_NORM, 0xFFFF, brew_resume_prev },
	{ LCD_W / 2 + 1, LCD_H - 2 * HH - 1, LCD_W / 2 - 1, HH, ">",    COL_BG_NORM, 0xFFFF, brew_resume_next },
	{ 0,             LCD_H - 1 * HH,     LCD_W / 2,     HH, "Back", COL_BG_NORM, 0xFFFF, NULL },
	{ LCD_W / 2 + 1, LCD_H - 1 * HH,     LCD_W / 2 - 1, HH, "Brew", COL_BG_NORM, 0xFFFF, brew_resume_go   },
	{ 0, 0, 0, 0, NULL}
};

void brew_resume(int init)
{
	if (init)
	{
		g_state.in_alarm = 0;
		lcd_lock();
	    lcd_fill(0, 0, LCD_W, CRUMB_H, 0x0);
	    lcd_text(0, 0, "Brewbot:Resume at step");
		lcd_fill(0, CRUMB_H - 2, LCD_W, 2, 0xFFFF);
		lcd_fill(0, CRUMB_H, LCD_W, LCD_H - CRUMB_H - 2 * HH, COL_BG_NORM);
		lcd_fill(0, LCD_H - 2 * HH - 2, LCD_W, 1, 0xFFFF);
		lcd_fill(0, LCD_H - 1 * HH - 1, LCD_W, 1, 0xFFFF);
		lcd_fill(LCD_W / 2, LCD_H - 2 * HH - 1, 1, 2 * HH, 0xFFFF);
		
		button_paint(resume_buttons);		
		lcd_background(COL_BG_NORM);
		lcd_printf(10, 4, 30, "%d. %s", g_state.step + 1, brew_step_name(g_state.step));
		lcd_release();
	}
	else
	{
//		log_brew(&g_state.log_file, "%.2d:%.2d Brew stopped\n", g_state.total_runtime / 60, g_state.total_runtime % 60);
		brewTaskStop(&brew_task);
	}
}

int brew_resume_touch(int xx, int yy)
{
	return button_touch(resume_buttons, xx, yy);
}
