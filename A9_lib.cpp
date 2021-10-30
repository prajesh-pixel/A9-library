#include "Arduino.h"
#include "A9_lib.h"
#include <SoftwareSerial.h>

SoftwareSerial A9(RX_PIN,TX_PIN);

void A9lib::begin(){
  A9.begin(115200);
  // Set caller ID on.
//  A9command("AT+CLIP=1", "OK", "yy", A6_CMD_TIMEOUT, 2, NULL);
  byte result = _checkResponse(1000);
}


//
//PRIVATE METHODS
//
byte A9lib::_checkResponse(uint16_t timeout)
 {
   // This function handles the response from the radio and returns a status response
   uint8_t Status = 99; // the defualt stat and it means a timeout
   unsigned long t = millis();
   //unsigned long count = 0;
   
   // loop through until their is a timeout or a response from the device
   while(millis()<t+timeout)
   {
     //count++;
     if(A9.available()) //check if the device is sending a message
     {
       String tempData = A9.readString(); // reads the response
       if(DEBUG)
         Serial.println(tempData);
       char *mydataIn = strdup(tempData.c_str()); // convertss to char data from
       
     /*
       * Checks for the status response
       * Response are - OK, ERROR, READY, >, CONNECT OK
       * SEND OK, DATA ACCEPT, SEND FAIL, CLOSE, CLOSED
       * note_ LOCAL iP COMMANDS HAS NO ENDING RESPONSE 
       * ERROR - 0
       * READY - 1
       * CONNECT OK - 2
       * SEND OK - 3
       * SEND FAIL - 4
       * CLOSED - 5
       * > - 6
       * OK - 7
       * 
       * 
       */
       for (byte i=0; i<_responseInfoSize; i++)
       {
      if((strstr(mydataIn, _responseInfo[i])) != NULL)
      {
        Status = i;
        return Status;
        }
       }
     }
     //Serial.println(count);
   }
   return Status;
 }

String A9lib::_readSerial(){
  // this function just reads the raw data
   uint16_t timeout=0;
   while (!A9.available() && timeout<10000)
   {
     delay(10);
     timeout++;
   }
   if(A9.available())
   {
     String output = A9.readString();
     if(DEBUG)
       Serial.println(output);
     return output;
   }
}

//
//PUBLIC METHODS
//

// Set SMS storage to the GSM modem. If this doesn't work for you, try changing the command to:
// "AT+CPMS=SM,SM,SM"
void A9lib::setSMSstorage(){
  A9.print (F("AT+CPMS=SM,SM,SM\r\n"));
  byte result = _checkResponse(2000); // timeout 10s
   if (result == OK)
     A9.print("SMS storage set in GSM");
}


void A9::setPhoneFunctionality(){
  /*AT+CFUN=<fun>[,<rst>] 
  Parameters
  <fun> 0 Minimum functionality
  1 Full functionality (Default)
  4 Disable phone both transmit and receive RF circuits.
  <rst> 1 Reset the MT before setting it to <fun> power level.
  */
   bool nowReady = false;
   A9.print(F("AT+CFUN=1\r\n"));
   // Let's confirm if this was valid
   byte result = _checkResponse(20000); // timeout 10s
   if (result == OK || result == READY)
     nowReady = true;
   delay(10);
   // sometimes some excess data usually come out
   result = _checkResponse(5000); // timeout 5s 
   if (nowReady)   
     return true;
   else
     return false;
}
// Turn the modem power off
void A9lib::powerOff(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}


// Turn the modem power on.
void A9lib::powerOn(int pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void A9lib::powerCycle(int pin) {
    logln("Power-cycling module...");

    powerOff(pin);

    delay(2000);

    powerOn(pin);

    // Give the module some time to settle.
    logln("Done, waiting for the module to initialize...");
    delay(20000);
    logln("Done.");

    A9.flush();
}

int A9lib::getSignalStrength() {
    String response = "";
    uint32_t respStart = 0;
    int strength, error  = 0;

    // Issue the command and wait for the response.
    A9.print(F("AT+CSQ\r\n"));
    byte result = _checkResponse(2000);
    if(result == OK)
    {
    A9.print(F("AT+CSQ\r\n"));
    response = _readSerial(); //reads the result    
    respStart = response.indexOf("+CSQ");
    if (respStart < 0) {
        return 0;
    }
    sscanf(response.substring(respStart).c_str(), "+CSQ: %d,%d",
           &strength, &error);

    // Bring value range 0..31 to 0..100%, don't mind rounding..
    strength = (strength * 100) / 31;
    return strength;
    }
}


bool A9lib::answer(){
   String result ="";
   A9.print (F("ATA\r\n"));
   buffer=_readSerial();
   //Response in case of data call, if successfully connected 
   if ( (result.indexOf("OK") )!=-1 ) return true;  
   else return false;
}

void A9lib::dial(String number) {
    char buffer[50];
    sprintf(buffer, "ATD%s;", number.c_str());
    A9.print(buffer);
    byte result = _checkResponse(2000);
    if(result == OK)
    {
      A9.print("Calling...%s",number.c_str());
    }
}

void A9lib::redial() {
    String response="";
    logln("Redialing last number...");
    A9.print("AT+DLST\r\n");
    byte result = _checkResponse(2000);
    response = _readSerial();
    if(result == OK && response.indexOf("BUSY") ) != -1)
    {
      A9.print("Number is currently busy");
    }
    else if(result == OK && response.indexOf("NO ANSWER") ) != -1)
    {
      A9.print("Not Answering");
    }
    else if(result == OK && response.indexOf("NO CARRIER") ) != -1)
    {
      A9.print("Not connected to network");
    }
    else if(result == ERROR)
    {
      A9.print("Error issuing command");
    }
}

