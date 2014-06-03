#include <stdint.h>
#include "brewbot.h"
#include "stm32f10x.h"

void brewbotOutput(int peripheral, int on)
{
    GPIO_InitTypeDef GPIO_InitStructure;
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
			outputChannelInit.TIM_Pulse = on * 10;
	    TIM_OC2Init(TIM5, &outputChannelInit);
		break;
	case STIRRER:
		if (on)
			outputChannelInit.TIM_Pulse = 1000;
	    TIM_OC1Init(TIM5, &outputChannelInit);
		break;
	case PUMP:
		if (on)
			outputChannelInit.TIM_Pulse = 1000;
	    TIM_OC3Init(TIM5, &outputChannelInit);
		break;
	case VALVE:
	    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init( GPIOA, &GPIO_InitStructure );
	    GPIO_WriteBit( GPIOA, GPIO_Pin_3, on != 0 );
		break;
	}
}

