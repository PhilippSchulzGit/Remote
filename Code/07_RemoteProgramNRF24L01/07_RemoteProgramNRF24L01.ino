/*
  RemoteProgramNRF24L01

  Reads all inputs from the Hexapod remote and sends them via radio to a device.
  The cycle time can be adjusted by the constant "cycleTime".
  As for how to write the code for the receiving device, please refer to the program "NRF24L01_client".
  The LEDs are cycled with the cycle time "ledCycleTime" unless a connection is made.

  //TODO: JOYSTICK MAPPING
  
  Author: Philipp Schulz
  Created on Wed, 08 February 2023, 07:27 CET
*/
// imports
#include "Arduino.h"
#include "ADS1115-Driver.h"
#include <Adafruit_MCP23X17.h>
#include <SPI.h>
#include "RF24.h"

// define all used pins
#define LED_1 7
#define LED_2 6
#define LED_3 5
#define LED_4 4
#define LED_5 3
#define BUTTON_R1 1
#define BUTTON_R2 2
#define STICK_BUTTON_R 0
#define SW_1 8
#define SW_2 9
#define SW_3 10
#define SW_4 11
#define SW_5 12
#define BUTTON_L1 13
#define BUTTON_L2 14
#define STICK_BUTTON_L 15

// ADC boards
ADS1115 stickLeft = ADS1115(0x48);
ADS1115 stickRight = ADS1115(0x49);
// IO expander object
Adafruit_MCP23X17 mcp;

// cycle time between sending values [ms]
int cycleTime = 50;
int ledCycleTime = 250; // cycle time for indicating no connection in [ms]
unsigned long oldTimestamp = 0;
unsigned long cyclingTimestamp = 0;
// variables for all IO
byte leds[] = {1,0,0,0,0};       // led1, led2, led3, led4, led5
byte switches[] = {0,0,0,0,0};   // sw1, sw2, sw3, sw4, sw5
byte buttons[] = {0,0,0,0,0,0};  // L1, L2, StickL, R1, R2, StickR
int axes[] = {0,0,0,0,0,0};    // Lx, Ly, Lz, Rx, Ry, Rz

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

// method for setting the data from gloval variables to be sent
void setPayloadContent() {
  dataToSend.button1 = buttons[0];
  dataToSend.button2 = buttons[1];
  dataToSend.button3 = buttons[2]; // stick left
  dataToSend.button4 = buttons[3];
  dataToSend.button5 = buttons[4];
  dataToSend.button6 = buttons[5]; // stick right
  dataToSend.switch1 = switches[0];
  dataToSend.switch2 = switches[1];
  dataToSend.switch3 = switches[2];
  dataToSend.switch4 = switches[3];
  dataToSend.switch5 = switches[4];
  dataToSend.rx_left = axes[0];
  dataToSend.ry_left = axes[1];
  dataToSend.rz_left = axes[2];
  dataToSend.rx_right = axes[3];
  dataToSend.ry_right = axes[4];
  dataToSend.rz_right = axes[5];
}

// method for getting the received data to global variables
void getPayloadContent() {
  leds[0] = dataToRead.led1;
  leds[1] = dataToRead.led2;
  leds[2] = dataToRead.led3;
  leds[3] = dataToRead.led4;
  leds[4] = dataToRead.led5;
}

// method for cycling the LEDs in case no connection is established
void cycleLEDs() {
  // get current timestamp
  unsigned long currentCycleTimestamp = millis();
  // check if delay has been reached to avoid the LEDs cycling too fast
  if(currentCycleTimestamp - cyclingTimestamp >= ledCycleTime) {
    // save timestamp for next cycle
    cyclingTimestamp = currentCycleTimestamp;
    byte onIndex = 0;
    // cycle the LEDs
     if(leds[0]) {
      onIndex = 1;
    } else if(leds[1]) {
      onIndex = 2;
    } else if(leds[2]) {
      onIndex = 3;
    } else if(leds[3]) {
      onIndex = 4;
    } else if(leds[4]) {
      onIndex = 0;
    }
    // reset qll LEDs
    for(int i=0;i < 5;i++) {
      leds[i] = 0;
    }
    // set current LED
    leds[onIndex] = 1;
  }
}

// method for reading a value from the left ADS1115 board
uint16_t readValueLeft(uint8_t input) {
	stickLeft.setMultiplexer(input);
	stickLeft.startSingleConvertion();
	delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 us to do that
	while (stickLeft.getOperationalStatus() == 0);
	return stickLeft.readConvertedValue();
}

// method for reading a value from the right ADS1115 board
uint16_t readValueRight(uint8_t input) {
  stickRight.setMultiplexer(input);
  stickRight.startSingleConvertion();
  delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 us to do that
  while (stickRight.getOperationalStatus() == 0);
  return stickRight.readConvertedValue();
}

