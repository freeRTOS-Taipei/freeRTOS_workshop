#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void TaskFunction (void *Parameter);
void TaskPeriodic (void *Parameter);
void TaskDelay_ms(uint32_t ms);
void TaskDelayUntil_ms(TickType_t * const pxPreviousWakeTime, uint32_t ms);

TaskHandle_t Task2Handle;

void setup() {

	Serial.begin(115200);
	xTaskCreate(TaskFunction, "Task1", 1000, (void*) 0x01, 2, NULL);
	xTaskCreate(TaskFunction, "Task2", 1000, (void*) 0x02, 1, &Task2Handle);
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
	uint8_t TaskID;
	UBaseType_t Priority;

	TaskID = (uint8_t) Parameter; //Convert type
	Priority = uxTaskPriorityGet(NULL);

	switch(TaskID)
	{
		case 0x01:
			while(1)
			{
				Serial.println("Task 1 is running");
				Serial.println("Set Task 2 priority higher than task 1");
				vTaskPrioritySet(Task2Handle, Priority + 1); //Set Task2 priority = 3, higher than task1
				delay(300);
			}
			break;
		case 0x02:
			while(1)
			{
				Serial.println("Task 2 is running");
				Serial.println("Set Task 2 priority lower than task 1");
				vTaskPrioritySet(Task2Handle, Priority - 2); //Set Task2 priority = 1, higher than task1
				delay(300);
			}
			break;
		default:
			break;
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

