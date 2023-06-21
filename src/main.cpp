/*
 * Copyright (c) 2023, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "as5047x.h"
#include "mbed.h"
#include "motor_sensor_AS5047x.h"

// Encoder parameters
#define UPDATE_RATE 50ms
#define ENC_RESOLUTION 16384
#define MOTOR_REDUCTION 50
#define ENC_WHEEL_RADIUS 0.035f
SPI spiAS5047p(PA_7, PA_6, PA_5); // mosi, miso, sclk
#define AS5047_CS PA_8
sixtron::AS5047x sensor_as5047(&spiAS5047p, AS5047_CS);

// Sensor update
#define SENSOR_FLAG 0x01
Ticker SensorFlagUpdateTicker;
EventFlags SensorFlag;

static bool print_values = false;

void SensorFlagUpdate()
{
    // Using flag, because can't call an SPI object while in ISR context
    SensorFlag.set(SENSOR_FLAG);
}

int main()
{

    // Convert current rate of the loop in seconds (float)
    auto f_secs = std::chrono::duration_cast<std::chrono::duration<float>>(UPDATE_RATE);
    float dt_pid = f_secs.count();

    // Create AS5047p sensor
    sixtron::MotorSensorAS5047x my_sensor(&sensor_as5047,
            dt_pid,
            ENC_RESOLUTION,
            ENC_RESOLUTION * MOTOR_REDUCTION,
            ENC_WHEEL_RADIUS,
            DIR_INVERTED);

    // Init sensor
    my_sensor.init();

    // Setup ticker to update odometry at exactly the defined rate
    SensorFlagUpdateTicker.attach(&SensorFlagUpdate, UPDATE_RATE);

    while (true) {
        // Waiting for snesor update flag
        SensorFlag.wait_any(SENSOR_FLAG);

        // Update sensor
        my_sensor.update();

        print_values = !print_values;
        if (print_values) {
            printf("Sensor values : %8lldticks, %2.3fm/s, %2.3frad\n",
                    my_sensor.getTickCount(),
                    my_sensor.getSpeed(),
                    my_sensor.getAngle());
        }
    }
}
