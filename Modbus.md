## Modbus协议

### 工业常用通讯协议（请求/应答 ） 工业串行链路的事实标准 包括RTU(16进制)、ASCII、TCPModbusModbus是⼀个请求/应答协议，并且提供功能码规定的服务。 Modbus功能码是 Modbus请求/应答PDU的元素

### 单播模式
#### “⼀对⼀”通讯。主站只寻找某⼀确定的从站，从站接收到命令后处理，并返回⼀个应答报⽂。主站需要发出⼀个命令报⽂并处理从站返回的报⽂，从站需要接收主站的命令报⽂并发出⼀个应答报⽂。每个⼦节点必须有唯⼀的地址（1-247）
### 广播模式
#### ⼀对所有”。主站向所有从站发送请求，对于主站发送的⼴播请求没有应答返回，⼴播请求必须是写命令。所有从站必须接收写功能的⼴播。地址0⽤来⼴播通信
### 寄存器
![alt text](image-2.png)
---

### 1. Modbus-RTU

#### ModbusRTU是⼀种主从通讯模式的通讯协议，也就是说，Modbus有⼀个主机，可以进⾏通讯的主动要求，其他从机只能对主机进⾏响应⽽不能主动发送数据到通讯总线中。这种⽅法规定了通讯过程中的通讯次序等关系，避免了多个设备同时⼯作的情况下通讯冲突的产⽣。Modbus协议不规定⼀个字节如何传输，⽽是规定如何进⾏⼀次数据帧的传输。数据帧可以理解为若⼲个具有特殊功能意义的字节的组合。
- 那么Modbus如何定义⼀个数据帧？对于Modbus来说，当进⾏数据传输过程中，出现空闲时间超过**3.5**个字节持续时间，就认为⼀次数据帧的结束，之前接收到的字节就是这次数据帧的所有字节。之后再接收到的字节则为下⼀个数据帧的字节。
- 例如，在9600bit/s的传输速率下，⼀个字节传输的时间约为0.8ms，那么当数据传输中，出现约3ms的空闲时间时，设备就认为⼀帧数据接收完成

#### 数据帧=设备码+功能码+数据码+校验码
1. 设备码（从机地址）主机地址为0 分配255个从机不同设备码
2. 功能码（区分读/写）
3. 数据码
- 数据码是功能码的进一步解释，功能码，后⾯跟的数据码包括2个字节表⽰寄存器地址，2个字节表⽰读取的寄存器个数（寄存器的位数为16位，因此⼀个寄存器有两个字节的数据）。返回的功能码，后接1个字节的返回字节个数（该个数应为上述读取寄存器个数的两倍，因为⼀个寄存器对应两个字节），和若⼲个字节的数据
4. 校验码（CRC 16位）
5. 查询功能码

```
//常用03 06
0x01: 读线圈寄存器
0x02: 读离散输入寄存器
0x03: 读保持寄存器
0x04: 读输入寄存器
0x05: 写单个线圈寄存器
0x06: 写单个保持寄存器
0x0f: 写多个线圈寄存器
0x10: 写多个保持寄存器
```
- 线圈寄存器，实际上就可以类⽐为开关量，⼀个bit都对应⼀个信号的开关状态。所以⼀个byte就可以同时控制8路的信号。⽐如控制外部8路io的⾼低。 线圈寄存器⽀持读也⽀持写，写在功能码⾥⾯⼜分为写单个线圈寄存器和写多个线圈寄存器。
`对应上⾯的功能码也就是：0x01 0x05 0x0f`
- 离散输⼊寄存器，如果线圈寄存器理解了这个⾃然也明⽩了。离散输⼊寄存器就相当于线圈寄存器的只读模式，他也是每个bit表⽰⼀个开关量，⽽他的开关量只能读取输⼊的开关信号，是不能够写的。⽐如我读取外部按键的按下还是松开。
`所以功能码也简单就⼀个读的 0x02`
- 保持寄存器，这个寄存器的单位不再是bit⽽是两个byte，也就是可以存放具体的数据量的，并且是可读写的。⽐如我我设置时间年⽉⽇，不但可以写也可以读出来现在的时间。写也分为单个写和多个写。
`所以功能码有对应的三个：0x03 0x06 0x10`
- 输⼊寄存器，只剩下这最后⼀个了，这个和保持寄存器类似，但是也是只⽀持读⽽不能写。⼀个寄存器也是占据两个byte的空间。类⽐我通过读取输⼊寄存器获取现在的AD采集值。
`对应的功能码也就⼀个 0x04`

