#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include "FreeRTOSConfig.h"
/*Task functions*/
static void Task1( void *pvParameters );
static void Task2( void *pvParameters );
/*Macros*/
#define MS_TO_TICK(MS) ( MS / portTICK_PERIOD_MS )

void setup() {
	/*Initial Serial port*/
	Serial.begin(115200);
	/*Create tasks*/
	xTaskCreate(Task1,"print_str1", 1000, NULL,1,NULL);
	xTaskCreate(Task2,"print_str2", 1000, NULL,1,NULL);

	/* Start the scheduler so the created tasks start executing. */
	vTaskStartScheduler();

}

void loop() {
	/*Do nothing here*/
}

void Task1( void *pvParameters ){

	while(1){
		 /* Suspend All Tasks */
		vTaskSuspendAll();

		Serial.print("Hallow");
		vTaskDelay(MS_TO_TICK(250));
		Serial.println(" World");

		/* Print function has finished, Resume all tasks */
		xTaskResumeAll();

		vTaskDelay(MS_TO_TICK(250));
	}
}

void Task2( void *pvParameters){

	while(1){
		/*Create a print string task to interrupt*/
		Serial.print("--Abort!--");
		vTaskDelay(MS_TO_TICK(250));
	}
}
