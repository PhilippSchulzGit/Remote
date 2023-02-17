#include "Arduino.h"
#include "ADS1115-Driver.h"

ADS1115 stickLeft = ADS1115(0x48);
ADS1115 stickRight = ADS1115(0x49);

uint16_t readValueLeft(uint8_t input) {
	stickLeft.setMultiplexer(input);
	stickLeft.startSingleConvertion();

	delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 uS to do that

	while (stickLeft.getOperationalStatus() == 0);

	return stickLeft.readConvertedValue();
}
uint16_t readValueRight(uint8_t input) {
  stickRight.setMultiplexer(input);
  stickRight.startSingleConvertion();

  delayMicroseconds(25); // The ADS1115 needs to wake up from sleep mode and usually it takes 25 uS to do that

  while (stickRight.getOperationalStatus() == 0);

  return stickRight.readConvertedValue();
}

void setup() {
	Serial.begin(9600);

	stickLeft.reset();
	stickLeft.setDeviceMode(ADS1115_MODE_SINGLE);
	stickLeft.setDataRate(ADS1115_DR_250_SPS);
	stickLeft.setPga(ADS1115_PGA_4_096);
  stickRight.reset();
  stickRight.setDeviceMode(ADS1115_MODE_SINGLE);
  stickRight.setDataRate(ADS1115_DR_250_SPS);
  stickRight.setPga(ADS1115_PGA_4_096);
}

void loop() {
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
}
