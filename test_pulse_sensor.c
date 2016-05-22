#define 	MAIN_Fosc		11059200L	

#include	"STC15Fxxxx.H"

#include	"stdio.h"
#include	"string.h"

#define Baud_Rate 	9600

#define	Timer0_Reload	(65536UL -(MAIN_Fosc / Baud_Rate))		

#define DIS_DOT		0x20
#define DIS_BLACK	0x10
#define DIS_		0x11

#define MAX_MSG 	16

u8 bit_pulse;

u8 timer_10ms;

sbit state = P0^0;
sbit data_o = P0^1;
sbit data_i = P0^2;

sbit err_led = P1^0;	//debug
sbit rsv_led = P1^1;	//debug
sbit send_led = P1^2;	//debug

void init(){
	P0M1 = 0;	P0M0 = 0;	
	P1M1 = 0;	P1M0 = 0;	
	P2M1 = 0;	P2M0 = 0;	
	P3M1 = 0;	P3M0 = 0;	
	P4M1 = 0;	P4M0 = 0;	
	P5M1 = 0;	P5M0 = 0;	
	P6M1 = 0;	P6M0 = 0;	
	P7M1 = 0;	P7M0 = 0;	
	
	AUXR = 0x80;	
	TH0 = (u8)(Timer0_Reload / 256);
	TL0 = (u8)(Timer0_Reload % 256);
	ET0 = 1;	//Timer0 interrupt enable
	TR0 = 1;	//Tiner0 run
	EA = 1;		
	
	P1ASF = 0x10;		
	ADC_CONTR = 0xE0;	//90T, ADC power on	
	
	bit_pulse = 0;
	
	timer_10ms = 0;
	
	state = 0;
	data_o = 1;
	data_i = 1;
	
	err_led = 0;	//debug
	rsv_led = 0;	//debug
	send_led = 0;	//debug
}

void error()
{
	err_led = 1;	//debug
}

void timer0 (void) interrupt TIMER0_VECTOR
{
	bit_pulse = 1;		
	if (++timer_10ms == Baud_Rate / 100) timer_10ms = 0;
}

u8 bt_rsv(char msg[MAX_MSG])
{
	u8 i,j;
	u8 len,stop;
	
	rsv_led = 1;	//debug
	
	len = 0;
	bit_pulse = 0;
	stop = 0;
	
	while (1)
	{
		if (data_i == 0)
		{
			i = 0;
			j = 0;
			while (1)
			{
				if (bit_pulse)
				{
					bit_pulse = 0;
					switch (j)
					{
						case 0: 
							if (data_i || i == MAX_MSG) stop = 1;
							else msg[i] = 0;
							break; 
						case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:
							if (data_i) msg[i] += 1 << (j - 1);
							break;
						case 9:
							if (!data_i) error();
							else
							{
								i++;
								len++;
							}
					}
					if (++j == 10) j = 0;
				}
				if (stop) break;
			}	
		}
		if (stop) break;
	}
	
	rsv_led = 0;	//debug
	
	return (len);
}


void bt_send(char msg[MAX_MSG])
{
	u8 i,j;
	u8 len = strlen(msg);
	
	send_led = 1;	//debug
	
	bit_pulse = 0;
	for (i = 0;i < len;i++)
	{
		j = 0;
		while (1)
		{
			if (bit_pulse)
			{
				bit_pulse = 0;
				switch (j)
				{
					case 0:
						data_o = 0;
						break;
					case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:
						data_o = (msg[i] >> (j - 1)) % 2;
						break;
					case 9:
						data_o = 1;
						break;
				}
				j++;
				if (j == 10) break;
			}
		}
	}
	
	send_led = 0;	//debug
	
}

void clear(char msg[MAX_MSG])
{
	u8 i;
	for (i = 0;i < MAX_MSG;i++)
	{
		msg[i] = 0;
	}
}

void delay_1s()
{
	u8 cnt_1s = 0;
	while (1)
	{
		if (!timer_10ms) cnt_1s++;
		if (cnt_1s == 100) break;
	}
}

void main()
{
	u8 i;
	char msg[MAX_MSG];

	init();
	clear(msg);
	
	while (1)
	{
		bt_rsv(msg);
		bt_send(msg);
		clear(msg);
	}
	//while(1)
	//{
		//if (!state) continue;
		/*for (i = 0;i < 10;i++)
		{
			sprintf(msg,"%d",i);
			bt_send(msg);
			delay_1s();
		}*/
	//}
}
