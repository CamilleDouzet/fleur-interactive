#include "LPC17xx.h"
#define FSGNL 12000000


// on a un seul fichier son sur une seule période qu'il va falloir lire plus vite et plus lentement
unsigned int son1K[37]={511,599,685,766,839,902,953,991,1014,1022,1014,991,953,902,
	839,766,685,599,511,422,336,255,182,119,68,30,7,0,7,30,68,119,182,255,336,422,511}; 
unsigned char Flag_depassement = 0; // Milieu buffer
unsigned char Flag_fin = 0; // Flag fin de buffer
// Faire la meme chose avec un buffer 	
	// tableau contenant un  sinus à 1kHz échantillonné à 36KHz.

// frequence DTMF des 16 sons DTMF, frequence basse....permet de lire le 1KHZ plus lentement
//unsigned int tableau_DTMFB[16]={0.941*256,0.697*256,0.697*256,0.697*256,0.770*256,0.770*256,0.770*256,0.852*256,0.852*256,
	//	0.852*256,0.941*256,0.941*256,0.697*256,0.770*256,0.852*256,0.941*256 } ;  			
// frequence DTMF des 16 sons DTMF, frequence haute....permet de lire le 1KHZ plus lentement
//unsigned int tableau_DTMFH[16]={1.336*256,1.209*256,1.336*256,1.477*256,1.209*256,1.336*256,1.447*256,1.209*256,
	//	1.336*256,1.447*256,1.209*256,1.447*256,1.633*256,1.633*256,1.633*256,1.633*256}; // tableau pas_y

	


unsigned int seq_freq_haute[12];

unsigned int coef = 3000;
// unsigned int dcp_100ms=3600		 ; // 36 echantillons de 100ms = 3600 
unsigned int index_son;  
unsigned int inc_freqH;
unsigned int index_a_virguleH		 ;

unsigned int virgule            ;

unsigned int ind;
		
unsigned int ech_avant;
unsigned int ech_apres;
unsigned int val_son =511  ;

unsigned char poste=12; // CHOIX DU POSTE APPELANT

void maj_dacr()
{ 
	LPC_DAC->DACR=(val_son>>3)&(0xFFC0);//car le calcul est multiplié par 256, et doublé...donc décalé de 9 bits
	// il faut le redécaler de 3 bits... voir ci dessous le calcul...
	// chaque son est joué seulement 100 ms soit 3600 échantillons à 36 KHz	
	//dcp_100ms--;
	//if(dcp_100ms==0)
	//{
		//dcp_100ms=3600;
		inc_freqH=seq_freq_haute[index_son];
		if(index_son<11) {
			index_son++;
		}  /* pour boucler automatiquement*/ // else {index_son=0;}
	//}
									
	index_a_virguleH+=inc_freqH;
	if(index_a_virguleH>=36*256){
		index_a_virguleH-=36*256;
	}

	virgule=index_a_virguleH&255;
	ind=index_a_virguleH>>8;
	ech_avant=son1K[ind];
	ech_apres=son1K[ind+1];
	
	
	val_son=(ech_avant*(256-virgule)+ech_apres*virgule)  ;

	ech_avant=son1K[ind];
	ech_apres=son1K[ind+1]			 ;
  val_son+=(ech_avant*(256-virgule)+ech_apres*virgule) ;
	
									
} 

void PWM1_IRQHandler()
{
    maj_dacr();
		LPC_PWM1->IR=0xFF     ; //acquittement
}
		
	void init_pwm () 
	{
//		PINSEL4=0;
//		FIO2DIR=0x02;
		LPC_PINCON->PINSEL4|=0x1; // PWM1.1
		LPC_PWM1->TCR=0x003;//blocage en reset			
		LPC_PWM1->PCR=0x200; // activation de PWM1.1
		LPC_PWM1->MR0=693; // 25 000 000 / 36 000 =
		LPC_PWM1->MR1=  (LPC_PWM1->MR0)>>2;
		LPC_PWM1->MCR=0x003; // RAZ du timer si TC=MR0      + flag de PWM1IR généré

		LPC_PWM1->LER=0x03; // Prise en compte des PWMs
 // NVIC_SetPriority (PWM1_IRQn, 31);
  NVIC_EnableIRQ(PWM1_IRQn)  ;
		LPC_PWM1->TCR=0x009;	

	}
