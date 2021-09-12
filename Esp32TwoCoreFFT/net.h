#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"

#include "secrets.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

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
  Serial.println(messageTemp);
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
    if (client.connect(CLIENT_NAME)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("leds/loopspeed");
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
  Serial.println("getting current time");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  xTaskCreatePinnedToCore(
      NetTaskCode, /* Function to implement the task */
      "NetTaskName", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &NetTaskHandle,  /* Task handle. */
      0); /* Core where the task should run */
}
