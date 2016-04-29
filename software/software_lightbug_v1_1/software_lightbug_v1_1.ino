// Lightbug v 1.1
// by Michael Happl
// based on Lightfollower V1.0 by Johann Leinweber
// Adds a serial communication with simple commands to make it possible
// to control the robot over bluetooth.

#include <Servo.h> 

#define led_left 2           //digital pin for the left LED
#define led_right 3          //digital pin for the right LED
#define led_mode 5           //digital pin for the mode LED
#define servopin_left 10     //digital pin for the left servo
#define servopin_right 9     //didital pin for the right servo
#define servo_high 150       //value for the left servo
#define servo_low 30         //value for the right servo

Servo myservo_left;          //create servo object to control a servo
Servo myservo_right;         //create servo object to control a servo

int left_LDR = A1;           //analog pin used to connect the left LDR
int right_LDR = A3;          //analog pin used to connect the right LDR
int trh_time = 6000;         //time for measuring thresholdvalue in ms
//int time = 0;                //temporary time for measuring time
int val_left = 0;            //variable to read the value from left LDR
int val_right = 0;           //variable to read the value from left LDR
int mean_left = 0;           //meanvalue left LDR
int mean_right = 0;          //meanvalue right LDR
int trh_left = 0;            //threshold for left LDR
int trh_right = 0;           //threshold for right LDR
int loop_time = 10;          //execute loop only every 10ms
unsigned long time = 0;      //variable for measuring time

// State flag
boolean auto_mode = true;

//====================== Serial stuff for the bluetooth module ======================
// constants
#define Baudrate 9600                //baudrate for communicating with the HC-06 Bluetooth module

// variables
boolean debug = 1;                   //Debug mode for testing the commands
boolean command_ok = false;          //indicates finished command receive

char command_string[3];              //the received characters, max. 3
char sensor_left_string[] = "LLX#";  //placeholder for the left sensor value
char sensor_right_string[] = "LRX#"; //placeholder for the rigth sensor value
int char_index = 0;                  //index of the received character


//====================== Setup ======================
void setup()                              //setup, will be executed only once at the start
{   
  pinMode(led_left,OUTPUT);                   //left LED pin as output
  pinMode(led_right,OUTPUT);                  //right LED pin as output
  pinMode(led_mode,OUTPUT);                   //mode LED pin as output
  digitalWrite(led_left, HIGH);               //switch LED left on
  digitalWrite(led_right, HIGH);              //switch LED right on
  digitalWrite(led_mode,LOW);                 //remote controlled mode is off
  delay(5000);                                //wait after power on
  threshold_detect();                         //call routine for thresholddetect
  init_serial();
}                                         //end of setup


//====================== main loop ======================
void loop()                               //main loop
{ 
  receive_characters();                       //receive commands from the bluetooth module
  mode_check();                               //check the command for the current mode
  if ((auto_mode) && (millis() > time))       //run the automatic mode only at a fixed intervall
  {
    time = millis() + loop_time;              //save the current time plus the loop time
    val_left = analogRead(left_LDR);          //left LDR read
    val_right = analogRead(right_LDR);        //right LDR read

    if (val_right > trh_right+40)             //if right LDR is above threshold_right
    {
      servo_left_on(servo_high);              //turn left servo on
    }
    else
    {
      servo_left_off();                       //turn left servo off
    }

    if (val_left > trh_left+40)               //if left LDR is above threshold_left
    {
      servo_right_on(servo_low);              //turn right servo on
    }
    else
    {
      servo_right_off();                      //turn right servo off
    }
    
    sensor_left_string[2] = (char)(val_left / 4);
    send_serial(sensor_left_string);          //send the current left sensor value to the app
    sensor_right_string[2] = (char)(val_right / 4);
    send_serial(sensor_right_string);         //send the current right sensor value to the app
  }                                           //end of automatic mode
  else                                        //begin of remote controlled mode
  { 
    if (command_ok)                           //only if the current command is correct
    {
      if (command_string == "MLF")            //forward left motor
        servo_left_on(servo_high);
      if (command_string == "MLR")            //reverse left motor
        servo_left_on(servo_low);
      if (command_string == "MLO")            //stop left motor
        servo_left_off();
      if (command_string == "MRF")            //forward right motor
        servo_right_on(servo_high);
      if (command_string == "MRR")            //reverse right motor
        servo_right_on(servo_low);
      if (command_string == "MRO")            //stop right motor
        servo_right_off();
    }
  }
}                                         //end of main loop

