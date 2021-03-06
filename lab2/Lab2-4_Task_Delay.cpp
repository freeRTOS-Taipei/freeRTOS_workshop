#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void TaskFunction (void *Parameter);
void vTaskDelay_ms(uint32_t ms);

void setup() {
	static const char *TextForTask1 = "Task 1 is running";
	static const char *TextForTask2 = "Task 2 is running";

	Serial.begin(115200);
	xTaskCreate(TaskFunction, "Task1", 1000, (void*)TextForTask1, 1, NULL);
	xTaskCreate(TaskFunction, "Task2", 1000, (void*)TextForTask2, 2, NULL);
}

int counter = 0;
void loop() {
	/*Do nothing*/
}

void TaskFunction(void *Parameter)
{
	char *print_string;
	print_string = (char*) Parameter;

	while(1)
	{
		Serial.println(print_string);
		vTaskDelay_ms(500);
	}
}

void vTaskDelay_ms(uint32_t ms)
{
	vTaskDelay(ms/portTICK_PERIOD_MS);
}