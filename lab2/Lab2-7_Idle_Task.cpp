#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void TaskFunction (void *Parameter);
void TaskPeriodic (void *Parameter);
void TaskDelay_ms(uint32_t ms);
void TaskDelayUntil_ms(TickType_t * const pxPreviousWakeTime, uint32_t ms);

void setup() {
	static const char *TextForTask1 = "Task 1 is running";
	static const char *TextForTask2 = "Task 2 is running";

	Serial.begin(115200);
	xTaskCreate(TaskFunction, "Task1", 1000, (void*)TextForTask1, 1, NULL);
	xTaskCreate(TaskFunction, "Task2", 1000, (void*)TextForTask2, 2, NULL);
	xTaskCreate(TaskPeriodic, "PeriodicTask", 1000, NULL, 3, NULL);
}

int counter = 0;
void loop() {
	/*Do nothing*/
}
unsigned long uIdleCycleCount = 0UL;
void vApplicationIdleHook(void)
{
	uIdleCycleCount++;
}

void TaskFunction(void *Parameter)
{
	char *print_string;
	TickType_t LastWakeTime;

	print_string = (char*) Parameter;

	while(1)
	{
		Serial.println(print_string);
		TaskDelay_ms(200);
	}
}

void TaskPeriodic(void *Parameter)
{
	char *print_string;
	TickType_t LastWakeTime;

	LastWakeTime = xTaskGetTickCount();

	while(1)
	{
		Serial.println("Periodic Task is running");
		Serial.print("Idle:");
		Serial.println(uIdleCycleCount); //Show Idle count per second
		uIdleCycleCount = 0; //Zero counter
		TaskDelayUntil_ms(&LastWakeTime,1000);
	}
}

void TaskDelay_ms(uint32_t ms)
{
	vTaskDelay(ms/portTICK_PERIOD_MS);
}

void TaskDelayUntil_ms(TickType_t * const pxPreviousWakeTime, uint32_t ms)
{
	vTaskDelayUntil(pxPreviousWakeTime,(ms/portTICK_PERIOD_MS));
}

