#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskSerialPort( void *pvParameters );
void TaskSerialPort_Priority( void *pvParameters );
void TaskHookCheckOut( void );

uint32_t idleCounter = 0 ;
TaskHandle_t HdrTask2 ; // task handler for task 2

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);



  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskBlink // Task function name
    ,  (const portCHAR *)"Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL // by pass paramater to task function
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL ); // task handler pointer address

  /*
  xTaskCreate(
    TaskAnalogRead
    ,  (const portCHAR *) "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

    */

  xTaskCreate(
    TaskSerialPort
    ,  (const portCHAR *) "TaskSerialPort 11"
    ,  128  // Stack size
    ,  (void*) 0x01
    ,  2  // Priority
    ,  NULL );

  xTaskCreate(
    TaskSerialPort
    ,  (const portCHAR *) "TaskSerialPort 22"
    ,  128  // Stack size
    , (void*) 0x02
    ,  1  // Priority
    ,  &HdrTask2 );

  xTaskCreate(
    TaskSerialPort_Priority
    ,  (const portCHAR *) "TaskSerialPort_Priority 11"
    ,  128  // Stack size
    , (void*) "TaskSerialPort_Priority 11"
    ,  2  // Priority
    ,  NULL );

  xTaskCreate(
    TaskHookCheckOut
    ,  (const portCHAR *) "TaskHookCheckOut"
    ,  128  // Stack size
    , (void*) "TaskHookCheckOut"
    ,  5  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}


void vApplicationIdleHook(void){
  idleCounter ++;
}

void TaskHookCheckOut( void *pvParameters){
  while(1){
    Serial.print("idleCounter = ");
    Serial.println( idleCounter );
    vTaskDelay( 5000/ portTICK_PERIOD_MS );
  }

}

void TaskSerialPort( void *pvParameters){
  /*int counter = 0;*/
  uint32_t counter = 0;


  while(1){
    counter ++;
    UBaseType_t myPriority = uxTaskPriorityGet( NULL );
    Serial.print(counter);
    Serial.print(" : My Priority = ");
    Serial.print(myPriority);
    Serial.print(" : task=");
    Serial.println( ( int ) pvParameters );

    delay(200);
    switch( (int) pvParameters ){
      case 0x01 :
        vTaskPrioritySet(HdrTask2 , myPriority+1 );
        break;
      case 0x02 :
        vTaskPrioritySet(HdrTask2 , myPriority-2 );
        break;
    }
    /*_delay_ms(500);*/
    /*vTaskDelay( 2500/ portTICK_PERIOD_MS );*/
    /*TickType_t xLastTickTime;*/
    /*vTaskDelayUntil( & xLastTickTime , 2500/ portTICK_PERIOD_MS );*/
  }

}

void TaskSerialPort_Priority( void *pvParameters){
  uint32_t counter = 0;
  while(1){
    counter ++;
    Serial.print(counter);
    Serial.println( ( char*) pvParameters );
    /*delay(1000);*/
    vTaskDelay( 5000/ portTICK_PERIOD_MS );
  }

}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, LEONARDO, MEGA, and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN takes care
  of use the correct LED pin whatever is the board used.

  The MICRO does not have a LED_BUILTIN available. For the MICRO board please substitute
  the LED_BUILTIN definition with either LED_BUILTIN_RX or LED_BUILTIN_TX.
  e.g. pinMode(LED_BUILTIN_RX, OUTPUT); etc.

  If you want to know what pin the on-board LED is connected to on your Arduino model, check
  the Technical Specs of your board  at https://www.arduino.cc/en/Main/Products

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald

  modified 2 Sep 2016
  by Arturo Guadalupi
*/

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for one second
  }
}

void TaskAnalogRead(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A0);
    // print out the value you read:
    Serial.println(sensorValue);
    vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
  }
}