// method for reading all values
void readAllValues() {
  long startTime = micros();
  // read states from switches
  switches[0] = !mcp.digitalRead(SW_1);
  switches[1] = !mcp.digitalRead(SW_2);
  switches[2] = !mcp.digitalRead(SW_3);
  switches[3] = !mcp.digitalRead(SW_4);
  switches[4] = !mcp.digitalRead(SW_5);
  // read states from buttons
  buttons[0] = !mcp.digitalRead(BUTTON_L1);
  buttons[1] = !mcp.digitalRead(BUTTON_L2);
  buttons[2] = !mcp.digitalRead(STICK_BUTTON_L);
  buttons[3] = !mcp.digitalRead(BUTTON_R1);
  buttons[4] = !mcp.digitalRead(BUTTON_R2);
  buttons[5] = !mcp.digitalRead(STICK_BUTTON_R);
  // read positions from joysticks
  // TODO: CALIBRATE ANALOG VALUES BASED ON RIGHT HAND RULE
  axes[0] = map(readValueLeft(ADS1115_MUX_AIN2_GND),0,4095,1024,-1024);  // Rx left
  axes[1] = map(readValueLeft(ADS1115_MUX_AIN1_GND),0,4095,-1024,1024);  // Ry left
  axes[2] = map(readValueLeft(ADS1115_MUX_AIN3_GND),0,4095,-1024,1024);  // Rz left
  axes[3] = map(readValueRight(ADS1115_MUX_AIN2_GND),0,4095,1024,-1024);  // Rx right
  axes[4] = map(readValueRight(ADS1115_MUX_AIN1_GND),0,4095,-1024,1024);  // Ry right
  axes[5] = map(readValueRight(ADS1115_MUX_AIN3_GND),0,4095,-1024,1024);  // Rz right
  // temporary print to console for cycle time measurements
  Serial.println("Measurement time [us]: "+String(micros()-startTime));
}

// method for setting all LEDs to new states
void setAllLEDs() {
  mcp.digitalWrite(LED_1,leds[0]);
  mcp.digitalWrite(LED_2,leds[1]);
  mcp.digitalWrite(LED_3,leds[2]);
  mcp.digitalWrite(LED_4,leds[3]);
  mcp.digitalWrite(LED_5,leds[4]);
  // ------DEBUGGING---------
  Serial.print("LEDs: ");
  for(int i=0;i<5;i++) {
    Serial.print(String(leds[i])+" ");
  }
  Serial.println("");
  // ------DEBUGGING---------
}

// method for handling all radio related code
void handleRadio() {
  setPayloadContent();  // set the payload
  // save current timestamp for next cycle for measuring cycle time
  unsigned long measurementOld = micros(); // --------DEBUGGING----------
  // write data to client device
  radio.write(&dataToSend, sizeof(SEND_DATA_STRUCTURE));  
  // print cycle time to Serial
  unsigned long measurementOldMid = micros();
  Serial.print(measurementOldMid-measurementOld);   // --------DEBUGGING----------
  Serial.println(" us to send");           // --------DEBUGGING----------
  // check if radio has an ACK payload
  uint8_t pipe; // pipe number that receives the ACK
  if(radio.available(&pipe)) {
    // read Acknowledge from client device
    radio.read(&dataToRead,sizeof(dataToRead));
    getPayloadContent();
  Serial.print(micros()-measurementOldMid);   // --------DEBUGGING----------
  Serial.println(" us to read");           // --------DEBUGGING----------
  } else {
    // if no answer, indicate via LEDs
    Serial.println("no answer, cycling LEDs");
    cycleLEDs();
  }
}

void setup() {
	Serial.begin(115200);
  // initialize left joystick
	stickLeft.reset();
	stickLeft.setDeviceMode(ADS1115_MODE_SINGLE);
	stickLeft.setDataRate(ADS1115_DR_250_SPS);
	stickLeft.setPga(ADS1115_PGA_4_096);
  // initialize right joystick
  stickRight.reset();
  stickRight.setDeviceMode(ADS1115_MODE_SINGLE);
  stickRight.setDataRate(ADS1115_DR_250_SPS);
  stickRight.setPga(ADS1115_PGA_4_096);
  // initialize IO expander
  mcp.begin_I2C(0x27);
  // configure pins
  mcp.pinMode(LED_1, OUTPUT);
  mcp.pinMode(LED_2, OUTPUT);
  mcp.pinMode(LED_3, OUTPUT);
  mcp.pinMode(LED_4, OUTPUT);
  mcp.pinMode(LED_5, OUTPUT);
  mcp.pinMode(BUTTON_R1, INPUT_PULLUP);
  mcp.pinMode(BUTTON_R2, INPUT_PULLUP);
  mcp.pinMode(STICK_BUTTON_R, INPUT_PULLUP);
  mcp.pinMode(SW_1, INPUT_PULLUP);
  mcp.pinMode(SW_2, INPUT_PULLUP);
  mcp.pinMode(SW_3, INPUT_PULLUP);
  mcp.pinMode(SW_4, INPUT_PULLUP);
  mcp.pinMode(SW_5, INPUT_PULLUP);
  mcp.pinMode(BUTTON_L1, INPUT_PULLUP);
  mcp.pinMode(BUTTON_L2, INPUT_PULLUP);
  mcp.pinMode(STICK_BUTTON_L, INPUT_PULLUP);
  // set up NRF24L01 for being the sender
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
  unsigned long currentTimestamp = millis();
  // check if cycle time has been reached
  if(currentTimestamp- oldTimestamp >= cycleTime) {
    // set old timestamp for next cycle
    oldTimestamp = currentTimestamp;
    // read all inputs
    readAllValues();
    // transfer values
    handleRadio();
    // set outputs
    setAllLEDs();
  }
}
