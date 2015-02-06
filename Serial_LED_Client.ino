#include "test.h" //Include VITAL RGB TILE FUNCTIONS!
#include "Energia.h"
#define mode_max 6//Define the maximum number of modes, so the pushbuttons will iterate through them all properly. Mode 0 is direct bluetooth command, and Mode 1 is the default start-up mode (rgb mixer). 
#define pingPin P2_5
#define rpingPin P2_6
unsigned char mode = 1; //Global mode variable allows board to save/switch between different operating modes.


//Settings for strobe
char s_r = 255;
char s_g = 255;
char s_b = 255;

//setting for sound activated colors
char ccolor = 0;
int avg = 1023; //variable for keeping a running average of the sound levels. Start at 1023 to prevent accidental triggering in a low volume situation

//setting for rainbow fade
// global variables
unsigned int rbow;     //0-360
unsigned int relative; //0-59
unsigned char rr, rg, rb;

//setting for clap activated white light
boolean clapper = 0;


void _set_rgb(char command[128], char command_length, char base_length){
  //parse out the individual strings
  char temp_num[4]; //3 chars, plus a null character to terminate it as a string for atoi()
  char color = 2;
  char pos;
  char temp = command_length - 3; //we want an index of the leftmost data value (not ")" or ";" )
  unsigned char r,g,b;
  while ( temp > base_length){
     temp_num[0] = '0';//clear the buffer for a single color
     temp_num[1] = '0';
     temp_num[2] = '0';
     temp_num[3] = 0; //null character termination
     pos = 2;
    while (command[temp] != ',' && command[temp] != '('){ //parse out separate colors
        temp_num[pos] = command[temp]; //data? I hope so!
        temp--;
        pos--;
        if (pos<-1) { //You forgot a comma, or put WAY too many digits...
           invalid_input();
           return;
        }
    }
    temp--;

    //the temp_num array should now contain a 3 digit padded null terminated ascii representation of a number
    if (color == 2) b = atoi(temp_num);
    if (color == 1) g = atoi(temp_num);
    if (color == 0) r = atoi(temp_num);
    color--; //now read in the next color   
  }
  set_rgb(r,g,b);
}


void _target_rgb(char command[128], char command_length, char base_length){
  
  //!!! THIS IS A BLOCKING COMMAND!!!
 
  //parse out the individual strings
  char temp_num[11]; //10 chars, plus a null character to terminate it as a string for atoi()
  char color = 3; //color here also includes the target time
  char pos;
  char temp = command_length - 3; //we want an index of the leftmost data value (not ")" or ";" )
  unsigned char r,g,b;
  unsigned int t; //store the target fade time
  
  while ( temp > base_length){
     temp_num[0] = '0';//clear the buffer for a single color
     temp_num[1] = '0';
     temp_num[2] = '0';
     temp_num[3] = '0';
     temp_num[4] = '0';
     temp_num[5] = '0';
     temp_num[6] = '0';
     temp_num[7] = '0';
     temp_num[8] = '0';
     temp_num[9] = '0';
     temp_num[10] = '0';
     temp_num[11] = 0; //null character termination
     pos = 10;
    while (command[temp] != ',' && command[temp] != '('){ //parse out separate colors
        temp_num[pos] = command[temp]; //data? I hope so!
        temp--;
        pos--;
        if (pos<-1) { //You forgot a comma, or put WAY too many digits...
           invalid_input();
           return;
        }
    }
    temp--;

    //the temp_num array should now contain a 3 digit padded null terminated ascii representation of a number
    if (color == 3) t = atoi(temp_num);
    if (color == 2) b = atoi(temp_num);
    if (color == 1) g = atoi(temp_num);
    if (color == 0) r = atoi(temp_num);
    color--; //now read in the next color   
  }
  target_rgb(r,g,b,t);  
}

void reboot(){
  set_rgb(0,0,0);
  s_r = 255;
  s_g = 255;
  s_b = 255;
  avg = 1023;//reset the sound average.
  setup();
}


 void invalid_input(){
   Serial.println("Error in input command. Type ""help();"" for available commands."); 
 }

void establishContact() {
  Serial.println(" _______  ___   ___      _______  ______"); 
  Serial.println("|       ||   | |   |    |       ||      |");
  Serial.println("|    _  ||   | |   |    |    ___||  _    |");
  Serial.println("|   |_| ||   | |   |    |   |___ | | |   |");
  Serial.println("|    ___||   | |   |___ |    ___|| |_|   |");
  Serial.println("|   |    |   | |       ||   |___ |       |");
  Serial.println("|___|    |___| |_______||_______||______|");
  Serial.println("\nSerial command shell v1.2");
  Serial.println("Copyright 2012 LIB3 Inc.");
  Serial.println("__________________________________________\n\n");
  Serial.print('>');
}



//dec_mode and inc_mode serve as the ISR functions for the two pushbuttons
void dec_mode(){
  mode--;
  if (mode == 0){
    mode = mode_max; //if we cycle to the end, continue at other end
  }
}

void inc_mode(){
  mode++;
  if (mode > mode_max){
    mode = 1;
  }  
}


void setup()
{
       setup_piled();
       mode = 1;          //Defaults to RGB mixer mode. 
       establishContact();  // send the fun initialization data OUT     
}


long microsecondsToInches(long microseconds)
{
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  return microseconds / 29 / 2;
}


int superfilter;

void loop()
{
  //Variables for serial parsing
char command[32]; //command buffer
int command_length = 0; //length of command (ending with a ;)
char command_temp = 0;
int base_length = 0;
  //Check mode, and do mode-specific stuff\
 
      set_rgb(255,0,0);
      delay(1000);
     set_rgb(0,255,0);
      delay(1000);
       set_rgb(0,0,255);
      delay(1000);

   //Demotest - read in ultrasonic sensor
    
    while(1){
     // establish variables for duration of the ping, 
    // and the distance result in inches and centimeters:
    long duration, inches, cm;

    // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(20);
    digitalWrite(pingPin, LOW);

     // The same pin is used to read the signal from the PING))): a HIGH
    // pulse whose duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(pingPin, INPUT);
    duration = pulseIn(rpingPin, HIGH);

    // convert the time into a distance
    inches = microsecondsToInches(duration);
    cm = microsecondsToCentimeters(duration);
  
    Serial.print(inches);
    Serial.print("in, ");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println();
        int varrr = inches * 5;
        if (varrr > 255) varrr = 255;
        superfilter *= 7;
       superfilter += varrr;
        superfilter /= 8;
        set_rgb(superfilter,superfilter,superfilter);
    }
  
  //check for serial stuff... Max length is 32 chars
  
     
     
  
}




