/* MQTT Client for a Neopixel Library Output
 *  clientID = ESP_1
 *  Author = maja
 *  
*/

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <Hash.h>

///////////////////////////////////////////////////////////////////////
///                      Definitions                                ///
///////////////////////////////////////////////////////////////////////

#define PIN D1
#define NUMPIXELS 22
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


///////////////////////////////////////////////////////////////////////
///                      WiFi                                       ///
///////////////////////////////////////////////////////////////////////

const char* cfg_wifi_ssid = "WiFi-Name";
const char* cfg_wifi_pwd = "WiFi-Password";

///////////////////////////////////////////////////////////////////////
///                      MQTT                                       ///
///////////////////////////////////////////////////////////////////////

const char* mqtt_server = "www.XXXXXX.de";  //your server domain
const unsigned int mqtt_port = 8883;
const char* mqtt_user =   "username";    //broker username
const char* mqtt_pass =   "password";    //broker password
const char* clientID = "ESP_1"; //evtl. nicht gebraucht

///////////////////////////////////////////////////////////////////////
///                      OPEN-SSL                                   ///
///////////////////////////////////////////////////////////////////////

//echo | openssl s_client -connect 192.168.178.26:8883 | openssl x509 -fingerprint -noout
//send on raspberry pi bash
//enter the result hash here:
const char* mqtt_fprint = "4F:BD:64:C3:52:FF:4B:F9:61:69:44:A3:93:84:3C:XX:XX:XX:XX:XX";

///////////////////////////////////////////////////////////////////////
///                      DECLERATION                                ///
///////////////////////////////////////////////////////////////////////

WiFiClientSecure espClient;
PubSubClient client(espClient);

// temp & hum
float t = 0.0;
float h = 0.0;

int R;
int G;
int B;

int mask_R = 0xFF0000;
int mask_R = 0x00FF00;
int mask_R = 0x0000FF;

unsigned long previousMillis = 0;   // last updeate time
const long interval = 1000;         // Updates DHT readings
int wifi_retry = 0;                 // WiFi Reconnect


///////////////////////////////////////////////////////////////////////
///                      void setup                                 ///
///////////////////////////////////////////////////////////////////////

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);
  Serial.println("TestMQTT");

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg_wifi_ssid, cfg_wifi_pwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// ---------------FINGERPRINT----------------
// ------------------------------------------
  espClient.setFingerprint(mqtt_fprint);
  
// ---------------MQTT-SERVER----------------
// ------------------------------------------
  client.setServer(mqtt_server, mqtt_port);

// ---------------CALLBACK-------------------
// ------------------------------------------
  client.setCallback(callback);

// ---------------SECURE-Connect-------------
// ------------------------------------------
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    verifyFingerprint();
    if (client.connect(WiFi.macAddress().c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      
// Subsribe to Topic: eg. colourcode
      client.subscribe("colourcode");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

///////////////////////////////////////////////////////////////////////
///                      void callback                              ///
///////////////////////////////////////////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print("topic: ")
  Serial.print(topic);
  Serial.print(", payload: ");
  Serial.print(payload);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
    }
    message[length] = '\0';
    Serial.println(message);



R = mask_R && (message.ToInt());
G = mask_G && (message.ToInt());
B = mask_B && (message.ToInt());


for(int i=0;i<NUMPIXELS;i++){
  pixels.setPixelColor(i, pixels.Color(R,G,B));
  pixels.show();
}


/*
if (strcmp(message,"1")==0){
     digitalWrite(LED_BUILTIN, LOW);
     digitalWrite(pinRelay, LOW); 
    }
else if (strcmp(message,"0")==0){
    digitalWrite(LED_BUILTIN, HIGH); 
    digitalWrite(pinRelay, HIGH);
    }
*/
}

///////////////////////////////////////////////////////////////////////
///                      void verifyFingerprint                     ///
///////////////////////////////////////////////////////////////////////

void verifyFingerprint() {
  if(client.connected() || espClient.connected()) return; //Already connected
  
  Serial.print("Checking TLS @ ");
  Serial.print(mqtt_server);
  Serial.print("...");
  
  if (!espClient.connect(mqtt_server, mqtt_port)) {
    Serial.println("Connection failed. Rebooting.");
    Serial.flush();
    ESP.restart();
  }
  if (espClient.verify(mqtt_fprint, mqtt_server)) {
    Serial.print("Connection secure -> .");
  } else {
    Serial.println("Connection insecure! Rebooting.");
    Serial.flush();
    ESP.restart();
  }

  espClient.stop();
  delay(100);
}

unsigned int counter = 0;


///////////////////////////////////////////////////////////////////////
///                      void loop                                  ///
///////////////////////////////////////////////////////////////////////

void loop() {

// WIFI Reconnect
    while(WiFi.status() != WL_CONNECTED && wifi_retry < 5 ) {
        wifi_retry++;
        Serial.println("WiFi not connected. Try to reconnect");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);
        WiFi.begin(cfg_wifi_ssid, cfg_wifi_pwd);
        delay(100);
    }
    if(wifi_retry >= 5) {
        Serial.println("\nReboot");
        ESP.restart();
    }

  client.loop();
  
  delay(1000);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;


}
