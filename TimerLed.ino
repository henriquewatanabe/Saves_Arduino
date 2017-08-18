/*
 * LedTest.cpp
 *
 * Created: 11/07/2017 20:29:05
 * Author : Henrique
 */ 

#include<avr/io.h>
void setup()
{
  Serial.begin(9600);  
}
void timer0_init()
{
  TCCR0A |= (1 << CS00);
  TCNT0 = 0;
}
bool Status = false;
void loop()
{
Serial.println("ligado");  
}
int main(void)
{
  DDRD = 0b10000000;
  timer0_init();
  Serial.println("ligado");
  while(1)
  {
    if(TCNT0 >= 1000)
    {
      if(Status == false)
      {
        PORTD = 0b10000000;
        Status = true;
        TCNT0 = 0;
        Serial.println("ligado");
      }
      else
      {
        PORTD = 0b00000000;
        Status = false;
        TCNT0 = 0;
        Serial.println("Desligado");
      }
    }
  }
}

