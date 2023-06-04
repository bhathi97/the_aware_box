#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//LED
#define LED1  10

//worning LED
#define LEDw 0

//DHT11 for reading temperature and humidity value
#define DHTTYPE DHT11

//HC-SR04 ultrasonic distance sensor pins
#define trigPin   16
#define echoPin   5 

//HC-SR04 ultrasonic distance sensor variables
long duration;
float distance;
int x;

//DHT11 sensor variables
const int DHTPin = 13;      
DHT dht(DHTPin, DHTTYPE);
uint32_t delayMS;

//WiFi Access Point

#define WLAN_SSID       "Bhathi"
#define WLAN_PASS       "12345678"

//Adafruit.io Setup 

#define AIO_SERVER      "broker.hivemq.com" 
#define AIO_SERVERPORT  1883               
#define AIO_USERNAME    ""
#define AIO_KEY         ""

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
//WiFiClientSecure client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

//feeds
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bhathi_humidity");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bhathi_temperature");

Adafruit_MQTT_Subscribe led1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/bhathi_led");
Adafruit_MQTT_Publish dist = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bhathi_dis");

void MQTT_connect();
void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println(F("Project"));

  // Connect to WiFi access point.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  
  // HC-SR04 ultrasonic distance sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //LEDs
  pinMode(LED1, OUTPUT);

  //worning LED
  pinMode(LEDw, OUTPUT);

  //Setting up DHT sensor
  dht.begin();

  // Setup MQTT subscription for on and off feed.
  mqtt.subscribe(&led1);


}

void loop() {
  //DHT11 reading 
  float t1 = dht.readTemperature();
  String Temp = "Temperature : " + String(t1) + " °C";

  //clear the trigpin
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);

  //set the trigpin in HIGH state for 10 ms
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  //read the echopin
  duration = pulseIn(echoPin, HIGH);

  //Calculate the distance
  distance = duration * (0.034/2); //a=t*v
  String dis = "Distance : "+ String(distance)+ " cm";
  
  x = distance;

  //print in serial
  Serial.println(dis);
  Serial.println(Temp);
  delay(1000);

  if( x > 10){
    //int a= 2;
    //for(int b=1; b<a; b++){
    //  digitalWrite(LEDw,HIGH);
    //  delay(1000);
    //  digitalWrite(LEDw,LOW);
    //  delay(1000);
    //  if(x > 10){
    //    a++;
    //  }else{
    //    a=0;
    //  }
    //}
    digitalWrite(LEDw,HIGH);
  }else{
    digitalWrite(LEDw,LOW);
  }

  

 //-----------------------------------

  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &led1) {
      Serial.print(F("Got: "));
      Serial.println((char *)led1.lastread);
      int led1_State = atoi((char *)led1.lastread);
      try {
        if(led1_State==1 || led1_State==0){
          digitalWrite(LED1, led1_State);
        }
        else{
            throw 50;
        }
      }catch(int e){
        Serial.println("error error! publish only 1 or 0");
      } 
    } 
  }
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float d = distance;
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (! Humidity.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! Temperature.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! dist.publish(d)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }
  
  Serial.print("Connecting to MQTT");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 1 seconds...");
    mqtt.disconnect();
    delay(500);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");

}
