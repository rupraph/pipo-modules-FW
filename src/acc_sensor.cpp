#include "acc_sensor.h"


// review with exemples setup and acquisition structure.


// review with exemples setup and acquisition structure.

// todo: improve where the bias is applied. 
// computation of spatial position not finished. not working yet. have to check bias and calculations

// UPDATE ON STUFF TO DO
// + could not find how to easily access linear accelerations indepednant from gravity (seem difficult through dmp. best option is likely to get orientation and correct for it. )
// should check with https://github.com/UT2UH/Arduino_ICM20948_DMP_Full-Function and https://github.com/isouriadakis/Arduino_ICM20948_DMP_Full-Function see if those managed....
// see also https://github.com/ZaneL/Teensy-ICM-20948/issues/2
// otherwise likely go for a manual correction -> check maybe this, not sure https://wolles-elektronikkiste.de/en/icm-20948-9-axis-sensor-part-ii 

// with current setup, seems like acquisition rate is 20ms. enabling or disabling some sensor data does not seem to change things


void sensor::init()
{

    myICM.enableDebugging();
    initialized = false;
    while (!initialized)
    {
        myICM.begin(Wire, 1);
        Serial.println(myICM.statusString());
        if (myICM.status != ICM_20948_Stat_Ok)
        {
        Serial.println(F("Trying again..."));
        delay(500);
        }
        else
        {
        initialized = true;
        }
    }
    Serial.println(F("Device connected!"));


}

// DMP sensor options are defined in ICM_20948_DMP.h
  //    INV_ICM20948_SENSOR_ACCELEROMETER               (16-bit accel)
  //    INV_ICM20948_SENSOR_GYROSCOPE                   (16-bit gyro + 32-bit calibrated gyro)
  //    INV_ICM20948_SENSOR_RAW_ACCELEROMETER           (16-bit accel)
  //    INV_ICM20948_SENSOR_RAW_GYROSCOPE               (16-bit gyro + 32-bit calibrated gyro)
  //    INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED (16-bit compass)
  //    INV_ICM20948_SENSOR_GYROSCOPE_UNCALIBRATED      (16-bit gyro)
  //    INV_ICM20948_SENSOR_STEP_DETECTOR               (Pedometer Step Detector)
  //    INV_ICM20948_SENSOR_STEP_COUNTER                (Pedometer Step Detector)
  //    INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR        (32-bit 6-axis quaternion)
  //    INV_ICM20948_SENSOR_ROTATION_VECTOR             (32-bit 9-axis quaternion + heading accuracy)
  //    INV_ICM20948_SENSOR_GEOMAGNETIC_ROTATION_VECTOR (32-bit Geomag RV + heading accuracy)
  //    INV_ICM20948_SENSOR_GEOMAGNETIC_FIELD           (32-bit calibrated compass)
  //    INV_ICM20948_SENSOR_GRAVITY                     (32-bit 6-axis quaternion)
  //    INV_ICM20948_SENSOR_LINEAR_ACCELERATION         (16-bit accel + 32-bit 6-axis quaternion)
  //    INV_ICM20948_SENSOR_ORIENTATION                 (32-bit 9-axis quaternion + heading accuracy)



void sensor::setup()
{
    bool success = true; // Use success to show if the DMP configuration was successful
    // Initialize the DMP. initializeDMP is a weak function. You can overwrite it if you want to e.g. to change the sample rate
    
    success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);


    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ACCELEROMETER) == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_GYROSCOPE) == ICM_20948_Stat_Ok);

    ICM_20948_fss_t myFSS; // This uses a "Full Scale Settings" structure that can contain values for all configurable sensors
    myFSS.a = gpm8; //2 to 16
    //myFSS.g = dps250; // 250 to 2000
    success &= (myICM.setFullScale(ICM_20948_Internal_Acc,myFSS) == ICM_20948_Stat_Ok); // Set to the maximum


    success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok); // Set to the maximum
    success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Gyro, 0) == ICM_20948_Stat_Ok); // Set to the maximum
    success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Accel, 0) == ICM_20948_Stat_Ok); 
    

    
    // Enable the FIFO
    success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
    // Enable the DMP
    success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
    // Reset DMP
    success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
    // Reset FIFO
    success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);
    // #endif
    Serial.println("icm ready");
    if (success)
    {
        Serial.println(F("DMP enabled!"));

    }
    else
    {
        Serial.println(F("Enable DMP failed!"));
        Serial.println(F("Please check that you have uncommented line 29 (#define ICM_20948_USE_DMP) in ICM_20948_C.h..."));
        while (1)
        { // Loop forever
        }
    }
}

