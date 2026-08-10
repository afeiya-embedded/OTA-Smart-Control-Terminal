## OTA

### OTA（Over-The-Air）是⼀项通过⽆线⽹络（如Wi-Fi或蜂窝移动⽹络）远程更新设备软件、固件或配置的技术，它让设备升级像⼿机系统更新⼀样⽅便，⽆需连接电脑或前往服务中⼼

- SOTA：主要指应⽤软件的在线升级，⽐如更新⻋载导航地图、娱乐系统⾥的App，或者为⼿机安装新版本的应⽤。它通常不涉及底层系统核⼼功能的改变
- FOTA：指固件的在线升级，是更深层次、更彻底的更新。它可以改变操作系统的核⼼部分，从⽽优化⻋辆的动⼒表现、刹⻋距离，甚⾄开放⾃动驾驶等⾼级功能。特斯拉通过FOTA为⻋辆提升加速性能和续航⾥程就是典型的例⼦

### ⼀次完整的OTA升级通常包含三个步骤：
1. ⽣成更新包：⼚商制作包含新功能或修复补丁的软件包，常采⽤“差分升级”技术，只⽣成新旧版本间的差异部分，⼤⼤减⼩下载体积
2. 传输更新包：加密后的更新包被推送到云端服务器，设备在合适的时机（如连接Wi-Fi时）⾃动或经⽤⼾同意后下载。
3. 安装与验证：设备安装更新，并严格验证其完整性和正确性。成功后，新版本即开始运⾏。
安全是OTA的⽣命线。为确保升级过程可靠，会采⽤端到端加密（如AES-256）防⽌数据篡改，使⽤数字签名验证软件包来源的合法性，并设计回滚机制，以便在升级失败时能⾃动恢复⾄旧版本，保障设备基本功能不受影响

### BootLoader
BootLoader 可以理解成是引导程序, 它的作⽤是启动正式的App应⽤程序。换⾔之， BootLoader 是⼀个程序，App也是⼀个程序， BootLoader程序是⽤于启动App程序的。Bootloader是在应⽤程序开始前运⾏的⼀个⼩程序，⾥⾯可以进⾏⼀些初始化操作，升级引⽤程序等

### 总体流程图
1. 先执⾏ BootLoader 程序, 先去检查 APP2 区有没有程序, 如果有就将App2区(备份区)的程序拷⻉到 App1区 , 然后再跳转去执⾏ App1 的程序
2. 然后执⾏ App1 程序, 因为 BootLoader 和 App1 这两个程序的向量表不⼀样, 所以跳转到 App1 之后第⼀步是先去更改程序的向量表
3. 然后再去执⾏其他的应⽤程序 在应⽤程序⾥⾯会加⼊程序升级的部分, 这部分主要⼯作是拿到升级程序, 然后将他们放到 App2区(备份区) , 以便下次启动的时候通过BootLoader 更新 App1 的程序
4. 将 App2区 的最后4个字节( 0x0801FFFC )⽤来表⽰ App2区 是否有升级程序，STM32在擦除之后Flash的数据存放的都是 0xFFFFFFFF 

```c
#ifndef __BOOT_H_
#define __BOOT_H_

#define PageSize FLASH_PAGE_SIZE // 1K

/*=====用户配置(根据自己的分区进行配置)=====*/
// 0x400    1K 
// 0x800    2K 
// 0x1000   4K  
// 0x2000   8K  
// 0x7000   4K*7 = 28K   
#define BootLoader_Size         0x2000U         // BootLoader的大小 8K
#define Application_Size        0x7000U         // APP 应用程序的大小 28K

#define Application_1_Addr      0x08002000U     // 应用程序1的首地址 大小 28K 0x08002000 + 0x7000
#define Application_2_Addr      0x08009000U     // 应用程序2的首地址 大小 28K 0x08009000 + 0x7000
/*==========================================*/

/* 启动的步骤 */
#define Startup_Normol 0xFFFFFFFF   //  正常启动
#define Startup_Update 0xAAAAAAAA   //  升级再启动

void Start_BootLoader(void);

#endif

```

