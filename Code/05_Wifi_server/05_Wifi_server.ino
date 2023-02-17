#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RemoteCredentials.h>

const char *ssid = REMOTE_SSID;
const char *password = REMOTE_PASSWORD;

ESP8266WebServer server(80);

const long cycle_time_ms = 5000;

bool led1 = 0;
bool led2 = 0;
bool led3 = 0;
bool led4 = 0;
bool led5 = 0;
bool switch1 = 0;
bool switch2 = 0;
bool switch3 = 0;
bool switch4 = 0;
bool switch5 = 0;
int stepCounter = 0;
long last_time = 0;

void cycleLEDs() {
  switch(stepCounter) {
    case 0:
      led1 = 1;
      led5 = 0;
      stepCounter++;
      break;
    case 1:
      led1 = 0;
      led2 = 1;
      stepCounter++;
      break;
    case 2:
      led2 = 0;
      led3 = 1;
      stepCounter++;
      break;
    case 3:
      led3 = 0;
      led4 = 1;
      stepCounter++;
      break;
    case 4:
      led4 = 0;
      led5 = 1;
      stepCounter = 0;
      break;
  }
}

void handleSentVar() {
  long startTime = millis();
  if(server.hasArg("s1")) {
    switch1 = server.arg("s1").toInt();
  }
  if(server.hasArg("s2")) {
    switch2 = server.arg("s2").toInt();
  }
  if(server.hasArg("s3")) {
    switch3 = server.arg("s3").toInt();
  }
  if(server.hasArg("s4")) {
    switch4 = server.arg("s4").toInt();
  }
  if(server.hasArg("s5")) {
    switch5 = server.arg("s5").toInt();
  }
  String answer = "?";
  answer += "l1=";
  answer += String(led1);
  answer += "&l2=";
  answer += String(led2);
  answer += "&l3=";
  answer += String(led3);
  answer += "&l4=";
  answer += String(led4);
  answer += "&l5=";
  answer += String(led5);
  answer += "#";
  server.send(200,"text/plain",answer);
  //Serial.println("I took "+String(millis()-startTime)+" ms to answer"); // usually 2-3 ms
}

void handleGetVar() {
  String answer = "?";
  answer += "l1=";
  answer += String(led1);
  answer += "&l2=";
  answer += String(led2);
  answer += "&l3=";
  answer += String(led3);
  answer += "&l4=";
  answer += String(led4);
  answer += "&l5=";
  answer += String(led5);
  answer += "#";
  /**
  String answer = "?";
  if(server.hasArg("l1")) {
    answer += "l1=";
    answer += String(led1);
  }
  if(server.hasArg("l2")) {
    if(answer.length()>1) {
      answer += "&";
    }
    answer += "l2=";
    answer += String(led2);
  }
  if(server.hasArg("l3")) {
    if(answer.length()>1) {
      answer += "&";
    }
    answer += "l3=";
    answer += String(led3);
  }
  if(server.hasArg("l4")) {
    if(answer.length()>1) {
      answer += "&";
    }
    answer += "l4=";
    answer += String(led4);
  }
  if(server.hasArg("l5")) {
    if(answer.length()>1) {
      answer += "&";
    }
    answer += "l5=";
    answer += String(led5);
  }
  answer += "#";
  //server.sendContent(answer);
  **/
  server.send(200,"text/plain",answer);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  // WiFi
  WiFi.mode(WIFI_AP);
  WiFi.setOutputPower(10);
  WiFi.softAP(ssid,password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);
  server.on("/data/sending/", HTTP_GET, handleSentVar);
  //server.on("/data/receiving/", HTTP_GET, handleGetVar);
  server.begin();
  last_time = millis();
}

void loop() {
  server.handleClient();
  if(millis()-last_time>cycle_time_ms) {
    cycleLEDs();
    Serial.println("Server report: ");
    Serial.println("LEDs:(sent) "+String(led1)+" "+String(led2)+" "+String(led3)+" "+String(led4)+" "+String(led5));
    Serial.println("Switches:   "+String(switch1)+" "+String(switch2)+" "+String(switch3)+" "+String(switch4)+" "+String(switch5));
    last_time = millis();
  }
}
