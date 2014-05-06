///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, licensed under the GPL3.
//
// Authors: Matthew Pratt
//
// Date:  7 Feb 2011
//
///////////////////////////////////////////////////////////////////////////////

#ifndef HEAT_H
#define HEAT_H

#include "brew_task.h"

void    heat_start_task();
void    heat_start(void (*taskErrorHandler)(brew_task_t *), const char *log_dir, int log_number);
void    heat_stop();
char    heat_task_is_running();
uint8_t heat_has_reached_target();
void    heat_set_target_temperature(uint16_t target);
void    heat_set_dutycycle(int duty_cycle);
uint8_t heat_is_heating();

#endif