```bash
//03
主机发送:`01 03 00 00 00 01 84 0A`
从机回复: `01 03 02 19 98 B2 7E`
发送：从机的地址+我要⼲嘛的功能码+我要查的寄存器的地址+我要查的寄存器地址的个数+校验码
回复：从机的地址+主机发我的功能码+要发送给主机数据的字节数+数据+校验码

//06
主机发送: 01 06 00 00 00 01 48 0A
从机回复: 01 06 00 00 00 01 48 0A
发送：从机的地址+我要⼲嘛的功能码+我要改的寄存器的地址+我要修改的数据+校验码
回复：从机的地址+主机发我的功能码+我要改的寄存器的地址+我要修改的数据+校验码
```

---

## 2. FreeModbus

### FreeModbus 是⼀个开源的 Modbus 协议栈实现。Modbus 是⼀种通信协议，⽤于在⼯业⾃动化系统中传输数据 FreeModbus ⽀持 Modbus RTU（串⾏）和 Modbus TCP（以太⽹）两种传输模式  

| 名称        | 功能                                                         |
| ----------- | ------------------------------------------------------------ |
| eMBInit()   | 完成MODBUS的初始化配置                                       |
| eMBEnable() | 使能Modbus协议栈                                             |
| eMBPoll()   | 轮询Modbus的数据接收，并进⾏数据的处理，这个函数需要循环调⽤ |

### 源码

#### 从机地址(0~255)
```
static UCHAR ucMBAddress
```

#### 从机模式
```
static eMBMode eMBCurrentMode

typedef enum
{
MB_RTU, /*!< RTU transmission mode. */
MB_ASCII, /*!< ASCII transmission mode. */
MB_TCP /*!< TCP mode. */
} eMBMode;
```

#### modbus协议函数

| 函数名          | 功能                                                         |
| :-------------- | ------------------------------------------------------------ |
| eMBInit()       | 主要实现modbus协议栈的初始化，这⾥主要初始化MODBUS-RTU和MODBUS-ASCII，不包括MODBUS-TCP；该函数的接收参数为modbus的⼯作模式、从机地址、端⼝号、波特率、奇偶校验设置。⼴播地址为：0xFF   函数进来之后⾸先检查设置的地址合法性，如果设置的地址为⼴播地址或者不在最⼩地址和最⼤地址范围之内，则返回故障 |
| eMBTCPInit()    | 主要完成MODBUS-TCP的初始化  `eMBTCPInit()` 函数和 `eMBInit()` 函数类似，⼀个是初始化RTU和ASCII协议，⼀个是初始化TCP协议。这⾥ eMBTCPInit() |
| eMBRegisterCB() | 函数初始化注册新的功能码和相应的处理函数到功能码数组中 当传⼊的函数指针为NULL的时候，注销功能码和它的处理函数 |
| eMBClose()      | 关闭modbus协议栈  该函数主要是关闭串⼝传输                   |
| eMBEnable()     | TCP协议栈使能modbus协议栈 该函数主要是使能UART的接收中断和开启定时器 接收中断⽤来接收主栈发送过来的数据，定时器⽤来进⾏超时检测 |
| eMBDisable()    | 禁⽌modbus协议栈 关闭UART接收中断和发送中断，关闭定时器      |
| eMBPoll()       | modbus轮询函数，主要完成事件的查询和相关处理函数的调⽤ ⾸先从接收到的数据中获取到功能码，然后查找功能码表（上⾯说到的xFuncHandlers数组），然后调⽤相应功能码的处理函数进⾏数据处理。数据处理完之后，判断是否需要发送返回帧，如果不是⼴播地址就需要返回，如果错误，返回的功能码最⾼位置1，没有错误，则调⽤发送函数，将返回帧发送出去 |

