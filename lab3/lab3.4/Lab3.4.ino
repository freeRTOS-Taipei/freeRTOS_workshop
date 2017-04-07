/*
  Example 12

  Update: for Arduino_FreeRTOS version 9.0.X
  JunYuan Yan (jyyan.tw@gmail.com)
*/

#include <Arduino_FreeRTOS.h>
#include <queue.h>

/* Declare two variables of type QueueHandle_t. Both queues are added to the same
   queue set. */
static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL;
/* Declare a variable of type QueueSetHandle_t.
  This is the queue set to which the
   two queues are added. */
static QueueSetHandle_t xQueueSet = NULL;

void setup( void )
{
  Serial.begin(9600);
  /* Create the two queues, both of which send character pointers. The priority
     of the receiving task is above the priority of the sending tasks, so the queues
     will never have more than one item in them at any one time*/
  xQueue1 = xQueueCreate( 1, sizeof( char * ) );
  xQueue2 = xQueueCreate( 1, sizeof( char * ) );
  /* Create the queue set. Two queues will be added to the set, each of which can
     contain 1 item, so the maximum number of queue handles the queue set will ever
     have to hold at one time is 2 (2 queues multiplied by 1 item per queue). */
  xQueueSet = xQueueCreateSet( 1 * 2 );
  /* Add the two queues to the set. */
  xQueueAddToSet( xQueue1, xQueueSet );
  xQueueAddToSet( xQueue2, xQueueSet );
  /* Create the tasks that send to the queues. */
  xTaskCreate( vSenderTask1, "Sender1", 1000, NULL, 1, NULL );
  xTaskCreate( vSenderTask2, "Sender2", 1000, NULL, 1, NULL );
  /* Create the task that reads from the queue set to determine which of the two
     queues contain data. */
  xTaskCreate( vReceiverTask, "Receiver", 1000, NULL, 2, NULL );
  /* Start the scheduler so the created tasks start executing. */
  vTaskStartScheduler();
  /* As normal, vTaskStartScheduler() should not return, so the following lines
     Will never execute. */
  for( ;; );
  /*return 0;*/
}

void vSenderTask1( void *pvParameters )
{
  const TickType_t xBlockTime = pdMS_TO_TICKS( 100 );
  const char * const pcMessage = "Message from vSenderTask1";
  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* Block for 100ms. */
    vTaskDelay( xBlockTime );
    /* Send this task's string to xQueue1. It is not necessary to use a block
       time, even though the queue can only hold one item. This is because the
       priority of the task that reads from the queue is higher than the priority of
       this task; as soon as this task writes to the queue it will be pre-empted by
       the task that reads from the queue, so the queue will already be empty again
       by the time the call to xQueueSend() returns. The block time is set to 0. */
    xQueueSend( xQueue1, &pcMessage, 0 );
  }
}
/*-----------------------------------------------------------*/
void vSenderTask2( void *pvParameters )
{
  const TickType_t xBlockTime = pdMS_TO_TICKS( 200 );
  const char * const pcMessage = "Message from vSenderTask2";
  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* Block for 200ms. */
    vTaskDelay( xBlockTime );
    /* Send this task's string to xQueue2. It is not necessary to use a block
       time, even though the queue can only hold one item. This is because the
       priority of the task that reads from the queue is higher than the priority of
       this task; as soon as this task writes to the queue it will be pre-empted by
       the task that reads from the queue, so the queue will already be empty again
       by the time the call to xQueueSend() returns. The block time is set to 0. */
    xQueueSend( xQueue2, &pcMessage, 0 );
  }
}

void vReceiverTask( void *pvParameters )
{
  QueueHandle_t xQueueThatContainsData;
  char *pcReceivedString;
  /* As per most tasks, this task is implemented within an infinite loop. */
  for( ;; )
  {
    /* Block on the queue set to wait for one of the queues in the set to contain data.
       Cast the QueueSetMemberHandle_t value returned from xQueueSelectFromSet() to a
       QueueHandle_t, as it is known all the members of the set are queues (the queue set
       does not contain any semaphores). */
    xQueueThatContainsData = ( QueueHandle_t ) xQueueSelectFromSet( xQueueSet,
        portMAX_DELAY );
    /* An indefinite block time was used when reading from the queue set, so
       xQueueSelectFromSet() will not have returned unless one of the queues in the set
       contained data, and xQueueThatContainsData cannot be NULL. Read from the queue. It
       is not necessary to specify a block time because it is known the queue contains
       data. The block time is set to 0. */
    xQueueReceive( xQueueThatContainsData, &pcReceivedString, 0 );
    /* Print the string received from the queue. */
    Serial.println( pcReceivedString );
  }
}

//------------------------------------------------------------------------------
void loop() {}




