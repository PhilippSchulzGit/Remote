#include <SPI.h>
//#include "printf.h"
#include "RF24.h"

// Acknowledge parts taken from https://nrf24.github.io/RF24/examples_2AcknowledgementPayloads_2AcknowledgementPayloads_8ino-example.html

// variables and constants to control cycle time
unsigned long old_timestamp = 0;
const long cycleTime = 1000;

// helper variables for changing analog signals
int rx_left_target = 1023;
int ry_left_target = 1023;
int rz_left_target = 1023;
int rx_right_target = -1023;
int ry_right_target = -1023;
int rz_right_target = -1023;

// variables for button states
bool button1 = 1;
bool button2 = 0;
bool button3 = 0;
bool button4 = 0;
bool button5 = 0;
bool button6 = 0;
// variables for switch states
bool switch1 = 1;
bool switch2 = 0;
bool switch3 = 0;
bool switch4 = 0;
bool switch5 = 0;
// variables for LEDs
bool led1 = 0;
bool led2 = 0;
bool led3 = 0;
bool led4 = 0;
bool led5 = 0;
// variables for joystick axes
int rx_left = -1023;
int ry_left = -682;
int rz_left = -341;
int rx_right = 1023;
int ry_right = 682;
int rz_right = 341;
// structure for data to send to client device
struct SEND_DATA_STRUCTURE{
    bool button1;  
    bool button2;    
    bool button3;  
    bool button4;  
    bool button5; 
    bool button6;
    bool switch1;
    bool switch2;  
    bool switch3;
    bool switch4;
    bool switch5;
    int16_t rx_left;
    int16_t ry_left;
    int16_t rz_left;
    int16_t rx_right;
    int16_t ry_right;
    int16_t rz_right;
};
// structure for data to receive from client device
struct RECEIVE_DATA_STRUCTURE{
  bool led1;
  bool led2;
  bool led3;
  bool led4;
  bool led5;
};
// create objects of structures
SEND_DATA_STRUCTURE dataToSend;
RECEIVE_DATA_STRUCTURE dataToRead;
// initialize the NRF24L01 chip
RF24 radio(2, 16); // CE, CSN
// addresses for sender and receiver
const byte addresses[][6] = {"01", "02"};

void changeVariables() {
  // cycle buttons
  if(button1) {
    button1 = 0;
    button2 = 1;
  } else if(button2) {
    button2 = 0;
    button3 = 1;
  } else if(button3) {
    button3 = 0;
    button4 = 1;
  } else if(button4) {
    button4 = 0;
    button5 = 1;
  } else if(button5) {
    button5 = 0;
    button6 = 1;
  } else if(button6) {
    button6 = 0;
    button1 = 1;
  }
  // cycle switches
  if(switch1) {
    switch1 = 0;
    switch2 = 1;
  } else if(switch2) {
    switch2 = 0;
    switch3 = 1;
  } else if(switch3) {
    switch3 = 0;
    switch4 = 1;
  } else if(switch4) {
    switch4 = 0;
    switch5 = 1;
  } else if(switch5) {
    switch5 = 0;
    switch1 = 1;
  }
  // adjust analog signals
  int steps = 5;
  if(rx_left_target > 0) {
    rx_left += steps;
    if(rx_left <= rx_left_target) {
      rx_left_target *= -1;
    }
  } else {
    rx_left -= steps;
    if(rx_left >= rx_left_target) {
      rx_left_target *= -1;
    }
  }
  if(ry_left_target > 0) {
    ry_left += steps;
    if(ry_left <= ry_left_target) {
      ry_left_target *= -1;
    }
  } else {
    ry_left -= steps;
    if(ry_left >= ry_left_target) {
      ry_left_target *= -1;
    }
  }
  if(rz_left_target > 0) {
    rz_left += steps;
    if(rz_left <= rz_left_target) {
      rz_left_target *= -1;
    }
  } else {
    rz_left -= steps;
    if(rz_left >= rz_left_target) {
      rz_left_target *= -1;
    }
  }
  if(rx_right_target > 0) {
    rx_right += steps;
    if(rx_right <= rx_right_target) {
      rx_right_target *= -1;
    }
  } else {
    rx_right -= steps;
    if(rx_right >= rx_right_target) {
      rx_right_target *= -1;
    }
  }
  if(ry_right_target > 0) {
    ry_right += steps;
    if(ry_right <= ry_right_target) {
      ry_right_target *= -1;
    }
  } else {
    ry_right -= steps;
    if(ry_right >= ry_right_target) {
      ry_right_target *= -1;
    }
  }
  if(rz_right_target > 0) {
    rz_right += steps;
    if(rz_right <= rz_right_target) {
      rz_right_target *= -1;
    }
  } else {
    rz_right -= steps;
    if(rz_right >= rz_right_target) {
      rz_right_target *= -1;
    }
  }
}

void setPayloadContent() {
  dataToSend.button1 = button1;
  dataToSend.button2 = button2;
  dataToSend.button3 = button3;
  dataToSend.button4 = button4;
  dataToSend.button5 = button5;
  dataToSend.button6 = button6;
  dataToSend.switch1 = switch1;
  dataToSend.switch2 = switch2;
  dataToSend.switch3 = switch3;
  dataToSend.switch4 = switch4;
  dataToSend.switch5 = switch5;
  dataToSend.rx_left = rx_left;
  dataToSend.ry_left = ry_left;
  dataToSend.rz_left = rz_left;
  dataToSend.rx_right = rx_right;
  dataToSend.ry_right = ry_right;
  dataToSend.rz_right = rz_right;
}

void getPayloadContent() {
  led1 = dataToRead.led1;
  led2 = dataToRead.led2;
  led3 = dataToRead.led3;
  led4 = dataToRead.led4;
  led5 = dataToRead.led5;
}

void setup() {
  // start Serial for debugging
  Serial.begin(115200);
  // code for NRF24L01
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(addresses[1]); // 01
  radio.openReadingPipe(1, addresses[0]); // 02
  setPayloadContent();  // set the payload
  radio.stopListening();
}

void loop() {
  // check if cycle time has been reached
  if (millis()-old_timestamp >= cycleTime) { 
    Serial.println("I am god. I shall enlighten you.");
    setPayloadContent();  // set the payload
    // save current timestamp for next cycle
    old_timestamp = micros();
    // write data to client device
    radio.write(&dataToSend, sizeof(SEND_DATA_STRUCTURE));  
    // print cycle time to Serial
    Serial.print(micros()-old_timestamp); 
    Serial.println(" us to send");       
    // check if radio has an ACK payload
    uint8_t pipe; // pipe number that receives the ACK
    if(radio.available(&pipe)) {
      old_timestamp = micros();
      // read Acknowledge from client device
      radio.read(&dataToRead,sizeof(dataToRead));
    Serial.print(micros()-old_timestamp); 
    Serial.println(" us to read"); 
    Serial.print("LED states: ");
    Serial.print(String(dataToRead.led1)+" ");
    Serial.print(String(dataToRead.led2)+" ");
    Serial.print(String(dataToRead.led3)+" ");
    Serial.print(String(dataToRead.led4)+" ");
    Serial.println(String(dataToRead.led5));
    } else {
      Serial.println("not available :(");
    }
    // adjust variables so that their transmission can be tested
    changeVariables();    
    old_timestamp = millis();  
  }       
}
