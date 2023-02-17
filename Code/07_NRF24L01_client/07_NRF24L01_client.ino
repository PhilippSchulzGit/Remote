/*
  NRF24L01_client

  Dummy client for receiving data from the remote running the program "RemoteProgramNRF24L01".
  

  Author: Philipp Schulz
  Created on Wed, 08 February 2023, 16:53 CET
 */
#include <SPI.h>
#include <RF24.h>

// variables for all IO
const byte leds[] = {1,1,1,1,1}; // led1, led2, led3, led4, led5
byte switches[] = {0,0,0,0,0};   // sw1, sw2, sw3, sw4, sw5
byte buttons[] = {0,0,0,0,0,0};  // L1, L2, StickL, R1, R2, StickR
int axes[] = {0,0,0,0,0,0};      // Lx, Ly, Lz, Rx, Ry, Rz

// structure for data to send to client device
struct SEND_DATA_STRUCTURE{
    bool button1;  
    bool button2;    
    bool button3; // stick left
    bool button4;  
    bool button5; 
    bool button6; // stick right
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
// scaling factor for plotting
float scalingFactor = 500;

// method for setting the data from global variables to be sent
void setPayloadContent() {
  dataToRead.led1 = leds[0];
  dataToRead.led2 = leds[1];
  dataToRead.led3 = leds[2];
  dataToRead.led4 = leds[3];
  dataToRead.led5 = leds[4];
}

// method for getting the received data to global variables
void getPayloadContent() {
  buttons[0] = dataToSend.button1;
  buttons[1] = dataToSend.button2;
  buttons[2] = dataToSend.button3; // stick left
  buttons[3] = dataToSend.button4;
  buttons[4] = dataToSend.button5;
  buttons[5] = dataToSend.button6; // stick right
  switches[0] = dataToSend.switch1;
  switches[1] = dataToSend.switch2;
  switches[2] = dataToSend.switch3;
  switches[3] = dataToSend.switch4;
  switches[4] = dataToSend.switch5;
  axes[0] = dataToSend.rx_left;
  axes[1] = dataToSend.ry_left;
  axes[2] = dataToSend.rz_left;
  axes[3] = dataToSend.rx_right;
  axes[4] = dataToSend.ry_right;
  axes[5] = dataToSend.rz_right;
}

// method for handling all radio related code
void handleRadio() {
  uint8_t pipe;
  // check if radio has a new payload
  if (radio.available(&pipe)) { 
    // get dynamic payload size
    uint8_t bytes = radio.getDynamicPayloadSize();
    // read data
    radio.read(&dataToSend, sizeof(dataToSend));
    // handle payload
    //getPayloadContent();
    setPayloadContent();
    // send ack ACK payload
    radio.writeAckPayload(1,&dataToRead,sizeof(dataToRead));
    // for Serial plotter
    Serial.print("b1: "+String(buttons[0]*scalingFactor)+",");
    Serial.print("b2: "+String(buttons[1]*scalingFactor)+",");
    Serial.print("b3: "+String(buttons[2]*scalingFactor)+",");
    Serial.print("b4: "+String(buttons[3]*scalingFactor)+",");
    Serial.print("b5: "+String(buttons[4]*scalingFactor)+",");
    Serial.print("b6: "+String(buttons[5]*scalingFactor)+",");
    Serial.print("s1: "+String(switches[0]*scalingFactor)+",");
    Serial.print("s2: "+String(switches[1]*scalingFactor)+",");
    Serial.print("s3: "+String(switches[2]*scalingFactor)+",");
    Serial.print("s4: "+String(switches[3]*scalingFactor)+",");
    Serial.print("s5: "+String(switches[4]*scalingFactor)+",");
    Serial.print("rx_l: "+String(axes[0])+",");
    Serial.print("ry_l: "+String(axes[1])+",");
    Serial.print("rz_l: "+String(axes[2])+",");
    Serial.print("rx_r: "+String(axes[3])+",");
    Serial.print("ry_r: "+String(axes[4])+",");
    Serial.println("rz_r: "+String(axes[5]));
  }
}

void setup() {  
  // start Serial for debugging
  Serial.begin(115200);
  // code for NRF24L01
  radio.begin();
  radio.setPALevel(RF24_PA_LOW); //RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(addresses[0]); // 02
  radio.openReadingPipe(1, addresses[1]); // 01
  setPayloadContent();
  // load the payload for the first received transmission on pipe 0
  radio.writeAckPayload(1, &dataToRead, sizeof(dataToRead));
  radio.startListening();  // put radio in RX mode
}

void loop() {
  handleRadio();
}
