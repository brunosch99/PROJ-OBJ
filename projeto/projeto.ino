//Inclusão de bibliotecas
#include <Timing.h> //Utilizada para controlar tempo do envio de comandos
#include <PubSubClient.h> //Utilizada para comunicação com o broker MQTT
#include <ESP8266WiFi.h> //Utilizada para conectar com o Wi-Fi
#include "DHT.h" //Utilizada para medição de temperatura
#include <IRremoteESP8266.h> //Utilizada para envio de comandos infravermelho

//Definição de tópicos MQTT
#define TOPICO_REMOTE "commands/remote" //Recebe comandos de controle do ar-condicionado
#define TOPICO_ENVIO_TEMP   "status/temp" //Envio de temperatura
#define TOPICO_ENVIO_UMI   "status/umi" //Envio de umidade
#define TOPICO_TEMP_DESEJADA "commands/temp_desejada" //Recebe a temperatura desejada pelo usuário
#define TOPICO_HABILITAR_CONTROLE "commands/enable_control" //Recebe a opção de habilitar ou não o controle inteligente de temperatura

#define ID_MQTT  "NodeMCU" //Define qual será o id de conexão do NodeMCU no broker MQTT

IRsend irsend(D5); //Cria o objeto irsend do tipo IRsend que será responsável por envia os comandos infravermelho. Ele estará no pino D5 do NodeMCU

Timing timer; //Cria o objeto timer do tipo Timing responsável por controlar o tempo entre um envio e o outro
Timing timerLed;

WiFiClient espClient; //Cria o objeto espCliente do tipo WiFiCliente responsável por se conectar no WiFi
PubSubClient MQTT(espClient); //Cria o objeto MQTT do tipo PubSubClient responsável por se conectar no broker MQTT, recebe como parâmetro o objeto espCliente para se conectar no broker através da conexão Wi-Fi desse objeto

DHT dht; //Cria o objeto dht do tipo DHT responsável por medir a temperatura e a umidade

//Definição dos códigos infravermelho
//Todos são do tipo uint64_t, eles são números inteiros representados por um Hexadecimal que será lido pelo sensor infravermelho
uint64_t liga = 0x4AB0F7B5; //Código de ligar o ar-condicionado
uint64_t desliga = 0x4B498E99; //Código de desligar o ar-condicionado
uint64_t aumenta = 0x4BE2257D; //Código de aumentar a temperatura do ar-condicionado
uint64_t diminui = 0x4C7AB879; //Código de diminuir a temperatura do ar-condicionado

//Definindo informações de conexão do Wi-Fi
const char *ssid =  "iPhone"; //SSID do Wi-Fi
const char *pass =  "bruno123"; //Senha do Wi-Fi

const char* BROKER_MQTT = "smartair.eastus.cloudapp.azure.com";  //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

//Inicialização de variáveis
float temp_desejada; //Variável que contém a temperatura de desejada pelo usuário
boolean controle_habilitado; //Variável que controla se o controle inteligente está habiltado
int temp_ac; //Variável que contém a temperatura atual do ar-condicionado
int min_ac;
int max_ac;

void setup() 
{
      pinMode(D2, OUTPUT);
      pinMode(D3, OUTPUT);
      pinMode(D6, OUTPUT);
      pinMode(D8, OUTPUT);
      digitalWrite(D2, LOW);
      digitalWrite(D3, LOW);
      digitalWrite(D6, LOW);
      digitalWrite(D8, HIGH);
      Serial.begin(9600); //Define a taxa de transferência de bit pos segundo como 9600
      dht.setup(D1); //Inicia o medidor de temperatura no pino D1
      wifi_setup(); //Inicia a função de configuração de conexão Wi-Fi
      mqtt_setup(); //Inicia a função de configuração de conexão MQTT
      timer.begin(0); //Inicia o timer
      timerLed.begin(0);
      controle_habilitado = false; //Define que o controle está desabilitado
      temp_desejada = dht.getTemperature(); //Define que a temperatura desejada pelo usuário de inicio é igual a temperatura atual
      temp_ac = 20; //Define que a temperatura de inicio do ar-condicionado 
      min_ac = 15; //Define a temperatura mínima do ar-condicionado
      max_ac = 32; //Define a temperatura máxima do ar-condicionado
}
 
