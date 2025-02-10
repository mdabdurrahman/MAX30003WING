//////////////////////////////////////////////////////////////////////////////////////////
//
//  Arduino code for the MAX30003WING board
//
//  Arduino UNO R3 to MAX30003WING connections:
//
//  |MAX30003 pin label| Pin Function         |Arduino Connection|
//  |----------------- |:--------------------:|-----------------:|
//  | MISO             | Slave Out            |  D12             |
//  | MOSI             | Slave In             |  D11             |
//  | SCLK             | Serial Clock         |  D13             |
//  | CS               | Chip Select          |  D10             |
//  | VCC              | Digital VDD          |  3.3V            |
//  | GND              | Digital Gnd          |  GND             |
//	| INTB             | Interrupt            |  D3              |
//  Copyright © Md Abdur Rahman
//
/////////////////////////////////////////////////////////////////////////////////////////

#include<SPI.h>
#include "max30003.h"

#define INT_PIN 3

#define MAX30003_CS_PIN 10

MAX30003 max30003(MAX30003_CS_PIN);

bool rtorIntrFlag = false;
uint8_t statusReg[3];

void rtorInterruptHndlr(){
  rtorIntrFlag = true;
}

void enableInterruptPin(){

  pinMode(INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), rtorInterruptHndlr, CHANGE);
}

void setup()
{
    Serial.begin(115200); //Serial begin

    pinMode(MAX30003_CS_PIN,OUTPUT);
    digitalWrite(MAX30003_CS_PIN,HIGH); //disable device

    SPI.begin();

    bool ret = max30003.max30003ReadInfo();
    if(ret){
      Serial.println("Max30003 ID Success");
    }else{

      while(!ret){
        //stay here untill the issue is fixed.
        ret = max30003.max30003ReadInfo();
        Serial.println("Failed to read ID, please make sure all the pins are connected");
        delay(5000);
      }
    }

    //Serial.println("Initialising the chip ...");
    max30003.max30003BeginRtorMode();   // initialize MAX30003
    //enableInterruptPin();
    //max30003.max30003RegRead(STATUS, statusReg);
}

void loop()
{
      //rtorIntrFlag = false;
      max30003.max30003RegRead(STATUS, statusReg);// getting status register values

      if(statusReg[1] & RTOR_INTR_MASK){ //checking if ECG R to R Detector is set

        max30003.getHRandRR();   //It will store HR to max30003.heartRate and rr to max30003.RRinterval.
        Serial.print("Heart Rate  = ");
        Serial.println(max30003.heartRate);

        Serial.print("RR interval  = ");
        Serial.println(max30003.RRinterval);
        delay(8);

      }

}
