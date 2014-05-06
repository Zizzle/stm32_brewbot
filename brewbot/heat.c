#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "brewbot.h"
#include "semphr.h"
#include "ds1820.h"
#include "lcd.h"
#include <stdio.h>
#include "brew_task.h"
#include "level_probes.h"

static void display_status();

#define NUM_READINGS 5

static struct brew_task heat_task;
static volatile int     heat_target = 0.0;
static volatile int     heat_duty_cycle = 50;
static volatile uint8_t heat_target_reached;
static int              heat_readings[NUM_READINGS];
static uint8_t          heat_reading_idx;
static uint8_t          heat_reading_cnt;

static char    temp_as_str[10];
const char *heat_as_str(int temp)
{
	snprintf(temp_as_str, sizeof(temp_as_str), "%d.%d", temp / 100, temp % 100);
	temp_as_str[4] = 0;
	return temp_as_str;
}

static void heat_log()
{
	/*
	log_brew(&log_file, "%d,%s,%.1f,%d,%d\n",
			brewTaskTick(&heat_task),
			heat_as_str(ds1820_get_temperature()),
			heat_target,
			heat_target_reached,
			heat_duty_cycle);
			*/
}

static void allOff()
{
	brewbotOutput(SSR, OFF);
}

static void display_status()
{
	lcd_background(COL_BG_NORM);
	lcd_printf(22, 5, 19, "Target %s hit %d", heat_as_str(heat_target), heat_target_reached);
	lcd_printf(22, 6, 18, "Temp %s (%d%)",    heat_as_str(ds1820_get_temperature()), heat_duty_cycle);
	lcd_printf(22, 7, 19, "Probe: %d %d",level_hit_heat(), level_probe_heat_adc());
	heat_log();
}

static void heat_keep_temperature()
{
	heat_readings[heat_reading_idx] = ds1820_get_temperature();
	if (++heat_reading_idx >= NUM_READINGS)
	{
		heat_reading_idx  = 0;
	}

	if (heat_reading_cnt < NUM_READINGS)
		heat_reading_cnt++; 
}

static uint8_t test_had_reached_target()
{
	int ii;

	if (heat_target_reached)
		return 1;

	if (heat_reading_cnt < NUM_READINGS)
		return 0;

	for (ii = 0; ii < heat_reading_cnt; ii++)
	{
		if (heat_readings[ii] < heat_target)
			return 0;
	}
	return 1;
}

static void _heat_start(struct brew_task *bt)
{
	allOff();

	ds1820_blocking_sample();

	// make sure we have consistent readings on the level probes
	level_wait_for_steady_readings();
}

static void heat_iteration(struct brew_task *bt)
{
	int ii = 0;
	display_status();

	ds1820_convert_start();

	// 1 second delay, run heat according to the duty cycle
	for (ii = 0; ii < 100; ii++)
	{
		if (ds1820_get_temperature() < heat_target &&
				ii <= heat_duty_cycle &&
				level_hit_heat() == 1) // check the element is covered
		{
			brewbotOutput(SSR, ON);
			lcd_printf(22, 8, 10, "Heat ON");
		}
		else
		{
			lcd_printf(22, 8, 10, "Heat OFF");
			heat_target_reached = test_had_reached_target();
			allOff();
		}
		vTaskDelay(10); // wait for the conversion to happen
	}

	ds1820_convert_complete();
	heat_keep_temperature();
}

void _heat_stop(struct brew_task *bt)
{
	allOff();
}

void heat_start_task()
{
	startBrewTask(&heat_task,
			"Heat", 400, 2, 10000000,
			_heat_start,
			heat_iteration,
			_heat_stop);
}

void heat_start(void (*taskErrorHandler)(brew_task_t *), const char *log_dir, int log_number)
{
	brewTaskStart(&heat_task, taskErrorHandler);
	//log_brew(&log_file, "%d,Starting", brewTaskTick(&heat_task));
}

void heat_stop()
{
	brewTaskStop(&heat_task);
//	log_brew(&log_file, "%d,Stopped", brewTaskTick(&heat_task));
//	log_close(&log_file);
}

char heat_task_is_running()
{
	return heat_task.running;
}

uint8_t heat_has_reached_target()
{
	return heat_target_reached;
}

void heat_set_target_temperature(uint16_t target)
{
	heat_target_reached = 0;
	heat_target         = target;
	display_status();
}

void heat_set_dutycycle(int duty_cycle)
{
	heat_duty_cycle = duty_cycle;
	display_status();
}

uint8_t heat_is_heating()
{
	return ds1820_get_temperature() < heat_target &&
	level_hit_heat() == 1; // check the element is covered
}
