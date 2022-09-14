/*
 * COD_FINAL.c
 *
 * Created: 01-Jun-20 10:34:56 PM
 * Author : Iulia
 */ 
#include <avr/io.h> //header to enable data flow control over pins
#include <avr/interrupt.h>
//#include <stdio.h>
#include <avr/sfr_defs.h>
#define F_CPU 8000000UL //frequency of the internal clock
//#define BAUD 9600
//#define MYUBRR ((F_CPU/16/BAUD)-1)
#include <util/delay.h> //header to enable delay function in program
//#include <stdlib.h>

//global variables
unsigned int current_floor=1;
unsigned int desired_floor;
static volatile int pulse = 0; //stores the count value from the TCNT1 register
static volatile int i = 0; //indicates the current status of the echo pin

int main(void)
{
	///initialization; set direction for input/output on pins
	
    DDRB &=(~(1<<PINB0)); //button on port B pin 0 is an input (0)
    DDRB &=(~(1<<PINB1)); //button on port B pin 1 is an input (0)
    DDRB &=(~(1<<PINB2)); //button on port B pin 2 is an input (0)
    DDRD &=(~(1<<PIND5)); //button on port D pin 5 is an input (0)
	
    //setting motor as output
    DDRA |=(1<<PINA0);
    DDRA |=(1<<PINA1);
    DDRA |=(1<<PINA2);
    DDRA |=(1<<PINA3);
   
    DDRD |= (1<<PIND1);//set LED as output (1)
    DDRD |= (1<<PIND3);//set LED as output (1)
    DDRD |= (1<<PIND4);//set LED as output (1)
	DDRD |= (1<<PIND6);//set LED as output (1)
	
    ///my buttons are not pressed yet
    PORTB |= (1<<PINB0); //set to a high reading; 5V assigned to pin 0 on port B 
    PORTB |= (1<<PINB1); //set to a high reading; 5V assigned to pin 1 on port B
    PORTB |= (1<<PINB2); //set to a high reading; 5V assigned to pin 2 on port B
	PORTD |= (1<<PIND5); //setting the pull up resistor, as known as 5V assigned to pin5 portB
	
	//initializing sensor HCSR04
	int16_t count_a=0;
	DDRD |=1<<PIND0; //set trigger as output (1)
	DDRD &=(~(1<<PIND2)); //set echo as input (0)
	_delay_ms(50);
	
	//setting interrupt registers
	EIMSK |=1<<INT0; //the external pin interrupt is enabled
	EICRA |=1<<ISC00;//any logical change on INT0 generates an interrupt request
	EIFR |=1<<INTF0; //external Interrupt Flag 0 set
	MCUCR |=1<<ISC00;//setting interrupt triggering logic change
	
	sei(); // enabling global interrupts
    while (1) 
    {
			//checking if the pinD5 has changed its state into 0 (pressed button; 0V assigend due to ground terminal)
			if(bit_is_clear(PIND,5))
			{
				PORTD |= (1<<PIND6); //if pressed, we light up the led on pinD6
			}
	//// NOW THE MICROCONTROLLER IS READY
			//we light up the led corresponding to our current state of the elevator's floor
			if(current_floor==1) {PORTD |= (1<<PIND1);} //1st floor 
			if(current_floor==2) {PORTD |= (1<<PIND3);} //2nd floor
			if(current_floor==3) {PORTD |= (1<<PIND4);} //3rd floor
		
			//choosing destination
			if(bit_is_clear(PINB,0)) desired_floor=1;
			if(bit_is_clear(PINB,1)) desired_floor=2;
			if(bit_is_clear(PINB,2)) desired_floor=3;
			
			PORTD |=1<<PIND0; //setting trigger to a high reading, 5V assigned
			_delay_us(15);
			PORTD &=(~(1<<PIND0)); //setting trigger to a low reading, 0 V, so we have just sent a pulse of 15 us	
			count_a=pulse/460; // our measured distance in cm
			//1st floor is at 13cm far from our sensor
			//2nd floor is at 23cm far from our sensor
			//3rd floor is at 33cm far from our sensor
		
			if(desired_floor==2 && current_floor==1)
			{
				//elevator up; starting our 1st motor 
				PORTA |=(1<<PINA0); // set high reading, 5V
				PORTA &=~(1<<PINA1); //set low reading, 0V
				if(count_a==23) //if we reached 2nd floor we stop the motor
				{
					PORTA |=(1<<PINA0);
					PORTA |=(1<<PINA1);
					current_floor=2;
					PORTD |= (1<<PIND3); //current_floor is the 2st floor, so the blue led is lit
					PORTD &=~ (1<<PIND1); //no other leds are lit
					PORTD &=~ (1<<PIND4);
				}
			}
		
			if(desired_floor==3 && current_floor==1)
			{
				//elevator up; starting our 1st motor 
				PORTA |=(1<<PINA0);
				PORTA &=~(1<<PINA1);
				if(count_a==33) //if we reached 3rd floor we stop the motor
				{
					PORTA |=(1<<PINA0);
					PORTA |=(1<<PINA1);
					current_floor=3;
					PORTD |= (1<<PIND4); //current_floor is the 3st floor, so the led is lit
					PORTD &=~ (1<<PIND1);
					PORTD &=~ (1<<PIND3);
				}
			}
		
			if(desired_floor==1 && current_floor==2)
			{
				//elevator down; starting our 2nd motor 
				PORTA |=(1<<PINA3);
				PORTA &=~(1<<PINA2);
				if(count_a==13)//if we reached 1st floor we stop the motor
				{
					PORTA |=(1<<PINA2);
					PORTA |=(1<<PINA3);
					current_floor=1;
					PORTD |= (1<<PIND1); //current_floor is the 1st floor, so the led is lit
					PORTD &=~ (1<<PIND3);
					PORTD &=~ (1<<PIND4);

				}
			}
			if(desired_floor==3 && current_floor==2)
			{
				//elevator up; starting our 1st motor
				PORTA |=(1<<PINA0);
				PORTA &=~(1<<PINA1);
				if(count_a==33)//if we reached 3rd floor we stop the motor
				{
					PORTA |=(1<<PINA0);
					PORTA |=(1<<PINA1);
					current_floor=3;
					PORTD |= (1<<PIND4); //current_floor is the 3st floor, so the led is lit
					PORTD &=~ (1<<PIND3);
					PORTD &=~ (1<<PIND1);
				}
			}
			if(desired_floor==2 && current_floor==3)
			{
				//elevator down; starting our 2nd motor
				PORTA |=(1<<PINA3);
				PORTA &=~(1<<PINA2);
				if(count_a==23)//if we reached 2nd floor we stop the motor
				{
					PORTA |=(1<<PINA2);
					PORTA |=(1<<PINA3);
					current_floor=2;
					PORTD |= (1<<PIND3); //current_floor is the 2st floor, so the led is lit
					PORTD &=~ (1<<PIND1);
					PORTD &=~ (1<<PIND4);
				}
			}
			if(desired_floor==1 && current_floor==3)
			{
				//elevator down; starting our 2nd motor
				PORTA |=(1<<PINA3);
				PORTA &=~(1<<PINA2);
			
				if(count_a==13)//if we reached 1st floor we stop the motor
				{
					PORTA |=(1<<PINA2);
					PORTA |=(1<<PINA3);
					current_floor=1;
					PORTD |= (1<<PIND1); //current_floor is the 1st floor, so the led is lit
					PORTD &=~ (1<<PIND3);
					PORTD &=~ (1<<PIND4);
			}
		}
		
		
    }
}

	ISR(INT0_vect) //interrupt service routine when there is a change in logic level of echo
	{
		if(i==0) //we have received an interrupt and execute the following //echo goes from 0 to 1
		{
			TCCR1B|=1<<CS10; //timer counter control register;STARTS THE COUNTER OF THE uC
			//no prescaling so that means a counting with the ticking of the clock
			//the internal counter has started
			i=1;	
		}
		else //echo goes from 1 to 0 and that means the end of the transmitted pulse
		{
			TCCR1B=0;//timer/counter stopped
			pulse=TCNT1; //takes the stored value of the counter
			TCNT1=0; //resetting the counter memory
			i=0;
		}
	}

