#include "Arduino.h"
#include "ADS1115-Driver.h"
#include <Adafruit_MCP23X17.h>

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
// IO expander
Adafruit_MCP23X17 mcp;

// variables for all IO
byte leds[] = {0,0,0,0,0};       // led1, led2, led3, led4, led5
byte switches[] = {0,0,0,0,0};   // sw1, sw2, sw3, sw4, sw5
byte buttons[] = {0,0,0,0,0,0};  // L1, L2, StickL, R1, R2, StickR
byte axes[] = {0,0,0,0,0,0};    // Lx, Ly, Lz, Rx, Ry, Rz


// method for reading a value from the left ADS1115 board
uint16_t readValueLeft(uint8_t input) {
	stickLeft.setMultiplexer(input);
	stickLeft.startSingleConvertion();
	delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 uS to do that
	while (stickLeft.getOperationalStatus() == 0);
	return stickLeft.readConvertedValue();
}

// method for reading a value from the right ADS1115 board
uint16_t readValueRight(uint8_t input) {
  stickRight.setMultiplexer(input);
  stickRight.startSingleConvertion();
  delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 uS to do that
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
  // TODO: CALIBRATE ANALOG VALUES
  axes[0] = (float)map(readValueLeft(ADS1115_MUX_AIN2_GND),0,4095,1000,-1000) / 1000.0;  // Rx left
  axes[1] = (float)map(readValueLeft(ADS1115_MUX_AIN1_GND),0,4095,-1000,1000) / 1000.0;  // Ry left
  axes[2] = (float)map(readValueLeft(ADS1115_MUX_AIN3_GND),0,4095,-1000,1000) / 1000.0;  // Rz left
  axes[3] = (float)map(readValueRight(ADS1115_MUX_AIN2_GND),0,4095,1000,-1000) / 1000.0;  // Rx right
  axes[4] = (float)map(readValueRight(ADS1115_MUX_AIN1_GND),0,4095,-1000,1000) / 1000.0;  // Ry right
  axes[5] = (float)map(readValueRight(ADS1115_MUX_AIN3_GND),0,4095,-1000,1000) / 1000.0;  // Rz right
  // temporary print to console for cycle time measurements
  Serial.println("Cycle time [us]: "+String(micros()-startTime));
}

// method for setting all LEDs to new states
void setAllLEDs() {
  mcp.digitalWrite(LED_1,leds[0]);
  mcp.digitalWrite(LED_2,leds[1]);
  mcp.digitalWrite(LED_3,leds[2]);
  mcp.digitalWrite(LED_4,leds[3]);
  mcp.digitalWrite(LED_5,leds[4]);
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
}

void loop() {
  // read all inputs
  readAllValues();
  // set outputs
  setAllLEDs();
  delay(1000);
}
