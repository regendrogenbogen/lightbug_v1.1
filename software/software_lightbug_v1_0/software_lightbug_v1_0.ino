// Lightfollower V1.0
// by Johann Leinweber

#include <Servo.h> 

#define servo_high 150
#define servo_low 30
#define led_left 2
#define led_right 3
#define servopin_left 10
#define servopin_right 9
//#define 
//#define 

Servo myservo_left;          //create servo object to control a servo
Servo myservo_right;         //create servo object to control a servo

int left_LDR = A1;           //analog pin used to connect the left LDR
int right_LDR = A3;          //analog pin used to connect the right LDR
//int led_left = 2;            //pin left LED
//int led_right = 3;           //pin right LED
//int servopin_left = 10;      //pin left servo
//int servopin_right = 9;      //pin rigth servo
int trh_time = 6000;         //time for measuring thresholdvalue in ms
int time = 0;                //temporary time for measuring time
int val_left = 0;            //variable to read the value from left LDR
int val_right = 0;           //variable to read the value from left LDR
int mean_left = 0;           //meanvalue left LDR
int mean_right = 0;          //meanvalue right LDR
int trh_left = 0;            //threshold for left LDR
int trh_right = 0;           //threshold for right LDR

void setup() 
{   
  pinMode(led_left,OUTPUT);        //left LED pin as output
  pinMode(led_right,OUTPUT);       //right LED pin as output
  digitalWrite(led_left, HIGH);    //switch LED left on
  digitalWrite(led_right, HIGH);   //switch LED right on
  delay(5000);                     //wait after power on
  threshold_detect();              //call routine for thresholddetect
} 

void loop() 
{ 
  val_left = analogRead(left_LDR);      //left LDR read
  val_right = analogRead(right_LDR);    //right LDR read

  if (val_right > trh_right+40)         //if right LDR is above threshold_right
  {
    myservo_left.attach(servopin_left); //attaches the left servo on pin "left" to the servo object
    myservo_left.write(servo_high);     //left servo turn left
    digitalWrite(led_left, HIGH);       //left LED on
  }
  else
  {
    myservo_left.detach();               //deactivate left servo
    digitalWrite(led_left, LOW);         //left LED off
  }

  if (val_left > trh_left+40)            //if left LDR is above threshold_left
  {
    myservo_right.attach(servopin_right);    //attaches the right servo on pin "right" to the servo object
    myservo_right.write(servo_low);          //right servo turn right
    digitalWrite(led_right, HIGH);           //right LED on
  }
  else
  {
    myservo_right.detach();                  //deactivate right Servo
    digitalWrite(led_right, LOW);            //right LED off
  }

} 





void threshold_detect ()
{
  myservo_left.attach(servopin_left);    //attaches the left servo on pin 10 to the servo object
  myservo_left.write(servo_high);        //left servo turn left
  digitalWrite(led_left, HIGH);          //left LED on

  time = millis();
  do
  {
    mean_left = 0;                       //reste variables
    mean_right = 0;                      //reste variables
    for (int i = 0; i<5; i++)            //5 runs do get average offsetvalues from LDRs
    {
      mean_left += analogRead(left_LDR);    //sum
      mean_right += analogRead(right_LDR);  //sum
    }
    mean_left /= 5;
    mean_right /= 5;

    trh_left = max(trh_left,mean_left );      //get max value while rotating
    trh_right = max(trh_right,mean_right );   //get max value while rotating
  } 
  while (millis() < time + trh_time);         //do for the desired time, until one round is done

  myservo_left.detach();                      //deactivate left Servo
  digitalWrite(led_left, LOW);                //left LED off



}

