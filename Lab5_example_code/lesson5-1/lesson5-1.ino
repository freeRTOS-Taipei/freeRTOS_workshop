#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>
#include "FreeRTOSConfig.h"
#define mainINTERRUPT_NUMBER  3
/* The tasks to be created. */
static void vHandlerTask( void *pvParameters );
static void vPeriodicTask( void *pvParameters );
#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS( 3333 )
TimerHandle_t xTimer1;
const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 1000 );
/* The service routine for the (simulated) interrupt.  This is the interrupt
that the task will be synchronized with. */


/*-----------------------------------------------------------*/
/* Declare a variable of type SemaphoreHandle_t.  This is used to reference the
semaphore that is used to synchronize a task with an interrupt. */
SemaphoreHandle_t xBinarySemaphore;
void setup() {

  Serial.begin(9600);
  // put your setup code here, to run once:
  xBinarySemaphore = xSemaphoreCreateBinary();
  /* Check the semaphore was created successfully. */
  if( xBinarySemaphore != NULL )
  {
    /* Create the 'handler' task, which is the task to which interrupt
    processing is deferred, and so is the task that will be synchronized
    with the interrupt.  The handler task is created with a high priority to
    ensure it runs immediately after the interrupt exits.  In this case a
    priority of 3 is chosen. */
    xTaskCreate( vHandlerTask, "Handler", 1000, NULL, 3, NULL );
    /* Create the task that will periodically generate a software interrupt.
    This is created with a priority below the handler task to ensure it will
    get preempted each time the handler task exits the Blocked state. */
    xTaskCreate( vPeriodicTask, "Periodic", 1000, NULL, 1, NULL );
    /* Install the handler for the software interrupt.  The syntax necessary
    to do this is dependent on the FreeRTOS port being used.  The syntax
    shown here can only be used with the FreeRTOS Windows port, where such
    interrupts are only simulated. */
/*  vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );
    /* Start the scheduler so the created tasks start executing. */
    xTimer1 = xTimerCreate( "OneShot", mainONE_SHOT_TIMER_PERIOD, pdTRUE,
                              /* The timer’s ID is initialized to 0. */
                              0,
                              /* prvTimerCallback() is used by both timers. */
                              prvTimerCallback );
    xTimerStart(xTimer1,xHealthyTimerPeriod);                          
    vTaskStartScheduler();
    
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
static void prvTimerCallback( TimerHandle_t xTimer )
{
  
  BaseType_t xHigherPriorityTaskWoken;

  /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
  it will get set to pdTRUE inside the interrupt safe API function if a
  context switch is required. */
  xHigherPriorityTaskWoken = pdFALSE;
 
  /* 'Give' the semaphore to unblock the task. */
  xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
  Serial.println("ISR SET - Timer event.");
 }

static void vHandlerTask( void *pvParameters )
{
  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* Use the semaphore to wait for the event.  The semaphore was created
    before the scheduler was started so before this task ran for the first
    time.  The task blocks indefinitely meaning this function call will only
    return once the semaphore has been successfully obtained - so there is
    no need to check the returned value. */
    xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );

    /* To get here the event must have occurred.  Process the event (in this
    case just print out a message). */

    Serial.println("Handler task - Processing event.");

  }
}
/*-----------------------------------------------------------*/

static void vPeriodicTask( void *pvParameters )
{
const TickType_t xDelay500ms = pdMS_TO_TICKS( 500UL );

  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* This task is just used to 'simulate' an interrupt.  This is done by
    periodically generating a simulated software interrupt.  Block until it
    is time to generate the software interrupt again. */
    vTaskDelay( xDelay500ms );

    /* Generate the interrupt, printing a message both before and after
    the interrupt has been generated so the sequence of execution is evident
    from the output.

    The syntax used to generate a software interrupt is dependent on the
    FreeRTOS port being used.  The syntax used below can only be used with
    the FreeRTOS Windows port, in which such interrupts are only
    simulated. */
    
    Serial.println("Periodic task - About to generate an interrupt.");
    //vPrintString( "Periodic task - About to generate an interrupt.\r\n" );
   // vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
  //  Serial.println("Periodic task - Interrupt generated.");//
    //vPrintString( "Periodic task - Interrupt generated.\r\n\r\n\r\n" );
  }
}
/*-----------------------------------------------------------*/
