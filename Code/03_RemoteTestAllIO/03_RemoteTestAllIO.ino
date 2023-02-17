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

void setup() {
	Serial.begin(9600);
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
  // test order of LEDs         ###works### 1 is left, 5 is right
  /**
  mcp.digitalWrite(LED_5,LOW);
  mcp.digitalWrite(LED_1,HIGH);
  delay(500);
  mcp.digitalWrite(LED_1,LOW);
  mcp.digitalWrite(LED_2,HIGH);
  delay(500);
  mcp.digitalWrite(LED_2,LOW);
  mcp.digitalWrite(LED_3,HIGH);
  delay(500);
  mcp.digitalWrite(LED_3,LOW);
  mcp.digitalWrite(LED_4,HIGH);
  delay(500);
  mcp.digitalWrite(LED_4,LOW);
  mcp.digitalWrite(LED_5,HIGH);
  delay(400);
  **/
  
  // testing joystick buttons   ###works###
  /**
  mcp.digitalWrite(LED_1,!mcp.digitalRead(STICK_BUTTON_L));
  mcp.digitalWrite(LED_5,!mcp.digitalRead(STICK_BUTTON_R));
  **/
  
  // testing push buttons       ###works###
  /**
  mcp.digitalWrite(LED_1,!mcp.digitalRead(BUTTON_R1));
  mcp.digitalWrite(LED_2,!mcp.digitalRead(BUTTON_R2));
  mcp.digitalWrite(LED_3,!mcp.digitalRead(BUTTON_L1));
  mcp.digitalWrite(LED_4,!mcp.digitalRead(BUTTON_L2));
  **/
  
  // testing switches           ###works#### reading switches inverted in second write argument
  /**
  mcp.digitalWrite(LED_1,!mcp.digitalRead(SW_1));
  mcp.digitalWrite(LED_2,!mcp.digitalRead(SW_2));
  mcp.digitalWrite(LED_3,!mcp.digitalRead(SW_3));
  mcp.digitalWrite(LED_4,!mcp.digitalRead(SW_4));
  mcp.digitalWrite(LED_5,!mcp.digitalRead(SW_5));
  **/
  
  // testing joysticks          ###works### NEED OFFSETS THOUGH
  /**
  uint16_t A1L = readValueLeft(ADS1115_MUX_AIN1_GND);
  uint16_t A2L = readValueLeft(ADS1115_MUX_AIN2_GND);
  uint16_t A3L = readValueLeft(ADS1115_MUX_AIN3_GND);
  uint16_t A1R = readValueRight(ADS1115_MUX_AIN1_GND);
  uint16_t A2R = readValueRight(ADS1115_MUX_AIN2_GND);
  uint16_t A3R = readValueRight(ADS1115_MUX_AIN3_GND);
  float l1 = (float)map(A1L,0,4095,-1000,1000) / 1000.0;  // Ry
  float l2 = (float)map(A2L,0,4095,1000,-1000) / 1000.0;  // Rx
  float l3 = (float)map(A3L,0,4095,-1000,1000) / 1000.0;  // Rz
  float r1 = (float)map(A1R,0,4095,-1000,1000) / 1000.0;  // Ry
  float r2 = (float)map(A2R,0,4095,1000,-1000) / 1000.0;  // Rx
  float r3 = (float)map(A3R,0,4095,-1000,1000) / 1000.0;  // Rz
  
  Serial.print("L1: ");
  Serial.print(l1);
  Serial.print(" L2: ");
  Serial.print(l2);
  Serial.print(" L3: ");
  Serial.print(l3);
  
  Serial.print(" R1: ");
  Serial.print(r1);
  Serial.print(" R2: ");
  Serial.print(r2);
  Serial.print(" R3: ");
  Serial.println(r3);
  **/

  delay(100);
  
  // OLD LOOP FROM ADC tests
  /**
	uint16_t A0L = readValueLeft(ADS1115_MUX_AIN0_GND);
	uint16_t A1L = readValueLeft(ADS1115_MUX_AIN1_GND);
	uint16_t A2L = readValueLeft(ADS1115_MUX_AIN2_GND);
	uint16_t A3L = readValueLeft(ADS1115_MUX_AIN3_GND);
  uint16_t A0R = readValueRight(ADS1115_MUX_AIN0_GND);
  uint16_t A1R = readValueRight(ADS1115_MUX_AIN1_GND);
  uint16_t A2R = readValueRight(ADS1115_MUX_AIN2_GND);
  uint16_t A3R = readValueRight(ADS1115_MUX_AIN3_GND);

	Serial.print("A0L: ");
	Serial.print(A0L);
	Serial.print(" A1L: ");
	Serial.print(A1L);
	Serial.print(" A2L: ");
	Serial.print(A2L);
	Serial.print(" A3L: ");
	Serial.print(A3L);
  
  Serial.print(" A0R: ");
  Serial.print(A0R);
  Serial.print(" A1R: ");
  Serial.print(A1R);
  Serial.print(" A2R: ");
  Serial.print(A2R);
  Serial.print(" A3R: ");
  Serial.println(A3R);

	delay(100);
 **/
}
