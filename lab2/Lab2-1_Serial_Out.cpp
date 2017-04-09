#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void setup() {
	Serial.begin(115200);
}

int counter = 0;
void loop() {
	Serial.print("Hello Arduino~");
	Serial.println(counter++);
	delay(200);
}
