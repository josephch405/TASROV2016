#include <Servo.h>
#include <Wire.h>
#include "definitions.h"

Servo SERVOS[6];
int SRV[] = {22, 33, 34, 35, 36, 37};

// const static uint8_t i2c_bus_addresses[9] = {0, 0x64, 0x65, 0x66, 0x67, 0x68,
// 0x69, 0x6A, 0x6B};
const static uint8_t i2c_bus_addresses[9] = {0,    0x64, 0x65, 0x66, 0x67,
                                             0x68, 0x69, 0x6A, 0x6B};
static uint8_t i2c_motor_fault[8] = {0, 0, 0, 0, 0, 0, 0, 0};
const static uint8_t i2c_register_address = 0x00;

const static uint8_t slave_address = 11;
volatile static uint8_t servo_angles[7] = {0, 0, 0, 0, 0, 0, 0};

volatile static uint8_t motor_direction[6] = {1, 1, 1, 1, 1, 1};
volatile static uint8_t motor_speed[6] = {0, 0, 0, 0, 0, 0};

void setup() {
  delay(1000);   // Allow Attiny85s to initialize
  Wire.begin();  // I2C
  controlCOM.begin(38400);
  initPins();
}

void loop() {
  while (true) {
    delay(5);

    //process complete signals received from onshore
    if (controlCOM.available() > 14 && controlCOM.read() == 0xAA) {
      digitalWrite(13, HIGH);
      for (int i = 0; i < 6; x++) {
        motor_speed[x] = controlCOM.read();
        motor_direction[x] = controlCOM.read();
      };
      for (int i = 0; i < 7; x++) {
        servo_angles[x] = controlCOM.read();
      }
    }

    // pushes values to motors via i2c
    // notice that motor channels are shifted +1
    for (int i = 0; i < 6; i++) {
      motorWrite(i + 1, motor_direction[i], motor_speed[i]);
      i2c_motor_fault[i] = motorReadStatus(i + 1);
    }

    for (int x = 0; x < 6; x++) {
      SERVOS[x].write(servo_angles[x]);
    }
    /*
    PLACEHOLDER CODE: pushes last servo to bot2
     */

    // calculates and returns a fault byte
    uint8_t motor_fault_byte;
    bitWrite(motor_fault_byte, 0, i2c_motor_fault[0]);  // need to revert
    bitWrite(motor_fault_byte, 1, i2c_motor_fault[1]);
    bitWrite(motor_fault_byte, 2, i2c_motor_fault[2]);
    bitWrite(motor_fault_byte, 3, i2c_motor_fault[3]);
    bitWrite(motor_fault_byte, 4, i2c_motor_fault[4]);
    bitWrite(motor_fault_byte, 5, i2c_motor_fault[5]);

    // if (motor_fault_byte) digitalWrite(STATUS_LED_1, HIGH);
    // else digitalWrite(STATUS_LED_1, LOW);

    // digitalWrite(STATUS_LED_2, LOW);

    // controlCOM.println(motor_speed[0]);
    
    Wire.requestFrom(11,4);
  }
}

/**
   [motorReadStatus]
   @param  channel Channel #, 1-8
   @return         True/False; True for FAULT
*/
uint8_t motorReadStatus(uint8_t channel) {
  Wire.requestFrom(i2c_bus_addresses[channel], 1);
  uint8_t status;
  while (Wire.available()) status = Wire.read();
  if (!status)
    return true;
  else
    return false;
}

/**
   [motorWrite]
   @param  channel   Channel #, 1-8
   @param  direction Direction, FORWARD or REVERSE
   @param  power     Motor speed, 0-255
*/
void motorWrite(uint8_t channel, uint8_t direction, uint8_t power) {
  if (direction)
    bitSet(power, 0);
  else
    bitClear(power, 0);

  Wire.beginTransmission(i2c_bus_addresses[channel]);
  Wire.write(i2c_register_address);
  Wire.write(power);
  Wire.endTransmission();
}

void servoPush() {
  Wire.beginTransmission(slave_address);
  // Wire.write(i2c_register_address);
  for (int i = 0; i < sizeof(servo_angles); i++) {
    Wire.write(servo_angles[i]);
  }
  Wire.endTransmission();
}