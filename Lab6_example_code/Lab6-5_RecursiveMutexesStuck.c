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

/* Declare a variable of type SemaphoreHandle_t.  This is used to reference the
mutex type semaphore that is used to ensure mutual exclusive access to stdout. */
SemaphoreHandle_t xMutex;

void setup() {
	/* Before a semaphore is used it must be explicitly created.  In this example
		a mutex type semaphore is created. */
	xMutex = xSemaphoreCreateMutex();

	/*Initial Serial port*/
	Serial.begin(115200);
	/*Create tasks*/
	xTaskCreate(Task1,"print_str1", 1000, NULL,1,NULL);
	xTaskCreate(Task2,"print_str2", 1000, NULL,2,NULL);

	/* Start the scheduler so the created tasks start executing. */
	vTaskStartScheduler();
}

void loop() {
	/*Do nothing here*/
}

void PrintFunction(char *str){

	xSemaphoreTake( xMutex, portMAX_DELAY );
	Serial.print(str);
	xSemaphoreGive( xMutex );
}

void Task1( void *pvParameters ){

	while(1){
		/*Take Semaphore to avoid task interrupt*/
		xSemaphoreTake( xMutex, portMAX_DELAY );
		PrintFunction("Recursive Mutexes Task... I don't wanna be idiot\n");
		/*Print end, give the Semaphore*/
		xSemaphoreGive( xMutex );
		vTaskDelay(MS_TO_TICK(500));
	}
}

void Task2( void *pvParameters){

	while(1){
		Serial.println("--1st Abort!--");
		vTaskDelay(MS_TO_TICK(250));
		Serial.println("--2nd Abort!--");
		vTaskDelay(MS_TO_TICK(250));
	}
}
