#include <IRremoteESP8266.h> //INCLUSÃO DE BIBLIOTECA


int RECV_PIN = D6; //PINO DIGITAL EM QUE O FOTORRECEPTOR ESTÁ CONECTADO - GPIO12 / PINO D6

IRrecv irrecv(RECV_PIN); //VARIÁVEL DO TIPO IRrecv

decode_results results; //VARIÁVEL QUE ARMAZENA OS RESULTADOS

void setup(){
  Serial.begin(115200); //INICIALIZA A SERIAL
  irrecv.enableIRIn();  //INICIALIZA O RECEPTOR
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
    Serial.print(", ");
  }
  Serial.println();
}

void loop() {
  //RETORNA NA SERIAL AS INFORMAÇÕES FINAIS SOBRE O COMANDO IR QUE FOI IDENTIFICADO
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); //RECEBE O PRÓXIMO VALOR
  }
}
