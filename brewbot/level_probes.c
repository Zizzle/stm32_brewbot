///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, licensed under the GPL3+.
//
// Authors: Matthew Pratt
//
// Date: 16 Feb 2011
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"
#include "brewbot.h"

//
// Ok so the level probes sit in water in the pot.
// Effective resistance of the water seems to be about 30k. So I'm
// using a 100k pullup.
//
// But I noticed an interesting effect. Over a few seconds the resistance
// goes up to much higher. And it seems affected by the voltage.
//
// Anyway, we can use a threshold and the ADC to good affect.
//
// Here the ADC runs in continuous scan mode on the level probes and the
// application samples it occasionally.
//

#define THRESHOLD 3500

void level_probe_init()
{
}

uint16_t level_probe_heat_adc()
{
	return 0;
}

uint16_t level_probe_full_adc()
{
	return 0;
}

#define NUM_READINGS 5
struct reading
{
	uint8_t readings[NUM_READINGS];
	uint8_t idx;
	uint8_t cnt;
};

struct reading heat_reading;
struct reading full_reading;

static void keep_reading(struct reading *rd, uint8_t value)
{
	rd->readings[rd->idx] = value;

	if (++(rd->idx) >= NUM_READINGS)
	{
		rd->idx  = 0;
	}

	if (rd->cnt < NUM_READINGS)
		rd->cnt++; 
}

static uint8_t readings_same(struct reading *rd)
{
	int ii;

	if (rd->cnt < NUM_READINGS)
		return 0;

	uint8_t value = rd->readings[0];
	for (ii = 1; ii < rd->cnt; ii++)
	{
		if (rd->readings[ii] != value)
			return 0;
	}
	return 1;
}

int8_t level_hit_heat()
{
	uint8_t value = level_probe_heat_adc() < THRESHOLD;

	keep_reading(&heat_reading, value);
	if (readings_same(&heat_reading))
	{
		return value;
	}
	return -1; // unsure of the value
}

int8_t level_hit_full()
{
	uint8_t value = level_probe_full_adc() < THRESHOLD;
	keep_reading(&full_reading, value);
	if (readings_same(&full_reading))
	{
		return value;
	}
	return -1; // unsure of the value
}

void level_wait_for_steady_readings()
{
	while (level_hit_heat() == -1)
		vTaskDelay(10);
	while (level_hit_full() == -1)
		vTaskDelay(10);
}
