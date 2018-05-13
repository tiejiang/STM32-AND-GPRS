#include "DHT11.h"
#include "SysTick.h"

void dht11_gpio_input(void)
{
    GPIO_InitTypeDef g;
    
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE);
    
    g.GPIO_Pin = DHT11_GPIO_PIN;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    
    GPIO_Init(DHT11_GPIO_TYPE, &g);
}

void dht11_gpio_output(void)
{
    GPIO_InitTypeDef g;
    
    RCC_APB2PeriphClockCmd(DHT11_RCC, ENABLE);
    
    g.GPIO_Pin = DHT11_GPIO_PIN;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Mode = GPIO_Mode_Out_PP; //????

    GPIO_Init(DHT11_GPIO_TYPE, &g);
}
void DHT11_delay_us(u32 n)
{
    while(--n);
}
void dht11_reset(void)
{
    // ??DHT11????
    dht11_gpio_output();
    DHT11_OUT_L;
//    delay_us(19000);
		delay_ms(20);
    DHT11_OUT_H;
//    delay_us(30);
	DHT11_delay_us(60);
    dht11_gpio_input();
}

u16 dht11_scan(void)
{
    return DHT11_IN;
}

u16 dht11_read_bit(void)
{
    while (DHT11_IN == RESET);
    delay_us(40);
    if (DHT11_IN == SET)
    {
        while (DHT11_IN == SET);
        return 1;
    }
    else
    {
        return 0;
    }
}

u16 dht11_read_byte(void)
{
    u16 i;
    u16 data = 0;
    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= dht11_read_bit();
    }
    return data;
}

u16 dht11_read_data(u8 buffer[5])
{
    u16 i = 0;
    u8 checksum;
    dht11_reset();
    if (dht11_scan() == RESET)
    {
        while (dht11_scan() == RESET);
        while (dht11_scan() == SET);
        for (i = 0; i < 5; i++)
        {
            buffer[i] = dht11_read_byte();
        }
        
        while (dht11_scan() == RESET);
        dht11_gpio_output();
        DHT11_OUT_H;
        
        checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
        if (checksum != buffer[4])
        {
            // checksum error
            return 1;
        }
    }
    
    return 0;
}
