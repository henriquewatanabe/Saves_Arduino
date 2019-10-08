/*
 * LedTest.cpp
 *
 * Created: 11/07/2017 20:29:05
 * Author : Henrique
 */ 

#include<avr/io.h>

int main(void)
{
  // editado com o GIT
  //Initialize port for LEDs
  DDRB= 0b00100000;
  PORTB = 0b00000000;
  TCCR1B |= 1<<CS10;
  uint16_t repeatCount = 0;
  while(1)
  {
    if (TCNT1 > 1000)
    {
      
      repeatCount++;
      TCNT1 = 0;
      if (repeatCount > 10000)
      {
        repeatCount = 0;
        PORTB ^= 1 << PINB5;
      }
    }
    // Count and turn an LED ON and OFF
  }
}
