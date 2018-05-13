/**********************************************************************************
   STM32F103C8T6  	 
 * 硬件连接说明
	 使用单片串口2与GPRS模块通信  注：使用串口2可以避免下载和通信不会冲突
	 STM32      GPRS模块
	 打板子接线方法:               mini板子接线方法:
     PA3 (RXD2)->U_RX               PA3 (RXD2)->U_TX
	 PA2 (TXD2)->U_TX               PA2 (TXD2)->U_RX
	 GND	   ->GND                GND	   ->GND
	 
	 PA9(TXD1)--->调试信息端口
	 PA10(RXD1)-->调试信息端口
设计：
(1)LED0-7设置的引脚为:PB4-7 PB12-15
(2)KEY0-3设置的引脚为:PA4-7
  
**********************************************************************************/
#include "stm32f10x.h"
#include "usart.h"
#include "Led.h"
#include "SysTick.h"
#include "timer.h"
#include "string.h"
#include "key.h"
#include "GA6_module_errors.h"
#include <stdio.h>
#include "DHT11.h"
//#include "time_delay.h"

#define Buf2_Max 	  70 //串口2缓存长度
#define Buf1_Max 	  70 //串口2缓存长度
#define STABLE_TIMES  10  //等待系统上电后的稳定

/*************	本地常量声明	**************/

/*************  本地变量声明	**************/
char Uart2_Buf[Buf2_Max];//串口2接收缓存
char Uart1_Buf[Buf1_Max];//串口1接收缓存
char data_from_uart1[20];
u8 First_Int = 0;
u8 UART1_First_Int = 0;
char error_result[20];
char GetTemp[4];
char Gethumi[4];

static char  *phone_num = "AT+CMGS=\"13067802058\""; //修改这里可以修改电话号码

/*************	本地函数声明	**************/
void CLR_Buf(void);     //清除串口2接收缓存
void CLR_UART1_Buffer(void);  //清除串口1接收缓存 
u8 Wait_CREG(u8 query_times);    //等待模块注册成功
u8 Find(char *a);
void Show_Dth11(void);
u8 UART1_Send_QUERY_Command(u8 query_times);
//void CLR_UART1_Buf(void);

/*************  外部函数和变量声明*****************/


int check_status(void)
{
	int ret;
	
	ret = UART2_Send_AT_Command("AT","OK",3,50);//测试通信是否成功
	if(ret == 0)
	{
		return COMMUNITE_ERROR;
	}
	
	ret = UART2_Send_AT_Command("AT+CPIN?","READY",2,50);//查询卡是否插上
	if(ret == 0)
	{
		return NO_SIM_CARD_ERROR;
	}
	
	ret = Wait_CREG(3);//查询卡是否注册到网络
	if(ret == 0)
	{
		return SIM_CARD_NO_REG_ERROR;
	}
	return 1;
}

/***************************************************************
注：当然你可以返回其他值，来确定到底是哪一步发送指令出现失败了。
****************************************************************/
int send_text_message(char *content)
{
	u8 ret;
	char end_char[2];
	
	end_char[0] = 0x1A;//结束字符
	end_char[1] = '\0';
	
	//设置存储位置
	ret = UART2_Send_AT_Command("AT+CPMS=\"SM\",\"ME\",\"SM\"","OK",3,100);
	if(ret == 0)
	{
		return AT_CPMS_ERROR;
	}
	
	ret = UART2_Send_AT_Command("AT+CMGF=1","OK",3,50);//配置为TEXT模式
	if(ret == 0)
	{
		return AT_CMGF_ERROR;
	}
	
	ret = UART2_Send_AT_Command("AT+CSCS=\"GSM\"","OK",3,50);//设置字符格式
	if(ret == 0)
	{
		return AT_CSCS_ERROR;
	}
	
	ret = UART2_Send_AT_Command(phone_num,">",3,50);//输入收信人的电话号码
	if(ret == 0)
	{
		return AT_CMGS_ERROR;
	}
	
	UART2_SendString(content);
	ret = UART2_Send_AT_Command_End(end_char,"OK",1,250);//发送结束符，等待返回ok,等待5S发一次，因为短信成功发送的状态时间比较长
	if(ret == 0)
	{
		return END_CHAR_ERROR;
	}
	
	return 1;
}