### RTU代码

| 文件名称 | 说明                                                       |
| -------- | ---------------------------------------------------------- |
| mbcrc.c  | 这个⽂件只包含⼀个函数，就是标准的CRC16校验函数            |
| mbcrc.h  | 包含CRC校验函数的函数声明                                  |
| mbrtu.c  | 实现RTU协议的具体函数，rtu协议相关的实现函数都在这个⽂件中 |
| mbrtu.h  | 头⽂件，包含rtu函数的声明                                  |

#### 数据类型

| 接收器状态     | 说明           |
| -------------- | -------------- |
| STATE_RX_INIT  | 接收器已初始化 |
| STATE_RX_IDLE  | 接收器空闲     |
| STATE_RX_RCV   | 接收器正在接收 |
| STATE_RX_ERROR | 接收器错误     |

| 发送器状态    | 说明       |
| ------------- | ---------- |
| STATE_TX_IDLE | 发送器空闲 |
| STATE_TX_XMIT | 正在发送   |

#### 超时时间

##### 根据eMBRTUInit()函数我们可以看出，FreeModbus将波特率⼤于19200的超时时间固定为1750us，其他的按照3.5个字符的传送时间来设置。这⾥我们定时器配置为每50us中断⼀次，因此我们只需要计算不同波特率下的定时器的中断次数即可。
- baudrate>19200时 此种情况下次数T=1750/50=35；所以我们从代码中可以看到波特率⼤于19200的时候，次数固定为35。
- baudrate≤19200时我们知道串⼝⼀般发送的格式为⼀个起始位、8或者9位数据位、⼀位停⽌位、⼀般⽆校验或者⼀位校验位。加起来⼀帧⼤概有11个⼆进制位（不同的配置有所差别，这⾥取11个⽐较合适）。所以传送⼀个字符的时间就是11/baudrate（单位：s），传送3.5个字符的时间就是（7/2）（11/baudrate）（单位：s），由于我们定时器50us中断⼀次，1s=20000个50us，所以对应的50us的次数就是(7/2) (11/baudrate)20000=(7220000)/(2*baudrate);

