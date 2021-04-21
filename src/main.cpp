#include <Arduino.h>
#include <EthernetClient.h>
#include <PubSubClient.h>
#include <Ethernet.h>

const int PUMP_PIN = 2;
const int SENSOR_PIN = 0;
const int WATERING_DELAY = 3000;

String topic = "Moisture_Sensor/";
String pumpActionTopic = topic+"pumpAction";
String pumpStateTopic = topic+"pumpState";

Ethernet WSA;
EthernetClient ethClient;
PubSubClient mqttClient;

String payloadToString(byte* payload, unsigned int length){
  char buffer[length];
  sprintf(buffer, "%.*s", length, payload);
  return String(buffer);
}


void startPump(){
    if(!mqttClient.publish(pumpStateTopic.c_str(), String("on").c_str())){
      Serial.println("Unable to publish started pump state value..");
      return;
    }
    Serial.println("Start watering the plant.");
    digitalWrite(PUMP_PIN, 1);
    delay(WATERING_DELAY);
    digitalWrite(PUMP_PIN, 0);
    Serial.println("Finish watering the plant.");
    if(!mqttClient.publish(pumpStateTopic.c_str(), String("off").c_str())){
      Serial.println("Unable to publish finished pump state value..");
      return;
    }
}

void actionCallback(char * topicChar, byte* payloadByte, unsigned int length)
{
  Serial.println("New message received");
  String topic = String(topicChar);
  String payload = payloadToString(payloadByte, length);

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  if(!topic.equals(pumpActionTopic))
  {
    return;
  }

  if(payload.equalsIgnoreCase("on"))
  {
    startPump();
  }else if (payload.equalsIgnoreCase("off")){
    if(!mqttClient.publish(pumpStateTopic.c_str(), String("off").c_str())){
      Serial.println("Unable to publish finished pump state value..");
      return;
    }
  }
  

}


float getCaptorValue() {
  //float analogValue = analogRead(SENSOR_PIN);
  float analogValue = random(500);
  Serial.print("Analog value : ");
  Serial.println(analogValue);
  float captormV = ((analogValue*5.0) / 1024.0) *1000.0;
  Serial.print("Captor voltage in mV : ");
  Serial.println(captormV);

  return captormV;
}

void subscribePumpActionTopic(){
  if(!mqttClient.subscribe(pumpActionTopic.c_str())){
    Serial.println("Can't subscribe to the pump action topic");
    return;
  }
  Serial.println("Subscribe success.");
  
}

void setupMqtt(){
  mqttClient.setClient(ethClient);
  mqttClient.setServer("192.168.1.210",1883);
  mqttClient.setCallback(actionCallback);
}

void connectMqtt(){
  if(!mqttClient.connected()){
    Serial.println("Trying to connect to MQTT Broker.");
    if(!mqttClient.connect("CapteurPlante")){
      Serial.println("Connection to MQTT Broker failed.");
      return;
    }
      Serial.println("Connection to MQTT Broker succed.");
      subscribePumpActionTopic();
  }
}

void publishCaptor(){
  if(!mqttClient.connected()){
    Serial.println("Can't publish moisture sensore value. Mqtt isnt connected.");
    return;
  }
  float captorVal = getCaptorValue();
  String topicName = topic+"sensor";
  if(!mqttClient.publish(topicName.c_str(), String(captorVal).c_str())){
      Serial.println("Can't publish moisture sensore value.");
  }  
}


void setup()
{
  setupMqtt();
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop()
{
  connectMqtt();  
  publishCaptor();
  mqttClient.loop();
  delay(1000);
}
