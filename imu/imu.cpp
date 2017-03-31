#include "stm32f10x.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "MPU6050.h"
#include <stdio.h>

MPU6050 mpu6050;
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
float quaternion[4];

static void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);
}

static void SPI_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7; //CLK/MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; //SSEL
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);

    SPI_InitTypeDef   SPI_InitStructure;
	SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial     = 7;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; /* 72/2/64 Mhz */
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

static void EXTI_Configuration(){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);

    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_StructInit(&EXTI_InitStructure);
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void mpu_select(){
	delay_us(1);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
	delay_us(1);
}
void mpu_deselect(){
	delay_us(1);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
}

uint8_t spi_send(uint8_t data){
	SPI_I2S_SendData(SPI1, data);
	while( SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET );
	while( SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET );
	while( SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY)==SET );
	return SPI_I2S_ReceiveData(SPI1);
}
void imu_init(){
    RCC_Configuration();
    SPI_Configuration();
    mpu6050.reset();
    delay_ms(50);
    while(!mpu6050.testConnection());
    while(mpu6050.dmpInitialize()!=0);
    mpu6050.setDMPEnabled(true);
    packetSize = mpu6050.dmpGetFIFOPacketSize();
    mpu6050.setIntDataReadyEnabled(true);
    EXTI_Configuration();
}
extern "C" void EXTI3_IRQHandler(){
    if (EXTI_GetITStatus(EXTI_Line3) != RESET){
    	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_RESET);
        uint8_t mpuIntStatus = mpu6050.getIntStatus();

        // get current FIFO count
        uint16_t fifoCount = mpu6050.getFIFOCount();

        // check for overflow (this should never happen unless our code is too inefficient)
        if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
            // reset so we can continue cleanly
        	mpu6050.resetFIFO();
        // otherwise, check for DMP data ready interrupt (this should happen frequently)
        } else if (mpuIntStatus & 0x02) {
            // wait for correct available data length, should be a VERY short wait
            while (fifoCount < packetSize) fifoCount = mpu6050.getFIFOCount();

            // read a packet from FIFO
            mpu6050.getFIFOBytes(fifoBuffer, packetSize);

            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            fifoCount -= packetSize;

			// display Euler angles in degrees
            mpu6050.dmpGetQuaternion(&q, fifoBuffer);
            quaternion[0]=q.w;
            quaternion[1]=q.x;
            quaternion[2]=q.y;
            quaternion[3]=q.z;
            /*
            q.normalize();
            Quaternion qd=q0.getConjugate().getProduct(q);
            q0=q;
            float yprd[3]={
            		atan2(2*qd.w*qd.z + 2*qd.x*qd.y, 1-2*qd.y*qd.y-2*qd.z*qd.z)*180/M_PI,
            		asin(2*qd.w*qd.y - 2*qd.z*qd.x)*180/M_PI,
            		atan2(2*qd.w*qd.x + 2*qd.y*qd.z, 1-2*qd.x*qd.x-2*qd.y*qd.y)*180/M_PI,
            };
            if(yprd[0]==yprd[0]){
            	ypr[0]+=yprd[0];
            	if(ypr[0]>180)ypr[0]-=360;
            	if(ypr[0]<-180)ypr[0]+=360;
            }
            if(yprd[1]==yprd[1]){
            	ypr[1]+=yprd[1];
            	if(ypr[1]>180)ypr[1]-=360;
            	if(ypr[1]<-180)ypr[1]+=360;
            }
            if(yprd[2]==yprd[2]){
            	ypr[2]+=yprd[2];
            	if(ypr[2]>180)ypr[2]-=360;
            	if(ypr[2]<-180)ypr[2]+=360;
            }*/
            /*
            mpu6050.dmpGetGravity(&gravity, &q);
            mpu6050.dmpGetYawPitchRoll(ypr, &q, &gravity);
            */
        }
        EXTI_ClearITPendingBit(EXTI_Line3);
    	GPIO_WriteBit(GPIOB, GPIO_Pin_3, Bit_SET);
    }
}
