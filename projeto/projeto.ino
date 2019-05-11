#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "DHT.h"


const char *ssid =  "Wi-Fi 2";     // replace with your wifi ssid and wpa2 key
const char *pass =  "canada2016";

#define TOPICO_SUBSCRIBE "Teste/Recibo"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "Teste/Envio"
#define TOPICO_ENVIO_TEMP   "status/temp"
#define TOPICO_ENVIO_UMI   "status/umi"
#define TOPICO_ENVIO_SENSOR   "status/sensor"
#define ID_MQTT  "NodeMCU"
const char* BROKER_MQTT = "192.168.0.6"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

WiFiClient espClient;
PubSubClient MQTT(espClient);

DHT dht;
 
void setup() 
{
      Serial.begin(9600);
      dht.setup(D1);
      wifi_setup();
      mqtt_setup();
      if(MQTT.publish(TOPICO_PUBLISH, (char*) "ENVIO")){
        Serial.println("Envio realizado com sucesso!");   
      } else{
        Serial.println("Não foi possível realizar o envio!");
      }
}
 
void loop() 
{      
        MQTT.loop();
        envio_temp();
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

    Serial.println(msg);
     
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
            MQTT.subscribe(TOPICO_ENVIO_TEMP); 
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
  Serial.print("Umidade = ");
  Serial.print(humidity);
  Serial.print("\t\t");
  Serial.print("Temperatura = ");
  Serial.print(temperature);
  Serial.print("\n");
  char temp[3];
  char umi[3];
  if (isnan(temperature) || isnan(humidity)){
    Serial.println("Erro no Sensor!");
    MQTT.publish(TOPICO_ENVIO_SENSOR, (char*) "OFFLINE");
    } else{
      MQTT.publish(TOPICO_ENVIO_SENSOR, (char*) "ONLINE");
    if(MQTT.publish(TOPICO_ENVIO_TEMP, (char*) dtostrf(temperature, 6, 2, temp))){
          Serial.println("Envio realizado com sucesso!");   
    } else{
          Serial.println("Não foi possível realizar o envio!");
    }
    if(MQTT.publish(TOPICO_ENVIO_UMI, (char*) dtostrf(humidity, 6, 2, umi))){
          Serial.println("Envio realizado com sucesso!");   
    } else{
          Serial.println("Não foi possível realizar o envio!");
    }
  }
  delay(2000);
}
