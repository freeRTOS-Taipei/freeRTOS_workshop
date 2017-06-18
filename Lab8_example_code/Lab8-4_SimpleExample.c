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
#include <queue.h>

/*Macros*/
#define MS_TO_TICK(MS) ( MS / portTICK_PERIOD_MS )

/* The tasks to be created. */
static void APP_Task1( void *pvParameters );
static void APP_Task2( void *pvParameters );
static void ServerTask( void *pvParameters );
BaseType_t CloudRead( uint32_t ulDataID, uint32_t *pulValue );
uint32_t CloudWrite( uint32_t ulDataID, uint32_t ulDataValue );

/* Stores the handle of the task to which interrupt processing is deferred. */
static TaskHandle_t CloudReadHandler = NULL;
static TaskHandle_t CloudWriteHandler = NULL;

/*-----------------------------------------------------------*/

static QueueHandle_t xServerTaskQueue;
/*-----------------------------------------------------------*/
/* Setting commend type*/
typedef enum CloudOperations
{
    eRead,                     /* Send data to the cloud server. */
    eWrite                     /* Receive data from the cloud server. */
} Operation_t;

typedef struct CloudCommand
{
    Operation_t eOperation;    /* The operation to perform (read or write). */
    uint32_t ulDataID;         /* Identifies the data being read or written. */
    uint32_t ulDataValue;      /* Only used when writing data to the cloud server. */
    TaskHandle_t xTaskToNotify;/* The handle of the task performing the operation. */
} CloudCommand_t;
/*-----------------------------------------------------------*/

void setup()
{
	/* Initial UART*/
	Serial.begin(115200);

	/* Before a queue is used it must be explicitly created.  The queue is created
	to hold a maximum of 5 character pointers. */
	xServerTaskQueue = xQueueCreate( 5, sizeof( CloudCommand_t) );

	/* Create the 'UARTRecive' task, which is the task to which interrupt
	processing is deferred, and so is the task that will be synchronized
	with the interrupt. The handle of the task is saved for use by the
	ISR. */
	xTaskCreate( APP_Task1, "CloudRead", 1000, NULL, 3, &CloudReadHandler );

	/* Create the task that will Send the UART.*/
	xTaskCreate( APP_Task2, "CloudWrite", 1000, NULL, 2, &CloudWriteHandler );

	/*Create server task*/
	xTaskCreate( ServerTask, "ServerTask", 1000, NULL, 1, NULL );

	/* Start the scheduler so the created tasks start executing. */
	vTaskStartScheduler();
}

void loop() {
	/*Do nothing here*/
}
/*-----------------------------------------------------------*/

/* Status bits used by the cloud write operation. */
#define SEND_SUCCESSFUL_BIT             ( 0x01 << 0 )
#define OPERATION_TIMED_OUT_BIT         ( 0x01 << 1 )
#define NO_INTERNET_CONNECTION_BIT      ( 0x01 << 2 )
#define CANNOT_LOCATE_CLOUD_SERVER_BIT  ( 0x01 << 3 )

/* A mask that has the four status bits set. */
#define CLOUD_WRITE_STATUS_BIT_MASK     ( SEND_SUCCESSFUL_BIT | \
                                          OPERATION_TIMED_OUT_BIT | \
										  NO_INTERNET_CONNECTION_BIT | \
										  CANNOT_LOCATE_CLOUD_SERVER_BIT )

