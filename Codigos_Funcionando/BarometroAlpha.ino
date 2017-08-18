/*versão 0.3 
  modificado: sensor usando adafruit diferença de altura apenas pela dezena
*/
#include<Servo.h>
#include <Adafruit_BMP280.h>
Servo esc;      // servo
Adafruit_BMP280  barometro;   // barometro
void setup()
{
  Serial.begin(9600);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  // put your setup code here, to run once:
  esc.attach(4);  
  if (!barometro.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  } 
  else
    Serial.println("SensorFound!!");
    
}
/* função que pega o valor da pressao atmosférica
  Parâmetros: Entrada unsigned long *ptrValor - ponteiro que aponta para o valor
  Retorno: void - não retorna nada
*/
void ObterPressao(unsigned long *ptrValor)
{  
  *ptrValor = barometro.readPressure();   
}
/*
 função que faz o calculo do acelerador diante do receptor
 Parâmetros: Entrada unsigned int *ptrValorAcele - ponteiro que
 aponta para uma variável aceleracao
                     unsigned int *ptrAux ponteiro que aponta para o valor aux
 Retorno bool - se true possui valor se false não tem sinal no rc
 */
bool CalcAceleracao(unsigned int *ptrValorAcele, unsigned int *ptrAux)
{
  unsigned int nCanal = 0;      // para pegar o valor de entrada do receptor
  nCanal = pulseIn(2, HIGH);
  *ptrValorAcele = map(nCanal, 1024, 1830, 800, 2100);      
  *ptrAux = pulseIn(3, HIGH);
  if(*ptrValorAcele < 850 || nCanal == 0)
    return false;
  // ocorreu tudo ok
  return true;
}
// definições de variáveis globais
//------------------variáveis editáveis-----------------
unsigned int nCorMilisegundo = 75;          // tempo de milisegundo para efetuar a correção
//------------------------------------------------------
//--
//--
// -----------------variáveis não editáveis--------------
unsigned int nThrottle = 0,           // para o acelerador
             nThrottleControle = 0,   // para alterar valores na estabilização 
             nAux = 0,
             nUnidadeSub = 0;        // unidade para subtrair o valor oscilante do barometro
unsigned long lPressaoAnt = 0,       // para conter o valor da pressao anterior
              lPressaoAtu = 0,       // para conter a pressao atual 
              CorDelay = 0,          // delay de correção
              IncrementDelay = 0,    // delay de incrementação da aceleração 
              DecrementDelay = 0;    // tempo de deley de decrementação
bool LeituraDeAltura = true,         // para pegar o valor de altura   
     PrimeiraLeitura = true;
//--------------------------------------------------------  
void loop() {
  // put your main code here, to run repeatedly:
  if (PrimeiraLeitura) // se não tem sinal vamos para de emitir pulso
    {
     PrimeiraLeitura = false;
     esc.write(800);      
     return;            // retorna ao inicio do loop
    }
 
  if(nAux > 1300) // nAux é maior que 1300?
  { // maior significa que o usuário ativou o sistema    
    while(true)
    {
     CalcAceleracao(&nThrottle, &nAux);     
      if(nAux < 1300)
        break;          // cai fora do loop
      if(LeituraDeAltura) // é verdadeiro?
      { // vamos pegar o valor da altura atual
        ObterPressao(&lPressaoAnt);        // pegar a pressao
        nThrottleControle = nThrottle;  // guardar o valor da aceleração
         
        LeituraDeAltura = false;        // indicar que já está ativo dentro da correção
      }        
      // vamos ler a pressao para ver se houve variação       
      if(millis() - CorDelay >= nCorMilisegundo) // tempo maior?
      { 
        CorDelay = millis();          // para receber o tempo atual        
        ObterPressao(&lPressaoAtu);   // para pegar a pressao atual    
        nUnidadeSub = ((lPressaoAtu % 100) % 10); // tirar a unidade de diferença
        lPressaoAtu = lPressaoAtu - nUnidadeSub;  // subtrair a pressao atual com a unidade    
        if(lPressaoAtu == lPressaoAnt) // pressao atu = ant?
          esc.write(nThrottleControle);
              
        if(lPressaoAtu < (lPressaoAnt)) // subiu muito a altura?
        {
          if(nThrottleControle > 900)
            nThrottleControle--;           
        }
        if(lPressaoAtu > lPressaoAnt) // desceu muito?
        {
          if(nThrottleControle < 2000)
            nThrottleControle++;  
        }        
       }
          esc.write(nThrottleControle);   // para escrever o valor da aceleração    
          Serial.println(nThrottleControle);       
    }
  } // if aux
  else
  {
    // ler acelerador atual;    
    CalcAceleracao(&nThrottle, &nAux);    
    if(nThrottleControle == 0)
    { // verificar se a aceleração está abaixo
      if (!CalcAceleracao(&nThrottle, &nAux)) // se não tem sinal vamos para de emitir pulso
      {
      esc.write(800);      
      return;            // retorna ao inicio do loop
      }
      esc.write(nThrottle);   // vamos escrever a sua aceleração
    }
    else
    {     
      if(nThrottle > nThrottleControle)
      {          
        do
        {
          if(millis() - IncrementDelay > 10)  // maior que 15 milisegundos
          {
            IncrementDelay = millis();        // tempo atual
            nThrottleControle++;               // incrementa a aceleração
            esc.write(nThrottleControle);
          }  
          CalcAceleracao(&nThrottle, &nAux);  // atualiza o valor da aceleração
        }while(nThrottle > nThrottleControle);
        nThrottleControle = 0;                // para zerar o valor anterior da correção
        LeituraDeAltura = true;
      }
      else
      {   // para decrementar se o acelerador estiver abaixo do valor corrigido
        do
        {
          if(millis() - DecrementDelay > 10)  // maior que 15 milisegundos
          {
            DecrementDelay = millis();        // tempo atual
            nThrottleControle--;               // incrementa a aceleração
            esc.write(nThrottleControle);
          }  
          CalcAceleracao(&nThrottle, &nAux);  // atualiza o valor da aceleração          
        }while(nThrottle < nThrottleControle);
        nThrottleControle = 0;                // para zerar o valor anterior da correção
        LeituraDeAltura = true;               // fazer a próxima leitura
      } // nThrottle < nThrottleControle
      
    } // else
  }

}
