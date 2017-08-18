#include <Servo.h>

Servo s1;




void setup() 
{
  s1.attach(9);           // pino 9 para o servo!
  
  pinMode(13,OUTPUT);
  pinMode(2,INPUT);
}

void loop() 
{
  int mov=0,angle=0,potpin=2,push=0,pos=2;
  
  mov=analogRead(potpin);
  angle=mov/6;
  angle=angle+10; 
  if(angle >=0 and angle<=170)
     {
  s1.write(angle);
   delay(20);
   
     } 
     pos=digitalRead(pos); 
     if(pos==LOW)
       {
        digitalWrite(13,LOW);
        s1.write(90);
        delay(2000);
       }
       else
       {
      
       if(angle>95)
         {
          digitalWrite(13,HIGH);
         } 
         if(angle<90)
           {
            digitalWrite(13,LOW);
            }
       }         
}                   
