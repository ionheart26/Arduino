//get data from the sensor
//and send those to "api.thingspeak.com"

/*
 *  
 *  You need to get ssid and pw
 *
 */
 
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>

// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

Ticker sensor;

// Use WiFiClient class to create TCP connections
WiFiClient client;

const char* ssid     = "ssid";
const char* password = "pw";

const char* Thingspeak = "api.thingspeak.com";
const char* apiKey = "BDOUFDAQ2G11K81P";     //  Enter your Write API key from ThingSpeak

float tempFromSensor;

void setup() {
  Serial.begin(115200);
  delay(10);
  WiFiSetup();
  // Start the DS18B20 sensor
  sensors.begin();
}

void loop() {
  post();
  delay(10*60*1000);
}

void WiFiSetup(){ 
  // We start by connecting to a WiFi network
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void post(){
    getTempFromSensor();

    //send data
    connectToServer(Thingspeak);
    // We now create a URI of the Thingspeak
    String Thingspeak_url = "/update";
    Thingspeak_url += "?api_key=";
    Thingspeak_url += apiKey;
    Thingspeak_url += "&field1=";
    Thingspeak_url += tempFromSensor;
    requestToServer(Thingspeak, Thingspeak_url);
    Serial.println("data is sent to \"api.thingspeak.com\"");
    closeConnection();
}

void getTempFromSensor(){
    //get data from sensor
    sensors.requestTemperatures();
    tempFromSensor = sensors.getTempCByIndex(0); 
    Serial.println();
    Serial.print("temperature from sensor: ");
    Serial.println(tempFromSensor);
    Serial.println();
}

void connectToServer(const char* host){
    Serial.print("connecting to ");
    Serial.println(host);
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }
}

void requestToServer(const char* host, String url){
    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            return;
        }
    }
}

void closeConnection(){
    Serial.println();
    Serial.println("closing connection");
    Serial.println();

    client.flush();
    client.stop();
}
