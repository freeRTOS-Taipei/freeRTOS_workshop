#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include <queue.h>
#include "FreeRTOSConfig.h"
#define mainINTERRUPT_NUMBER  3

TimerHandle_t xTimer1;

/* The tasks to be created. */
static void vIntegerGenerator( void *pvParameters );
static void vStringPrinter( void *pvParameters );

/* The service routine for the (simulated) interrupt.  This is the interrupt
that the task will be synchronized with. */
static uint32_t ulExampleInterruptHandler( void );

/*-----------------------------------------------------------*/

/* Declare two variables of type QueueHandle_t.  One queue will be read from
within an ISR, the other will be written to from within an ISR. */
QueueHandle_t xIntegerQueue, xStringQueue;
const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 1000 );
void setup() {

   Serial.begin(9600);
  // put your setup code here, to run once:
  /* Before a queue can be used it must first be created.  Create both queues
  used by this example.  One queue can hold variables of type uint32_t,
  the other queue can hold variables of type char*.  Both queues can hold a
  maximum of 10 items.  A real application should check the return values to
  ensure the queues have been successfully created. */
    xIntegerQueue = xQueueCreate( 10, sizeof( uint32_t ) );
    xStringQueue = xQueueCreate( 10, sizeof( char * ) );

  /* Create the task that uses a queue to pass integers to the interrupt
  service routine.  The task is created at priority 1. */
  //xTaskCreate( vIntegerGenerator, "IntGen", 1000, NULL, 1, NULL );
    xTaskCreate( vIntegerGenerator, "Handler", 1000, NULL, 3, NULL );
  /* Create the task that prints out the strings sent to it from the interrupt
  service routine.  The task is created at the higher priority of 2. */
  xTaskCreate( vStringPrinter, "String", 1000, NULL, 2, NULL );
  
  /* Install the handler for the software interrupt.  The syntax necessary
  to do this is dependent on the FreeRTOS port being used.  The syntax
  shown here can only be used with the FreeRTOS Windows port, where such
  interrupts are only simulated. */
//  vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );
  xTimer1 = xTimerCreate( "OneShot", xHealthyTimerPeriod, pdTRUE,
                              /* The timer’s ID is initialized to 0. */
                              0,
                              /* prvTimerCallback() is used by both timers. */
                              ulExampleInterruptHandler );
    xTimerStart(xTimer1,xHealthyTimerPeriod);
  /* Start the scheduler so the created tasks start executing. */
  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:

}

static void vIntegerGenerator( void *pvParameters )
{
TickType_t xLastExecutionTime;
const TickType_t xDelay200ms = pdMS_TO_TICKS( 200UL ), xDontBlock = 0;
uint32_t ulValueToSend = 0;
BaseType_t i;

  /* Initialize the variable used by the call to vTaskDelayUntil(). */
  xLastExecutionTime = xTaskGetTickCount();

  for( ;; )
  {
    /* This is a periodic task.  Block until it is time to run again.
    The task will execute every 200ms. */
    vTaskDelayUntil( &xLastExecutionTime, xDelay200ms );

    /* Send five numbers to the queue, each value one higher than the
    previous value.  The numbers are read from the queue by the interrupt
    service routine.  The interrupt service routine always empties the
    queue, so this task is guaranteed to be able to write all five values
    without needing to specify a block time. */
    for( i = 0; i < 5; i++ )
    {
      xQueueSendToBack( xIntegerQueue, &ulValueToSend, xDontBlock );
      ulValueToSend++;
    }

    /* Generate the interrupt so the interrupt service routine can read the
    values from the queue. The syntax used to generate a software interrupt
    is dependent on the FreeRTOS port being used.  The syntax used below can
    only be used with the FreeRTOS Windows port, in which such interrupts
    are only simulated.*/
    Serial.println("Generator task - About to generate an interrupt." );
//    vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
    Serial.println( "Generator task - Interrupt generated." );
  }
}
/*-----------------------------------------------------------*/

static void vStringPrinter( void *pvParameters )
{
char *pcString;

  for( ;; )
  {
    /* Block on the queue to wait for data to arrive. */
    xQueueReceive( xStringQueue, &pcString, portMAX_DELAY );

    /* Print out the received string. */
    Serial.println( pcString );
    //vPrintString( pcString );
  }
}
/*-----------------------------------------------------------*/

static uint32_t ulExampleInterruptHandler( void )
{
BaseType_t xHigherPriorityTaskWoken;
uint32_t ulReceivedNumber;

/* The strings are declared static const to ensure they are not allocated on the
interrupt service routine's stack, and exist even when the interrupt service
routine is not executing. */
static const char *pcStrings[] =
{
  "String 0\r\n",
  "String 1\r\n",
  "String 2\r\n",
  "String 3\r\n"
};

  /* As always, xHigherPriorityTaskWoken is initialized to pdFALSE to be able
  to detect it getting set to pdTRUE inside an interrupt safe API function. */
  xHigherPriorityTaskWoken = pdFALSE;

  /* Read from the queue until the queue is empty. */
  while( xQueueReceiveFromISR( xIntegerQueue, &ulReceivedNumber, &xHigherPriorityTaskWoken ) != errQUEUE_EMPTY )
  {
    /* Truncate the received value to the last two bits (values 0 to 3
    inc.), then use the truncated value as an index into the pcStrings[]
    array to select a string (char *) to send on the other queue. */
    ulReceivedNumber &= 0x03;
    xQueueSendToBackFromISR( xStringQueue, &pcStrings[ ulReceivedNumber ], &xHigherPriorityTaskWoken );
  }
  Serial.println("ISR SET - Timer event.");
    /* If receiving from xIntegerQueue caused a task to leave the Blocked state,
  and if the priority of the task that left the Blocked state is higher than
  the priority of the task in the Running state, then xHigherPriorityTaskWoken
  will have been set to pdTRUE inside xQueueReceiveFromISR().

    If sending to xStringQueue caused a task to leave the Blocked state, and
    if the priority of the task that left the Blocked state is higher than the
    priority of the task in the Running state, then xHigherPriorityTaskWoken
  will have been set to pdTRUE inside xQueueSendFromISR().

    xHigherPriorityTaskWoken is used as the parameter to portYIELD_FROM_ISR().
  If xHigherPriorityTaskWoken equals pdTRUE then calling portYIELD_FROM_ISR()
    will request a context switch.  If xHigherPriorityTaskWoken is still pdFALSE
  then calling portYIELD_FROM_ISR() will have no effect.

    The implementation of portYIELD_FROM_ISR() used by the Windows port includes
  a return statement, which is why this function does not explicitly return a
    value. */
  //portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

