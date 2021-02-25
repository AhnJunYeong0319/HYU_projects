//Paro Collar - Sensor Code


//Setup pins
int led1 = 13;
int Sound_pin = 1; 
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
  if(loop_ctr>10)
  {
    // Convert Max Sound value into a message and set LED
    Serial.print("MAX: ");
    Serial.print(sound_max);
    Serial.print(": ");
    if(sound_max <= 100)
    {
      Serial.print("Quiet.");
      digitalWrite(led1, LOW);
    }
    else if( (sound_max > 100) && ( sound_max <= 200) )
    {
      Serial.print("Moderate.");
      digitalWrite(led1, HIGH);
    }
    else if(sound_max > 200)
    {
      Serial.print("Loud.");
      digitalWrite(led1, HIGH);
    }
    Serial.print("\n");
    sound_max=0;
    loop_ctr=0;
  }
  
  else
  {
    int sound_val;

    // Check the Sound Value (envelope input)
    sound_val = analogRead(Sound_pin);
    
    //Compare sound_max
    if(sound_val>sound_max)
    {  sound_max=sound_val;  }
  
    // Convert envelope Sound value into a message and set LED
    Serial.print("Status: ");
    Serial.print(sound_val);
    Serial.print(": ");
    if(sound_val <= 100)
    {
      Serial.print("Quiet.");
      digitalWrite(led1, LOW);
    }
    else if( (sound_val > 100) && ( sound_val <= 200) )
    {
      Serial.print("Moderate.");
      digitalWrite(led1, HIGH);
    }
    else if(sound_val > 200)
    {
      Serial.print("Loud.");
      digitalWrite(led1, HIGH);
    }
    Serial.print("\n");
  }
  
  
  //Finish loop
  loop_ctr+=1;                                 //Increment loop_flag
  delay(500);                                   // pause for 500 ms
  
  
}
