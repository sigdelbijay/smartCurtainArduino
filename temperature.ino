#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
 
#define ssid "smartcurtain"
#define password "curtain123"
#define SERIAL_BAUDRATE                 115200

#define STOP D3
#define DOWN D1
#define UP D2
//#define ledPin D3


//int state = 0;
int stateFromTemp, stateFromDB1, stateFromDB2 = 0;

int sensePin = A0;  //This is the Arduino Pin that will read the sensor output
int sensorInput;
double temp, newTemp;  

void setup() {

//  pinMode(ledPin, OUTPUT);
  pinMode(DOWN,OUTPUT);
  pinMode(UP,OUTPUT);
  pinMode(STOP, OUTPUT);
  digitalWrite(DOWN,HIGH); // Set UP and DOWN HIGH so the remote doesn't activate on start up
  digitalWrite(UP,HIGH);
  digitalWrite(STOP, HIGH);
 
  Serial.begin(115200);
  delay(4000);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");

}

double checkTemp() {
  sensorInput = analogRead(A0);    //read the analog sensor and store it
  Serial.println("sensor input");
  Serial.println(sensorInput);
  temp = (double)sensorInput/4095;       //find percentage of input reading
  temp = temp * 3.3;                 //multiply by 5V to get voltage
  temp = temp - 0.5;               //Subtract the offset 
  temp = temp * 100;               //Convert to degrees 
 
  Serial.print("Current Temperature: ");
  Serial.println(temp);
  return temp;

}

void changeCurtainState(int state) {
  Serial.println("state");
  Serial.println(state);
  if (state == 1) {
    Serial.println("ON/DOWN");
    digitalWrite(DOWN,LOW);
    delay(1000);
    digitalWrite(DOWN, HIGH);
  }else if (state == 0){
    Serial.println("OFF/UP");
    digitalWrite(UP,LOW);
    delay(1000);
    digitalWrite(UP, HIGH);
  }else if(state == 2) {
    Serial.println("STOP");
    digitalWrite(STOP, LOW);
    delay(1000);
    digitalWrite(STOP, HIGH);
  }
}

void httpPostRequest() {
  HTTPClient http;
 
  http.begin("http://jsonplaceholder.typicode.com/posts"); //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/plain");
  int httpResponseCode = http.POST("POSTING from ESP32"); //Send the actual POST request

  if(httpResponseCode>0){
 
    String response = http.getString();  //Get the response to the request
 
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
 
  }else{
 
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
 
  }
}
 
void loop() {
  
  double newTemp = checkTemp();
  if(newTemp > 30) {
    stateFromTemp = 0;
  } else {
   stateFromTemp = 1;
  }
  changeCurtainState(stateFromTemp);
  delay(8000);

  httpPostRequest();
  
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient https;
    String thumbprint = "72d40092773750c99ba138fa218a9bfdbacfcd49";
 
    https.begin("https://xg2bxzrcog.execute-api.us-east-1.amazonaws.com/prod", thumbprint); //Specify the URL
    int httpCode = https.GET();                                        //Make the request
    Serial.println("HTTP CODE"+httpCode);
 
    if (httpCode > -1) { //Check for the returning code
     
 
        String payload = https.getString();
//        Serial.println(httpCode);
        Serial.println(payload);


        DynamicJsonBuffer jsonBuffer(200);
        JsonObject& root = jsonBuffer.parseObject(payload);
        if(!root.success()){
          Serial.println("parseObject() failed");
        }
        stateFromDB2 = root["CurentState"];
        if(!stateFromDB1 || stateFromDB1 != stateFromDB2) {
           changeCurtainState(stateFromDB2);
           stateFromDB1 = stateFromDB2;
        }
        
        delay(10000);
    }
    
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    https.end(); //Free the resources
  }


 
  delay(10000);
 
}
