#include "acc_sensor.h"

// review with exemples setup and acquisition structure.

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

void sensor::setup()
{
    bool success = true; // Use success to show if the DMP configuration was successful
    // Initialize the DMP. initializeDMP is a weak function. You can overwrite it if you want to e.g. to change the sample rate
    
    success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ORIENTATION) == ICM_20948_Stat_Ok);
    success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok); // Set to the maximum
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
      data_map["roll"] = atan2(t0, t1) * 180.0 / PI;

      // pitch (y-axis rotation)
      double t2 = +2.0 * (q0 * q2 - q3 * q1);
      t2 = t2 > 1.0 ? 1.0 : t2;
      t2 = t2 < -1.0 ? -1.0 : t2;
      data_map["pitch"] = asin(t2) * 180.0 / PI;

      // yaw (z-axis rotation)
      double t3 = +2.0 * (q0 * q3 + q1 * q2);
      double t4 = +1.0 - 2.0 * (q2sqr + q3 * q3);
      data_map["yaw"] = atan2(t3, t4) * 180.0 / PI;


    //   adafruit style visualizer  
    //   Serial.print("Orientation: ");
    //   Serial.print(data_map["yaw"]);
    //   Serial.print(", ");
    //   Serial.print(data_map["pitch"]);
    //   Serial.print(", ");
    //   Serial.println(data_map["roll"]);
      
    
}