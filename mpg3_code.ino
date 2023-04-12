/****************************************************************
 * Swish and Flick  - modified Example1_Basics.ino
 * ICM 20948 Arduino Library Demo
 * Use the default configuration to stream 9-axis IMU data
 * Owen Lyke @ SparkFun Electronics
 * Original Creation Date: April 17 2019
 *
 * Please see License.md for the license information.
 *
 * Distributed as-is; no warranty is given.
 ***************************************************************/
#include "ICM_20948.h" // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
#include <Stepper.h>

//#define USE_SPI       // Uncomment this to use SPI
#define SERIAL_PORT Serial
#define SPI_PORT SPI // Your desired SPI port.       Used only when "USE_SPI" is defined
#define CS_PIN 2     // Which pin you connect CS to. Used only when "USE_SPI" is defined
#define WIRE_PORT Wire // Your desired Wire port.      Used when "USE_SPI" is not defined
// The value of the last bit of the I2C address.
// On the SparkFun 9DoF IMU breakout the default is 1, and when the ADR jumper is closed the value becomes 0
#define AD0_VAL 1 
#ifdef USE_SPI
ICM_20948_SPI myICM; // If using SPI create an ICM_20948_SPI object
#else
ICM_20948_I2C myICM; // Otherwise create an ICM_20948_I2C object
#endif
ICM_20948_I2C *myICM_ptr; 

const int stepsPerRevolution = 513;
// initialize the stepper library on pins:
// Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11); 
Stepper myStepper(stepsPerRevolution, 4, 5, 6, 3);


// int motorPin = A1; // old motor

// Threshholds for different aspects of determining the movement of our wand
float Z_ACCEL_HIGH =  1500;
float Y_GYRO_HIGH   =  175;
float Y_GYRO_LOW    = -175;

// Logic checks for:
// two peaks in z-axis acceleration
// two peaks in y-axis gyroscopic change
// one valley in y-axis gyroscopic change
// all these conditions constitute the wand movement
float z_accel_high_1 = 0;
float z_accel_high_2 = 0;
float y_gyro_high_1 = 0;
float y_gyro_high_2 = 0;
float y_gyro_low_1 = 0;

void setup()
{
  // pinMode(motorPin, OUTPUT);
  SERIAL_PORT.begin(9600);
  Serial.print("\tSDA = "); Serial.println(SDA);
  Serial.print("\tSCL = "); Serial.println(SCL);
  // set the speed at 60 rpm:
  myStepper.setSpeed(50);
  while (!SERIAL_PORT)
  {
  };

#ifdef USE_SPI
  SPI_PORT.begin();
#else
  WIRE_PORT.begin(); // 23? 24?
  WIRE_PORT.setClock(400000); // 400000
#endif

  myICM.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  bool initialized = false;
  while (!initialized)
  {

#ifdef USE_SPI
    myICM.begin(CS_PIN, SPI_PORT);
#else
    myICM.begin(WIRE_PORT, AD0_VAL);
#endif

    SERIAL_PORT.print(F("Initialization of the sensor returned: "));
    SERIAL_PORT.println(myICM.statusString());
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      SERIAL_PORT.println("Trying again...");
      delay(500);
    }
    else
    {
      initialized = true;
    }
  }
}



void loop()
{
  if (myICM.dataReady())
  {
    myICM.getAGMT();         // The values are only updated when you call 'getAGMT'
   
    myICM_ptr = &myICM;
    
    // Printing out variables to determine whether motion has been performed for debugging
    //SERIAL_PORT.println(myICM_ptr->accZ()); 
    //SERIAL_PORT.println(myICM_ptr->gyrY());

    // Verifying that two large peaks in z axis acceleration took place
    if (myICM_ptr->accZ() > Z_ACCEL_HIGH)
    {
        if (z_accel_high_1 == 0)
        {
          z_accel_high_1 = myICM_ptr->accZ();
        }
        else if (z_accel_high_2 == 0)
        {
          z_accel_high_2 = myICM_ptr->accZ();
        }
    }

    // Verifying that two large peaks in y axis gyroscopic measurement took place
    if (myICM_ptr->gyrY() > Y_GYRO_HIGH)
    {
      if (y_gyro_high_1 == 0)
      {
        y_gyro_high_1 = myICM_ptr->gyrY();
      }
      else if (y_gyro_high_2 == 0)
      {
        y_gyro_high_2 = myICM_ptr->gyrY();
      }
    }

    // Verifying one large drop in y axis gyroscopic measurement took place
    if (myICM_ptr->gyrY() < Y_GYRO_LOW)
    {
      y_gyro_low_1 = myICM_ptr->gyrY();
    }

    // If all thresholds have been met, turn motor on
    if(z_accel_high_1 != 0 && z_accel_high_2 != 0 && y_gyro_high_1 != 0 && y_gyro_high_2 != 0 && y_gyro_low_1 != 0)
    {
      SERIAL_PORT.println("\n\n\n\n WINGARDIUM LEVIOSA \n\n\n\n");

      for(int i = 0; i < 27; i++)
      {
        Serial.println(i);
        myStepper.step(-stepsPerRevolution); // remove the negative to reverse?
        delay(10);
      }
      
      // Turn motor off after spool is fully rolled up
      // delay(2000);
      //analogWrite(motorPin, 0);

      // Reset all variables
      z_accel_high_1 = 0;
      z_accel_high_2 = 0;
      y_gyro_high_1 = 0;
      y_gyro_high_2 = 0;
      y_gyro_low_1 = 0;
    }
    
    delay(30);
  }
  else
  {
    SERIAL_PORT.println("Waiting for data");
    delay(500);
  }
}










