#ifndef DS1820_H
#define DS1820_H

#define DS1820_PORT GPIOE
#define DS1820_PIN  GPIO_Pin_0

#define BUS_ERROR      0xFE
#define PRESENCE_ERROR 0xFD
#define NO_ERROR       0x00

//Scheduled task for FreeRTOS
void          vTaskDS1820Convert( void *pvParameters ); //task to

//get temp of a particular sensor
uint32_t         ds1820_get_temperature();

uint32_t ds1820_blocking_sample();

void ds1820_convert_start();
void ds1820_convert_complete();


#endif
