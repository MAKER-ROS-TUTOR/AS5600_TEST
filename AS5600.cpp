/*
    AS5600.cpp
    Copyright (C) 2017 Niryo

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "AS5600.h"

volatile long sensor_position = 0;
volatile long last_sensor_position = 0;
volatile long sensor_position_with_rotations = 0;
volatile long motor_rotation_count = 0;

volatile long motor_position_without_offset = 0;
volatile long motor_position_steps = 0;
volatile long offset = 0;

bool init_startup = true; // prevent start value to -1

/*
 * Ask for position register
 */
void init_position_sensor()
{
  
  Wire.beginTransmission(AS5600_ADDRESS); 
  Wire.write(AS5600_REG_ANGLE_H);
  Wire.endTransmission();
  delay(50);
}

/*
 * Will make sensor give a more recent value (from 2200 micros to 290 micros)
 * --> also means more noise
 * --> it is better not to activate it for normal Niryo One usage
 * 
 * !!! you need to call this before init_position_sensor !!!
 */
void speed_up_position_sensor_response_time()
{
  Wire.beginTransmission(AS5600_ADDRESS);
  Wire.write(AS5600_REG_CONF); 
  Wire.write(0x1F); // WD : 0, FTH : 111, SF : 11 : 0x1f
  Wire.endTransmission();
  delay(50);
}

/*
 * Read sensor position register
 */
int read_encoder()
{
  int angle;
  
  Wire.requestFrom(AS5600_ADDRESS, 2);
    
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  angle = ((msb & 0b00001111) << 8) + lsb;

 
  return angle;  
}

void update_current_position(int microsteps) 
{
  // read from encoder
  sensor_position = read_encoder();

  // check if motor did one rotation
  if ((sensor_position - last_sensor_position) < -AS5600_CPR_HALF) {
    ++motor_rotation_count;
  }
  else if ((sensor_position - last_sensor_position) > AS5600_CPR_HALF) {
    --motor_rotation_count;
  }

  if( init_startup ){ // prevent nagative value when startup
     motor_rotation_count =0;
     init_startup = false;
  }

  // get total sensor position
  sensor_position_with_rotations = sensor_position + AS5600_CPR * motor_rotation_count;
  last_sensor_position = sensor_position;

  // translate sensor position to motor micro steps
  motor_position_without_offset = (sensor_position_with_rotations * microsteps * STEPPER_CPR) / AS5600_CPR;
  motor_position_steps =  ( motor_position_without_offset - offset);

 // Serial.print("RAW CPR: ");
 // Serial.println( sensor_position);
//  Serial.print(" ,MOTOR POS: ");
  Serial.println(motor_position_steps);

 
  
}
