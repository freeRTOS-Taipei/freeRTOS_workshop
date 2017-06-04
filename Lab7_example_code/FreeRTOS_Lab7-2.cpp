#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <time.h>

/* FreeRTOS.org includes. */
//#include "FreeRTOS.h"
#include <task.h>
#include <event_groups.h>

/*Macros*/
#define MS_TO_TICK(MS) ( MS / portTICK_PERIOD_MS )

/* Definitions for the event bits in the event group. */
#define mainFIRST_TASK_BIT	( 1UL << 0UL ) /* Event bit 0, which is set by the first task. */
#define mainSECOND_TASK_BIT	( 1UL << 1UL ) /* Event bit 1, which is set by the second task. */
#define mainTHIRD_TASK_BIT	( 1UL << 2UL ) /* Event bit 2, which is set by the third task. */

/* Pseudo random number generation functions - implemented in this file as the
MSVC rand() function has unexpected consequences. */
static uint32_t prvRand( void );
static void prvSRand( uint32_t ulSeed );

/* Three instances of this task are created. */
static void vSyncingTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Use by the pseudo random number generator. */
static uint32_t ulNextRand;

/* Declare the event group used to synchronize the three tasks. */
EventGroupHandle_t xEventGroup;
TaskStatus_t task;

void setup()
{
	/*Initial Serial port*/
	Serial.begin(115200);
	/* The tasks created in this example block for a random time.  The block
	time is generated using rand() - seed the random number generator. */
	prvSRand( ( uint32_t ) time( NULL ) );

	/* Before an event group can be used it must first be created. */
	xEventGroup = xEventGroupCreate();

	/* Create three instances of the task.  Each task is given a different name,
	which is later printed out to give a visual indication of which task is
	executing.  The event bit to use when the task reaches its synchronization
	point is passed into the task using the task parameter. */
	xTaskCreate( vSyncingTask, "Task 1", 1000, ( void * ) mainFIRST_TASK_BIT, 1, NULL );
	xTaskCreate( vSyncingTask, "Task 2", 1000, ( void * ) mainSECOND_TASK_BIT, 1, NULL );
	xTaskCreate( vSyncingTask, "Task 3", 1000, ( void * ) mainTHIRD_TASK_BIT, 1, NULL );

	/* Start the scheduler so the created tasks start executing. */
	vTaskStartScheduler();

}
/*-----------------------------------------------------------*/

void loop() {
	/*Do nothing here*/
}

static void vSyncingTask( void *pvParameters )
{
const EventBits_t uxAllSyncBits = ( mainFIRST_TASK_BIT | mainSECOND_TASK_BIT | mainTHIRD_TASK_BIT );
const TickType_t xMaxDelay = MS_TO_TICK( 4000UL );
const TickType_t xMinDelay = MS_TO_TICK( 200UL );
char *TaskName;
TickType_t xDelayTime;
EventBits_t uxThisTasksSyncBit;

	/* Three instances of this task are created - each task uses a different
	event bit in the synchronization.  The event bit to use by this task
	instance is passed into the task using the task's parameter.  Store it in
	the uxThisTasksSyncBit variable. */
	uxThisTasksSyncBit = ( EventBits_t ) pvParameters;

	switch(uxThisTasksSyncBit){
		case mainFIRST_TASK_BIT:
			TaskName = "Task 1";
			break;
		case mainSECOND_TASK_BIT:
			TaskName = "Task 2";
			break;
		case mainTHIRD_TASK_BIT:
			TaskName = "Task 3";
			break;
	}

	for( ;; )
	{
		/* Simulate this task taking some time to perform an action by delaying
		for a pseudo random time.  This prevents all three instances of this
		task from reaching the synchronization point at the same time, and
		allows the example's behavior to be observed more easily. */
		xDelayTime = ( prvRand() % xMaxDelay ) + xMinDelay;
		vTaskDelay( xDelayTime );

		/* Print out a message to show this task has reached its synchronization
		point.  pcTaskGetTaskName() is an API function that returns the name
		assigned to the task when the task was created. */
		Serial.print( TaskName );
		Serial.println( " reached sync point\r\n" );

		/* Wait for all the tasks to have reached their respective
		synchronization points. */
		xEventGroupSync( /* The event group used to synchronize. */
						 xEventGroup,

						 /* The bit set by this task to indicate it has reached
						 the synchronization point. */
						 uxThisTasksSyncBit,

						 /* The bits to wait for, one bit for each task taking
						 part in the synchronization. */
						 uxAllSyncBits,

						 /* Wait indefinitely for all three tasks to reach the
						 synchronization point. */
						 portMAX_DELAY );

		/* Print out a message to show this task has passed its synchronization
		point.  As an indefinite delay was used the following line will only be
		reached after all the tasks reached their respective synchronization
		points. */
		Serial.print( TaskName );
		Serial.println(" exited sync point" );
	}
}
/*-----------------------------------------------------------*/

static uint32_t prvRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;
uint32_t ulReturn;

	/* Utility function to generate a pseudo random number as the MSVC rand()
	function has unexpected consequences. */
	taskENTER_CRITICAL();
		ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
		ulReturn = ( ulNextRand >> 16UL ) & 0x7fffUL;
	taskEXIT_CRITICAL();
	return ulReturn;
}
/*-----------------------------------------------------------*/

static void prvSRand( uint32_t ulSeed )
{
	/* Utility function to seed the pseudo random number generator. */
	ulNextRand = ulSeed;
}
