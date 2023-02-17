#include <SPI.h>
//#include <nRF24L01.h>
#include <RF24.h>

// Acknowledge parts taken from https://nrf24.github.io/RF24/examples_2AcknowledgementPayloads_2AcknowledgementPayloads_8ino-example.html

// variables for button states
bool button1 = 0;
bool button2 = 0;
bool button3 = 0;
bool button4 = 0;
bool button5 = 0;
bool button6 = 0;
// variables for switch states
bool switch1 = 0;
bool switch2 = 0;
bool switch3 = 0;
bool switch4 = 0;
bool switch5 = 0;
// variables for LEDs
bool led1 = 1;
bool led2 = 0;
bool led3 = 0;
bool led4 = 0;
bool led5 = 0;
// variables for joystick axes
int rx_left = 0;
int ry_left = 0;
int rz_left = 0;
int rx_right = 0;
int ry_right = 0;
int rz_right = 0;
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
  // cycle LEDs
  if(led1) {
    led1 = 0;
    led2 = 1;
  } else if(led2) {
    led2 = 0;
    led3 = 1;
  } else if(led3) {
    led3 = 0;
    led4 = 1;
  } else if(led4) {
    led4 = 0;
    led5 = 1;
  } else if(led5) {
    led5 = 0;
    led1 = 1;
  }
}

void setPayloadContent() {
  dataToRead.led1 = led1;
  dataToRead.led2 = led2;
  dataToRead.led3 = led3;
  dataToRead.led4 = led4;
  dataToRead.led5 = led5;
}

void getPayloadContent() {
  button1 = dataToSend.button1;
  button2 = dataToSend.button2;
  button3 = dataToSend.button3;
  button4 = dataToSend.button4;
  button5 = dataToSend.button5;
  button6 = dataToSend.button6;
  switch1 = dataToSend.switch1;
  switch2 = dataToSend.switch2;
  switch3 = dataToSend.switch3;
  switch4 = dataToSend.switch4;
  switch5 = dataToSend.switch5;
  rx_left = dataToSend.rx_left;
  ry_left = dataToSend.ry_left;
  rz_left = dataToSend.rz_left;
  rx_right = dataToSend.rx_right;
  ry_right = dataToSend.ry_right;
  rz_right = dataToSend.rz_right;
}

void setup() {  
  // start Serial for debugging
  Serial.begin(115200);
  // code for NRF24L01
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(addresses[0]); // 02
  radio.openReadingPipe(1, addresses[1]); // 01
  setPayloadContent();
  // load the payload for the first received transmission on pipe 0
  radio.writeAckPayload(1, &dataToRead, sizeof(dataToRead));
  radio.startListening();  // put radio in RX mode
  Serial.println("hemlo am alive");
}

void loop() {
  uint8_t pipe;
  // check if radio has a new payload
  if (radio.available(&pipe)) { 
    Serial.println("I am Moses. Enlighten me, God.");
    // get dynamic payload size
    uint8_t bytes = radio.getDynamicPayloadSize();
    // read data
    radio.read(&dataToSend, sizeof(dataToSend));
    Serial.println("I got an update!");
    Serial.print("Button states: ");
    Serial.print(String(dataToSend.button1)+" ");
    Serial.print(String(dataToSend.button2)+" ");
    Serial.print(String(dataToSend.button3)+" ");
    Serial.print(String(dataToSend.button4)+" ");
    Serial.print(String(dataToSend.button5)+" ");
    Serial.println(String(dataToSend.button6));
    Serial.print("Switch states: ");
    Serial.print(String(dataToSend.switch1)+" ");
    Serial.print(String(dataToSend.switch2)+" ");
    Serial.print(String(dataToSend.switch3)+" ");
    Serial.print(String(dataToSend.switch4)+" ");
    Serial.println(String(dataToSend.switch5)+" ");
    Serial.print("Stick states: ");
    Serial.print(String(dataToSend.rx_left)+" ");
    Serial.print(String(dataToSend.ry_left)+" ");
    Serial.print(String(dataToSend.rz_left)+" ");
    Serial.print(String(dataToSend.rx_right)+" ");
    Serial.print(String(dataToSend.ry_right)+" ");
    Serial.println(String(dataToSend.rz_right)+" ");
    // handle payload
    getPayloadContent();
    changeVariables();
    setPayloadContent();
    // send ack ACK payload
    radio.writeAckPayload(1,&dataToRead,sizeof(dataToRead));
  }
}