/*******************************************************************************
* 函数名 : main 
* 描述   : 主函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 串口2负责与GA6模块通信，串口1用于串口调试，
*******************************************************************************/
int main(void)
{
//	u16 key_value;
	u8  i;
	u8  run_led_flag = 0;
	int  ret;
	u8 buffer[5];
	char str_hum[30];
	char str_temp[10];
	double hum;
  double temp;
	SysTick_Init_Config();   //系统滴答时钟初始化
	GPIO_Config();           //GPIO初始化
	Key_GPIO_Config();
//	USART2_Init_Config(115200);  //串口2初始化
	USART2_Init_Config(9600);  //串口2初始化
	Timer2_Init_Config();        //定时器2初始化
//	delay_init();
	USART1_Init_Config(9600);//UART1用作串口调试信息
	
	UART1_SendString("系统启动.......................\r\n");
	
	for(i = 0;i < STABLE_TIMES;i++)
	{
		delay_ms(50);
	}
	
	memset(error_result,'\0',20);
	
	while(1)
	{ 		
//		UART1_SendString("系统启动1.......................\r\n");
		if (dht11_read_data(buffer) == 0)
		{
				hum = buffer[0] + buffer[1] / 10.0;
				temp = buffer[2] + buffer[3] / 10.0;
		}
		
//		str_hum = (char)hum;
//		sprintf(str_hum, "%6f%6f", hum, temp);
//		sprintf(str_temp, "%f", temp);
		
//		UART1_SendString(str_hum);
//		UART1_SendString("\r\n");
		
		UART1_Send_QUERY_Command(2);
		
		sprintf(str_hum, "%6f%6f%s", hum, temp, data_from_uart1);
//		sprintf(str_hum, "%s", data_from_uart1);
		
		if(run_led_flag == 0)
		{
			LED1_ON();
//			UART1_SendString("LED0_ON\r\n");
			run_led_flag = 1;
		}
		else
		{
			LED1_OFF();
//			UART1_SendString("LED0_OFF\r\n");
			run_led_flag = 0;
		}
//		key_value = Key_Down_Scan();
//		switch (key_value)
//		{
//			case (0x0001 << 5):
//			{
				ret = check_status();
				if(ret == 1)
				{
//					ret = send_text_message(str);//发送TEXT短信
					ret = send_text_message(str_hum);
				}
				if(ret == 1)
				{
//					sprintf(error_result,"成功发送TEXT短信\r\n");
					UART1_SendString(error_result);
					LED2_ON();
				}
				else
				{
					sprintf(error_result,"错误代码 : %d\r\n",ret);
					UART1_SendString(error_result);
					LED2_OFF();
					
				}
								
//				break;
//			}
//			
//			default: 
//				break;
			
//		}
//			CLR_UART1_Buffer();
			delay_ms(500);
	}
}

/*******************************************************************************
* 函数名  : USART2_IRQHandler
* 描述    : 串口1中断服务程序
* 输入    : 无
* 返回    : 无 
* 说明    : 
*******************************************************************************/
void USART2_IRQHandler(void)                	
{
	u8 Res=0;
	Res = USART_ReceiveData(USART2);
	Uart2_Buf[First_Int] = Res;  	  //将接收到的字符串存到缓存中
	First_Int++;                	  //缓存指针向后移动
	if(First_Int > Buf2_Max)       	  //如果缓存满,将缓存指针指向缓存的首地址
	{
		First_Int = 0;
	}
} 	

