#include <IRremote.h>
const int RECV_PIN = 7;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup(){
  pinMode(4, OUTPUT);
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true);
  digitalWrite(4, LOW); 
}

void loop(){
  if (irrecv.decode(&results)){
        Serial.println(results.value,HEX);
//        if(results.value == 4272685799,HEX){
//            Serial.println("Hello!");
//            digitalWrite(4, HIGH);
//          }
        irrecv.resume();
  }
}
