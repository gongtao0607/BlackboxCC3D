#ifndef _MPU_H_
#define _MPU_H_
uint8_t spi_send(uint8_t);
void mpu_select();
void mpu_deselect();
void mpu_init();
extern float ypr[3];
extern float quaternion[4];
extern volatile uint8_t dmp_count;
#endif
