#include "lpc17xx.h"
#include <cstring> 
#include "button.h"


void delay_ms(unsigned int ms)
{
    volatile unsigned int i,j;
    for(i=0;i<ms;i++)
    for(j=0;j<6000;j++);
}

void led_on()
{
	LPC_GPIO2->FIODIR |=(1<<13); //set P2.13 pin as output
	LPC_GPIO2->FIOCLR       |=(1 << 13);      // LED defined as an output HIGH p2.13

}
void led_off()
{
	LPC_GPIO2->FIODIR |=(1<<13); //set P2.13 pin as output
	LPC_GPIO2->FIOSET		|=(1<<13);		// LED defined as an output LOW p2.13
}


int button_state()
{
extern	uint8_t state;
	if (LPC_GPIO0->FIOPIN & (1<<9))
	  {
		state = 0;
		}
		else
		{
		state = 1;
	  }
	return state;
}