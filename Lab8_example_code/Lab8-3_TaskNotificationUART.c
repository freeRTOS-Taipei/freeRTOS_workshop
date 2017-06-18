/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* FreeRTOS.org includes. */
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>

/*Macros*/
#define MS_TO_TICK(MS) ( MS / portTICK_PERIOD_MS )

/* The tasks to be created. */
static void UARTReceiveTask( void *pvParameters );
static void UARTSendTask( void *pvParameters );

/* Stores the handle of the task to which interrupt processing is deferred. */
static TaskHandle_t UARTReciveHandler = NULL;
static TaskHandle_t UARTSendHandler = NULL;

/*-----------------------------------------------------------*/

void setup()
{
	/* Initial UART*/
	Serial.begin(115200);
	/* Create the 'UARTRecive' task, which is the task to which interrupt
	processing is deferred, and so is the task that will be synchronized
	with the interrupt. The handle of the task is saved for use by the
	ISR. */
	xTaskCreate( UARTReceiveTask, "UARTRecive", 1000, NULL, 1, &UARTReciveHandler );

	/* Create the task that will Send the UART.*/
	xTaskCreate( UARTSendTask, "UARTSend", 1000, NULL, 1, &UARTSendHandler );

	/* Start the scheduler so the created tasks start executing. */
	vTaskStartScheduler();
}

void loop() {
	/*Do nothing here*/
}
/*-----------------------------------------------------------*/

static void UARTSendTask( void *pvParameters )
{
	/* xMaxExpectedBlockTime is set to be a little longer than the maximum expected
	time between events. */
	uint32_t ulEventsToProcess;
	uint32_t ClearAllPendingNotifications = 0xffffffffUL;
	uint32_t ClearNotificationValue = 0xffffffffUL;
	uint32_t ReceivedFormNotification;

	/* As per most tasks, this task is implemented within an infinite loop. */
	for( ;; )
	{
		/* Wait to receive a notification sent directly to this task from the
		interrupt service routine.  The xClearCountOnExit parameter is now pdFALSE,
		so the task's notification value will be decremented by ulTaskNotifyTake(),
		and not cleared to zero. */
		xTaskNotifyWait(
						/* Clear all bits on Entry which will clear all pending
						 * notification when it exit the function */
						ClearAllPendingNotifications,
						/* Clear notification value when it exit the function*/
						ClearNotificationValue,
						/*Received value*/
						&ReceivedFormNotification,
						/*Set MAX Delay system to wait*/
						portMAX_DELAY);


		/* To get here an event must have occurred.  Process the event (in this
			case just print out a message). */
		Serial.print("Received data:");
		Serial.println((char)ReceivedFormNotification);

	}
}
/*-----------------------------------------------------------*/

static void UARTReceiveTask( void *pvParameters)
{
	char recived;
	BaseType_t xHigherPriorityTaskWoken;
	/* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE as
	it will get set to pdTRUE inside the interrupt safe API function if a
	context switch is required. */
	xHigherPriorityTaskWoken = pdFALSE;

	while(1){
		/*If any data received*/
		if(Serial.available()){
			/*Read a byte data*/
			Serial.readBytes(&recived,1);
			/* Send a notification directly to the handler task. */
			xTaskNotifyFromISR( /* The handle of the task to which the notification
								is being sent.  The handle was saved when the task
								was created. */
								UARTSendHandler,
								/* Send received data to send task*/
								(uint32_t)recived,
								/*Set notification value can't be overwrite*/
								eSetValueWithoutOverwrite,
								/* xHigherPriorityTaskWoken is used in the usual way. */
								&xHigherPriorityTaskWoken );
		}
	}
}
/*-----------------------------------------------------------*/
