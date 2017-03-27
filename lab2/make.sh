#!/bin/sh
arduino --board arduino:avr:mega:cpu=atmega2560 --port /dev/ttyACM0 --upload ./freeRTOS_Blink_AnalogRead.ino

