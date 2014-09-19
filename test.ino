#include <Wire.h>
#include "bmp085.h"
#include "crossfade.h"
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(12); // PIN 12
DallasTemperature sensors(&oneWire);

void setPwmFrequency(int pin, int divisor);

const int numReadings = 100;
float readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

// The current color
int current[3] = {0, 0, 0};
int target[3] = {0, 0, 0};
int prevTarget[3] = {0, 0, 0};
float distance[3] = {0, 0, 0};
int currentStep = 0;

// Color arrays
int black[3]  = { 0, 0, 0 };
int white[3]  = { 100, 100, 100 };
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };
int yellow[3] = { 40, 95, 0 };
int dimWhite[3] = { 30, 30, 30 };

void setup() {
  pinMode(11, OUTPUT);     
  pinMode(10, OUTPUT);     
  pinMode(3, OUTPUT);    
 
  setPwmFrequency(10,1); 
  setPwmFrequency(11,1); 
  analogWrite(10, 100);
  analogWrite(11, 100);

  pinMode(redPin, OUTPUT);
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 

  Serial.begin(9600);
  Wire.begin();
  bmp085Calibration();

  sensors.begin();


  average = bmp085GetPressure(bmp085ReadUP());
}

float temperature;
long pressure;
float temperature2;
boolean bAnimation;


byte cmd0, cmd1, arg0, arg1, arg2, arg3;


void loop() {
  if( Serial.available() >= 6 ) {  // command length is 6 bytes
    cmd0 = Serial.read();
    cmd1 = Serial.read();
    arg0 = Serial.read();
    arg1 = Serial.read();
    arg2 = Serial.read();
    arg3 = Serial.read();

	switch(cmd0 + cmd1) {
		case 1:
			target[0] = map(arg0,0,255,0,255);
			target[1] = map(arg1,0,255,0,255);
			target[2] = map(arg2,0,255,0,255);
			
			currentStep = 1;
		
			distance[0] = target[0] - current[0];
			distance[1] = target[1] - current[1];
			distance[2] = target[2] - current[2];

			break;
	}
  }

  if ( currentStep > 0 ) {
	current[0] = target[0] - distance[0]*(cos(currentStep*PI/360)+1)/2;
	current[1] = target[1] - distance[1]*(cos(currentStep*PI/360)+1)/2;
	current[2] = target[2] - distance[2]*(cos(currentStep*PI/360)+1)/2;
	currentStep++;
	
  	analogWrite(redPin, current[0]);
  	analogWrite(grnPin, current[1]);   
  	analogWrite(bluPin, current[2]); 

	if ( currentStep > 360 ) {
		currentStep = 0;
	}

	delay(10);
  } else {
		  temperature = (float)bmp085GetTemperature(bmp085ReadUT())/10;
		  pressure = bmp085GetPressure(bmp085ReadUP());

		  total= total - readings[index];         
		  readings[index] = pressure; // < ----
		  total= total + readings[index];       
		  index = index + 1;                    

		  if (index >= numReadings)              
			index = 0;                           

		  average = total / numReadings / 100;         

		  sensors.requestTemperatures();  
		  temperature2 = sensors.getTempCByIndex(0);

		  Serial.print("<");
		  Serial.print( temperature );
		  Serial.print("|");
		  Serial.print(pressure, DEC);
		  Serial.print("|");
		  Serial.print(average);
		  Serial.print("|");
		  Serial.print(sensors.getTempCByIndex(0));   
		  Serial.print(">\n");
  }
}



void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
