#include <stdint.h>
#include "FreeRTOS.h"
#include "brewbot.h"
#include "brew_task.h"
#include "task.h"
#include "lcd.h"

#define timerINTERRUPT_FREQUENCY		( 20000UL )
#define timerHIGHEST_PRIORITY			( 15 )

/* Counts the number of high frequency interrupts - used to generate the run
time stats. */
volatile unsigned long ulHighFrequencyTickCount = 0UL;
static int servoPwmCount = 0;
static struct brew_task hops_task;
#define NUM_SERVOS 3

static int servoPwm;
static int servo;
static int pos;
static char dirDown;
static char on = 0;

static uint32_t timeoutOn;
static uint32_t timeoutOff;


#define MAX_POS 160
#define MIN_POS 5

/*-----------------------------------------------------------*/
void servo_set_pos(short degrees)
{
	servoPwm = 10 + (degrees * 40) / 180;
	timeoutOn  = servoPwm         * ((configCPU_CLOCK_HZ / 2000) / 20);
	timeoutOff = (400 - servoPwm) * ((configCPU_CLOCK_HZ / 2000) / 20);
}
/*-----------------------------------------------------------*/

static void _hops_start(struct brew_task *bt)
{
	dirDown = 1;
	pos = MAX_POS;
	servo_set_pos(MAX_POS);

//	PIT_LDVAL0 = timeoutOn;
	/* Enable the timer and enable interrupts */
//	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK;

//	if (servo == 0) outputEnable(HOP_DROPPER_1); else inputEnable(HOP_DROPPER_1);
//	if (servo == 1) outputEnable(HOP_DROPPER_2); else inputEnable(HOP_DROPPER_2);
//	if (servo == 2) outputEnable(HOP_DROPPER_3); else inputEnable(HOP_DROPPER_3);
}
/*-----------------------------------------------------------*/
static void _hops_stop(struct brew_task *bt)
{
//	inputEnable(HOP_DROPPER_1);
//	inputEnable(HOP_DROPPER_2);
//	inputEnable(HOP_DROPPER_3);
}

static void hops_iteration(struct brew_task *bt)
{
	if (dirDown)
	{
		servo_set_pos(pos--);
		if (pos <= MIN_POS)
			dirDown = 0;
	}
	else
	{
		servo_set_pos(pos++);

		if (pos >= MAX_POS)
			bt->running = 0;
	}

	vTaskDelay(10);
}

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
	if (servo >= NUM_SERVOS)
		return;

	while (hops_are_dropping())
		vTaskDelay(100); // wait for the current drop to finish to happen

	servo = dropper;

	brewTaskStart(&hops_task, taskErrorHandler);
}

void hops_stop()
{
	brewTaskStop(&hops_task);
}
