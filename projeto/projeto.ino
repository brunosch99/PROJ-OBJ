#include <Timing.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "DHT.h"



const char *ssid =  "Wi-Fi 2";     // replace with your wifi ssid and wpa2 key
const char *pass =  "canada2016";

Timing timer;
#define TOPICO_SUBSCRIBE "commands/remote"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "Teste/Envio"
#define TOPICO_ENVIO_TEMP   "status/temp"
#define TOPICO_ENVIO_UMI   "status/umi"
#define TOPICO_ENVIO_SENSOR   "status/sensor"
#define ID_MQTT  "NodeMCU"
#define TOPICO_TEMP_DESEJADA "commands/temp_desejada"
#define HABILITAR_CONTROLE "commands/enable_control"
const char* BROKER_MQTT = "192.168.0.6"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

WiFiClient espClient;
PubSubClient MQTT(espClient);

DHT dht;

float temp_desejada;
boolean controle_habilitado;
int temp_ac;
 
void setup() 
{
      Serial.begin(9600);
      dht.setup(D1);
      wifi_setup();
      mqtt_setup();
      timer.begin(0);
      controle_habilitado = false;
      temp_desejada = dht.getTemperature();
      temp_ac = 20;
}
 
void loop() 
{      
        MQTT.loop();
        envio_temp();
        mqtt_setup();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }

    if(strcmp(topic,"commands/temp_desejada") == 0){
      temp_desejada = msg.toFloat();
    } else if (strcmp(topic, "commands/enable_control") == 0){
      controle_habilitado = msg == "true";
    } else if (strcmp(topic, "commands/remote") == 0){
      controle_remoto(msg);
    }
      //Serial.println(msg);
     
}

void wifi_setup(){
  Serial.begin(9600);
  delay(10);
               
  Serial.println("Connecting to ");
  Serial.println(ssid); 
 
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_setup(){
      MQTT.setCallback(mqtt_callback); 
      MQTT.setServer(BROKER_MQTT, BROKER_PORT);
      while (!MQTT.connected()) 
      {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        int i = 0;
        if (MQTT.connect(ID_MQTT+i)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
            MQTT.subscribe(TOPICO_TEMP_DESEJADA);
            MQTT.subscribe(HABILITAR_CONTROLE);
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
            i++;
        }
     }
}

void envio_temp(){
  float humidity = dht.getHumidity();/* Get humidity value */
  float temperature = dht.getTemperature();/* Get temperature value */
  controle_temp(temperature);
  if (!isnan(temperature) && !isnan(humidity)){
    char temp[3];
    char umi[3];
    MQTT.publish(TOPICO_ENVIO_TEMP, (char*) dtostrf(temperature, 6, 2, temp));
    MQTT.publish(TOPICO_ENVIO_UMI, (char*) dtostrf(humidity, 6, 2, umi));
  } 
  //Serial.println(temperature);
}

void controle_temp(float temp){
  if(timer.onTimeout(2000)){
  if(controle_habilitado == true){
      if(temp > temp_desejada){
        if(temp_ac > 15){
          Serial.println("Diminuir Temperatura");
          temp_ac--;
        } else{
          Serial.println("Temperatura mínima do Ar-condicionado atingida!");
        }
      } else if(temp < temp_desejada){
        if(temp_ac < 25){
          Serial.println("Aumentar Temperatura");
          temp_ac++;
        } else{
          Serial.println("Temperatura máxima do Ar-condicionado atingida!");
        }
      }
    }
  }
}

void controle_remoto(String comando){
  if(comando.equals("ON")){
    Serial.println("Ligando Ar-Condicionado");
  }else if(comando.equals("OFF")){
    Serial.println("Desligando Ar-Condicionado");
  } else if(comando.equals("PLUS")){
    if(temp_ac < 25){
      Serial.println("Aumentando a Temperatura");
      temp_ac++;
    }else{
      Serial.println("Temperatura máxima do Ar-condicionado atingida!");
    }
  } else if(comando.equals("MINUS")){
    if(temp_ac > 15){
      Serial.println("Diminuindo a Temperatura");
      temp_ac--;
    } else{
      Serial.println("Temperatura mínima do Ar-condicionado atingida!");
    }
  }
}