```c
//port.c


#include "mb.h"
#include "mbport.h"
#include <stdio.h>
#include "modbus.h"

// 十路线圈   可读可写的 位控制
#define REG_COILS_SIZE 10
uint8_t REG_COILS_BUF[REG_COILS_SIZE] = {1, 1, 1, 1, 0, 0, 0, 0, 1, 1};
// 低位在前， 高位再后
// 1, 1, 1, 1, 0, 0, 0, 0, | 1, 1
// 0  1  2  3  4  5  6  7  | 8  9 
// 0f                      | 03 
// 对应的十六进制数为： 0x0f 0x03

// 十路离散量 只读的线圈  只读的位控制 
#define REG_DISC_SIZE  10
uint8_t REG_DISC_BUF[REG_DISC_SIZE] = {1,1,1,1,0,0,0,0,1,1};

// 十路保持寄存器  可读可写的2字节寄存器
#define REG_HOLD_SIZE   10
uint16_t REG_HOLD_BUF[REG_HOLD_SIZE];


// 十路输入寄存器  只读保持寄存器 只读的2字节寄存器 
#define REG_INPUT_SIZE  10
uint16_t REG_INPUT_BUF[REG_INPUT_SIZE];


// CMD4命令处理回调函数 
//0x04: 读输入寄存器
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    USHORT usRegIndex = usAddress - 1;

    // 非法检测
    if((usRegIndex + usNRegs) > REG_INPUT_SIZE)
    {
        return MB_ENOREG;
    }

    // 循环读取
    while( usNRegs > 0 )
    {
        *pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] >> 8 );
        *pucRegBuffer++ = ( unsigned char )( REG_INPUT_BUF[usRegIndex] & 0xFF );
        usRegIndex++;
        usNRegs--;
    }

    // 模拟输入寄存器被改变
    for(usRegIndex = 0; usRegIndex < REG_INPUT_SIZE; usRegIndex++)
    {
        REG_INPUT_BUF[usRegIndex]++;
    }

    return MB_ENOERR;
}

// CMD6、3、16命令处理回调函数 
//0x03: 读保持寄存器 0x06: 写单个保持寄存器 0x0f: 写多个线圈寄存器
eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    USHORT usRegIndex = usAddress - 1;

    // 非法检测
    if((usRegIndex + usNRegs) > REG_HOLD_SIZE)
    {
        return MB_ENOREG;
    }

    // 写寄存器
    if(eMode == MB_REG_WRITE)
    {
        while( usNRegs > 0 )
        {
            REG_HOLD_BUF[usRegIndex] = (pucRegBuffer[0] << 8) | pucRegBuffer[1];
            pucRegBuffer += 2;
            usRegIndex++;
            usNRegs--;
        }
			//printf("usAddress=%d\n",usAddress);
			// 修改从机地址 REG_HOLD_BUF[9] 的值就是新的从机地址
			if(usAddress == 10 ) 
			{
				Modify_SlaveAdress_Flag = 1; //  会触发1次修改从机地址
			}
    }

    // 读寄存器
    else
    {
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( REG_HOLD_BUF[usRegIndex] & 0xFF );
            usRegIndex++;
            usNRegs--;
        }
    }

    return MB_ENOERR;
}

// CMD1、5、15命令处理回调函数
//0x01: 读线圈寄存器 0x05: 写单个线圈寄存器 
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    USHORT usRegIndex   = usAddress - 1;
    UCHAR  ucBits       = 0;
    UCHAR  ucState      = 0;
    UCHAR  ucLoops      = 0;

    // 非法检测
    if((usRegIndex + usNCoils) > REG_COILS_SIZE)
    {
        return MB_ENOREG;
    }

    if(eMode == MB_REG_WRITE)
    {
        ucLoops = (usNCoils - 1) / 8 + 1;
        while(ucLoops != 0)
        {
            ucState = *pucRegBuffer++;
            ucBits  = 0;
            while(usNCoils != 0 && ucBits < 8)
            {
                REG_COILS_BUF[usRegIndex++] = (ucState >> ucBits) & 0X01;
                usNCoils--;
                ucBits++;
            }
            ucLoops--;
        }
    }
    else
    {
        ucLoops = (usNCoils - 1) / 8 + 1;
        while(ucLoops != 0)
        {
            ucState = 0;
            ucBits  = 0;
            while(usNCoils != 0 && ucBits < 8)
            {
                if(REG_COILS_BUF[usRegIndex])
                {
                    ucState |= (1 << ucBits);
                }
                usNCoils--;
                usRegIndex++;
                ucBits++;
            }
            *pucRegBuffer++ = ucState;
            ucLoops--;
        }
    }

    return MB_ENOERR;
}

// CMD2命令处理回调函数
//0x02: 读离散输入寄存器
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    USHORT usRegIndex   = usAddress - 1;
    UCHAR  ucBits       = 0;
    UCHAR  ucState      = 0;
    UCHAR  ucLoops      = 0;

    // 非法检测
    if((usRegIndex + usNDiscrete) > REG_DISC_SIZE)
    {
        return MB_ENOREG;
    }

    ucLoops = (usNDiscrete - 1) / 8 + 1;
    while(ucLoops != 0)
    {
        ucState = 0;
        ucBits  = 0;
        while(usNDiscrete != 0 && ucBits < 8)
        {
            if(REG_DISC_BUF[usRegIndex])
            {
                ucState |= (1 << ucBits);
            }
            usNDiscrete--;
            usRegIndex++;
            ucBits++;
        }
        *pucRegBuffer++ = ucState;
        ucLoops--;
    }

    // 模拟离散量输入被改变
    for(usRegIndex = 0; usRegIndex < REG_DISC_SIZE; usRegIndex++)
    {
        REG_DISC_BUF[usRegIndex] = !REG_DISC_BUF[usRegIndex];
    }

    return MB_ENOERR;
}
```

