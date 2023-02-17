// Blinks an LED attached to a MCP23XXX pin.

// ok to include only the one needed
#include <Adafruit_MCP23X17.h>

#define LED_PIN 7     // MCP23XXX pin LED is attached to

// uncomment appropriate line
Adafruit_MCP23X17 mcp;

void setup() {
  Serial.begin(9600);
  Serial.println("MCP23xxx Blink Test!");

  if (!mcp.begin_I2C(0x27)) {
    Serial.println("Error.");
    while (1);
  }

  // configure pin for output
  mcp.pinMode(LED_PIN, OUTPUT);

  Serial.println("Looping...");
}

void loop() {
  mcp.digitalWrite(LED_PIN, HIGH);
  delay(500);
  mcp.digitalWrite(LED_PIN, LOW);
  delay(500);
}
