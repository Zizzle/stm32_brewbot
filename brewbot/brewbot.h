///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2011, Matthew Pratt, licensed under the GPL3.
//
// Authors: Matthew Pratt
//
// Date:  6 Jan 2011
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BREWBOT_H
#define BREWBOT_H

#define COL_BG_NORM 0x0890
#define COL_BG_HIGH 0x0000

#define ON 1
#define OFF 0

#define OPEN 1
#define CLOSED 0

#define SSR 0
#define SOLENOID 1
#define STIRRER 2
#define PUMP 3
#define VALVE 4

#define HOPS1 5
#define HOPS2 6
#define HOPS3 7

// hops can be off (no pwm) or up or down
#define HOPS_UP   1
#define HOPS_DOWN 2

void brewbotOutput(int peripheral, int on);

#endif
