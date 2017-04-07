/*
  FreeRTOS.org V5.0.4 - Copyright (C) 2003-2008 Richard Barry.

  This file is part of the FreeRTOS.org distribution.

  FreeRTOS.org is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  FreeRTOS.org is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FreeRTOS.org; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  A special exception to the GPL can be applied should you wish to distribute
  a combined work that includes FreeRTOS.org, without being obliged to provide
  the source code for any proprietary components.  See the licensing section
  of http://www.FreeRTOS.org for full details of how and when the exception
  can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

  Please ensure to read the configuration and relevant port sections of the
  online documentation.

  http://www.FreeRTOS.org - Documentation, latest information, license and
  contact details.

  http://www.SafeRTOS.com - A version that is certified for use in safety
  critical systems.

  http://www.OpenRTOS.com - Commercial support, development, porting,
  licensing and training services.

  Update: for Arduino_FreeRTOS version 9.0.X
  JunYuan Yan (jyyan.tw@gmail.com)
*/

#include <Arduino_FreeRTOS.h>
#include <queue.h>

/* The tasks to be created.  Two instances are created of the sender task while
only a single instance is created of the receiver task. */
static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */


void setup( void )
{
  Serial.begin(9600);
  /* The queue is created to hold a maximum of 5 long values. */

  if( xQueue != NULL )
  {
    /* Create two instances of the task that will write to the queue.  The
    parameter is used to pass the value that the task should write to the queue,
    so one task will continuously write 100 to the queue while the other task
    will continuously write 200 to the queue.  Both tasks are created at
    priority 1. */

    /* Create the task that will read from the queue.  The task is created with
    priority 2, so above the priority of the sender tasks. */

    /* Start the scheduler so the created tasks start executing. */
  }
  else
  {
    /* The queue could not be created. */
  }

    /* If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was
    insufficient heap memory available for a resource to be created. */
  for( ;; );
//  return 0;
}
/*-----------------------------------------------------------*/

static void vSenderTask( void *pvParameters )
{
  /* Drop your sender code here */

}
/*-----------------------------------------------------------*/

static void vReceiverTask( void *pvParameters )
{
  /* Declare the variable that will hold the values received from the queue. */

  /* This task is also defined within an infinite loop. */
  for( ;; )
  {
    /* As this task unblocks immediately that data is written to the queue this
    call should always find the queue empty. */

    /* The first parameter is the queue from which data is to be received.  The
    queue is created before the scheduler is started, and therefore before this
    task runs for the first time.

    The second parameter is the buffer into which the received data will be
    placed.  In this case the buffer is simply the address of a variable that
    has the required size to hold the received data.

    the last parameter is the block time, the maximum amount of time that the
    task should remain in the Blocked state to wait for data to be available should
    the queue already be empty. */

  }
}
//------------------------------------------------------------------------------
void loop() {}
