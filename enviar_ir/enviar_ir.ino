  #include <IRremoteESP8266.h> //INCLUSÃO DE BIBLIOTECA

IRsend irsend(D5); //FUNÇÃO RESPONSÁVEL PELO MÉTODO DE ENVIO DO SINAL IR / UTILIZA O GPIO14(D5)

int tamanho = 20; //TAMANHO DA LINHA RAW(68 BLOCOS)
int frequencia = 38; //FREQUÊNCIA DO SINAL IR(32KHz)

// BOTÃO LIGA / DESLIGA
unsigned int LD[20] = {6000,-1250,1200,-1250,600,-650,600,-1250,600,-650,600,-1250,600,-650,1200,-650,1200,-650,600}; //COLE A LINHA RAW CORRESPONDENTE DENTRO DAS CHAVES
 
void setup(){
  irsend.begin(); //INICIALIZA A FUNÇÃO
  Serial.begin(115200); //INICIALIZA A PORTA SERIAL
}
void loop(){
    
    char c = Serial.read(); //VARIÁVEL RESPONSÁVEL POR RECEBER O CARACTER DIGITADO NA JANELA SERIAL
    
        irsend.sendRaw(LD,tamanho,frequencia);  // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
        Serial.println("Comando enviado: Liga / Desliga");
        delay(1000); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
    
}
