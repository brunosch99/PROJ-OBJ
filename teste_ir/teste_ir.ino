#include <IRremoteESP8266.h> //INCLUSÃO DE BIBLIOTECA

int RECV_PIN = D6;

IRrecv irrecv(RECV_PIN); //VARIÁVEL DO TIPO IRrecv
IRsend irsend(D5);

decode_results results; //VARIÁVEL QUE ARMAZENA OS RESULTADOS

int tamanho = 68; //TAMANHO DA LINHA RAW(68 BLOCOS)
int frequencia = 32; //FREQUÊNCIA DO SINAL IR(32KHz)

#define teste 

unsigned int TESTE[68] = {9750,-1200,5950,-1250,1200,-1250,600,-650,600,-1250,600,-650,600,-1250,600,-650,1200,-650,1200,-650,600};//unsigned int TESTE2[3] ={200, -5600, 150};
void setup(){
  Serial.begin(115200); //INICIALIZA A SERIAL
  irrecv.enableIRIn();  //INICIALIZA O RECEPTOR
  irsend.begin();
}
//MÉTODO RESPONSÁVEL POR FAZER A DECODIFICAÇÃO DO SINAL IR RECEBIDO
//OS DADOS SÃO PASSADOS PARA A BIBLIOTECA IRREMOTE QUE FAZ TODO O
//TRATAMENTO E RETORNA AS INFORMAÇÕES DE ACORDO COM O PROTOCOLO RECONHECIDO
void dump(decode_results *results) {
  int count = results->rawlen;
  Serial.print(results->value, HEX);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 1; i < count; i++) {
    if (i & 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    }
    else {
      Serial.write('-');
      Serial.print((unsigned long) results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(",");
  }
  Serial.println();
}

void loop() {
  //RETORNA NA SERIAL AS INFORMAÇÕES FINAIS SOBRE O COMANDO IR QUE FOI IDENTIFICADO
  if (irrecv.decode(&results)) {
    dump(&results);
    irrecv.resume(); //RECEBE O PRÓXIMO VALOR
  }
  //irsend.sendRaw(TESTE,tamanho,frequencia);
  irsend.sendNEC(0xAB89254F, 32);
  delay(1000);
}