uint8_t A9lib::getCallStatus(){
/*
  values of return:
 
 0 Ready (MT allows commands from TA/TE)
 2 Unknown (MT is not guaranteed to respond to tructions)
 3 Ringing (MT is ready for commands from TA/TE, but the ringer is active)
 4 Call in progress
*/
  String result = "";
  A9.print (F("AT+CPAS\r\n"));
  result=_readSerial();  
  return result.substring(result.indexOf("+CPAS: ")+7,result.indexOf("+CPAS: ")+9).toInt();

}

bool A9lib::hangup(){
  String result="";
  A9.print (F("ATH\r\n"));
  result =_readSerial();
  if ( (result.indexOf("OK") ) != -1) return true;
  else return false;
}

bool A9lib::sendSMS(const char* number, char* text){
  /* This sends an sms out 
  * First send out AT+CMGF=1 - activate text mode
  * The AT+CMGS=\number\
  AT+CMGS=<number><CR><message><CTRL-Z> +CMGS:<mr>
  OK
  */
  String response = "";
  byte result;
  A9.print(F("AT+CMGF=1\r\n")); // set sms to text mode
  result = _checkResponse(10000);
  if(result == ERROR)
    return false; // this just end the function here 
  delay(1000);
  A9.print(F("AT+CMGS=\"")); // command to send sms
  A9.print(number);
  A9.print(F("\"\r\n"));
  if(response.indexOf(">") ) != -1)
    {
    A9.print(text);
    A9.print("\r");
    result=_checkResponse(1000);
    A9.print((char)26);
    result = _checkResponse(20000);
    // if successfully sent we should get CMGS:xxx ending with OK
    if(result == OK)
      return true;
    else
      return false;
    }
}

String A9lib::readSMS(uint8_t index){
  String buffer = "";
  gsmSerial.print(F("AT+CMGF=1\r"));
  byte result = _checkResponse(10000);
  if(result == OK)
  {
    gsmSerial.print(F("AT+CMGR="));
    gsmSerial.print(index);
    gsmSerial.print("\r");
    buffer = _readSerial(); //reads the result
    if(buffer.indexOf("CMGR:") != -1)
    {
      // means message is found
      return buffer;
    }
    else
      return "";
  }
  else
    return "";
}

String A9lib::getNumberSms(uint8_t index){
  String result="";
  result=readSms(index);
  Serial.println(result.length());
  if (result.length() > 10) //avoid empty sms
  {
    uint8_t _idx1=result.indexOf("+CMGR:");
    _idx1=result.indexOf("\",\"",_idx1+1);
    return result.substring(_idx1+3,result.indexOf("\",\"",_idx1+4));
  }else{
    return "";
  }
}

byte A9lib::deleteSMS(int index, int flag) {
    String command = "AT+CMGD=";
    command += String(index);
    command += ",";
    command += String(flag);
    A9.print(command.c_str());
    byte result = _checkResponse(2000);
    if(result == OK)
    {
      A9.print("Messages deleted");
    }
}

void A9lib::RTCtime(int *day,int *month, int *year,int *hour,int *minute, int *second){
  String result="";
  A9.print(F("AT+CCLK?\r\n"));
  // if respond with ERROR try one more time. 
  result=_readSerial();
  if ((result.indexOf("ERROR"))!=-1){
    delay(50);
    A9.print(F("AT+CCLK?\r\n"));
  } 
  if ((result.indexOf("ERROR"))==-1){
    result=result.substring(result.indexOf("\"")+1,result.lastIndexOf("\"")-1);  
    *year=result.substring(0,2).toInt();
    *month= result.substring(3,5).toInt();
    *day=result.substring(6,8).toInt();
    *hour=result.substring(9,11).toInt();
    *minute=result.substring(12,14).toInt();
    *second=result.substring(15,17).toInt();
 }
}
