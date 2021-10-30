#ifndef A9_lib_h
#define A9_lib_h

#include <SoftwareSerial.h>
#include "Arduino.h"

#define RX_PIN 10
#define TX_PIN 11  

class A9lib    
{                 
  private:
  String _readSerial();
  const char* _responseInfo[_responseInfoSize] =
          {"ERROR",
          "NOT READY",
          "READY",
          "CONNECT OK",
          "CONNECT FAIL",
          "ALREADY CONNECT",
          "SEND OK",
          "SEND FAIL",
          "DATA ACCEPT",
          "CLOSED",
          ">",
          "OK"};
          
  // some private function 
  byte _checkResponse(uint16_t timeout);  
    
  public:

  void begin();
  void setSMSstorage()
  void powerOn(int pin); 
  void powerOff(int pin);
  void powerCycle(int pin);
  int getSignalStrength();
  bool answer();  
  void dial(String number);
  void redial();
  uint8_t getCallStatus();
  bool hangup();
  //Methods for sms 
  bool sendSms(char* number,char* text);   
  String readSms(uint8_t index); //return all the content of sms
  String getNumberSms(uint8_t index); //return the number of the sms..   
  byte deleteSMS(int index, int flag);     // return :  OK or ERROR .. 

  void setPhoneFunctionality();

  //get time with the variables by reference
  void RTCtime(int *day,int *month, int *year,int *hour,int *minute, int *second);  
};



#endif