void init_dac()
{

	 // attribution de la patte P0.26 à la sortie DAC
		LPC_PINCON->PINSEL1|=0x02<<20;
		LPC_PINCON->PINMODE1=0x02<<20;

LPC_DAC->DACR=0xFF80>>1; // BIAS 0, VALUE=1 du bit 6 a 15 // masque pou recuperer les bonnes valeurs, DACR initialise a 1.65Volt 
		
}

void init_trame_DTMF( unsigned char num_poste ) {
	// on recoit le numéro de poste DU  de 0 à 99, il faut le transmettre  par la séquence * D U #
	// creation d'une séquence de son, on va stocker dans les tableaux seq_freq_haute et seq_freq_basse
	// les vitesses de lecture des sons
	// on va jouer le son du symbole *  (200 ms donc 2 cases de suite)
	// on jouer un blanc  (100 ms donc 1 case)
	// on va jouer le son dtmf du chiffre de la dizaine  (200 ms donc 2 cases de suite)
	// on jouer un blanc  (100 ms donc 1 case)
	// on va jouer le son dtmf du chiffre  de l unite  (200 ms donc 2 cases de suite)
	// on jouer un blanc  (100 ms donc 1 case)
	// on va jouer le son dtmf du symbole #  (200 ms donc 2 cases de suite)
	// on va jouer un blanc  (100 ms donc 1 case)
	unsigned char dizaine=num_poste/10;
	unsigned char unite  =num_poste%10;
/*	unsigned int DTMF_Dizaine_H=tableau_DTMFH[dizaine];
	unsigned int DTMF_Dizaine_B=tableau_DTMFB[dizaine];
	unsigned int DTMF_Unite_H=tableau_DTMFH  [unite];
	unsigned int DTMF_Unite_B=tableau_DTMFB  [unite];
	unsigned int DTMF_ETOILEH=tableau_DTMFH[10];
	unsigned int DTMF_ETOILEB=tableau_DTMFB[10];
	unsigned int DTMF_DIESEH=tableau_DTMFH [11];
	unsigned int DTMF_DIESEB=tableau_DTMFB [11];*/
	
/*seq_freq_haute[ 0]=DTMF_ETOILEH;seq_freq_basse[ 0]=DTMF_ETOILEB;
seq_freq_haute[ 1]=DTMF_ETOILEH;seq_freq_basse[ 1]=DTMF_ETOILEB;
seq_freq_haute[ 2]=0					 ;seq_freq_basse[ 2]=0					 ;
seq_freq_haute[ 3]=			DTMF_Dizaine_H;seq_freq_basse[ 3]=			DTMF_Dizaine_B;
seq_freq_haute[ 4]=			DTMF_Dizaine_H;seq_freq_basse[ 4]=			DTMF_Dizaine_B;
seq_freq_haute[ 5]=0					 ;seq_freq_basse[ 5]=0		  		 ;
seq_freq_haute[ 6]=			DTMF_Unite_H;seq_freq_basse[ 6]=			DTMF_Unite_B;
seq_freq_haute[ 7]=			DTMF_Unite_H;seq_freq_basse[ 7]=			DTMF_Unite_B;
seq_freq_haute[ 8]=0					 ;seq_freq_basse[ 8]=0					 ;
seq_freq_haute[ 9]=DTMF_DIESEH ;seq_freq_basse[ 9]=DTMF_DIESEB ;
seq_freq_haute[10]=DTMF_DIESEH ;seq_freq_basse[10]=DTMF_DIESEB ;
seq_freq_haute[11]=0					 ;seq_freq_basse[11]=0					 ;*/
}


int main (void)
{
	inc_freqH=0;
	//inc_freqB=0;
	index_son=0;
  index_a_virguleH=0;
  //index_a_virguleB=0;
	//init_trame_DTMF(poste);
	init_pwm();
	init_dac();
	while(1)
	{
	}
	
}
	
