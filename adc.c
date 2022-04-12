#include "lpc17xx.h"

volatile unsigned int val_ADC0=0;
volatile unsigned int val_ADC1=0;
volatile unsigned int val_ADC2=0;

void IT_ADC(void) __irq
{LPC_ADC->ADCR &= ~(1<<16); //arret de la conversion
	//remarque on revient ici avec ADC0 en overrun
	//car le convertisseur a relancé la conversion sur ADC0 avant qu'on le stoppe
	val_ADC0= (AD0DR0>>6)&0x000003FF; //recuperation et effacement des bits DONE et OVERRUN
	val_ADC1= (AD0DR1>>6)&0x000003FF;
	val_ADC2= (AD0DR2>>6)&0x000003FF;
 // AD0STAT |= 1<<16 ; // acquittement de l'IT inutile car la lecture du troisieme efface le bit d'IT 
	VICVectAddr =0; 
}	
void IT_TIMER0(void)__irq
{
FIO2PIN ^=0x01;
	// ici on pourrait lire AD0DR0 pour effacer le DONE0
LPC_ADC->ADCR|=1<<16;//lancement d'un burst de conversion
LPC_TIM0->T0IR=0x0000003F;
VICVectAddr =0; 
}

void init_timer0(void){
		 LPC_TIM0->MR0=11999;	//valeur de match reg pour provoquer le reset compteur
		 LPC_TIM0->MCR = 0x00000003; // reset et it	  sur match0 rien sur le reste
		 LPC_TIM0->T0IR = 0x00000001; //acquittement d'un it parasite precedente
		 VICVectAddr4= (unsigned)IT_TIMER0;
		 VICIntEnable= (1<<4);//on aura une IT toutes les ms
		 LPC_TIM0->TCR=0x00000001; //Timer0 en mode Timer

}/*
void init_uart0(void)
{  LPC_SC->PCONP |= (1<<3)| (1<<4) ; // mise sous tension des peripheriques uart0 et uart1
    PINSEL0 |= 0x00000050; // pattes P0.2 et P0.3 pour le module UART
    PINMODE0 &= 0xFFFFFF0F; // on impose les pull UP sur les deux pattes

	U0LCR = 0x93 ;// DLAB /BREAK  PARITY forcee à 1  /PARITYENABLE  1bitSTOP  8bits
	 // la valeur est de  78.125 = 12000/16/9.6  
	 	 U0DLM = 0; U0DLL = 78;//	 ainsi baudrate = 12000/16/(0*256+78) = 9.615Kbauds
	   U0FDR = 0x10; 
 	 // ou 	plus precis a tester : 12000/16/9.6/(1+D/M) qui amene 	  71.0227 plus precis si D=1 et M=10
	 //U0DLM = 0; U0DLL = 71;
	 //U0FDR = 0xA1;	// M D 

	 U0LCR = 0x13 ;	   // remettre DLAB = 0 pour acceder aux registres d'envoi et de réception 

	 U0ACR =0x00;  // auto baud control désactivé
	 //U0ICR = 0x00;  // pas de liaison par infrarouge en IRDA (n existe pas pour U0)
   U0TER = 0x80; // autorise la transmission (on n'utilisera pas de controle de flux)
	 U0FCR = 0x07; // seuil 1ch  RST_TXfifo RST_RXfifo ENFIFO
	 U0IER = 0x00; // aucune interruption autorisée
	/********************************************************************************/	
	// dans le main pour gérer la liaison série en réception et en envoi :
	 //on lira les caractères recus dans U0RBR et on écrira dans U0THR
	 // les flag importants sont dans U0LSR

   // le Bit 0  pour savoir si on a reçu un caractère : on l'utilise ainsi
	 // status_rs232 = U0LSR;
	 // if(status_rs232 & 1)
	 // {// on a recu un caractère
	 //  if(status_rs232 & 0x0E) {/*il y a une erreur overrun ou parité ou bit stop, on est PERDU */}
	 //  else {ch_recu=U0RBR; /* on va gérer */} 

   // le bit5 pour savoir si on peut transmettre un caractère 
	 // savoir qu'on peut en déposer pendant qu'un autre est en cours de transmission
	 // le bit6 pour savoir si tous les caractères sont finis de transmettre (registre à décalage vide)
   //if(status_rs232 & 1<<5) { U0THR= ch_a_envoyer;}
  /***********************************************************************************/
//}

void init_adc(void)//lire le chapitre 27 de la doc
{LPC_SC->PCONP|=1<<12; //mettre l'adc sous tension...
 LPC_ADC->ADCR |= 1<<21 ;  //sortir du Power Down
	//  on ne touche pas à l'horloge de l'ADC qui est de 12MHz 
	// (sinon voir le LPC2300.S, configuration wizard,CLOCK SETUP, PCLKSEL0  PLCK_ADC bit 25 24)
	// mettre les pattes souhaitées en AD0.0 AD0.1 AD0.2   par exemple...
	//           P0.23       P0.24      P0.25  
  //Vref sur la carte est fixé à 3V3 par le hardware 	
	LPC_PINCON->PINSEL1 |= (0x01<<14)|(0x01<<16)|(0x01<<18); //3 entrées analogiques 
	LPC_ADC->PINMODE1 |= (0x02<<14)|(0x02<<16)|(0x02<<18); //3 entrées sans pull up ni pull down 
	LPC_ADC->ADCR|=2<<8; // 12MHz divisé par 2+1 = 3 pour avoir une horloge <4.5 MHz
	LPC_ADC->AD0INTEN= 0X04; // AD0.2 source de l'IT de fin de conversion...(dernière convertie parmi les 3)
	LPC_ADC->ADCR|=0x7; // on lancera 3 conversions AD0.0 AD0.1 AD0.2
	//on on veut une IT en fin de conversion
	// sinon encore plus simple,ne pas mettre d'IT pour arreter les conversions 
	// qui se font en boucle infinie par le convertisseur
	// et on peut lire les dernières valeurs converties dans les registres de résultat de conversion
		 VICVectAddr18= (unsigned)IT_ADC;
		 VICIntEnable= (1<<18);//on aura une IT à chaque fin de conversion de AD0.2
	 // par contre, une conversion sur AD0.0 se relancera avant qu'on ait le temps d'arreter...
}		
void init_proc()
{SCS|=1; // pour avoir PORT0 et PORT1 en mode FASTIO ...sinon FIO0PIN ne fonctionnera pas
FIO2DIR = 3;
init_adc();
init_timer0();
}
int main(void)
{		 
init_proc();
	while(1)
	{ 	 
	  FIO2SET=0x02;  
    FIO2CLR=0x02;  
	}	
}
