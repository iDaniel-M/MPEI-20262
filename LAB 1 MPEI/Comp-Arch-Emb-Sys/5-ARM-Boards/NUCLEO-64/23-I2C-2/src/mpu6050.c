#include "mpu6050.h"

void MPU_Write(uint16_t address, uint8_t reg, uint8_t dat){
  I2C_Master_Start(I2C1);
  I2C_Master_Address(I2C1,address);
  I2C_Master_Write(I2C1,reg);
  I2C_Master_Write(I2C1,dat);
  I2C_Master_Stop(I2C1);
}


void MPU_Read(uint16_t address, uint8_t reg, uint8_t *buffer, uint8_t size){
  I2C_Master_Start(I2C1);
  I2C_Master_Address(I2C1,address);
  I2C_Master_Write(I2C1,reg);
  I2C_Master_Start(I2C1);
  I2C_Master_Read(I2C1,address+0x01,buffer,size);
  I2C_Master_Stop(I2C1);
}


void MPU6050_Init (void)
{
	uint8_t check;
	uint8_t Data;

	// check device ID WHO_AM_I

	MPU_Read (MPU6050_ADDR,WHO_AM_I_REG, &check, 1);

	if (check == 104)  // 0x68 will be returned by the sensor if everything goes well
	{
		// power management register 0X6B we should write all 0's to wake the sensor up
		Data = 0;
		MPU_Write (MPU6050_ADDR, PWR_MGMT_1_REG, Data);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		Data = 0x07;
		MPU_Write(MPU6050_ADDR, SMPLRT_DIV_REG, Data);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> ? 2g
		Data = 0x00;
		MPU_Write(MPU6050_ADDR, ACCEL_CONFIG_REG, Data);

		// Set Gyroscopic configuration in GYRO_CONFIG Register
		// XG_ST=0,YG_ST=0,ZG_ST=0, FS_SEL=0 -> ? 250 ?/s
		Data = 0x00;
		MPU_Write(MPU6050_ADDR, GYRO_CONFIG_REG, Data);
	}

}