void ServerTask( void *pvParameters )
{
  CloudCommand_t xCommand;
  char ulReceivedValue;
  uint32_t ulBitwiseStatusCode;

  while(1)
  {
    /* Wait for the next CloudCommand_t structure to be received from a task. */
    xQueueReceive( xServerTaskQueue, &xCommand, portMAX_DELAY );

    switch( xCommand.eOperation ) /* Was it a read or write request? */
    {
      case eRead:

    	  /* Obtain the requested data item from the remote cloud server. */
    	  Serial.print("\r\nServer got request form ID No.");
    	  Serial.println(xCommand.ulDataID);
    	  Serial.print("Send back: ");
    	  ulReceivedValue = random(94)+33; //Match to ASCII table
    	  Serial.println(ulReceivedValue);
    	  /* Call xTaskNotify() to send both a notification and the value received from the
             cloud server to the task that made the request.  The handle of the task is
             obtained from the CloudCommand_t structure. */
    	  xTaskNotify( xCommand.xTaskToNotify, 			/* The taskfs handle is in the structure. */
    			       (uint32_t)ulReceivedValue,   /* Cloud data sent as notification value. */
				       eSetValueWithOverwrite );
        break;

      case eWrite:

    	  /* Send the data to the remote cloud server.  SetCloudData() returns a bitwise
             status code that only uses the bits defined by the CLOUD_WRITE_STATUS_BIT_MASK
             definition (shown in Listing 161). */
    	  Serial.println("\r\nSend to Server");
    	  Serial.print("Data ID: ");
    	  Serial.println(xCommand.ulDataID);
    	  Serial.print("Commend: ");
    	  Serial.println(xCommand.ulDataValue);

    	  /* Select server return value (Random)*/
    	  switch(random(4)){
    	  	  case 0:
    	  		  ulBitwiseStatusCode = SEND_SUCCESSFUL_BIT;
    	  		  break;
    	  	  case 1:
    	  		  ulBitwiseStatusCode = OPERATION_TIMED_OUT_BIT;
    	  		  break;
    	  	  case 2:
    	  		  ulBitwiseStatusCode = NO_INTERNET_CONNECTION_BIT;
    	  		  break;
    	  	  case 3:
    	  		  ulBitwiseStatusCode = CANNOT_LOCATE_CLOUD_SERVER_BIT;
    	  		  break;
    	  	  default:
    	  		  ulBitwiseStatusCode = SEND_SUCCESSFUL_BIT;
    	  		  break;
    	  }

		  /* Send a notification to the task that made the write request.  The eSetBits
		  action is used so any status bits set in ulBitwiseStatusCode will be set in the
		  notification value of the task being notified.  All the other bits remain
		  unchanged.  The handle of the task is obtained from the CloudCommand_t
		  structure. */
		  xTaskNotify( xCommand.xTaskToNotify, /* The taskfs handle is in the structure. */
					   ulBitwiseStatusCode,    /* Cloud data sent as notification value. */
					   eSetBits );
		break;

        /* Other switch cases go here. */
    }
  }
}

static void APP_Task2( void *pvParameters )
{
	uint32_t SimulateSendingCommendPeriod = MS_TO_TICK(5000);
	uint32_t DataID;
	uint32_t SendData;
	uint32_t ServerStatus;

	while(1)
	{
		/* This task is simulate to send data to
		 * remote server. Remote will return server status
		 */
		DataID = random(10)+1;
		SendData = random(3000)+1;
		ServerStatus = CloudWrite(DataID,SendData);
		/*Show returned message*/
		Serial.println("\r\n******************************");
		Serial.print("Server returned: ");

		switch(ServerStatus){
			case SEND_SUCCESSFUL_BIT:
				Serial.println("SEND_SUCCESSFUL");
				break;
			case OPERATION_TIMED_OUT_BIT:
				Serial.println("OPERATION_TIMED_OUT");
				break;
			case NO_INTERNET_CONNECTION_BIT:
				Serial.println("NO_INTERNET_CONNECTION");
				break;
			case CANNOT_LOCATE_CLOUD_SERVER_BIT:
				Serial.println("CANNOT_LOCATE_CLOUD_SERVER");
				break;
		}
		Serial.println("******************************");
		vTaskDelay(SimulateSendingCommendPeriod);
	}
}

