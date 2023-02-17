/*
  RemoteProgramNRF24L01

  Reads all inputs from the Hexapod remote and sends them via radio to a device.
  The cycle time can be adjusted by the constant "cycleTime".
  As for how to write the code for the receiving device, please refer to the program "NRF24L01_client".
  The LEDs are cycled with the cycle time "ledCycleTime" unless a connection is made.

  This version adds a timeout counter before showing that the connection was lost.
  This version uses optimized data structures to reduce the amount of data being sent and received.

  Author: Philipp Schulz
  Created on Thu, 09 February 2023, 07:00 CET
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

// flag for enabling or disabling Serial prints for debugging
const bool debugging = 0;

// timeout counter for remote to wait before LED cycling
const int timeoutCount = 10;
int timeoutCounter = 0;
// cycle time between sending values [ms]
const int cycleTime = 50;
const int ledCycleTime = 250; // cycle time for indicating no connection in [ms]
unsigned long startTime = 0; // for input reading measurements
unsigned long measurementOld = 0; // sending measurements
unsigned long measurementOldMid = 0;  // sending and receiving measurements
unsigned long oldTimestamp = 0;
unsigned long cyclingTimestamp = 0;
// variables for all IO
byte leds[] = {1,0,0,0,0};       // led1, led2, led3, led4, led5
byte switches[] = {0,0,0,0,0};   // sw1, sw2, sw3, sw4, sw5
byte buttons[] = {0,0,0,0,0,0};  // L1, L2, StickL, R1, R2, StickR
int axes[] = {0,0,0,0,0,0};      // Lx, Ly, Lz, Rx, Ry, Rz
long axesRef[] = {0,0,0,0,0,0};   // Lx, Ly, Lz, Rx, Ry, Rz, references for zero position

// structure for data to send to client device
struct SEND_DATA_STRUCTURE_OPTIMIZED{
    byte buttons;
    byte switches;
    int16_t rx_left;
    int16_t ry_left;
    int16_t rz_left;
    int16_t rx_right;
    int16_t ry_right;
    int16_t rz_right;
};
// structure for data to receive from client device
struct RECEIVE_DATA_STRUCTURE_OPTIMIZED{
  byte leds;
};
// create objects of structures
SEND_DATA_STRUCTURE_OPTIMIZED dataToSend;
RECEIVE_DATA_STRUCTURE_OPTIMIZED dataToRead;
// initialize the NRF24L01 chip
RF24 radio(2, 16); // CE, CSN
// addresses for sender and receiver
const byte addresses[][6] = {"01", "02"};

// method for setting the data from global variables to be sent
void setPayloadContent() {
  // code for bool to byte transfer from https://forum.arduino.cc/t/bool-array-to-bytes/612140/4
  // buttons
  byte buttonsByte = 0;
  for(int i=0;i<6;i++) {
    if(buttons[i]) {
      buttonsByte |= (1 << (7-i));
    }
  }
  dataToSend.buttons = buttonsByte;
  // switches
  byte switchesByte = 0;
  for(int i=0;i<6;i++) {
    if(switches[i]) {
      switchesByte |= (1 << (7-i));
    }
  }
  dataToSend.switches = switchesByte;
  // joystick values
  dataToSend.rx_left = axes[0];
  dataToSend.ry_left = axes[1];
  dataToSend.rz_left = axes[2];
  dataToSend.rx_right = axes[3];
  dataToSend.ry_right = axes[4];
  dataToSend.rz_right = axes[5];
}

// method for getting the received data to global variables
void getPayloadContent() {
  for(int i=0;i<5;i++) {
    leds[i] = bitRead(dataToRead.leds, (7-i));
  }
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
  // check if debugging
  if(debugging) {
    startTime = micros();
  }
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
  /**
  axes[0] = map(readValueLeft(ADS1115_MUX_AIN2_GND)-axesRef[0],0,4095,1024,-1024);  // Rx left
  axes[1] = map(readValueLeft(ADS1115_MUX_AIN1_GND)-axesRef[1],0,4095,-1024,1024);  // Ry left
  axes[2] = map(readValueLeft(ADS1115_MUX_AIN3_GND)-axesRef[2],0,4095,-1024,1024);  // Rz left
  axes[3] = map(readValueRight(ADS1115_MUX_AIN2_GND)-axesRef[3],0,4095,1024,-1024);  // Rx right
  axes[4] = map(readValueRight(ADS1115_MUX_AIN1_GND)-axesRef[4],0,4095,-1024,1024);  // Ry right
  axes[5] = map(readValueRight(ADS1115_MUX_AIN3_GND)-axesRef[5],0,4095,-1024,1024);  // Rz right
  **/
  axes[0] = joystickLinearModels(readValueLeft(ADS1115_MUX_AIN1_GND),axesRef[1],0,0,4095,-1024,1024);  // Rx left
  axes[1] = joystickLinearModels(readValueLeft(ADS1115_MUX_AIN2_GND),axesRef[0],0,0,4095,1024,-1024);  // Ry left
  axes[2] = joystickLinearModels(readValueLeft(ADS1115_MUX_AIN3_GND),axesRef[2],0,0,4095,1024,-1024);  // Rz left
  axes[3] = joystickLinearModels(readValueRight(ADS1115_MUX_AIN1_GND),axesRef[4],0,0,4095,-1024,1024);  // Rx right
  axes[4] = joystickLinearModels(readValueRight(ADS1115_MUX_AIN2_GND),axesRef[3],0,0,4095,1024,-1024);  // Ry right
  axes[5] = joystickLinearModels(readValueRight(ADS1115_MUX_AIN3_GND),axesRef[5],0,0,4095,1024,-1024);  // Rz right
  // check if debugging
  if(debugging) {
    // print to console for cycle time measurements
    Serial.println("Measurement time [us]: "+String(micros()-startTime));
  }
  // handle buttons for resetting joystick refs
  if(buttons[0] && buttons[1] && buttons[3] && buttons[4]) {
    setJoystickRef();
  }
}

