#include <stdio.h>
#include "aht20.h"
#include "i2c.h"
#include "oled.h"
#include "modbus.h"

float RH;		// 湿度，转换单位后的实际值，标准单位%
float Temp;		// 温度，转换单位后的实际值，标准单位°C

//读取AHT20的状态寄存器 读 bit0=1 写 bit=0 
uint8_t AHT20_Read_Status(void)
{	
	uint8_t tmp[1];
	HAL_I2C_Master_Receive(&hi2c1,ATH20_SLAVE_ADDRESS,tmp,1,0xFFFF);	
	return tmp[0];
}

// AHT20 触发测量命令
static void AHT20_TrigMeasureCmd(void)
{
	uint8_t tmp[3];
	tmp[0] = 0xac; // AHT20_TrigMeasure_REG	0xAC	//触发测量 寄存器地址
	tmp[1] = 0x33;
	tmp[2] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, ATH20_SLAVE_ADDRESS,tmp,3,0xFFFF);
}


//重新初始化AHT20
void AHT20_DeInit(void)   
{	
	uint8_t tmp[3];
	tmp[0] = 0xbe; //AHT20_INIT_REG				0xBE	//初始化 寄存器地址
	tmp[1] = 0x08;
	tmp[2] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, ATH20_SLAVE_ADDRESS,tmp,3,0xFFFF);
}

//初始化AHT20
void AHT20_Init(void)   
{	
	HAL_Delay(100);	//刚上电，延时100ms才可以读取状态
	
	uint8_t tmp[3];
	tmp[0] = 0xbe; //AHT20_INIT_REG				0xBE	//初始化 寄存器地址
	tmp[1] = 0x08;
	tmp[2] = 0x00;
	HAL_I2C_Master_Transmit(&hi2c1, ATH20_SLAVE_ADDRESS,tmp,3,0xFFFF);
	
	if(!((AHT20_Read_Status() & 0x08) == 0x08))
	{
       AHT20_DeInit(); //重新初始化AHT20 
	}
	
}

////没有CRC校验，直接读取AHT20的温度和湿度数据    
//void AHT20_ReadData(void) 
//{
//	uint32_t RetuData = 0;
//	uint16_t cnt = 0;
//	uint8_t tmp[6];
//	
//	AHT20_TrigMeasureCmd();	//向AHT20发送AC命令
//	HAL_Delay(80);			//大约延时80ms
//    
//	while(((AHT20_Read_Status() & 0x80) == 0x80))//测量标志 直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
//	{
//		HAL_Delay(1);
//		if(cnt++>=100) break;
//	}
//  	
//	HAL_I2C_Master_Receive(&hi2c1,ATH20_SLAVE_ADDRESS,tmp,6,0xFFFF);
//	
//	// 计算相对湿度RH。原始值，未计算为标准单位%。
//	RetuData = 0;
//	RetuData = (RetuData|tmp[1]) << 8;
//	RetuData = (RetuData|tmp[2]) << 8;
//	RetuData = (RetuData|tmp[3]);
//	RetuData = RetuData >> 4;
//	RH = RetuData*100.0/1024/1024;
//	
//	// 计算温度T。原始值，未计算为标准单位°C。
//	RetuData = 0;
//	RetuData = (RetuData|tmp[3]) << 8;
//	RetuData = (RetuData|tmp[4]) << 8;
//	RetuData = (RetuData|tmp[5]);
//	RetuData = RetuData&0xfffff;
//	Temp = RetuData*200.0/1024/1024-50;//计算得到温度值

//}

static uint8_t AHT20_CRC8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;

    for (uint8_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;//crc8: 1+x4+x5+x8=0x31 
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint8_t AHT20_ReadData(void)
{
    uint8_t tmp[7];
	uint16_t cnt = 0;
    uint32_t RetuData = 0;

	AHT20_TrigMeasureCmd();	//向AHT20发送AC命令
	HAL_Delay(80);			//大约延时80ms
    
	while(((AHT20_Read_Status() & 0x80) == 0x80))//测量标志 直到状态bit[7]为0，表示为空闲状态，若为1，表示忙状态
	{
		HAL_Delay(1);
		if(cnt++>=100) break;
	}

	//while(HAL_I2C_Master_Receive(&hi2c1,ATH20_SLAVE_ADDRESS,tmp, 7,HAL_MAX_DELAY) != HAL_OK);

	HAL_I2C_Master_Receive(&hi2c1,ATH20_SLAVE_ADDRESS,tmp, 7,HAL_MAX_DELAY);

	uint8_t count = AHT20_MAX_RETRY;
	uint8_t crc_err = 0;
	if(AHT20_CRC8(tmp, 6) != tmp[6])
	{
		while(count--)
		{
			HAL_I2C_Master_Receive(&hi2c1,ATH20_SLAVE_ADDRESS,tmp, 7,HAL_MAX_DELAY);
		}
		crc_err++;
	}

	// 计算相对湿度RH。原始值，未计算为标准单位%。
	RetuData = 0;
	RetuData = (RetuData|tmp[1]) << 8;
	RetuData = (RetuData|tmp[2]) << 8;
	RetuData = (RetuData|tmp[3]);
	RetuData = RetuData >> 4;
	RH = RetuData*100.0/1024/1024;
	
	// 计算温度T。原始值，未计算为标准单位°C。
	RetuData = 0;
	RetuData = (RetuData|tmp[3]) << 8;
	RetuData = (RetuData|tmp[4]) << 8;
	RetuData = (RetuData|tmp[5]);
	RetuData = RetuData&0xfffff;
	Temp = RetuData*200.0/1024/1024-50;//计算得到温度值
	
	return crc_err;
}

void AHT20_Read(void)
{
	if(AHT20_ReadData())
	{
		printf("crc failed");
	}
//	printf("RH  =%.1f %%\n",RH);
//	printf("Temp=%.1f C\n",Temp);
	REG_HOLD_BUF[1] = Temp * 100 ;  
	REG_HOLD_BUF[2] = RH * 100 ;  
}


