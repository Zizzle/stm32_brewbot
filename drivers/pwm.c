#include <stdio.h>
#include "FreeRTOS.h"
#include "stm32f10x.h"

void pwm_init()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM_TimeBaseInitTypeDef timerInitStructure;
    TIM_TimeBaseStructInit( &timerInitStructure );
    timerInitStructure.TIM_Prescaler = 4000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = 1000;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    timerInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM5, &timerInitStructure);
    TIM_Cmd(TIM5, ENABLE);

    GPIO_InitTypeDef gpioStructure;
    gpioStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    gpioStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioStructure);

    TIM_OCInitTypeDef outputChannelInit = {0,};
    TIM_OCStructInit( &outputChannelInit );
    outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputChannelInit.TIM_Pulse = 0;
    outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
    outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_Low;

    TIM_OC1Init(TIM5, &outputChannelInit); // stirrer
    TIM_OC2Init(TIM5, &outputChannelInit); // ssr heat
    TIM_OC3Init(TIM5, &outputChannelInit); // pump
//    TIM_ARRPreloadConfig( TIM5, ENABLE );
}
