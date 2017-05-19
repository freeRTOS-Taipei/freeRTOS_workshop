#include <Arduino_FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include <task.h>
#include <semphr.h>
#include <timers.h>

#define mainINTERRUPT_NUMBER  3
static void vPeriodicTask( void *pvParameters );

/* The function that performs the deferred interrupt processing.  This function
  is executed in the context of the daemon task. */
static void vDeferredHandlingFunction( void *pvParameter1, uint32_t ulParameter2 );
#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS( 3333 )
const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 1000 );
TimerHandle_t xTimer1;
/* The service routine for the (simulated) interrupt.  This is the interrupt
  that the task will be synchronized with. */
static uint32_t ulExampleInterruptHandler( void );
void setup() {
  // put your setup code here, to run once:
  /* The task that generates the software interrupt is created at a priority
    below the priority of the daemon task.  The priority of the daemon task is
    set by the configTIMER_TASK_PRIORITY compile time configuration constant in
    FreeRTOSConfig.h. */
  Serial.begin(9600);
  const UBaseType_t ulPeriodicTaskPriority = configTIMER_TASK_PRIORITY - 1;

  /* Create the task that will periodically generate a software interrupt. */
  xTaskCreate( vPeriodicTask, "Periodic", 1000, NULL, ulPeriodicTaskPriority, NULL );

  /* Install the handler for the software interrupt.  The syntax necessary
    to do this is dependent on the FreeRTOS port being used.  The syntax
    shown here can only be used with the FreeRTOS Windows port, where such
    interrupts are only simulated. */

  // vPortSetInterruptHandler( mainINTERRUPT_NUMBER, ulExampleInterruptHandler );
  xTimer1 = xTimerCreate( "OneShot", mainONE_SHOT_TIMER_PERIOD, pdTRUE,
                          /* The timer’s ID is initialized to 0. */
                          0, ulExampleInterruptHandler );
  xTimerStart(xTimer1, xHealthyTimerPeriod);
  /* Start the scheduler so the created task starts executing. */
  vTaskStartScheduler();

  /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
  for ( ;; );
  return 0;
}

void loop() {
  // put your main code here, to run repeatedly:

}

/*-----------------------------------------------------------*/

static void vPeriodicTask( void *pvParameters )
{
  const TickType_t xDelay500ms = pdMS_TO_TICKS( 500UL );

  /* As per most tasks, this task is implemented within an infinite loop. */
  for ( ;; )
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
    Serial.println( "Periodic task - About to generate an interrupt." );
    //   vPortGenerateSimulatedInterrupt( mainINTERRUPT_NUMBER );
    Serial.println( "Periodic task - Interrupt generated." );
  }
}
/*-----------------------------------------------------------*/

static uint32_t ulExampleInterruptHandler( void )
{
  static uint32_t ulParameterValue = 0;
  BaseType_t xHigherPriorityTaskWoken;

  /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
    it will get set to pdTRUE inside the interrupt safe API function if a
    context switch is required. */
  xHigherPriorityTaskWoken = pdFALSE;

  /* Send a pointer to the interrupt's deferred handling function to the
    daemon task.  The deferred handling function's pvParameter1 parameter is not
    used so just set to NULL.  The deferred handling function's ulParameter2
    parameter is used to pass a number that is incremented by one each time this
    interrupt occurs. */
  xTimerPendFunctionCallFromISR( vDeferredHandlingFunction, NULL, ulParameterValue, &xHigherPriorityTaskWoken );
  ulParameterValue++;

  /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR().  If
    xHigherPriorityTaskWoken was set to pdTRUE inside
    xTimerPendFunctionCallFromISR() then calling portYIELD_FROM_ISR() will
    request a context switch.  If xHigherPriorityTaskWoken is still pdFALSE then
    calling portYIELD_FROM_ISR() will have no effect.  The implementation of
    portYIELD_FROM_ISR() used by the Windows port includes a return statement,
    which is why this function does not explicitly return a value. */
  //  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/*-----------------------------------------------------------*/

static void vDeferredHandlingFunction( void *pvParameter1, uint32_t ulParameter2 )
{
  /* Remove the compiler warning indicating that pvParameter1 is not used, as
    pvParameter1 is not used in this example. */
  ( void ) pvParameter1;

  /* Process the event - in this case just print out a message and the value
    of ulParameter2. */


  Serial.println( "Handler function - Processing event ");
  Serial.println(ulParameter2, DEC);
}