static void APP_Task1( void *pvParameters)
{
	uint32_t DataID;
	uint32_t RecivedData;
	uint32_t SimulateSendingCommendPeriod = MS_TO_TICK(500);

	while(1){
		DataID = random(10)+1; //Set ID form 0 ~ 10
		CloudRead(DataID,&RecivedData);
		Serial.print("\r\nAPP Task 1 Received data: ");
		Serial.println((char)RecivedData);
		vTaskDelay(SimulateSendingCommendPeriod);
	}
}

/* ulDataID identifies the data to read.  pulValue holds the address of the variable into
which the data received from the cloud server is to be written. */
CloudCommand_t xReasdRequest;
BaseType_t CloudRead( uint32_t ulDataID, uint32_t *pulValue )
{
  BaseType_t xReturn;

  /* Set the CloudCommand_t structure members to be correct for this read request. */
  xReasdRequest.eOperation = eRead;     /* This is a request to read data. */
  xReasdRequest.ulDataID = ulDataID;    /* A code that identifies the data to read. */
  xReasdRequest.xTaskToNotify = xTaskGetCurrentTaskHandle(); /* Handle of the calling task. */

  /* Ensure there are no notifications already pending by reading the notification value
  with a block time of 0, then send the structure to the server task. */
  xTaskNotifyWait( 0, 0, NULL, 0 );
  xQueueSend( xServerTaskQueue, &xReasdRequest, portMAX_DELAY );

  /* Wait for a notification from the server task. The server task writes the value
  received from the cloud server directly into this taskfs notification value, so there is
  no need to clear any bits in the notification value on entry to or exit from the
  xTaskNotifyWait() function. The received value is written to *pulValue, so pulValue is
  passed as the address to which the notification value is written. */
  xReturn = xTaskNotifyWait( 0,                   /* No bits cleared on entry. */
                             0,                   /* No bits to clear on exit. */
                             pulValue,            /* Notification value into *pulValue. */
							 portMAX_DELAY); 	  /* Wait forever. */

  /* If xReturn is pdPASS, then the value was obtained.  If xReturn is pdFAIL, then the
  request timed out. */
  return xReturn;
}
CloudCommand_t xWriteRequest;
uint32_t CloudWrite( uint32_t ulDataID, uint32_t ulDataValue )
{
  uint32_t ulNotificationValue;

  /* Set the CloudCommand_t structure members to be correct for this write request. */
  xWriteRequest.eOperation = eWrite;       /* This is a request to write data. */
  xWriteRequest.ulDataID = ulDataID;       /* A code that identifies the data being written. */
  xWriteRequest.ulDataValue = ulDataValue; /* Value of the data written to the cloud server. */
  xWriteRequest.xTaskToNotify = xTaskGetCurrentTaskHandle(); /* Handle of the calling task. */

  /* Clear the three status bits relevant to the write operation by calling
  xTaskNotifyWait() with the ulBitsToClearOnExit parameter set to
  CLOUD_WRITE_STATUS_BIT_MASK, and a block time of 0. The current notification value is
  not required, so the pulNotificationValue parameter is set to NULL. */
  xTaskNotifyWait( 0, CLOUD_WRITE_STATUS_BIT_MASK, NULL, 0 );

  /* Send the request to the server task. */
  xQueueSend( xServerTaskQueue, &xWriteRequest, portMAX_DELAY );

  /* Wait for a notification from the server task. The server task writes a bitwise status
  code into this taskfs notification value, which is written to ulNotificationValue. */
  xTaskNotifyWait( 0,                           /* No bits cleared on entry.         */
				   CLOUD_WRITE_STATUS_BIT_MASK, /* Clear relevant bits to 0 on exit. */
				   &ulNotificationValue,        /* Notified value.                   */
				   portMAX_DELAY );      		/* Wait forever.          */

  /* Return the status code to the calling task. */
  return ( ulNotificationValue & CLOUD_WRITE_STATUS_BIT_MASK );
}
/*-----------------------------------------------------------*/