//====================== servo routines ======================

void servo_left_on (int servo_value)      //routine for turning the left servo on
{
      myservo_left.attach(servopin_left);     //attaches the left servo on pin "left" to the servo object
      myservo_left.write(servo_value);        //left servo turn left
      digitalWrite(led_left, HIGH);           //left LED on
      if (servo_value < 90)
      {
        send_serial("MLR#");                  //send motor left direction to app
      }
      else
      {
        send_serial("MLF#");                  //send motor left direction to app
      }
}                                         //end of servo left on


void servo_left_off ()                    //routine for turning the left servo off
{
      myservo_left.detach();                  //deactivate left servo
      digitalWrite(led_left, LOW);            //left LED off
      send_serial("MLO#");                    //send motor left off to app
}                                         //end of servo left off


void servo_right_on (int servo_value)     //routine for turning the right servo on
{
      myservo_right.attach(servopin_right);   //attaches the right servo on pin "right" to the servo object
      myservo_right.write(servo_value);       //right servo turn right
      digitalWrite(led_right, HIGH);          //right LED on
      if (servo_value < 90)
      {
        send_serial("MRF#");                  //send motor right direction to app
      }
      else
      {
        send_serial("MRR#");                  //send motor right direction to app
      }
}                                         //end of servo right on


void servo_right_off ()                   //routine for turning the right servo off
{
      myservo_right.detach();                 //deactivate right Servo
      digitalWrite(led_right, LOW);           //right LED off
      send_serial("MRO#");                    //send motor right off to app
}                                         //end of servo right off


void threshold_detect ()                  //routine for looking for the average brightness
{
  servo_left_on(servo_high);                 //left servo turn left

  time = millis();
  do
  {
    mean_left = 0;                            //reset variables
    mean_right = 0;                           //reset variables
    for (int i = 0; i<5; i++)                 //5 runs do get average offsetvalues from LDRs
    {
      mean_left += analogRead(left_LDR);      //sum
      mean_right += analogRead(right_LDR);    //sum
    }
    mean_left /= 5;
    mean_right /= 5;

    trh_left = max(trh_left,mean_left );      //get max value while rotating
    trh_right = max(trh_right,mean_right );   //get max value while rotating
  } 
  while (millis() < time + trh_time);         //do for the desired time, until one round is done

  servo_left_off();                           //turn left servo off
}                                         //end of the routine

//====================== communication stuff ======================

void init_serial ()                       //setup the serial connection
{
  if (debug)                                  //when debugging, use the serial over USB
  {
    Serial.begin(Baudrate);
  }
  else                                        //otherwise use the hardware UART
  {
    Serial1.begin(Baudrate);
  }
}                                         //end of serial setup


void send_serial (char send_string[4])    //routine for sending replies
{
  if (debug)                                  //when debugging, use the serial over USB
  {
    Serial.print(send_string);
  }
  else                                        //otherwise use the hardware UART
  {
    Serial1.print(send_string);
  }
}                                         //end of send routine


void receive_characters ()                //handler for the received characters    
{
  if(Serial1.available())                     //Data available at UART?
  {
    char ch = Serial1.read();                 //save the received character
    if ((ch == '#') && (char_index == 3))     //delimiter and last character?
    { 
      command_ok = true;                      //then the command must be correct
      char_index = 0;                         //reset the index counter
    }
    else                                      //for all other characters:
    {
      command_ok = false;                     //reset the flag
      if (char_index <= 2)                    //index in the correct range?
      {
        command_string[char_index] = ch;      //append to the command string
        char_index++;                         //increase the index
        
      }                                       //if out of range
      else char_index = 0;                    //reset the index counter
    }
  }                                   
}                                         //end of receive handler


void mode_check ()                        //mode switcher for the automatic/remote mode
{
  if (command_ok)                             //only when the command is ok
  {
    if (command_string == "AON")              //check for the right commands
    {                                         //automatic mode on?
      auto_mode = true;
      digitalWrite(led_mode,LOW);             //mode LED is OFF for automatic mode
      command_ok = false;                     //reset the flag so the command is executed only once
    }
    if (command_string == "AON")              //automatic mode off?
    {                                         
      auto_mode = false;
      digitalWrite(led_mode,LOW);             //mode LED is ON for remote controlled mode
      command_ok = false;                     //reset the flag so the command is executed only once
    }
  }
}                                         //end of mode switcher
