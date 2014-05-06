#ifndef FLASH_H
#define FLASH_H

uint8_t *flash_read();
void flash_write(uint8_t *data, int dataLenBytes);

#endif
