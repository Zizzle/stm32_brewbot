#include <stdint.h>
#include "FreeRTOS.h"
#include "brewbot.h"
#include "brew_task.h"
#include "task.h"
#include "lcd.h"
#include "stm32f10x.h"

static struct brew_task hops_task;
#define NUM_SERVOS 3
static int servo;
static int resetting = 0;


static int servo;
static int pos;
static char dirDown;
#define MAX_POS 210
#define MIN_POS 30


static void _hops_start(struct brew_task *bt)
{
	dirDown = 1;
	pos = MIN_POS;
}

static void hops_iteration(struct brew_task *bt)
{
	if (resetting)
	{
		vTaskDelay(1500); // wait for power up
		brewbotOutput(HOPS1, 40);
		vTaskDelay(1500); // wait for the servo to move
		brewbotOutput(HOPS1, OFF);
		brewbotOutput(HOPS2, MIN_POS);
		vTaskDelay(1500); // wait for the servo to move
		brewbotOutput(HOPS2, OFF);
		brewbotOutput(HOPS3, MIN_POS);
		vTaskDelay(1500); // wait for the servo to move
		brewbotOutput(HOPS3, OFF);
		bt->running = 0;
		return;
	}


	if (dirDown)
	{
		pos++;
		if (pos >= MAX_POS)
			dirDown = 0;
	}
	else
	{
		pos--;
		if (pos <= MIN_POS)
		{
			bt->running = 0;
			brewbotOutput(HOPS1 + servo, OFF); // disable PWM
			return;
		}
	}
	brewbotOutput(HOPS1 + servo, pos);
	vTaskDelay(10);
}
static void _hops_stop(struct brew_task *bt)
{
}
/*-----------------------------------------------------------*/

void hops_start_task()
{
	startBrewTask(&hops_task,
			"Hops", 200, 2, 10000,
			_hops_start,
			hops_iteration,
			_hops_stop);
}

char hops_are_dropping()
{
	return hops_task.running;
}

void hops_drop(short dropper, void (*taskErrorHandler)(brew_task_t *))
{
	if (dropper >= NUM_SERVOS)
		return;

	while (hops_are_dropping())
		vTaskDelay(100); // wait for the current drop to finish to happen

	servo = dropper;
	resetting = 0;

	brewTaskStart(&hops_task, taskErrorHandler);
}

void hops_stop()
{
	brewTaskStop(&hops_task);
}

void hops_reset()
{
	resetting = 1;
	brewTaskStart(&hops_task, NULL);
}
