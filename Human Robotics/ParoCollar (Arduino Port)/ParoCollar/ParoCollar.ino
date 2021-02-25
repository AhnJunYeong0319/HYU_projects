//Paro Collar - Sensor Code
//#include "particle-photon-sd-card-library.h"
//#include "SFE_MMA8452Q.h"
#include <Wire.h>                 // Must include Wire library for I2C
#include <SparkFun_MMA8452Q.h>    // Click here to get the library: http://librarymanager/All#SparkFun_MMA8452Q
#include <SPI.h>
#include <SD.h>

//Run code even without connection to Spark Cloud
//SYSTEM_MODE(MANUAL);

//Setup pins
int led1 = 13;
int Light_pin = 0;
int Sound_pin = 1;

//Create 3D accelerometer object
MMA8452Q accel;

//Global variables
int awake=1;                                //binary flag for whether collar is awake (1) or asleep(0)
int sleep_ctr=0;                            //Sleep counter, for counting down to sleep mode start during periods of inactivity          
int loop_ctr=1;                             //Loop counter, for counting loop iterations while asleep
int err_code=0;                             //Used for error checking
int battery_wake=0;                         //Flag used for waking battery by setting pins high (1=active or 0=inactive)
float battery_ctr=0.0;                      //Battery Wake Counter

//Old vars
float old_cx=0;
float old_cy=0;
float old_cz=0;
int old_sound_val=0;
int old_light_val=0;


// SD card Setup //
// Set SD variables using SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

//Define SD variables
#define FILE_SIZE_MB 1
#define FILE_SIZE (1000000UL*FILE_SIZE_MB)
#define BUF_SIZE 100

//Define SD Pins
#define SCK_pin 13
#define SS_pin 10
#define CD_pin 4



//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Useful Functions
//
/////////////////////////////////////////////////


//For intializing SD card with header line
int init_SD_header()
{
    String header_str="timestamp,ts_string,awake,sound_val,sound_cat,light_val,accel_x,accel_y,accel_z,orient,motion_detect,sound_detect)";
    
    //Serial Print
    Serial.println(header_str);
    
    //Print to SD card
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    dataFile.println(header_str);
    dataFile.close();
    
    int err=0;
    return err;
}


//For writing sensor data to SD card
int SD_writer(String ts, String ts_str, int awake, int sound_val, String sound_cat, int light_val, float accel_x, float accel_y, float accel_z, String orient, int motion_detect, int sound_detect)
{
    String data_str = "";
    
    data_str += String(ts);
    data_str += ",";
    data_str += String(ts_str);
    data_str += ",";
    data_str += String(awake);
    data_str += ",";
    data_str += String(sound_val);
    data_str += ",";
    data_str += String(sound_cat);
    data_str += ",";
    data_str += String(light_val);
    data_str += ",";
    data_str += String(accel_x);
    data_str += ",";
    data_str += String(accel_y);
    data_str += ",";
    data_str += String(accel_z);
    data_str += ",";
    data_str += String(orient);
    data_str += ",";
    data_str += String(motion_detect);
    data_str += ",";
    data_str += String(sound_detect);
    
    //Serial Print
    Serial.println(data_str);
    
    //Print to SD card
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    dataFile.println(data_str);
    dataFile.close();
    
    int err=0;
    return err;
}


//Takes in sound_val integer, returns sound category as string
String Get_SoundCat(int sound_val)
{
   String sound_cat;
   if(sound_val <= 100)
   {
     sound_cat="Quiet";
     digitalWrite(led1, LOW);
   }
   else if( (sound_val > 100) && (sound_val <= 200) )
   {
     sound_cat="Moderate";
     digitalWrite(led1, HIGH);
   }
   else if(sound_val > 200)
   {
     sound_cat="Loud";
     digitalWrite(led1, HIGH);
   }
   
   return sound_cat;
} 


