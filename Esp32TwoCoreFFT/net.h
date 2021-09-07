#include <WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"

WiFiClient espClient;
PubSubClient client(espClient);
TaskHandle_t NetTaskHandle;

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
// TODO action messageTemp
//  if (String(topic) == "leds/blueangle") {
//    blueangle = messageTemp.toInt();
//  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("leds/blueangle");
      client.subscribe("leds/bluelow");
      client.subscribe("leds/bluehigh");
      client.subscribe("leds/bluetime");
      client.subscribe("leds/blueshift");
      
      client.subscribe("leds/greenangle");
      client.subscribe("leds/greenlow");
      client.subscribe("leds/greenhigh");
      client.subscribe("leds/greentime");
      client.subscribe("leds/greenshift");
      
      client.subscribe("leds/redangle");
      client.subscribe("leds/redlow");
      client.subscribe("leds/redhigh");
      client.subscribe("leds/redtime");
      client.subscribe("leds/redshift");
      
      client.subscribe("leds/fadespeed");
      client.subscribe("leds/loopspeed");
      client.subscribe("leds/jazzhands");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void NetTaskCode( void * pvParameters ) {
  while(1) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  xTaskCreatePinnedToCore(
      NetTaskCode, /* Function to implement the task */
      "NetTaskName", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &NetTaskHandle,  /* Task handle. */
      0); /* Core where the task should run */
}
