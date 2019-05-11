#include "DHT.h"

DHT dht;

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println("Status\tHumidity (%)\tTemperature (C)\t(F)");
  dht.setup(D1);   /* D1 is used for data communication */
}

void loop()
{
  delay(2000);
  float humidity = dht.getHumidity();/* Get humidity value */
  float temperature = dht.getTemperature();/* Get temperature value */
  Serial.print("Humidade = ");
  Serial.print(humidity);
  Serial.print("\t\t");
  Serial.print("Temperatura = ");
  Serial.print(temperature);
  Serial.print("\n");
}