void sensor::update()
{
    // measure update rate
    

    // Read any DMP data waiting in the FIFO
    // Note:
    //    readDMPdataFromFIFO will return ICM_20948_Stat_FIFONoDataAvail if no data is available.
    //    If data is available, readDMPdataFromFIFO will attempt to read _one_ frame of DMP data.
    //    readDMPdataFromFIFO will return ICM_20948_Stat_FIFOIncompleteData if a frame was present but was incomplete
    //    readDMPdataFromFIFO will return ICM_20948_Stat_Ok if a valid frame was read.
    //    readDMPdataFromFIFO will return ICM_20948_Stat_FIFOMoreDataAvail if a valid frame was read _and_ the FIFO contains more (unread) data.
    
    myICM.readDMPdataFromFIFO(&data);

    if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)) // Was valid data available?
    {
        // unsigned long currentMillis = millis();
        // unsigned long interval = currentMillis - last_time;
        // last_time = currentMillis;
        // Serial.print(">interval:");
        // Serial.println(interval);

        
        //SERIAL_PORT.print(F("Received data! Header: 0x")); // Print the header in HEX so we can see what data is arriving in the FIFO
        //if ( data.header < 0x1000) SERIAL_PORT.print( "0" ); // Pad the zeros
        //if ( data.header < 0x100) SERIAL_PORT.print( "0" );
        //if ( data.header < 0x10) SERIAL_PORT.print( "0" );
        //SERIAL_PORT.println( data.header, HEX );

        if ((data.header & DMP_header_bitmap_Quat9) > 0) // We have asked for orientation data so we should receive Quat9
        {
            // Q0 value is computed from this equation: Q0^2 + Q1^2 + Q2^2 + Q3^2 = 1.
            // In case of drift, the sum will not add to 1, therefore, quaternion data need to be corrected with right bias values.
            // The quaternion data is scaled by 2^30.

            //SERIAL_PORT.printf("Quat9 data is: Q1:%ld Q2:%ld Q3:%ld Accuracy:%d\r\n", data.Quat9.Data.Q1, data.Quat9.Data.Q2, data.Quat9.Data.Q3, data.Quat9.Data.Accuracy);

            // Scale to +/- 1
            q1 = ((double)data.Quat9.Data.Q1) / 1073741824.0; // Convert to double. Divide by 2^30
            q2 = ((double)data.Quat9.Data.Q2) / 1073741824.0; // Convert to double. Divide by 2^30
            q3 = ((double)data.Quat9.Data.Q3) / 1073741824.0; // Convert to double. Divide by 2^30
            q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));

            // send to adafruit visualizer 
            if (enable_send_vizualizer)
            {
                Serial.print("Quaternion:");
                //Serial.print("qW: ");
                Serial.print(q0);
                Serial.print(",");
                Serial.print(q1);
                Serial.print(",");
                Serial.print(q2);
                Serial.print(",");
                Serial.println(q3);
            //   SERIAL_PORT.print(F(" Accuracy:"));
            //   SERIAL_PORT.println(data.Quat9.Data.Accuracy);
            // Output the Quaternion data in the format expected by ZaneL's Node.js Quaternion animation tool
            // Serial.print(F("{\"quat_w\":"));
            // Serial.print(q0, 3);
            // Serial.print(F(", \"quat_x\":"));
            // Serial.print(q1, 3);
            // Serial.print(F(", \"quat_y\":"));
            // Serial.print(q2, 3);
            // Serial.print(F(", \"quat_z\":"));
            // Serial.print(q3, 3);
            // Serial.println(F("}"));
            }
        }
        if ((data.header & DMP_header_bitmap_Accel) > 0) // We have asked for raw accel data
        {
            raw_accX = (float)data.Raw_Accel.Data.X; // Extract the raw accel data
            raw_accY = (float)data.Raw_Accel.Data.Y;
            raw_accZ = (float)data.Raw_Accel.Data.Z;

            unsigned long currentMillis = millis();
            unsigned long interval = currentMillis - last_time;
            last_time = currentMillis;

            // temporary solution to remove gravity and get linear acceleration
            accX_t=raw_accX-lp_filter_accX.process(raw_accX,interval/1000.0);
            accY_t=raw_accY-lp_filter_accY.process(raw_accY,interval/1000.0);
            accZ_t=raw_accZ-lp_filter_accZ.process(raw_accZ,interval/1000.0);
            convert_accell();

            //hp_accX=hp_filter_accX.process(raw_accX,interval/1000.0);

            Serial.print(">raw_accX:");
            Serial.println(raw_accX);
            Serial.print(">lp_accX:");
            Serial.println(sensor_dat["accX"].value);

        }
        if ((data.header & DMP_header_bitmap_Gyro) > 0) // We have asked for raw gyro data
        {
            raw_gyroX = (float)data.Raw_Gyro.Data.X; // Extract the raw gyro data
            raw_gyroY = (float)data.Raw_Gyro.Data.Y;
            raw_gyroZ = (float)data.Raw_Gyro.Data.Z;
            // Serial.print(">raw_gyroX:");
            // Serial.println(raw_gyroX);

        }
    // if  (myICM.status != ICM_20948_Stat_FIFOMoreDataAvail) // If more data is available then we should read it right away - and not delay
    // {
    //     delay(10);
    // }
    }

    // Convert the quaternions to Euler angles (roll, pitch, yaw)
    calc_euler_angles();

    }

