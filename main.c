#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 1000000
#define delay_on 600
#define preheat 0
bit DRL_ON, tmp, brl, motr = 0 , brtns;
void init(void)
{
	OSCCON = 0x5A;//12
	OSCTUNE = 0x00;
	BORCON = 0x00;
	WDTCON = 0x14;
	LATA = 0x00;
	TRISA = 0x1B;
	ANSELA = 0x13;
	WPUA = 0x00;
	OPTION_REGbits.nWPUEN = 1;
	APFCON = 0x00;
	ADCON1 = 0xF0;
	ADCON0 = 0x01;
	OPTION_REG = (unsigned char)((OPTION_REG & 0xC0) | (0xD6 & 0x3F));
	TMR0 = 0xD8;
	INTCONbits.TMR0IE = 1;
	PR2 = 0xFF;
	T2CON = 0x06;
	CCP1CON = 0x2C;
	ECCP1AS = 0x00;
	PWM1CON = 0x80;
	PSTR1CON = 0x01;
	CCPR1H = 0x00;
	CCPR1L = 0x4C;
	FVRCONbits.TSRNG  = 1;
	FVRCONbits.TSEN = 1;
  
	

}

void EPWM_LoadDutyValue(unsigned int dutyValue)
{
   if (dutyValue == 0) 
	{
		CCP1CONbits.CCP1M = 0;
		return;
	}
	else
	{
		CCP1CONbits.CCP1M = 0x0C;
	}
	// Writing to 8 MSBs of pwm duty cycle in CCPRL register
    CCPR1L = ((dutyValue & 0x03FC)>>2);
    
   // Writing to 2 LSBs of pwm duty cycle in CCPCON register
    CCP1CON = ((unsigned char)(CCP1CON & 0xCF) | ((dutyValue & 0x0003)<<4));
}
unsigned int GET_ADC(unsigned char channel)
{
	__delay_us(100);
	ADCON0bits.CHS = channel;    
	ADCON0bits.ADON = 1;
	__delay_us(50);
	ADCON0bits.GO_nDONE = 1;
	while (ADCON0bits.GO_nDONE)
	return ((unsigned int)((ADRESH << 8) + ADRESL));
}
void DRL(char motor, char trtn)
{
	static int cntr;
	if (motor==1 && cntr<delay_on) cntr++;
	if (motor==0) cntr=0;
	if (cntr>=delay_on && trtn==0) DRL_ON=1; else DRL_ON=0;
	if (cntr>=delay_on) LATA5=1; else LATA5=0;
}
void main(void)
{
	static int pwm, volt, temp, sens;
	init();
	EPWM_LoadDutyValue(preheat);
	GET_ADC(0x01); //AN0 Была проблема что первый опрос АЦП давал результат 28013
	__delay_ms(100);
	while(1)
	{
		volt = 0;
		sens = 0;
		temp = 0;
		for (char n=0; n<4;n++)
		{
			volt = volt + GET_ADC(0x01); //AN0
			sens = sens + GET_ADC(0x03); //AN1
			temp = temp + GET_ADC(0x00); //AN3		
		}		
		volt = volt/4;
		sens = sens/4;
		temp = temp/4;
		CLRWDT();
		if (volt > 540) motr=1;//445
		if (volt < 450) motr=0;//410
		
		if (sens > 400) brl =1;
		if (sens < 300) brl =0;
		
		if (temp < 132) tmp=1;
		if (temp > 140) tmp=0;
		__delay_us(1000);
		DRL(motr,brl);
		if (temp < 80)
		{
			CCP1CON = 0;
		}
		if (DRL_ON==1)
		{
			if (tmp==1 && pwm>0xAA)
			{
				pwm--; EPWM_LoadDutyValue(pwm);
			}
			else if (pwm<0x132)
			{
				pwm++; EPWM_LoadDutyValue(pwm);
			}
//			pwm++; EPWM_LoadDutyValue(pwm);
		} 
		//else DRL_ON=0;
		if (DRL_ON==0 && pwm>preheat)
		{
			pwm--; EPWM_LoadDutyValue(pwm);
		} 
		//else DRL_ON=1;
		CLRWDT();
	}
}
void TMR0_ISR(void)
{
    static volatile int CountCallBack = 0;

    // Clear the TMR0 interrupt flag
    INTCONbits.TMR0IF = 0;

    TMR0 = 0xD8;

    // callback function - called every 39th pass
    if (++CountCallBack >= 39)
    {
        // ticker function call


        // reset ticker counter
        CountCallBack = 0;
    }

    // add your TMR0 interrupt custom code
}
void interrupt isr(void)
{
    // interrupt handler
    if(INTCONbits.TMR0IE == 1 && INTCONbits.TMR0IF == 1)
    {
        TMR0_ISR();
    }
    else
    {
        //Unhandled Interrupt
    }
}