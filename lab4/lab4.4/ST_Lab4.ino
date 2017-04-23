#include <Arduino_FreeRTOS.h>
#include <timers.h>
#include <task.h>

/* The check timer is created with a period of 3000 milliseconds, resulting in the LED toggling
   every 3 seconds. If the self-checking functionality detects an unexpected state, then the check
   timer’s period is changed to just 200 milliseconds, resulting in a much faster toggle rate. */
const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 1000 );
const TickType_t xErrorTimerPeriod = pdMS_TO_TICKS( 100 );
/* The callback function used by the check timer. */

void setup(void){
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);

  TimerHandle_t xAutoReloadTimer;
  BaseType_t xTimer1Started;

  xAutoReloadTimer = xTimerCreate(
      /* Text name for the software timer - not used by FreeRTOS. */
      "AutoReload",
      /* The software timer's period in ticks. */
      xHealthyTimerPeriod,
      /* Setting uxAutoRealod to pdTRUE creates an auto-reload timer. */
      pdTRUE,
      /* This example does not use the timer id. */
      0,
      /* The callback function to be used by the software timer being created. */
      prvCheckTimerCallbackFunction);

  /* Check the software timers were created. */
  if( ( xAutoReloadTimer != NULL ) )
  {
    /* Start the software timers, using a block time of 0 (no block time). The scheduler has
       not been started yet so any block time specified here would be ignored anyway. */
    xTimer1Started = xTimerStart( xAutoReloadTimer, 0 );
    /* The implementation of xTimerStart() uses the timer command queue, and xTimerStart()
       will fail if the timer command queue gets full. The timer service task does not get
       created until the scheduler is started, so all commands sent to the command queue will
       stay in the queue until after the scheduler has been started. Check both calls to
       xTimerStart() passed. */
    if( ( xTimer1Started == pdPASS ) )
    {
      /* Start the scheduler. */
      vTaskStartScheduler();
    }
  }
  /* As always, this line should not be reached. */
  for( ;; );
}

void loop(void){

}


static void prvCheckTimerCallbackFunction( TimerHandle_t xTimer )
{
  static BaseType_t xErrorDetected = pdFALSE;

  if( xErrorDetected == pdFALSE )
  {
    /* No errors have yet been detected. Run the self-checking function again. The
       function asks each task created by the example to report its own status, and also checks
       that all the tasks are actually still running (and so able to report their status
       correctly). */
    /*if( CheckTasksAreRunningWithoutError() == pdFAIL )*/
    /*{*/
      /* One or more tasks reported an unexpected status. An error might have occurred.
         Reduce the check timer’s period to increase the rate at which this callback function
         executes, and in so doing also increase the rate at which the LED is toggled. This
         callback function is executing in the context of the RTOS daemon task, so a block
         time of 0 is used to ensure the Daemon task never enters the Blocked state. */
      xTimerChangePeriod( xTimer, /* The timer being updated. */
          xErrorTimerPeriod, /* The new period for the timer. */
          0 ); /* Do not block when sending this command. */
    /*}*/
    /* Latch that an error has already been detected. */
    xErrorDetected = pdTRUE;
  }
  /* Toggle the LED. The rate at which the LED toggles will depend on how often this function
     is called, which is determined by the period of the check timer. The timer’s period will
     have been reduced from 3000ms to just 200ms if CheckTasksAreRunningWithoutError() has ever
     returned pdFAIL. */
  ToggleLED();
}


boolean ToggleValue = HIGH;

void ToggleLED(void){
  digitalWrite(LED_BUILTIN, ToggleValue );   // turn the LED on (HIGH is the voltage level)
  ToggleValue = ! ToggleValue;
  Serial.println( ToggleValue );
}
