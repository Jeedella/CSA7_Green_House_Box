#include <p32xxxx.h>
#include "green_house.h"
#include <plib.h>
#include <stdio.h>
#include <stdlib.h>
#define  LED2     LATBbits.LATB11
#define  LED3     LATBbits.LATB12
#define  LED4     LATBbits.LATB13
#define  LED1     LATBbits.LATB10
#define Soil_sensor PORTDbits.RD12
#define pump        PORTFbits.RF13     
#define fan         PORTFbits.RF5       

  int Sensor_present;
  int Sensor_previous;


float ch2 , ch3 ;
double DAC ();
void ADC_init();
double ADC_skan( int c) ;



int de( unsigned t)        // half second delay
{
    T2CONSET = 0x8070;     // prescaller 256
    while(t--)   // in our case 1
    {
      TMR2 = 0;
      PR2 = 15624;     // 0.5 second, sine 2 Hz = 2 samples in second
     while (TMR2 < PR2){}  // wait
      
     
    }
    T2CONCLR = 0x8070; // clear the timer
}
//----------------------------------------------------

//----------------------------------------------------
double DAC (int j) // Convet ADC value to voltage
{
float q = (3.3 * j) / 1023 ;
return q;
}
//----------------------------------------------------
double ADC_scan( int c)
{
 AD1CHSbits.CH0SA = c; // selecting the input
 AD1CON1bits.SAMP = 1; // Begin sampling
 de(1);
 while( AD1CON1bits.SAMP ); // wait until acquisition is done
 while( ! AD1CON1bits.DONE ); // wait until conversion done
 return DAC(ADC1BUF0); // result stored in ADC1BUF0
}
//-------------------------------------------------
void ADC_init()
{
AD1PCFG = 0xFFF3; // PORTB = Digital; RB2 = analog in
TRISB = 0x000C; // AN2 and AN3 - our input pins
AD1CON1 = 0x00E0; // automatic convertation after sampling
AD1CON2 = 0x0000; // no input scan
AD1CON3 = 0x0002; // ADC clk period = 6 PB clk periods = 750 ns
AD1CON1SET = 0x8000; // turn ADC ON, defaults to software sampling
}

void LED_init()
{
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    LED4 = 0;
}

void Temperature_control(float t1) // 1.23 V at 20°, 1.06 V at 36.6°, so 10mV/C°
{                                  // The plants optimal temperature 24°, so 1.21 V treshold U
    if (t1 <= 1.21) // 1.19
    {
       LED4 = 1;  // for test
       fan = 1;
    }
    else
    {
       LED4 = 0;
       fan = 0;
    }
}


void Soil_sensor_test ()
{
    if (Soil_sensor == 1)
    {
        LED1 = 1;
    }
    else
    {
        LED1 = 0;
    }
}

int CheckSensor ()      // button 1
{

   if(Sensor_previous < Sensor_present)  // detects rising edge
    {
      Sensor_previous = Sensor_present;  // assigns 1 to previous state, so it returns 1 only once.Keeping the value until
      return 1; // returns 1                                                                     button is reliased.
    }
   else
    {
      return 0;
    }
}

void Pump_water ()
{
    unsigned t = 4;            // 0.5 * 10 = 5 s  // 4 = 2 s
    T3CONSET = 0x8070;        // set the timer and load prescaler 256
    while(t--)              // how many s
    {
      TMR3 = 0;             // set timer to 0
      PR3 = 15624;;            // 0.5 s        (8000000 * 0.5)/256 = 15625 -  1 = 15624
     while (TMR3 < PR3)
     {
        LED3 = 1;  // test LED
        pump = 1;
     }   // pump 0.5 s
    }
    T3CONCLR = 0x8070;     // clear the timer

    LED3 = 0; // test LED
    pump = 0; // off pump

}

int main( void )
{
  TRISDSET = 0b0000000000000010;
  TRISBCLR = 0b11110000000000; // clears LED's

  TRISF = 0; // A port is output
  pump = 0;
  fan = 0;

 ADC_init();

 LED_init();
 CheckSensor ();

    while(1)
    {
    ch2 = ADC_scan(2);
    ch3 = ADC_scan(3); // not used
    Temperature_control(ch2);

  Soil_sensor_test ();

if (Soil_sensor)
    {
         Sensor_present = 1;
    }
else
    {
         Sensor_present = 0;
         Sensor_previous = 0;
    }

    if (CheckSensor())
    {

        Pump_water();

    }

    }
    return 1;
}

   