```c
//porttimer.c

/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "tim.h"
#include <stdio.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvTIMERExpiredISR( void );

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim3.Instance = TIM3;
	// 72M/ (7199+1) = 10KHz 
	// 定时器工作一次的时间为 100us 
	// 72M/ (3599+1) = 20KHz 
	// 定时器工作一次的时间为 50us 
		
    htim3.Init.Prescaler = 3599;								  // 50us 记一次数
	//  htim3.Init.Prescaler = 7199;								// 100us记一次数
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = usTim1Timerout50us - 1;		// usTim1Timerout50us * 50即为定时器溢出时间
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        return FALSE;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        return FALSE;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        return FALSE;
    }

	__HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);// 先清除一下定时器的中断标记,防止使能中断后直接触发中断
   __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);// 使能定时器更新中断
    return TRUE;
}

inline void vMBPortTimersEnable(  )
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);		// 清空计数器
    __HAL_TIM_ENABLE(&htim3);					// 使能定时器
}

inline void vMBPortTimersDisable(  )
{
    __HAL_TIM_DISABLE(&htim3);					// 禁能定时器
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    ( void )pxMBPortCBTimerExpired(  );
}

// 定时器3中断服务程序
void TIM3_IRQHandler(void)
{
    if(__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE))			// 更新中断标记被置位
    {
        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_UPDATE);		// 清除中断标记
        prvvTIMERExpiredISR();								// 通知modbus3.5个字符等待时间到
    }
}

```

```c
//portserial.c

/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#include "port.h"
#include "usart.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    if(xRxEnable)
    {
		// 如果使用了485控制芯片,那么在此处将485设置为接收模式
		// 低电平  默认为接收模式	
		// 查阅资料得知，进入发送完成中断以后，还要延时2-4ms等待串口数据完全发送出去才使能485接收，不然会导致最后一个字节发送不完
		// 延时多久需要调整 9600波特率 5000 - 8000 , 9000后通信失效, 取中间值6500
		// 延时多久需要调整 115200波特率 600 - 900, 1000后通信失效 , 取中间值750
		//for( __IO uint32_t i=0;i<600;i++);
		HAL_GPIO_WritePin(RS485_WR_GPIO_Port, RS485_WR_Pin, GPIO_PIN_RESET);
			
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);		// 使能接收非空中断 
    }
    else
    {
        __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);		// 禁能接收非空中断
    }

    if(xTxEnable)
    {
		// 如果使用了485控制芯片,那么在此处将485设置为发送模式
		//  RS485_WR ENABLE 高电平  发送模式
		HAL_GPIO_WritePin(RS485_WR_GPIO_Port, RS485_WR_Pin, GPIO_PIN_SET);
		
		__HAL_UART_ENABLE_IT(&huart2, UART_IT_TXE);			// 使能发送为空中断
    }
    else
    {
        __HAL_UART_DISABLE_IT(&huart2, UART_IT_TXE);		// 禁能发送为空中断
    }
}

BOOL xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = ulBaudRate;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    switch(eParity)
    {
    // 奇校验
    case MB_PAR_ODD:
        huart2.Init.Parity = UART_PARITY_ODD;
        huart2.Init.WordLength = UART_WORDLENGTH_9B;			// 带奇偶校验数据位为9bits
        break;

    // 偶校验
    case MB_PAR_EVEN:
        huart2.Init.Parity = UART_PARITY_EVEN;
        huart2.Init.WordLength = UART_WORDLENGTH_9B;			// 带奇偶校验数据位为9bits
        break;

    // 无校验
    default:
        huart2.Init.Parity = UART_PARITY_NONE;
        huart2.Init.WordLength = UART_WORDLENGTH_8B;			// 无奇偶校验数据位为8bits
        break;
    }
    return HAL_UART_Init(&huart2) == HAL_OK ? TRUE : FALSE;
}

BOOL xMBPortSerialPutByte( CHAR ucByte )
{
    USART2->DR = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = (USART2->DR & (uint16_t)0x00FF);
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}

void USART2_IRQHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE))			// 接收非空中断标记被置位
    {
        __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_RXNE);		// 清除中断标记
        prvvUARTRxISR();										// 通知modbus有数据到达
    }

    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) && 
       __HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC))	// 发送为空中断标记被置位
    {
        __HAL_UART_CLEAR_FLAG(&huart2, UART_FLAG_TXE);		// 清除中断标记
        prvvUARTTxReadyISR();									// 通知modbus数据可以发送
    }
}

```

