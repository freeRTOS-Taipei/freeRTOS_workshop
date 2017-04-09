#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void Task1 (void *Parameter);
void Task2 (void *Parameter);

void setup() {
	Serial.begin(115200);
	xTaskCreate(Task1, "Task1", 1000, NULL, 1, NULL);
	xTaskCreate(Task2, "Task2", 1000, NULL, 1, NULL);
}

int counter = 0;
void loop() {
	/*Do nothing*/
}

void Task1(void *Parameter)
{
	while(1)
	{
		Serial.print("Task 1\r\n");
		delay(500);
	}
}

void Task2(void *Parameter)
{
	while(1)
	{
		Serial.print("Task 2\r\n");
		delay(500);
	}
}