// method for translating a given value from the original range to the new range
int joystickLinearModels(int value, int xRef, int yRef, int xMin, int xMax, int yMin, int yMax) {
  double m = 0;
  if(value <= xRef) {
    m = (float(yRef)-float(yMin))/(float(xRef)-float(xMin));
  } else {
    m = (float(yMax)-float(yRef))/(float(xMax)-float(xRef));
  }
  return (int)(m*(float(value)-float(xRef))+float(yRef));
}

// method for setting all LEDs to new states
void setAllLEDs() {
  mcp.digitalWrite(LED_1,leds[0]);
  mcp.digitalWrite(LED_2,leds[1]);
  mcp.digitalWrite(LED_3,leds[2]);
  mcp.digitalWrite(LED_4,leds[3]);
  mcp.digitalWrite(LED_5,leds[4]);
  // check if debugging
  if(debugging) {
    // ------DEBUGGING---------
    Serial.print("LEDs: ");
    for(int i=0;i<5;i++) {
      Serial.print(String(leds[i])+" ");
    }
    Serial.println("");
    // ------DEBUGGING---------
  }
}

// method for handling all radio related code
void handleRadio() {
  setPayloadContent();  // set the payload
  // check if debugging
  if(debugging) {
    // save current timestamp for next cycle for measuring cycle time
    measurementOld = micros();
  }
  // write data to client device
  radio.write(&dataToSend, sizeof(dataToSend));  
  // check if debugging
  if(debugging) {
    // print cycle time to Serial
    measurementOldMid = micros();
    Serial.print(measurementOldMid-measurementOld);
    Serial.println(" us to send");
  }
  // check if radio has an ACK payload
  uint8_t pipe; // pipe number that receives the ACK
  if(radio.available(&pipe)) {
    // read Acknowledge from client device
    radio.read(&dataToRead,sizeof(dataToRead));
    getPayloadContent();
    // check if debugging
    if(debugging) {
      // print reading time to Serial
      Serial.print(micros()-measurementOldMid);
      Serial.println(" us to read");
    }
  } else {
    // if no answer, check timeout counter
    if(timeoutCounter >= timeoutCount) {
      if(debugging) {
        Serial.println("no answer, cycling LEDs");
      }
      // indicate connection loss via LEDs
      cycleLEDs();
    } else {
      // increment counter
      timeoutCounter++;
    }
  }
}

void setJoystickRef() {
  // reset ref values
  for(int i=0;i<6;i++) {
    axesRef[i] = 0;
  }
  // get offsets of joysticks for zero positions
  int refSamples = 5;
  for(int i=0;i<refSamples;i++) {
    axesRef[0] += readValueLeft(ADS1115_MUX_AIN2_GND);
    axesRef[1] += readValueLeft(ADS1115_MUX_AIN1_GND);
    axesRef[2] += readValueLeft(ADS1115_MUX_AIN3_GND);
    axesRef[3] += readValueRight(ADS1115_MUX_AIN2_GND);
    axesRef[4] += readValueRight(ADS1115_MUX_AIN1_GND);
    axesRef[5] += readValueRight(ADS1115_MUX_AIN3_GND);
  }
  for(int i=0;i<6;i++) {
    axesRef[i] /= refSamples;
  }
  
}


void setup() {
  // check if debugging
  if(debugging) {
    Serial.begin(115200);
  }
  // initialize left joystick
	stickLeft.reset();
	stickLeft.setDeviceMode(ADS1115_MODE_SINGLE);
	stickLeft.setDataRate(ADS1115_DR_475_SPS); // 8,16,32,64,128,250,475,860
	stickLeft.setPga(ADS1115_PGA_4_096); // 0_256,0_512,1_024,2_048,4_096,6_144
  // initialize right joystick
  stickRight.reset();
  stickRight.setDeviceMode(ADS1115_MODE_SINGLE);
  stickRight.setDataRate(ADS1115_DR_475_SPS); // 8,16,32,64,128,250,475,860
  stickRight.setPga(ADS1115_PGA_4_096); // 0_256,0_512,1_024,2_048,4_096,6_144
  // set ref values
  setJoystickRef();
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
  radio.setPALevel(RF24_PA_LOW); //RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
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
