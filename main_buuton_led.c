#include "lpc17xx.h"
#include "button.h"
#include "ocf_lpc176x_lib.h"
#include <stdio.h>
uint8_t state;


int main(void)
{
	initUART0();
	
	LPC_GPIO0 ->FIODIR &= ~(1<<9); //set P0.9 as input
	delay_ms(200);
	while(1)
	{
		
		if(button_state()==0 )
		{
			
			led_on();
			delay_ms(20);
		}
		else 
		{
			led_off();
			delay_ms(20);
		}
	}
	return 0;
}