void loop() 
{      
        MQTT.loop(); //Chama a função loop do objeto MQTT 
        envio_temp(); //Chama a função de envio de temperatura
        mqtt_setup(); //Chama a função de configurar o MQTT para garantir a reconexão ao broker

        if(controle_habilitado == true){
          digitalWrite(D6, HIGH);
        } else{
          digitalWrite(D6, LOW);
        }
}

//Função de recebimento de mensagem do broker MQTT
//Recebe como parâmetros o tópico em que foi enviada a mensagem, a mensagem e o tamanho da mensagem
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg; //Variável que armazenará a mensagem
 
    //Obtem a string do payload recebido e armazena na variável msg
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    //Caso o tópico seja o que recebe a temperatura desejada
    if(strcmp(topic,TOPICO_TEMP_DESEJADA) == 0){
      temp_desejada = msg.toFloat(); //A temperatura desejada é atualizada conforme a mensagem recebida no tópico 

    //Caso o tópico seja o que recebe o status do controle inteligente
    } else if (strcmp(topic, TOPICO_HABILITAR_CONTROLE) == 0){
      controle_habilitado = msg == "true"; //A variável de controle é atualizada verificando se a mensagem recebida é igual a true

    //Caso o tópico seja o que recebe comandos do ar-condicionado
    } else if (strcmp(topic, TOPICO_REMOTE) == 0){
      controle_remoto(msg); //A função que envia comandos pro ar-condicionado é chamada passando a mensagem (O comando a ser enviado) como parâmetro
    }    
}

//Função responsável por iniciar a conexão Wi-Fi
void wifi_setup(){
  Serial.println("Connecting to ");
  Serial.println(ssid); 

  //Inicia a conexão o Wi-Fi conforme o SSID e a Senha definidas
  WiFi.begin(ssid, pass); 

  //Enquanto não estiver conectado printa na tela um ponto a cada meio segundo
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }

  //Quando se conecta printa que o Wi-Fi foi conectado e o IP do NodeMCU na rede
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Função que configura a conexão ao Broker MQTT
void mqtt_setup(){
      //Define a função responsável por receber as mensagens
      MQTT.setCallback(mqtt_callback); 

      //Define qual servidor e qual porta o NodeMCU se conectará
      MQTT.setServer(BROKER_MQTT, BROKER_PORT);

      //Enquanto o NodeMCU não estiver conectado
      while (!MQTT.connected()) 
      {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        int i = 0;
        
        //É tentada uma conexão com o id definido no inicio mais um número incrementado a cada iteração
        //Isso é necessário para que a cada conexão o id de conexão do NodeMCU seja diferente
        if (MQTT.connect(ID_MQTT+i)) 
        {
            //Caso seja conectado, o MQTT se conecta aos tópicos
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_REMOTE); //Tópico de comandos de controle do ar-condicionado 
            MQTT.subscribe(TOPICO_TEMP_DESEJADA); //Tópico de temperatura desejada 
            MQTT.subscribe(TOPICO_HABILITAR_CONTROLE); //Tópico de status de controle inteligente
        } 
        else
        {
            //Caso não consiga se conectar
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
            //Adiciona um na variável que será adicionada ao id de conexão
            i++;
        }
     }
}

//Função responsável por medir a temperatura e a umidade, chamar a função de controle inteligente, e enviar a temperatura e umidade ao broker
void envio_temp(){
  
  float humidity = dht.getHumidity(); //Variável que armazenará a umidade medida pelo sensor
  float temperature = dht.getTemperature(); //Variável que armazenará a temperatura medida pelo sensor

  //Caso as temperaturas medidas sejam válidas, isto é, diferentes de nan (Not a number)
  if (!isnan(temperature) && !isnan(humidity)){
    //Chama a função de controle inteligente passando a temperatura como parâmetro
    controle_temp(temperature);

    //Cria variáveis char que serão responsáveis por enviar a temperatura como char ao tópico
    char temp[3];
    char umi[3];

    //Envio de temperatura e umidade ao seus respectivos tópicos, convertendo o valor medido como float para as variáveis char
    MQTT.publish(TOPICO_ENVIO_TEMP, (char*) dtostrf(temperature, 6, 2, temp));
    MQTT.publish(TOPICO_ENVIO_UMI, (char*) dtostrf(humidity, 6, 2, umi));
  } 
}