### 设计自己的Modbus通信协议

1. 地址
- 从机地址：0x01(默认值)
- ⼴播地址：0x00

2. 写单个保持寄存器指令
| 功能码 | 设备名称 | 设备寄存器地址 | 控制的位    | 位说明       |
| ------ | -------- | -------------- | ----------- | ------------ |
| 06     | LED1     | 0x0000         | 0b0000 0001 | 为1开，为0关 |
| 06     | LED2     | 0x0000         | 0b0000 0010 | 为1开，为0关 |
| 06     | BEEP     | 0x0000         | 0b0000 0100 | 为1开，为0关 |
| 06     | RELAY    | 0x0000         | 0b0000 1000 | 为1开，为0关 |

  ```c
  // 发送指令
  01 06 00 00 00 07 C8 08
  // 接收指令
  01 06 00 00 00 07 C8 08
  ```

3. 读单个保持寄存器指令  

   | 功能码 | 设备名称     | 保持寄存器地址 | 字节数  |
   | ------ | ------------ | -------------- | ------- |
   | 03     | 温度tp       | 0x0001         | 2个字节 |
   | 03     | 湿度hm       | 0x0002         | 2个字节 |
   | 03     | 电压vl       | 0x0003         | 2个字节 |
   | 03     | 电流cu       | 0x0004         | 2个字节 |
   | 03     | 功率pw       | 0x0005         | 2个字节 |
   | 03     | 电位器电压vr | 0x0006         | 2个字节 |
   | 03     | cpu温度cpu   | 0x0007         | 2个字节 |

   ```c
   //读温度
   // 发送数据
   01 03 00 01 00 01 D5 CA
   // 接收数据
   //tp:39.14℃ 0xF4A == 3914  
   01 03 02 0A F4 BF 63
   ```

4. 读多个保持寄存器指令  

   ```c
   // 读7个寄存器的值
   01 03 00 01 00 07 55 C8
   // 返回值
   01 03 0E 0A F8 17 83 00 D6 01 D3 03 ED 01 49 0E 39 05 0C
   ```

5. 修改从机地址的⽅法  

- 使⽤保持寄存器的最后地址作为从机的新地址，默认Freemodbus的⼴播地址为0  

  | 功能码 | 设备名称   | 保持寄存器地址 | 位说明     |
  | ------ | ---------- | -------------- | ---------- |
  | 06     | 从机新地址 | 0x0009         | 从机新地址 |

  ```c
  // 发送指令 ， 设置从机地址位5
  01 06 00 09 00 05 99 CB
  // 返回值
  01 06 00 09 00 05 99 CB
  ```

  
