#include <ESP8266WiFi.h>
#include <RemoteCredentials.h>

WiFiClient client;

const char * host = "192.168.4.1";
const int httpPort = 80;

const long timeout_ms = 5000;
const long cycle_time_ms = 5000;
const long http_timeout = 300;

const char *ssid = REMOTE_SSID;
const char *password = REMOTE_PASSWORD;

bool switch1 = 0;
bool switch2 = 0;
bool switch3 = 0;
bool switch4 = 0;
bool switch5 = 0;
bool led1 = 0;
bool led2 = 0;
bool led3 = 0;
bool led4 = 0;
bool led5 = 0;
int stepCounter = 0;

void cycleSwitches() {
  switch(stepCounter) {
    case 0:
      switch1 = 1;
      switch5 = 0;
      stepCounter++;
      break;
    case 1:
      switch1 = 0;
      switch2 = 1;
      stepCounter++;
      break;
    case 2:
      switch2 = 0;
      switch3 = 1;
      stepCounter++;
      break;
    case 3:
      switch3 = 0;
      switch4 = 1;
      stepCounter++;
      break;
    case 4:
      switch4 = 0;
      switch5 = 1;
      stepCounter = 0;
      break;
  }
}

void sendPackages() {
  String url = "/data/sending/";
  url += "?s1=";
  url += String(switch1);
  url += "&s2=";
  url += String(switch2);
  url += "&s3=";
  url += String(switch3);
  url += "&s4=";
  url += String(switch4);
  url += "&s5=";
  url += String(switch5);
  Serial.println("trying to send: "+String("GET ")+url+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n\r\n");
  client.print(String("GET ")+url+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n\r\n");
  long timeout = millis();
  
  while(client.available() == 0) {
    int answer = client.read();
    if(millis() - timeout > timeout_ms) {
      Serial.println(">>> Client Timeout! SendPackages");
      client.stop();
      return;
    }
  }
}

void getPackages() {
  String url = "/data/receiving/";
  url += "?l1=";
  url += String(led1);
  url += "&l2=";
  url += String(led2);
  url += "&l3=";
  url += String(led3);
  url += "&l4=";
  url += String(led4);
  url += "&l5=";
  url += String(led5);
  Serial.println("trying to send: "+String("GET ")+url+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n\r\n");
  client.print(String("GET ")+url+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n\r\n");
  long timeout = millis();
  String line = client.readStringUntil('#');
  line = line.substring(line.indexOf('?'));
  Serial.println("temporary time: "+String(millis()-timeout));
  //Serial.println("response from server:");
  Serial.println(line);
  //Serial.println("response end.");
  /**
  while(client.available() == 0) {
    int answer = client.read();
    Serial.println(answer);
    if(millis() - timeout > timeout_ms) {
      Serial.println(">>> Client Timeout! GetPackages");
      client.stop();
      return;
    }
  }**/
}

void exchangePackages() {
  // send data first
  String url = "/data/sending/";
  url += "?s1=";
  url += String(switch1);
  url += "&s2=";
  url += String(switch2);
  url += "&s3=";
  url += String(switch3);
  url += "&s4=";
  url += String(switch4);
  url += "&s5=";
  url += String(switch5);
  Serial.println("trying to send: "+String("GET ")+url+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n\r\n");
  client.print(String("GET ")+url+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: keep-alive\r\n\r\n");

  // get data back
  String line = client.readStringUntil('#');
  
  //line = line.substring(line.indexOf('?'));
  Serial.println(line);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  // set up client
  client.setTimeout(http_timeout);
  // WiFi
  Serial.println("setting up WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected to WIFI!");
}

void loop() {
  if(!client.connect(host,httpPort)) {
    Serial.println("connection failed, couldn't connect to client.");
    return;
  }
  Serial.println("Start Transfer");
  long startTime = millis();
  exchangePackages();
  long stopTime = millis();
  Serial.println("Transfer done, took "+String(stopTime-startTime)+" ms");
  cycleSwitches();
  Serial.println(String(led1)+" "+String(led2)+" "+String(led3)+" "+String(led4)+" "+String(led5));
  Serial.println(String(switch1)+" "+String(switch2)+" "+String(switch3)+" "+String(switch4)+" "+String(switch5));
  Serial.println("---");
  delay(cycle_time_ms);
}