//Função de controle inteligente de temperatura
//Recebe como parâmetro a temperatura atual
void controle_temp(float temp){
  
  //Caso ja tenha se passado 2 segundos desde o último controle do ar-condicionado
  if(timer.onTimeout(2000)){
    //Caso o controle inteligente esteja habilitado
    if(controle_habilitado == true){

        //Caso a temperatura atual esteja maior do que a temperatura desejada pelo usuário
        if(temp > temp_desejada){
          //Caso o ar-condicionado não tenha alcançado sua temperatura mínima
          if(temp_ac > min_ac){
            Serial.println("Diminuir Temperatura");
            //É enviado ao ar-condicionado o comando de diminuir a temperatura
            irsend.sendNEC(diminui, 32);
            //Diminui a variável que contém a tempetura do ar-condicionado
            temp_ac--;
            digitalWrite(D2, HIGH);
            delay(500);
            digitalWrite(D2, LOW);
          } else{
            Serial.println("Temperatura mínima do Ar-condicionado atingida!");
          }
        } else if(temp < temp_desejada){
          //Caso o ar-condicionado não tenha alcançado sua temperatura máxima
          if(temp_ac < max_ac){
            Serial.println("Aumentar Temperatura");
            //É enviado ao ar-condicionado o comando de aumentar a temperatura
            irsend.sendNEC(aumenta, 32);
            //Aumenta a variável que contém a tempetura do ar-condicionado
            temp_ac++;
            digitalWrite(D3, HIGH);
            delay(500);
            digitalWrite(D3, LOW);
          } else{
            Serial.println("Temperatura máxima do Ar-condicionado atingida!");
          }
        }
    }
  }
}

//Função que envia comandos requisitados pelo usuário, recebe como parâmetro o comando requisitado
void controle_remoto(String comando){
  //Caso o comando seja ligar o ar-condicionado
  if(comando.equals("ON")){
    Serial.println("Ligando Ar-Condicionado");
    //Envia o comando de ligar o ar-condicionado
    irsend.sendNEC(liga, 32);
  
  //Caso o comando seja desligar o ar-condicionado
  }else if(comando.equals("OFF")){
    Serial.println("Desligando Ar-Condicionado");
    //Envia o comando de desligar o ar-condicionado
    irsend.sendNEC(desliga, 32);
    
  //Caso o comando seja aumentar a temperatura do ar-condicionado
  } else if(comando.equals("PLUS")){
    //Caso o ar-condicionado não tenha alcançado sua temperatura máxima
    if(temp_ac < 25){
      Serial.println("Aumentando a Temperatura");
      //Envia o comando de aumentar a temperatura do ar-condicionado
      irsend.sendNEC(aumenta, 32);
      //Aumenta a variável que contém a tempetura do ar-condicionado
      temp_ac++;
      digitalWrite(D3, HIGH);
      delay(500);
      digitalWrite(D3, LOW);
  }else{
      Serial.println("Temperatura máxima do Ar-condicionado atingida!");
  }
  
  //Caso o comando seja diminuir a temperatura do ar-condicionado 
  } else if(comando.equals("MINUS")){
    //Caso o ar-condicionado não tenha alcançado sua temperatura mínima
    if(temp_ac > 15){
      Serial.println("Diminuindo a Temperatura");
      //Envia o comando de diminuir a temperatura do ar-condicionado
      irsend.sendNEC(diminui, 32);
      //Diminui a variável que contém a tempetura do ar-condicionado
      temp_ac--;
      digitalWrite(D2, HIGH);
      delay(500);
      digitalWrite(D2, LOW);
    } else{
      Serial.println("Temperatura mínima do Ar-condicionado atingida!");
    }
  }
}
