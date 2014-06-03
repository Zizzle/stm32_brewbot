///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, licensed under the GPL3.
//
// Authors: Matthew Pratt
//
// Date:  9 Feb 2011
//
///////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "menu.h"
#include "lcd.h"
#include "brewbot.h"
#include "heat.h"
#include "hop_droppers.h"

#define DIAG_LOG_DIR "/diags"

static uint16_t heat_target = 6600;
static int      heat_duty   = 50;

void diag_error_handler(brew_task_t *bt)
{
	lcd_printf(0,5, 19, "Error: %s", bt->error);
}



//----------------------------------------------------------------------------
static void diag_heat(int initializing)
{
	if (initializing)
	{
	
		heat_start(diag_error_handler, DIAG_LOG_DIR, 0);
		heat_set_target_temperature(heat_target);
		heat_set_dutycycle(heat_duty);
	}
	else
	{
		heat_stop();
	}
}

//----------------------------------------------------------------------------
static void diag_mash(int initializing)
{
	brewbotOutput(STIRRER, OFF);
}

//----------------------------------------------------------------------------
static void solenoid_init(int initializing)
{
	brewbotOutput(SOLENOID, OFF);
}
static void pump_init(int initializing)
{
	brewbotOutput(PUMP, OFF);
}
static void valve_init(int initializing)
{
	brewbotOutput(VALVE, OFF);
}

static void stirrer_on(unsigned char button_down)
{
	if (button_down) brewbotOutput(STIRRER, ON);
}
static void stirrer_off(unsigned char button_down)
{
	if (button_down) brewbotOutput(STIRRER, OFF);
}
static void stirrer_pulse(unsigned char button_down)
{
	brewbotOutput(STIRRER, button_down);
}

static void solenoid_on(unsigned char button_down)
{
	if (button_down) brewbotOutput(SOLENOID, ON);
}
static void solenoid_off(unsigned char button_down)
{
	if (button_down) brewbotOutput(SOLENOID, OFF);
}
static void solenoid_pulse(unsigned char button_down)
{
	lcd_printf(20, 1, 10, "pulse %d", button_down);
	brewbotOutput(SOLENOID, button_down);
}

static void pump_on(unsigned char button_down)
{
	if (button_down) brewbotOutput(PUMP, ON);
}
static void pump_off(unsigned char button_down)
{
	if (button_down) brewbotOutput(PUMP, OFF);
}
static void pump_pulse(unsigned char button_down)
{
	lcd_printf(20, 1, 10, "pulse %d", button_down);
	brewbotOutput(PUMP, button_down);
}

static void valve_on(unsigned char button_down)
{
	if (button_down) brewbotOutput(VALVE, ON);
}
static void valve_off(unsigned char button_down)
{
	if (button_down) brewbotOutput(VALVE, OFF);
}

static void hops_1(unsigned char button_down)
{
	if (button_down) hops_drop(0, diag_error_handler);
}
static void hops_2(unsigned char button_down)
{
	if (button_down) hops_drop(1, diag_error_handler);
}
static void hops_3(unsigned char button_down)
{
	if (button_down) hops_drop(2, diag_error_handler);
}

static void heat_temp_up(unsigned char button_down)
{
	if (button_down)
	{
		if (heat_target <= 11000) heat_target += 50;
		heat_set_target_temperature(heat_target);
	}
}
static void heat_temp_down(unsigned char button_down)
{
	if (button_down)
	{
		if (heat_target >= 50) heat_target -= 50;
		heat_set_target_temperature(heat_target);
	}
}
static void heat_duty_up(unsigned char button_down)
{
	if (button_down)
	{
		if (heat_duty < 100) heat_duty += 5;
		heat_set_dutycycle(heat_duty);
	}
}
static void heat_duty_down(unsigned char button_down)
{
	if (button_down)
	{
		if (heat_duty >= 5) heat_duty -= 5;
		heat_set_dutycycle(heat_duty);
	}
}

struct menu heat_menu[] = 
{
		{"Target +0.5",    NULL,              NULL,           heat_temp_up},
		{"Target -0.5",    NULL,              NULL,           heat_temp_down},
		{"Duty   +5",      NULL,              NULL,           heat_duty_up},
		{"Duty   -5",      NULL,              NULL,           heat_duty_down},
		{"Back",           NULL,              NULL,           NULL},
		{"",               NULL,              NULL,           NULL},
		{"",               NULL,              NULL,           NULL},
		{NULL, NULL, NULL, NULL}
};

struct menu stirrer_menu[] = 
{
		{"On",             NULL,              NULL,           stirrer_on,                NULL},
		{"Off",            NULL,              NULL,           stirrer_off,               NULL},
		{"Pulse",          NULL,              NULL,           stirrer_pulse,             NULL},
		{"Back",           NULL,              NULL,           NULL,                      NULL},
		{NULL, NULL, NULL, NULL}
};

struct menu solenoid_menu[] = 
{
		{"On",             NULL,              NULL,           solenoid_on,               NULL},
		{"Off",            NULL,              NULL,           solenoid_off,              NULL},
		{"Pulse",          NULL,              NULL,           solenoid_pulse,            NULL},
		{"Back",           NULL,              NULL,           NULL,                      NULL},
		{NULL, NULL, NULL, NULL}
};

struct menu pump_menu[] =
{
		{"On",             NULL,              NULL,           pump_on,             		 NULL},
		{"Off",            NULL,              NULL,           pump_off,            	     NULL},
		{"Pulse",          NULL,              NULL,           pump_pulse,        	     NULL},
		{"Back",           NULL,              NULL,           NULL,                      NULL},
		{NULL, NULL, NULL, NULL}
};

struct menu valve_menu[] =
{
		{"On",             NULL,              NULL,           valve_on,            		 NULL},
		{"Off",            NULL,              NULL,           valve_off,           	     NULL},
		{"Back",           NULL,              NULL,           NULL,                      NULL},
		{NULL, NULL, NULL, NULL}
};

struct menu hops_menu[] = 
{
		{"Drop hops 1",    NULL,              NULL,           hops_1,               NULL},
		{"Drop hops 2",    NULL,              NULL,           hops_2,               NULL},
		{"Drop hops 3",    NULL,              NULL,           hops_3,               NULL},
		{"Back",           NULL,              NULL,           NULL,                 NULL},
		{NULL, NULL, NULL, NULL}
};

struct menu diag_menu[] =
{
		{"Heat",           heat_menu,         diag_heat,      NULL},
		{"Mash stirrer",   stirrer_menu,      diag_mash,      NULL},
		{"Solenoid",       solenoid_menu,     solenoid_init,  NULL},
		{"Pump",           pump_menu,         pump_init,      NULL},
		{"Valve",          valve_menu,        valve_init,      NULL},
		{"Hops",           hops_menu,         NULL,           NULL},
		{"Back",           NULL,              NULL,           NULL},
		{NULL, NULL, NULL, NULL}
};