String Get_3Daccel_Orient(byte pl)
{
   /* Takes pl byte from accel.readPL() containing information
      about the orientation of the sensor. It will be either
      PORTRAIT_U, PORTRAIT_D, LANDSCAPE_R, LANDSCAPE_L, or
      LOCKOUT. Reurn string.*/
  
   String orient;
   switch (pl)
   {
   case PORTRAIT_U:
     orient="Portrait Up";
     break;
   case PORTRAIT_D:
     orient="Portrait Down";
     break;
   case LANDSCAPE_R:
     orient="Landscape Right";
     break;
   case LANDSCAPE_L:
     orient="Landscape Left";
     break;
   case LOCKOUT:
     orient="Flat";
     break;
   }
  
   return orient;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main Program
//
////////////////////////////////////////////////

void setup() 
{

  Serial.begin(9600);

  //Initialize sensors
  pinMode(led1, OUTPUT);                    // Configure LED pin as output
  accel.init();                             // Initialize 3D acceleromer
  Serial.println("Sensors initialized");
  
  //Initialize SD Card
  Serial.print("Starting SD Card");

  if (!SD.begin(SS_pin)) 
  {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("Card initialized");
  
  //Initialize SD Data Header
  err_code=init_SD_header();
  
  //Battery Wake Pin Setup
  pinMode(2, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
}


void loop() 
{
  
  ////////////////////////////////////////////
  // Read Sensors
  //////////////
  
  //3D accelerator data
  int accel_wait=0;
  while(accel_wait==0)                                          //Make sure to wait till there is new 3D accelerometer data
  {
    if(accel.available())
    {
      accel.read();
      accel_wait=1;
    }
  }
  
  float accel_x=accel.cx;                                       //Get calculated X,Y,Z values from 3D accelerometer
  float accel_y=accel.cy;
  float accel_z=accel.cz;
  
  byte pl = accel.readPL();                                     //accel.readPL() will return a byte containing information about the orientation of the sensor.
  String orient = Get_3Daccel_Orient(pl);
  
  //Motion/FreeFall Detection
  /*byte source = accel.readRegister(0x16);
  accel.writeRegister(0x15, 0xB8);                                     //Enable Latch, Freefall, X-axis, Y-axis and Z-axis
  accel.writeRegister(0x17, 0x30);                                     //Set Motion Threshold to 48 counts
  accel.writeRegister(0x17, 0x03);                                     //Set FreeFall Threshold to 3 counts
  accel.writeRegister(0x18, 0x0A);                                     //100 ms debounce timing
  accel.writeRegister(0x2D, 0x04);
  accel.writeRegister(0x2E,0x04);
  
  int TM_FF_val=digitalRead(D3);
  Serial.println(TM_FF_val);*/
  
  
  //Sound data
  int sound_val = analogRead(Sound_pin);                        //Check the Sound Value (envelope input)
  String sound_cat = Get_SoundCat(sound_val);                   //Get sound_cat
  
  //Light data
  int light_val = analogRead(Light_pin);
  
  
  //Timestamp
  String ts_str = "NA"; //Time.timeStr();
  String ts = "NA"; //(String)Time.now();
  String ts_ms = (String)millis();
  
  ts_ms=ts_ms.substring(2);
  ts.concat(".");
  ts.concat(ts_ms);
  
  
  
  ////////////////////////////////////////////
  // Set Awake/Sleep status
  //////////////////////
  
  int motion_detect=0;
  int sound_detect=0;
  
  
  //Check 3D Accelerometer
  int comp_x=abs(abs(accel_x*100)-abs(old_cx*100));                                     //Everything has to be multiplied by 100, cause abs() only operates on integers and auto-rounds
  int comp_y=abs(abs(accel_y*100)-abs(old_cy*100));
  int comp_z=abs(abs(accel_z*100)-abs(old_cz*100));
  if(comp_x>5 || comp_y>5 || comp_z>5)
  {
     motion_detect=1; 
     awake=1;
     sleep_ctr=0;
  }
  
  //Check Sound
  else if(sound_cat!="Quiet" or sound_val>(old_sound_val*1.33) or sound_val<(old_sound_val*0.67))
  {
      sound_detect=1;
      awake=1;
      sleep_ctr=0;
  }
  
  //Check Ambient Light                                                 //Unimplemented still because unclear what light level changes are directly due to robot interaction
  /*else if()
  {
      awake=1;  
      sleep_ctr=0;
  }*/
  
  //Else increment sleep_ctr
  else
  {
      sleep_ctr+=1;
  }
  
  
  //Check sleep_ctr, put to sleep if above threshold
  if(sleep_ctr>=6)
  {
      awake=0;
  }
  
  //Set old vars for comparison on next loop iteration
  old_cx=accel_x;
  old_cy=accel_y;
  old_cz=accel_z;
  old_sound_val=sound_val;
  old_light_val=light_val;
  
  
  ////////////////////////////////////////////
  // Awake Section
  ///////////////
  
  if(awake==1)
  {
    err_code=SD_writer(ts, ts_str, awake, sound_val, sound_cat, light_val, accel_x, accel_y, accel_z, orient, motion_detect, sound_detect);  
  }
  
  
  ////////////////////////////////////////////
  // Sleep Section
  ///////////////
  
  if(awake==0)
  {
    if(loop_ctr==5)
    {
       err_code=SD_writer(ts, ts_str, awake, sound_val, sound_cat, light_val, accel_x, accel_y, accel_z, orient, motion_detect, sound_detect);
       loop_ctr=1;
    }
      
  }
  
  
  ////////////////////////////////////////////
  // Battery Wake Section
  /////////////////////
  
  if(battery_wake==0 and battery_ctr>55)
  {
      battery_wake=1;
      digitalWrite(2, HIGH);
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
      digitalWrite(7, HIGH);
  }
  
  else if(battery_wake==1 and battery_ctr>55.2)
  {
      battery_wake=0;
      battery_ctr=0.0;
      digitalWrite(2, LOW);
      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
      digitalWrite(7, LOW);
  }
  
  
    ////////////////////////////////////////////
  // Set Delays, based on Awake status
  /////////////////////
  
  if(awake==0)
  {
     //Finish loop
     loop_ctr+=1;                                       //Increment loop_ctr
     battery_ctr+=1;                                    //Increment battery_ctr
     delay(1000);                                       //pause for 1 second
  }
  
  else if(awake==1)
  {
     battery_ctr+=0.1;                                  //Increment battery_ctr
     delay(90);                                         //pause for 90 milliseconds (to create approx. 100 ms samplings)
  }
}