/*******************************************************************************
* 函数名  : TIM2_IRQHandler
* 描述    : 定时器2中断断服务函数
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void TIM2_IRQHandler(void)   //TIM2中断
{
	static u8 flag =1;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM2更新中断发生与否
	{
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIM2更新中断标志 
	
		if(flag)
		{
			//LED4_ON(); 
			flag=0;
		}
		else
		{
			//LED4_OFF(); 
			flag=1;
		}
	}	
}

/*******************************************************************************
* 函数名 : CLR_Buf
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //将缓存内容清零
	{
		Uart2_Buf[k] = 0x00;
	}
    First_Int = 0;              //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Wait_CREG
* 描述   : 等待模块注册成功
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
u8 Wait_CREG(u8 query_times)
{
	u8 i;
	u8 k;
	u8 j;
	i = 0;
	CLR_Buf();
	while(i == 0)        			
	{

		UART2_Send_Command("AT+CREG?");
		delay_ms(100); 
		
		for(k=0;k<Buf2_Max;k++)      			
		{
			if((Uart2_Buf[k] == '+')&&(Uart2_Buf[k+1] == 'C')&&(Uart2_Buf[k+2] == 'R')&&(Uart2_Buf[k+3] == 'E')&&(Uart2_Buf[k+4] == 'G')&&(Uart2_Buf[k+5] == ':'))
			{
					 
				if((Uart2_Buf[k+7] == '1')&&((Uart2_Buf[k+9] == '1')||(Uart2_Buf[k+9] == '5')))
				{
					i = 1;
					return 1;
				}
				
			}
		}
		j++;
		if(j > query_times)
		{
			return 0;
		}
		
	}
	return 0;
}

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(char *a)
{ 
	if(strstr(Uart2_Buf, a)!=NULL)
	{
		return 1;
	}	
	else
	{
		return 0;
	}
		
}

void USART1_IRQHandler(void)                	
{
	u8 Res=0;
	Res = USART_ReceiveData(USART1);
	Uart1_Buf[UART1_First_Int] = Res;	
//	Uart2_Buf[First_Int] = Res;  	  //将接收到的字符串存到缓存中
	UART1_First_Int++;                	  //缓存指针向后移动
	if(UART1_First_Int > Buf1_Max)       	  //如果缓存满,将缓存指针指向缓存的首地址
	{
		UART1_First_Int = 0;
	}
} 

/*******************************************************************************
* 函数名 : CLR_UART1_Buf
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_UART1_Buf(void)
{
	u16 k;
	for(k=0;k<Buf1_Max;k++)      //将缓存内容清零
	{
		Uart1_Buf[k] = 0x00;
	}
    UART1_First_Int = 0;              //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 UART1_Find(char *a)
{ 
	if(strstr(Uart1_Buf, a)!=NULL)
	{
		return 1;
	}	
	else
	{
		return 0;
	}
		
}


//请求浊度传感器数据
u8 UART1_Send_QUERY_Command(u8 query_times)         
{
	u8 i;
	u8 k;
	u8 j;
	i = 0;
	CLR_UART1_Buffer();
//	while(i == 0)        			
//	{

		UART1_SendString("AT+V\r\n");
		delay_ms(200); 
//		strcpy(data_from_uart1, Uart1_Buf);
		for(k=0; k<20; k++){
			data_from_uart1[k] = Uart1_Buf[k];
//			data_from_uart1[k] = Uart2_Buf[k];
		}
		
//		UART1_SendString("准备发送数据：");
//		UART1_SendString(Uart1_Buf);
//		for(k=0;k<Buf1_Max;k++)      			
//		{
//			
//			
//			if((Uart1_Buf[k] == '+')&&(Uart2_Buf[k+1] == 'C')&&(Uart2_Buf[k+2] == 'R')&&(Uart2_Buf[k+3] == 'E')&&(Uart2_Buf[k+4] == 'G')&&(Uart2_Buf[k+5] == ':'))
//			{
//					 
//				if((Uart2_Buf[k+7] == '1')&&((Uart2_Buf[k+9] == '1')||(Uart2_Buf[k+9] == '5')))
//				{
//					i = 1;
//					return 1;
//				}
//				
//			}
//		}
//		j++;
//		if(j > query_times)
//		{
//			return 0;
//		}
		
//	}
	return 0;
}

/*******************************************************************************
* 函数名 : CLR_Buf
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_UART1_Buffer(void)
{
	u16 k;
	for(k=0;k<Buf1_Max;k++)      //将缓存内容清零
	{
		Uart1_Buf[k] = 0x00;
	}
    UART1_First_Int = 0;              //接收字符串的起始存储位置
}

