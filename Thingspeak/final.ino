//get data from the sensor
//import data from "www.kma.go.kr"
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

const char* kma = "www.kma.go.kr";
const char* kma_url = "/wid/queryDFSRSS.jsp?zone=1147051000";
const char* Thingspeak = "api.thingspeak.com";
const char* apiKey = "BDOUFDAQ2G11K81P";     //  Enter your Write API key from ThingSpeak

float tempFromSensor;
String tempFromServer = "";

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
    getTempFromServer();

    //send data
    connectToServer(Thingspeak);
    // We now create a URI of the Thingspeak
    String Thingspeak_url = "/update";
    Thingspeak_url += "?api_key=";
    Thingspeak_url += apiKey;
    Thingspeak_url += "&field1=";
    Thingspeak_url += tempFromSensor;
    Thingspeak_url += "&field2=";
    Thingspeak_url += tempFromServer;
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

void getTempFromServer(){
    connectToServer(kma);
    requestToServer(kma, kma_url);

    // Read all the lines of the reply from server
    while(client.available()) {
        String line = client.readStringUntil('\n');
        if(!parsing(line)){
          Serial.println();
          Serial.println("temperature from the server: " + tempFromServer);
          Serial.println();
        }
    }
    closeConnection();
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

//return 0: success
int parsing(String line){
   String tmp_str = ""; 
   String data_seq = "";
   
   //Check Data Sequence
   int seq = line.indexOf("<data seq=\"");
   if(seq >= 0)
   {        
        tmp_str = "<data seq=\"";
        int seq_end = line.indexOf("\">");
        data_seq = line.substring(seq+tmp_str.length(), seq_end);
        // DEBUG_SERIAL.println(data_seq);
   }
   else return 1;
   
   if(data_seq == "0")
   { 
        String next_line = client.readStringUntil('\r');
        //get temperature
        int temp = next_line.indexOf("</temp>");
        if(temp >= 0)
        {
            tmp_str="<temp>";
            tempFromServer = next_line.substring(next_line.indexOf(tmp_str)+tmp_str.length(), temp);
            
        }
   }
   else return  1;

   
   return 0;
}

void closeConnection(){
    Serial.println();
    Serial.println("closing connection");
    Serial.println();

    client.flush();
    client.stop();
}
