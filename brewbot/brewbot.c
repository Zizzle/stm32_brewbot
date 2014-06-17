#include <stdint.h>
#include "brewbot.h"
#include "stm32f10x.h"

void brewbotOutput(int peripheral, int on)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    TIM_OCInitTypeDef outputChannelInit = {0,};
    TIM_OCStructInit( &outputChannelInit );
    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputChannelInit.TIM_Pulse = 0;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_Low;

	switch (peripheral)
	{
	case SSR:
		if (on)
			outputChannelInit.TIM_Pulse = on * 20;
	    TIM_OC1Init(TIM5, &outputChannelInit);
		break;
	case STIRRER:
	    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	    GPIO_Init( GPIOE, &GPIO_InitStructure );
	    GPIO_WriteBit( GPIOE, GPIO_Pin_2, on == 0 );
		break;
	case PUMP:
	    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	    GPIO_Init( GPIOE, &GPIO_InitStructure );
	    GPIO_WriteBit( GPIOE, GPIO_Pin_4, on == 0 );
		break;
	case VALVE:
	    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	    GPIO_Init( GPIOE, &GPIO_InitStructure );
	    GPIO_WriteBit( GPIOE, GPIO_Pin_6, on != 0 );
		break;

	case HOPS1:
	case HOPS2:
	case HOPS3:
		if (on == HOPS_UP)
			outputChannelInit.TIM_Pulse = 60;
		if (on == HOPS_DOWN)
			outputChannelInit.TIM_Pulse = 200;
		if (!on)
		    outputChannelInit.TIM_OutputState = TIM_OutputState_Disable;

		if (peripheral == HOPS1)
			TIM_OC2Init(TIM5, &outputChannelInit);
		if (peripheral == HOPS2)
			TIM_OC3Init(TIM5, &outputChannelInit);
		if (peripheral == HOPS3)
			TIM_OC4Init(TIM5, &outputChannelInit);
		break;
	}
}

