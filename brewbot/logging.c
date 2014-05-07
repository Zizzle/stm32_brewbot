///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, All Rights Reserved.
//
// Authors: Matthew Pratt
//
// Date:  2 Mar 2011
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "brewbot.h"
#include "semphr.h"
#include "lcd.h"
#include "brew_task.h"
#include "logging.h"

static brew_task_t log_task;

xSemaphoreHandle xFsSemaphore;
void fs_lock()
{
	if (xFsSemaphore == NULL)
		xFsSemaphore = xSemaphoreCreateMutex();

	while( xSemaphoreTake( xFsSemaphore, ( portTickType ) 100 ) != pdTRUE )
	{
		printf("Waiting a long time for FS lock\r\n");
	}
}

void fs_unlock()
{
	xSemaphoreGive(xFsSemaphore);		
}

static void log_start(brew_task_t *bt)
{
}

static void log_iteration(brew_task_t *bt)
{
	vTaskDelay(10);
}

static void log_stop_(brew_task_t *bt)
{

}

void start_log_task()
{
	startBrewTask(&log_task,
			"Log", 500, 5, 1000000,
			log_start,
			log_iteration,
			log_stop_);
}

void log_stop()
{
	brewTaskStop(&log_task);    
}

int log_find_max_number(char *path)
{
	int max = 0;
	return max;
}

int log_open(const char *dir, int number, char *name, FIL *file)
{
	int result = 0;
	return result;

}

void log_brew(FIL *file, char *fmt, ...)
{
#if 0
	if (file->fs)
	{
		fs_lock();
		char message[40];
		UINT written;
		va_list ap;
		va_start(ap, fmt);
		int len = vsnprintf(message, sizeof(message) - 1, fmt, ap);
		va_end(ap);

		f_write(file, message, len, &written);
		f_sync(file);

		fs_unlock();
	}
#endif
}

void log_close(FIL *file)
{
}