```c
#include "main.h"
#include <stdio.h>
#include "boot.h"

/**
 * @bieaf 擦除页
 *
 * @param pageaddr  起始地址	
 * @param num       擦除的页数
 * @return 1
 */
static int Erase_page(uint32_t pageaddr, uint32_t num)
{
	HAL_FLASH_Unlock();
	
	/* 擦除FLASH*/
	FLASH_EraseInitTypeDef FlashSet;
	FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashSet.PageAddress = pageaddr;
	FlashSet.NbPages = num;
	
	/*设置PageError，调用擦除函数*/
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&FlashSet, &PageError);
	
	HAL_FLASH_Lock();
	return 1;
}


/**
 * @bieaf 写若干个数据
 *
 * @param addr       写入的地址
 * @param buff       写入数据的起始地址
 * @param word_size  长度
 * @return 
 */
static void WriteFlash(uint32_t addr, uint32_t * buff, int word_size)
{	
	/* 1/4解锁FLASH*/
	HAL_FLASH_Unlock();
	
	for(int i = 0; i < word_size; i++)	
	{
		/* 3/4对FLASH烧写*/
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + 4 * i, buff[i]);	
	}

	/* 4/4锁住FLASH*/
	HAL_FLASH_Lock();
}



/**
 * @bieaf 读若干个数据
 *
 * @param addr       读数据的地址
 * @param buff       读出数据的数组指针
 * @param word_size  长度
 * @return 
 */
static void ReadFlash(uint32_t addr, uint32_t * buff, uint16_t word_size)
{
	for(int i =0; i < word_size; i++)
	{
		buff[i] = *(__IO uint32_t*)(addr + 4 * i);
	}
	return;
}


/* 读取启动模式 */
unsigned int Read_Start_Mode(void)
{
	unsigned int mode = 0;
	ReadFlash((Application_2_Addr + Application_Size - 4), &mode, 1);
	return mode;
}

/**
 * @bieaf 进行程序的覆盖
 * @detail 1.擦除目的地址
 *         2.源地址的代码拷贝到目的地址
 *         3.擦除源地址
 *
 * @param  搬运的源地址
 * @param  搬运的目的地址
 * @return 搬运的程序大小
 */
void MoveCode(uint32_t src_addr, uint32_t des_addr, uint32_t byte_size)
{
	/*1.擦除目的地址*/
	printf("> Start erase des flash......\r\n");
	Erase_page(des_addr, (byte_size/PageSize));
	printf("> Erase des flash down......\r\n");
	
	/*2.开始拷贝*/	
	unsigned int temp[256];
	
	printf("> Start copy......\r\n");
	for(int i = 0; i < byte_size/1024; i++)
	{
		ReadFlash((src_addr + i*1024), temp, 256);
		WriteFlash((des_addr + i*1024), temp, 256);
	}
	printf("> Copy down......\r\n");
	
	/*3.擦除源地址*/
	printf("> Start erase src flash......\r\n");
	Erase_page(src_addr, (byte_size/PageSize));
	printf("> Erase src flash down......\r\n");
}



/* 采用汇编设置栈的值 */
__asm void MSR_MSP (uint32_t ulAddr) 
{
    MSR MSP, r0 			                   //set Main Stack value
    BX r14
}



/* 程序跳转函数 */
typedef void (*Jump_Fun)(void);
void IAP_ExecuteApp (uint32_t App_Addr)
{
	Jump_Fun JumpToApp; 
    
	if ( ( ( * ( __IO uint32_t * ) App_Addr ) & 0x2FFE0000 ) == 0x20000000 )	//检查栈顶地址是否合法.
	{ 
		JumpToApp = (Jump_Fun) * ( __IO uint32_t *)(App_Addr + 4);	//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP( * ( __IO uint32_t * ) App_Addr );	//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		JumpToApp();		//跳转到APP.
	}
}



/**
 * @bieaf 进行BootLoader的启动
 *
 * @param none
 * @return none
 */
void Start_BootLoader(void)
{
	/*==========打印消息==========*/  
	 printf("\r\n");
	 printf("***********************************\r\n");
	 printf("*        BootLoader               *\r\n");
	 printf("***********************************\r\n");
	
	//printf("> Choose a startup method......\r\n");	
	switch(Read_Start_Mode())										//  读取是否启动应用程序 
	{
		case Startup_Normol:										//  正常启动 
		{
			printf("> Normal start...\n");
			break;
		}
		case Startup_Update:										// 升级再启动 
		{
			printf("> Start update...\n");		
			MoveCode(Application_2_Addr, Application_1_Addr, Application_Size);
			printf("> Update down......\r\n");
			break;
		}
		default:													//  启动失败
		{
			printf("> Error:%X!!!......\r\n", Read_Start_Mode());
			return;			
		}
	}
	
	/* 跳转到应用程序 */
	printf("> Start APP1...\r\n");	
	IAP_ExecuteApp(Application_1_Addr);
}

```
#### 设置向量表
```c
SCB->VTOR = FLASH_BASE | Application_1_Addr ; /* 更改中断向量表地址 偏移到APP1 地址 */
```

### 后续需要搭建物联⽹服务器 