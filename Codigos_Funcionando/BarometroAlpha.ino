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
unsigned int //nCorMilisegundo = 80,             // tempo de milisegundo para efetuar a correção
             nDelayCorrIncrement = 45,         // delay de incrementação correção
             nDelayCorrDecrement = 80,         // delay de decrementação para correção
             nGanhoSubida = 3,
             nGanhoDescida = 1;                // obs ganho de descida representa o numero de diferença * ganho 
//------------------------------------------------------
//--
//--
// -----------------variáveis não editáveis--------------
unsigned int nThrottle = 0,           // para o acelerador
             nThrottleControle = 0,   // para alterar valores na estabilização 
             nThrottleComparador = 0, // para comparar a aceleração             
             nAux = 0;
             
             
int          nSubDiferenca = 0,       // para conter o valor da subtração
             nSubDiferencaAnt = 0;    // diferença com o valor anterior
             
unsigned long lPressaoAnt = 0,       // para conter o valor da pressao anterior
              lPressaoAtu = 0,       // para conter a pressao atual 
              CorDelay = 0,          // delay de correção
              IncrementDelay = 0,    // delay de incrementação da aceleração 
              DecrementDelay = 0,    // tempo de deley de decrementação
              IncDelayBar = 0,       // tempo para o ajuste do barometro
              DecDelayBar = 0;       // tempo para o ajuste de decremento barometro
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
        nThrottleComparador = nThrottleControle = nThrottle;  // guardar o valor da aceleração
         
        LeituraDeAltura = false;        // indicar que já está ativo dentro da correção
      }        
      // vamos ler a pressao para ver se houve variação             
      ObterPressao(&lPressaoAtu);   // para pegar a pressao atual        
      lPressaoAtu = lPressaoAtu - (lPressaoAtu % 10); // pegar o resto para subtrair com o valor de oscilação atual
      lPressaoAnt = lPressaoAnt - (lPressaoAnt % 10); // idem anterior
      nSubDiferencaAnt = lPressaoAtu - lPressaoAnt;  // subtrair a diferença 
      
      if(nSubDiferencaAnt == 0)
        esc.write(nThrottleControle);   // para escrever o valor da aceleração
      if(nSubDiferencaAnt > 0)
      {
        if(nThrottleControle < (nThrottleComparador + nSubDiferencaAnt)) 
        {              
          nThrottleComparador = nThrottleControle;  // atribui o valor para o comparador do acelerador         
          do
          {
            if(millis() - IncDelayBar > nDelayCorrIncrement)
            {
               IncDelayBar = millis();             
               ObterPressao(&lPressaoAtu);   // para pegar a pressao atual
               lPressaoAtu = lPressaoAtu - (lPressaoAtu % 10); // pegar o resto para subtrair com o valor de oscilação atual              
               nSubDiferenca = lPressaoAtu - lPressaoAnt;  // subtrair a diferença 
               if(nSubDiferenca < 0)
                break;          // cai fora do loop
               if(!CalcAceleracao(&nThrottle, &nAux))
                  break;
               // como não é negativo vamos continuar 
               
                if(nThrottleControle < 2100)
                {
                  nThrottleControle++; // aumentar a aceleração
                  esc.write(nThrottleControle);   // para escrever o valor da aceleração                      
                }
             }          
          }while(nThrottleControle < (nThrottleComparador + (nSubDiferencaAnt * nGanhoSubida)));     
        } // if   
      }  
      // para subtrair o valor na altura     
      else
      {
        if(nThrottleControle > (nThrottleComparador + nSubDiferencaAnt)) 
        {           
          nThrottleComparador = nThrottleControle;  // atribui o valor para o comparador do acelerador       
          do
          {
            if(millis() - DecDelayBar > nDelayCorrDecrement)
            {
              DecDelayBar = millis();
               ObterPressao(&lPressaoAtu);   // para pegar a pressao atual
               lPressaoAtu = lPressaoAtu - (lPressaoAtu % 10); // pegar o resto para subtrair com o valor de oscilação atual               
               nSubDiferenca = lPressaoAtu - lPressaoAnt;  // subtrair a diferença 
               if(nSubDiferenca > 0)
                break;          // cai fora do loop
               // como não é positivo vamos continuar 
               if(!CalcAceleracao(&nThrottle, &nAux))
                  break;
               if(nThrottle > 1100)
               {
                nThrottleControle--; // decrementar a aceleração
                esc.write(nThrottleControle);   // para escrever o valor da aceleração                    
               }
            }          
          }while(nThrottleControle > (nThrottleComparador + (nSubDiferencaAnt * nGanhoDescida)));          
        }
       
      } // nSubDiferenca < 0                              
    } // while true
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
          if(millis() - IncrementDelay > 5)  // maior que 5 milisegundos
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
          if(millis() - DecrementDelay > 10)  // maior que 10 milisegundos
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
