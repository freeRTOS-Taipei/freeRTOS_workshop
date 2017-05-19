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
SemaphoreHandle_t xMutex,xRecursiveMutex;

void setup() {
	/* Before a semaphore is used it must be explicitly created.  In this example
		a mutex type semaphore is created. */
	xRecursiveMutex = xSemaphoreCreateRecursiveMutex();

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

	xSemaphoreTakeRecursive( xRecursiveMutex, portMAX_DELAY );
	Serial.print(str);
	xSemaphoreGiveRecursive( xRecursiveMutex );
}
void Task1( void *pvParameters ){

	while(1){
		/*Take Semaphore to avoid task interrupt*/
		xSemaphoreTakeRecursive( xRecursiveMutex, portMAX_DELAY );
		PrintFunction("Recursive Mutexes Task...");
		vTaskDelay(MS_TO_TICK(250));
		PrintFunction("I don't wanna be idiot\r\n");
		/*Print end, give the Semaphore*/
		xSemaphoreGiveRecursive( xRecursiveMutex );
		vTaskDelay(MS_TO_TICK(250));
	}
}
void Task2( void *pvParameters){

	while(1){
		/*Take Semaphore to avoid task interrupt*/
		xSemaphoreTake( xRecursiveMutex, portMAX_DELAY );
		Serial.println("--1st Abort!--");
		vTaskDelay(MS_TO_TICK(250));
		Serial.println("--2nd Abort!--");
		/*Print end, give the Semaphore*/
		xSemaphoreGive( xRecursiveMutex );
		vTaskDelay(MS_TO_TICK(250));
	}
}