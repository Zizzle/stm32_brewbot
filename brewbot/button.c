/*
 * button.c
 *
 *  Created on: Oct 9, 2011
 *      Author: matt
 */

#include <string.h>
#include <stdint.h>
#include "lcd.h"
#include "button.h"

void button_draw(struct button *bb, char down)
{
	lcd_fill(bb->xx, bb->yy, bb->ww, bb->hh, down ? 0x0 : bb->bgCol); 
	lcd_text_xy(bb->xx + bb->ww / 2 - (strlen(bb->label)/2) * 8, bb->yy + bb->hh / 2 - 8, bb->label, bb->fgCol, down ? 0 : bb->bgCol);
	bb->pressed = down;
}

int button_touch(struct button *buttons, int xx, int yy)
{
	int back = 0;
	for (int ii = 0; buttons[ii].label; ii++)
	{
		struct button *b = &buttons[ii];
		if (xx >= b->xx && xx <= b->xx + b->ww && yy >= b->yy && yy <= b->yy + b->hh)
		{
			if (!b->pressed)
			{
				button_draw(b, 1);
				if (b->callback)
					b->callback(1);
			}
		}
		else
		{
			if (b->pressed)
			{
				button_draw(b, 0);
				if (b->callback)
					b->callback(0);
				if (!strcmp("Back", b->label))
					back = 1;
			}
		}
	}
	return back;
}

void button_paint(struct button buttons[])
{
	for (int ii = 0; buttons[ii].label; ii++)
	{
		button_draw(&buttons[ii], 0);
	}
}
