/*
 * button.h
 *
 *  Created on: Oct 9, 2011
 *      Author: matt
 */

#ifndef BUTTON_H_
#define BUTTON_H_

struct button 
{
	int16_t xx;
	int16_t yy;
	int16_t ww;
	int16_t hh;
	const char *label;
	uint16_t bgCol;
	uint16_t fgCol;
	void (*callback)(char button_down);
	
	char pressed;
};

#endif /* BUTTON_H_ */
