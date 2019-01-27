# SENSE2CAN Atmel Studio Projects
This repository contains the main SENSE2CAN project code that is used on Ryerson Formula Racing's RF-19 vehicle.

## s2c_sensor_module
This is the main S2C firmware. This firmware will be flashed on all planned S2C modules. This code can be expanded on to make other S2C derivatives

## s2c_led_test
This project is used to help debug the S2C board during the bring-up process. This firmware will be flashed onto a board once it's assembled, and if the MCU survives the process, it should blink the on-board LED at a rate of 5 Hz. It's a quick and dirty preliminary test of whether the MCU is functional or not.