// When flipping the sensor, the roll and pitch values are inverted. 


void sensor::calc_euler_angles()
{      // Convert the quaternions to Euler angles (roll, pitch, yaw)
      // https://en.wikipedia.org/w/index.php?title=Conversion_between_quaternions_and_Euler_angles&section=8#Source_code_2

      double q2sqr = q2 * q2;

      // roll (x-axis rotation)
      double t0 = +2.0 * (q0 * q1 + q2 * q3);
      double t1 = +1.0 - 2.0 * (q1 * q1 + q2sqr);
      sensor_dat["roll"].value = atan2(t0, t1) * 180.0 / PI;

      // pitch (y-axis rotation)
      double t2 = +2.0 * (q0 * q2 - q3 * q1);
      t2 = t2 > 1.0 ? 1.0 : t2;
      t2 = t2 < -1.0 ? -1.0 : t2;
      sensor_dat["pitch"].value = asin(t2) * 180.0 / PI;

      // yaw (z-axis rotation)
      double t3 = +2.0 * (q0 * q3 + q1 * q2);
      double t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
      sensor_dat["yaw"].value = atan2(t3, t4) * 180.0 / PI;


    //   adafruit style visualizer  
    //   Serial.print("Orientation: ");
    //   Serial.print(data_map["yaw"]);
    //   Serial.print(", ");
    //   Serial.print(data_map["pitch"]);
    //   Serial.print(", ");
    //   Serial.println(data_map["roll"]);
      
    
}

void sensor::teleplot_data(string axis)
{
    Serial.print(">");
    Serial.print(axis.c_str());
    Serial.print(": ");
    Serial.println(sensor_dat[axis].value);
}


//config

json sensor::get_config(bool debug)
{
    json config;
    for (auto const& pair : sensor_dat)
    {
        string axis_name = pair.first;
        config[axis_name]["enabled"] = sensor_dat[axis_name].enabled;
        config[axis_name]["inverted"] = sensor_dat[axis_name].inverted;
        config[axis_name]["deadZone"] = sensor_dat[axis_name].deadZone;
        config[axis_name]["value"] = sensor_dat[axis_name].value;
        config[axis_name]["offset"] = sensor_dat[axis_name].offset;
    }
    if (debug)
    {
        Serial.println("returned_sensor_get_config");
        Serial.println(config.dump(4).c_str());
        Serial.println("returned_sensor_get_config_end");
    }
    return config;
}

void sensor::set_config(json& config, bool debug)
{
    for (auto const& pair : config.items())
    {
        string axis_name = pair.key();
        sensor_dat[axis_name].enabled = config[axis_name]["enabled"];
        sensor_dat[axis_name].inverted = config[axis_name]["inverted"];
        sensor_dat[axis_name].deadZone = config[axis_name]["deadZone"];
        sensor_dat[axis_name].value = config[axis_name]["value"];
        sensor_dat[axis_name].offset = config[axis_name]["offset"];
    }
    if (debug)
    {
        Serial.println("set_sensor_config");
        Serial.println(config.dump(4).c_str());
        Serial.println("set_sensor_config_end");
    }
}



void sensor::convert_accell()
{    
    sensor_dat["accX"].value = accX_t * accel_scale_coef;
    sensor_dat["accY"].value = accY_t * accel_scale_coef; 
    sensor_dat["accZ"].value = accY_t * accel_scale_coef; 
}


//gett setters

unordered_map<string, sensor::SensorDat> sensor::get_sensor_dat_map() {
    return sensor_dat;
}

bool sensor::get_enabled(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].enabled;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

bool sensor::get_inverted(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].inverted;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

int sensor::get_deadZone(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].deadZone;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float sensor::get_value(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

float sensor::get_offset(const std::string& axis) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        return sensor_dat[axis].offset;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void sensor::set_enabled(const std::string& axis, bool value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].enabled = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void sensor::set_inverted(const std::string& axis, bool value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].inverted = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void sensor::set_deadZone(const std::string& axis, int value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].deadZone = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void sensor::set_value(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].value = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}

void sensor::set_offset(const std::string& axis, float value) {
    if(sensor_dat.find(axis) != sensor_dat.end())
        sensor_dat[axis].offset = value;
    else
        throw std::invalid_argument("Axis not found: " + axis);
}