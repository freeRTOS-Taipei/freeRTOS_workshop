#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <queue.h>

/*Macros*/
#define MS_TO_TICK(MS) ( MS / portTICK_PERIOD_MS )
#define AutoReload_TIMER_PERIOD MS_TO_TICK( 2000 )
const TickType_t xHealthyTimerPeriod = MS_TO_TICK( 2000 );

/* The task that sends messages to the stdio gatekeeper.  Two instances of this
task are created. */
static void prvPrintTask( void *pvParameters );

/* The gatekeeper task itself. */
static void prvStdioGatekeeperTask( void *pvParameters );

/* The service routine for the (simulated) interrupt.  This is the interrupt
  that the task will be synchronized with. */
static uint32_t ulTimerInterruptHandler( void );


/* Define the strings that the tasks and interrupt will print out via the
gatekeeper. */
static const char *pcStringsToPrint[] =
{
	"Task 1 ****************************************************\r\n",
	"Task 2 ----------------------------------------------------\r\n",
	"Message printed from the timer interrupt ##############\r\n"
};

/* Declare a variable of type QueueHandle_t.  This is used to send messages from
the print tasks to the gatekeeper task. */
static QueueHandle_t xPrintQueue;
TimerHandle_t xTimer1;

void setup() {
	/* Before a queue is used it must be explicitly created.  The queue is created
	to hold a maximum of 5 character pointers. */
	xPrintQueue = xQueueCreate( 5, sizeof( char * ) );

	/* Check the queue was created successfully. */
	if( xPrintQueue != NULL )
	{
		/*Initial Serial port*/
		Serial.begin(115200);
		/* Create two instances of the tasks that send messages to the gatekeeper.
		The	index to the string they attempt to write is passed in as the task
		parameter (4th parameter to xTaskCreate()).  The tasks are created at
		different priorities so some pre-emption will occur. */
		xTaskCreate( prvPrintTask, "Print1", 1000, ( void * ) 0, 1, NULL );
		xTaskCreate( prvPrintTask, "Print2", 1000, ( void * ) 1, 2, NULL );

		/* Create the gatekeeper task.  This is the only task that is permitted
		to access standard out. */
		xTaskCreate( prvStdioGatekeeperTask, "Gatekeeper", 1000, NULL, 0, NULL );

		/* Create the timer interrupt*/
		xTimer1 = xTimerCreate( "Auto-reload", AutoReload_TIMER_PERIOD, pdTRUE,
								  /* The timerÅfs ID is initialized to 0. */
								  0,
								  /* prvTimerCallback() is used by both timers. */
								  ulTimerInterruptHandler );
		/*Start timer*/
		xTimerStart(xTimer1, xHealthyTimerPeriod);
		/* Start the scheduler so the created tasks start executing. */
		vTaskStartScheduler();
	}

	/* The following line should never be reached because vTaskStartScheduler()
	will only return if there was not enough FreeRTOS heap memory available to
	create the Idle and (if configured) Timer tasks.  Heap management, and
	techniques for trapping heap exhaustion, are described in the book text. */

}

void loop() {
	/*Do nothing here*/
}

/*-----------------------------------------------------------*/

static void prvStdioGatekeeperTask( void *pvParameters )
{
char *pcMessageToPrint;

	/* This is the only task that is allowed to write to the terminal output.
	Any other task wanting to write to the output does not access the terminal
	directly, but instead sends the output to this task.  As only one task
	writes to standard out there are no mutual exclusion or serialization issues
	to consider within this task itself. */
	for( ;; )
	{
		/* Wait for a message to arrive. */
		xQueueReceive( xPrintQueue, &pcMessageToPrint, portMAX_DELAY );

		/* There is no need to check the return	value as the task will block
		indefinitely and only run again when a message has arrived.  When the
		next line is executed there will be a message to be output. */
		Serial.print( pcMessageToPrint );

		/* Now simply go back to wait for the next message. */
	}
}
/*-----------------------------------------------------------*/

static void prvPrintTask( void *pvParameters )
{
int iIndexToString;

	/* Two instances of this task are created so the index to the string the task
	will send to the gatekeeper task is passed in the task parameter.  Cast this
	to the required type. */
	iIndexToString = ( int ) pvParameters;

	for( ;; )
	{
		/* Print out the string, not directly but by passing the string to the
		gatekeeper task on the queue.  The queue is created before the scheduler is
		started so will already exist by the time this task executes.  A block time
		is not specified as there should always be space in the queue. */
		xQueueSendToBack( xPrintQueue, &( pcStringsToPrint[ iIndexToString ] ), 0 );

		/* Wait a pseudo random time.  Note that rand() is not necessarily
		re-entrant, but in this case it does not really matter as the code does
		not care what value is returned.  In a more secure application a version
		of rand() that is known to be re-entrant should be used - or calls to
		rand() should be protected using a critical section. */
		vTaskDelay( MS_TO_TICK(500));
	}
}
/*-----------------------------------------------------------*/

static uint32_t ulTimerInterruptHandler( void )
{
	/* In this case the last parameter (xHigherPriorityTaskWoken) is not
	actually used and is set to NULL. */
	xQueueSendToFrontFromISR( xPrintQueue, &( pcStringsToPrint[ 2 ] ), NULL );
}
/*-----------------------------------------------------------*/