// Below here are some helper functions to print the data nicely!

void printPaddedInt16b(int16_t val)
{
  if (val > 0)
  {
    SERIAL_PORT.print(" ");
    if (val < 10000)
    {
      SERIAL_PORT.print("0");
    }
    if (val < 1000)
    {
      SERIAL_PORT.print("0");
    }
    if (val < 100)
    {
      SERIAL_PORT.print("0");
    }
    if (val < 10)
    {
      SERIAL_PORT.print("0");
    }
  }
  else
  {
    SERIAL_PORT.print("-");
    if (abs(val) < 10000)
    {
      SERIAL_PORT.print("0");
    }
    if (abs(val) < 1000)
    {
      SERIAL_PORT.print("0");
    }
    if (abs(val) < 100)
    {
      SERIAL_PORT.print("0");
    }
    if (abs(val) < 10)
    {
      SERIAL_PORT.print("0");
    }
  }
  SERIAL_PORT.print(abs(val));
}

void printRawAGMT(ICM_20948_AGMT_t agmt)
{
  SERIAL_PORT.print("RAW. Acc [ ");
  printPaddedInt16b(agmt.acc.axes.x);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.acc.axes.y);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.acc.axes.z);
  SERIAL_PORT.print(" ], Gyr [ ");
  printPaddedInt16b(agmt.gyr.axes.x);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.gyr.axes.y);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.gyr.axes.z);
  SERIAL_PORT.print(" ], Mag [ ");
  printPaddedInt16b(agmt.mag.axes.x);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.mag.axes.y);
  SERIAL_PORT.print(", ");
  printPaddedInt16b(agmt.mag.axes.z);
  SERIAL_PORT.print(" ], Tmp [ ");
  printPaddedInt16b(agmt.tmp.val);
  SERIAL_PORT.print(" ]");
  SERIAL_PORT.println();
}

void printFormattedFloat(float val, uint8_t leading, uint8_t decimals)
{
  float aval = abs(val);
  if (val < 0)
  {
    SERIAL_PORT.print("-");
  }
  else
  {
    SERIAL_PORT.print(" ");
  }
  for (uint8_t indi = 0; indi < leading; indi++)
  {
    uint32_t tenpow = 0;
    if (indi < (leading - 1))
    {
      tenpow = 1;
    }
    for (uint8_t c = 0; c < (leading - 1 - indi); c++)
    {
      tenpow *= 10;
    }
    if (aval < tenpow)
    {
      SERIAL_PORT.print("0");
    }
    else
    {
      break;
    }
  }
  if (val < 0)
  {
    SERIAL_PORT.print(-val, decimals);
  }
  else
  {
    SERIAL_PORT.print(val, decimals);
  }
}

#ifdef USE_SPI
void printScaledAGMT(ICM_20948_SPI *sensor)
{
#else
void printScaledAGMT(ICM_20948_I2C *sensor)
{
#endif
  SERIAL_PORT.print("Scaled. Acc (mg) [ ");
  printFormattedFloat(sensor->accX(), 5, 2);
  SERIAL_PORT.print(", ");
  printFormattedFloat(sensor->accY(), 5, 2);
  SERIAL_PORT.print(", ");
  printFormattedFloat(sensor->accZ(), 5, 2);
  SERIAL_PORT.print(" ], Gyr (DPS) [ ");
  printFormattedFloat(sensor->gyrX(), 5, 2);
  SERIAL_PORT.print(", ");
  printFormattedFloat(sensor->gyrY(), 5, 2);
  SERIAL_PORT.print(", ");
  printFormattedFloat(sensor->gyrZ(), 5, 2);
  SERIAL_PORT.print(" ], Mag (uT) [ ");
  printFormattedFloat(sensor->magX(), 5, 2);
  SERIAL_PORT.print(", ");
  printFormattedFloat(sensor->magY(), 5, 2);
  SERIAL_PORT.print(", ");
  printFormattedFloat(sensor->magZ(), 5, 2);
  SERIAL_PORT.print(" ], Tmp (C) [ ");
  printFormattedFloat(sensor->temp(), 5, 2);
  SERIAL_PORT.print(" ]");
  SERIAL_PORT.println();
}
