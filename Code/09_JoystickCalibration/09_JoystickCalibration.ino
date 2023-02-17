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
	Serial.begin(115200);
	stickLeft.reset();
	stickLeft.setDeviceMode(ADS1115_MODE_SINGLE);
	stickLeft.setDataRate(ADS1115_DR_475_SPS);
	stickLeft.setPga(ADS1115_PGA_4_096);
  stickRight.reset();
  stickRight.setDeviceMode(ADS1115_MODE_SINGLE);
  stickRight.setDataRate(ADS1115_DR_475_SPS);
  stickRight.setPga(ADS1115_PGA_4_096);
}

void loop() {
  uint16_t rxl = readValueLeft(ADS1115_MUX_AIN2_GND);
  uint16_t ryl = readValueLeft(ADS1115_MUX_AIN1_GND);
  uint16_t rzl = readValueLeft(ADS1115_MUX_AIN3_GND);
  uint16_t rxr = readValueRight(ADS1115_MUX_AIN2_GND);
  uint16_t ryr = readValueRight(ADS1115_MUX_AIN1_GND);
  uint16_t rzr = readValueRight(ADS1115_MUX_AIN3_GND);
  
  /**
  uint16_t rxl = map(readValueLeft(ADS1115_MUX_AIN2_GND),0,4095,1024,-1024);
	uint16_t ryl = map(readValueLeft(ADS1115_MUX_AIN1_GND),0,4095,-1024,1024);
	uint16_t rzl = map(readValueLeft(ADS1115_MUX_AIN3_GND),0,4095,-1024,1024);
  uint16_t rxr = map(readValueRight(ADS1115_MUX_AIN2_GND),0,4095,1024,-1024);
  uint16_t ryr = map(readValueRight(ADS1115_MUX_AIN1_GND),0,4095,-1024,1024);
  uint16_t rzr = map(readValueRight(ADS1115_MUX_AIN3_GND),0,4095,-1024,1024);
  **/
  
	Serial.print("rxl: ");
	Serial.print(rxl);
	Serial.print(" ryl: ");
	Serial.print(ryl);
	Serial.print(" rzl: ");
	Serial.print(rzl);
  Serial.print(" rxr: ");
  Serial.print(rxr);
  Serial.print(" ryr: ");
  Serial.print(ryr);
  Serial.print(" rzr: ");
  Serial.println(rzr);
	delay(100);
}
