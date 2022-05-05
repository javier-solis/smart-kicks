void setup_imu(){
  while (imu.readByte(MPU9255_ADDRESS, WHO_AM_I_MPU9255) != 0x73){
    Serial.println("NOT FOUND");
  }
  imu.initMPU9255();
  imu.MPU9255SelfTest(imu.selfTest);
  imu.calibrateMPU9255(imu.gyroBias, imu.accelBias);
  imu.initMPU9255();  
  imu.initAK8963(imu.factoryMagCalibration);
  imu.getAres(); //call this so the IMU internally knows its range/resolution
}