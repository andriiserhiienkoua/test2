#include <MPU9250_asukiaaa.h>

MPU9250_asukiaaa mpu_sensor(0x69);


struct mpu{
  float aX;
  float aY;
  float aZ;
  float gX;
  float gY;
  float gZ;
  } mpu_data;

bool initMPU(){
  uint8_t sensorId;
  mpu_sensor.setWire(&Wire);
  mpu_sensor.beginAccel();
  mpu_sensor.beginGyro();
  mpu_sensor.beginMag();
  return true;
}

mpu * get_mpu_data(){
   mpu_sensor.accelUpdate();
   mpu_sensor.gyroUpdate();
//   mpu_data.aX = mpu_sensor.accelX() - 1.13000002;
//   mpu_data.aY = mpu_sensor.accelY() + 0.34;
//   mpu_data.aZ = mpu_sensor.accelZ() - 0.615;

   mpu_data.aX = mpu_sensor.accelX();
   mpu_data.aY = mpu_sensor.accelY() - 0.008;
   mpu_data.aZ = mpu_sensor.accelZ() + 0.011;



//   mpu_data.aX = mpu_sensor.accelX() - 0.01;
//   mpu_data.aY = mpu_sensor.accelY() + 0.01;
//   mpu_data.aZ = mpu_sensor.accelZ() + 0.012;
   

//   mpu_data.gX = mpu_sensor.gyroX() - 0.7215;
//   mpu_data.gY = mpu_sensor.gyroY() + 1.5775;
//   mpu_data.gZ = mpu_sensor.gyroZ() - 0.291;


   mpu_data.gX = mpu_sensor.gyroX() - 2.3;
   mpu_data.gY = mpu_sensor.gyroY() - 0.5;
   mpu_data.gZ = mpu_sensor.gyroZ() - 0.9;
   


//   mpu_data.gX = mpu_sensor.gyroX() - 0.8;
//   mpu_data.gY = mpu_sensor.gyroY() + 1.7;
//   mpu_data.gZ = mpu_sensor.gyroZ() - 0.3;
   
//////////////////////////////////////////////////////////
   
   return &mpu_data;
}

void print_data(mpu * data_){
    /*
    uint8_t sensorId = 0x69;
    if (mySensor.readId(&sensorId) == 0) 
        Serial.println("MPU " + String(sensorId));
    else 
        Serial.println("Cannot read sensorId");
    */
    Serial.print("aX = ");
    Serial.print(data_->aX);
    Serial.print(" | ");
    Serial.print("aY = ");
    Serial.print(data_->aY);
    Serial.print(" | ");
    Serial.print("aZ = ");
    Serial.println(data_->aZ);

    Serial.println("------------------------------------");
    
    Serial.print("gX = ");
    Serial.print(data_->gX);
    Serial.print(" | ");
    Serial.print("gY = ");
    Serial.print(data_->gY);
    Serial.print(" | ");
    Serial.print("gZ = ");
    Serial.println(data_->gZ);

}
