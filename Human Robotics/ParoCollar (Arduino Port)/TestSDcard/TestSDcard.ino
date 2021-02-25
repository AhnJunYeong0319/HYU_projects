//Paro Collar - Sensor Code

#include <SPI.h>
#include <SD.h>

//Setup pins
int led1 = 13;
int Light_pin = 0;


//Global variables
int awake=1;                                //binary flag for whether collar is awake (1) or asleep(0)
int sleep_ctr=0;                            //Sleep counter, for counting down to sleep mode start during periods of inactivity          
int loop_ctr=1;                             //Loop counter, for counting loop iterations while asleep
int err_code=0;                             //Used for error checking

//Old vars
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
    String header_str="timestamp,ts_string,awake,light_val)";
    
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
int SD_writer(String ts, String ts_str, int awake, int light_val)
{
    String data_str = "";
    
    data_str += String(ts);
    data_str += ",";
    data_str += String(ts_str);
    data_str += ",";
    data_str += String(light_val);
    data_str += ",";
    
    //Serial Print
    Serial.println(data_str);
    
    //Print to SD card
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    dataFile.println(data_str);
    dataFile.close();
    
    int err=0;
    return err;
}


void setup() 
{

  Serial.begin(9600);

  //Initialize sensors
  pinMode(led1, OUTPUT);                    // Configure LED pin as output
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
}


void loop() 
{
  
  //Light data
  int light_val = analogRead(Light_pin);
  
  
  //Timestamp
  String ts_str = "NA"; //Time.timeStr();
  String ts = "NA"; //(String)Time.now();
  String ts_ms = (String)millis();
  
  ts_ms=ts_ms.substring(2);
  ts.concat(".");
  ts.concat(ts_ms);
  
  
  //Check sleep_ctr, put to sleep if above threshold
  if(sleep_ctr>=6)
  {
      awake=0;
  }
  
  old_light_val=light_val;
  
  
  ////////////////////////////////////////////
  // Awake Section
  ///////////////
  
  if(awake==1)
  {
    err_code=SD_writer(ts, ts_str, awake, light_val);  
  }
  
  
  ////////////////////////////////////////////
  // Sleep Section
  ///////////////
  
  if(awake==0)
  {
    if(loop_ctr==5)
    {
       err_code=SD_writer(ts, ts_str, awake, light_val);
       loop_ctr=1;
    }
      
  }
}
