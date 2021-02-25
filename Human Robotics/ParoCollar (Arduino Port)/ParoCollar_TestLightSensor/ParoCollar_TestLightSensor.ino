//Paro Collar - Sensor Code


//Setup pins
int led1 = 13;
int Light_pin = 0; 
int loop_ctr=0;
int sound_max=0;



void setup() 
{

  Serial.begin(9600);

  //  Configure LED pin as output
  pinMode(led1, OUTPUT);
  
  // Display status
  Serial.println("Initialized");

}


void loop() 
{
    int sound_val;

    //Light data
    int light_val = analogRead(Light_pin);
    
    
    Serial.print("Status: ");
    Serial.print(light_val);
    Serial.print(": ");

    if(light_val <= 10)
    {
      Serial.print("Dark.");
      digitalWrite(led1, LOW);
    }
    else if( (light_val > 10) && ( light_val <= 200) )
    {
      Serial.print("Dim.");
      digitalWrite(led1, HIGH);
    }
    else if(light_val > 200 && ( light_val <= 500)  )
    {
      Serial.print("Light.");
      digitalWrite(led1, HIGH);
    }
    else if(light_val > 500 && ( light_val <= 800)  )
    {
      Serial.print("Bright.");
      digitalWrite(led1, HIGH);
    }
    else if(light_val > 800  )
    {
      Serial.print("Very Bright.");
      digitalWrite(led1, HIGH);
    }
    Serial.print("\n");
  
  
    //Finish loop
    loop_ctr+=1;                                 //Increment loop_flag
    delay(200);                                   // pause for 500 ms
  
  
}
