/*
This example shows how to connect to Cayenne using a Serial USB connection and send/receive sample data.

The CayenneMQTT Library is required to run this sketch. If you have not already done so you can install it from the Arduino IDE Library Manager.

This requires the use of the Serial USB connection so you cannot use the Serial device for
printing messages. If you need to print you can use SoftwareSerial and connect another device
to read messages via the SoftwareSerial pins.

In order for this to work you must run the connection script on the machine the Arduino is connected to.
The scripts are located under the extras\scripts folder in the main library folder. This redirects the traffic
from the Arduino to the Cayenne server.

Steps:
1. Set the Cayenne authentication info to match the authentication info from the Dashboard.
2. Compile and upload this sketch.
3. Launch the connection script as described below for Windows or Linux/OSX.

Windows:
  1. Open the Windows command line (cmd.exe)
  2. Navigate to the scripts folder by typing "cd [path]", e.g.  "cd C:\Users\[YourUserName]\Documents\Arduino\libraries\CayenneMQTT\extras\scripts"
  3. Run the script by typing "cayenne-ser.bat -c COM4" (where COM4 is the Arduino serial port) and hitting Enter

Linux and OSX:
    ./cayenne-ser.sh (may need to run with sudo)
    
You can specify port, baud rate, and server endpoint like this:
    ./cayenne-ser.sh -c <serial port> -b <baud rate> -s <server address> -p <server port>

    For instance :
      ./cayenne-ser.sh -c /dev/ttyACM0 -b 9600 -s mqtt.mydevices.com -p 1883

    Run cayenne-ser.sh -h for more information

    Be sure to select the right serial port (there may be multiple).

ATTENTION!
  Do not use Serial to display any output in this sketch. It will interfere with the Serial
  USB connection. When uploading sketches the Arduino IDE may complain with "programmer is
  not responding" or "Access is denied." You will need to terminate the connection script
  before uploading new sketches since it blocks access to the Serial port. Also make sure 
  the Serial Monitor is disabled in the IDE since that can prevent the Arduino from 
  connecting to the Windows/Linux/OSX machine. If you use Visual Micro for Visual Studio make
  sure Automatic Debugging is disabled. Otherwise the Serial Monitor can interfere with the
  Serial connection.
*/

//USB Connection
#include <CayenneMQTTSerial.h>
//#include <Servo.h>
#include "CayenneUtils/CayenneDefines.h"


//Identitas Water Level
#define LEVEL_VIRTUAL_CHANNEL V1 
#define sensorLevel 5 // pin A5
int levelValue = 0;
float longSensor = 4.0;
float levelWater = 0;
int maxValue = 1023;

//Identitas pH Water
#define PH_VIRTUAL_CHANNEL V2
#define sensorPh 0 // pin A0
#define Offset 27.00
unsigned long int avgValue;
float phValue;
float phVol;

//Identitas Water Flow
#define FLOW_VIRTUAL_CHANNEL V3
unsigned char flowsensor = 2; // pin 2 Digital
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour; // Calculated litres/hour
unsigned long currentTime;
unsigned long cloopTime;

void flow () // Interrupt function
{
   flow_frequency++;
}

//Identitas Servo
//#define PAN_VIRTUAL_CHANNEL V4
//#define TILT_VIRTUAL_CHANNEL V5
//#define PAN_PIN 5 //digital pin 5
//#define TILT_PIN 6 //digital pin 6
//Servo pan;
//Servo tilt;

//Identitas HC-SR04
#define ULTRASONIC_VIRTUAL_CHANNEL V6
int trigPin = 11; //Trig 
int echoPin = 12; //Echo 
long duration, cm;
//inches;

void waterLevel()
{
  levelValue = analogRead(sensorLevel);
  levelWater = levelValue*longSensor/maxValue;
}

void waterFlow()
{
  currentTime = millis();
   // Every second, calculate and print litres/hour
   if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flow_frequency = 0; // Reset Counter
      //Serial.print(l_hour, DEC); // Print litres/hour
     //Serial.println(" L/hour");
   }
}

void pHWater()
{
  int buf[10];
  for(int i=0;i<10;i++)
  {
    buf[i]=analogRead(sensorPh);
    delay(10);    
  }
  for(int i=0;i<9;i++)
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
      int temp=buf[i];
      buf[i]=buf[j];
      buf[j]=temp;
      }
    }
  }

  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
  phVol=(float)avgValue*5.0/1024/6;
  phValue= -5.70 * phVol + Offset;
}

void ultrasonic ()
{
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  // convert the time into a distance
  cm = (duration/2) / 29.1;
 // inches = (duration/2) / 74;
 // Serial.print(inches);
 // Serial.print("in, ");
 // Serial.print(cm);
  //Serial.print("cm");
  //Serial.println();
  delay(250);
}


// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "77e49ac0-e57e-11e7-9544-9b91424e936c";
char password[] = "6856368d249a71f6498f0b2361ac37f3e92f62e5";
char clientID[] = "39c59180-e61d-11e9-a4a3-7d841ff78abf";

void setup() {
  // put your setup code here, to run once:
  Cayenne.begin(username, password, clientID);
  //flowsensor
  pinMode(flowsensor, INPUT);
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(0, flow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;
  //ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //Servo 
  //Serial.begin(115200);
//  pan.attach(PAN_PIN);
//  tilt.attach(TILT_PIN);
  
}


void loop() 
{
  // put your main code here, to run repeatedly:
  Cayenne.loop();
  waterLevel();
  waterFlow();
  pHWater();
  ultrasonic();
}

CAYENNE_OUT(LEVEL_VIRTUAL_CHANNEL)
{
  Cayenne.virtualWrite(LEVEL_VIRTUAL_CHANNEL, levelWater, TYPE_PROXIMITY, UNIT_CENTIMETER );
}

CAYENNE_OUT(FLOW_VIRTUAL_CHANNEL)
{
  Cayenne.virtualWrite(FLOW_VIRTUAL_CHANNEL, l_hour);
}

CAYENNE_OUT(PH_VIRTUAL_CHANNEL)
{
  Cayenne.virtualWrite(PH_VIRTUAL_CHANNEL, phValue);
}

CAYENNE_OUT(ULTRASONIC_VIRTUAL_CHANNEL)
{
  Cayenne.virtualWrite(ULTRASONIC_VIRTUAL_CHANNEL, cm, TYPE_PROXIMITY, UNIT_CENTIMETER );
}

//CAYENNE_IN(PAN_VIRTUAL_CHANNEL)
//{
//  Serial.print("move Pan to");
//  double currentValue = getValue.asInt();
//  int position = currentValue;
//  Serial.println(position);
//  pan.write(position);
//}

//CAYENNE_IN(TILT_VIRTUAL_CHANNEL)
//{
//  Serial.print("move Tilt to");
//  double currentValue = getValue.asInt();
//  int position = currentValue;
//  Serial.println(position);
//  tilt.write(position);
//}
