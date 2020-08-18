
// original copy : https://www.amobbs.com/forum.php?mod=viewthread&tid=4560171
/*
 * original file saved as GBK, converts to UTF8 CHT
 * tools: https://github.com/flier268/ConvertZZ
 * xiaolaba, 2020-AUG-18
 * 
 * port to avr-gcc, u8 -> uint8_t, u16 -> uint16_t
 * port to ATmeag8, oroginal ATmega16
 * 
 */



//#include "iom16v.h"

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#include "lcd.h"



//void LCD_Write_Str(unsigned char *str);
//uint16_t frame[4];

unsigned char Turn = 0, COM = 0;

/*-------------------------------------時序圖-------------------------------------------
        0 1  2 3  4 5  6 7  8 9  A B  C D  E F
             _                                       _
COM0        | |   _    _    _    _    _    _        | |
       _____| |__| |__| |__| |__| |__| |__| |_______| |__
                       _
COM1    _    _        | |   _    _    _    _    _    _
       | |__| |_______| |__| |__| |__| |__| |__| |__| |__
                                 _
COM2    _    _    _    _        | |   _    _    _    _
       | |__| |__| |__| |_______| |__| |__| |__| |__| |__
                                           _
COM3    _    _    _    _    _    _        | |   _    _
       | |__| |__| |__| |__| |__| |_______| |__| |__| |__
--------------------------------------------------------------------------------------*/



//#pragma interrupt_handler Timer0_Comp:20  // 捕獲中斷
//void Timer0_Comp(void)

//https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
ISR(TIMER0_COMP_vect, ISR_NAKED)  // avr-gcc


{
	// COMx為0或1，時序圖中的0、2、4、6......E
	TIFR &= ~0x02;

	if(Turn++ == 0)  // COMx為0，時序圖中的0、4、8、C
	{
		// COMx為0，需要顯示的SEG（COMx）為1
		SEG_LOAD(frame[COM]);

		// COMx為0，其它COM為VDD/2
		// 所有COM IO設定為懸浮輸入
		COM_ALL_HALF();

		// COMx IO設為推輓輸出低
		if(COM == 0)      {COM0_LOW();}
		else if(COM == 1) {COM1_LOW();}
		else if(COM == 2) {COM2_LOW();}
		else if(COM == 3) {COM3_LOW();}
	}
	else  // COMx為1，時序圖中的2、6、A、E
	{
		Turn = 0;

		// COMx為1，需要顯示的SEG（COMx）為0
		SEG_LOAD(~frame[COM]);

		// COMx為1，其它COM為VDD/2
		// 所有COM IO設定為懸浮輸入
		COM_ALL_HALF();

		// COMx IO設為推輓輸出高
		if(COM == 0)      {COM0_HIGH();}
		else if(COM == 1) {COM1_HIGH();}
		else if(COM == 2) {COM2_HIGH();}
		else if(COM == 3) {COM3_HIGH();}

		if(++COM > 3) COM = 0;
	}
}

//#pragma interrupt_handler Timer0_OVF:10   // 溢出中斷
//void Timer0_OVF(void)

//https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
ISR (TIMER0_OVF_vect, ISR_NAKED)    // avr-gcc


{
	// 時序圖中的1，3，5，7......F，COM和SEG都為0
	TIFR &= ~0x01;

	// COM和SEG都為0
	COM_SEG_ALL_LOW();
}
/*------------------------------------------------------------------------------------*/

/*
void Delayms(unsigned int ms)
{
	unsigned int i;

	for (; ms>0; ms--)
		for (i=1000; i>0; i--);
}
*/

void LCDShow_Init(void)  // Timer0快速PWM初始化
{
/*
	DDRA |= 0x0f;    // COM0~3
	DDRC |= 0x0f;      // SEG8~11
	DDRD |= 0x0f;      // SEG0~7
*/
	COM_DDR |= 0x0f; // COM0~3
	SEG_DDRC |= 0xff;  // SEG8~11
	SEG_DDRD |= 0xff;  // SEG0~7

	TCCR0 = 0x4A;   // 配置定時器工作在快速PWM模式，8分頻
	OCR0 = 0x3f;    // 設定捕捉比較初值，0x3f
	TIMSK |= 0x03;  // 開捕獲中斷，溢出中斷
	SREG |= 0x80;   // 開總中斷
}

int main(void)
{
//	Delayms(10);
	_delay_ms(10);

	LCDShow_Init();

	LCD_Write_Str((unsigned char*) "012345");
	LCD_Show_Dot(0);
	while(1);
    
    return 0;
}