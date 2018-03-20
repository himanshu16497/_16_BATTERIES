
/* Hardware Connections:-
 *  Analog pins: A0- Takes the output values from 8:1 multiplexer
 *               A1- Takes the output values from 8:1 multiplexer
 *          
 *  Digital Pins: 2,3,4- Select lines for 8:1 multiuplexer
 *                7,8- Gsm serial connection
*/
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
SoftwareSerial myGsm (7,8);
int count; 
float inputValue[8];
float BatteryVoltage[8];
void printSerialData()
{
 while(myGsm.available()!=0)
 Serial.write(myGsm.read());
}
void setup()
{
   
 myGsm.begin(9600);  
 Serial.begin(9600); 

//Taking IMEI no.
 myGsm.println("AT+CGSN");
 double imei=myGsm.read();
 delay(1000);
 printSerialData();

//Taking time in the epoch format
 double t= now();
 delay(100);
 printSerialData();

 label1:
  myGsm.println("AT+CIPMUX=0");
  delay(2000);
  if(myGsm.read()=="ERROR")
  goto label1;
  //printSerialData();

 label2:
  myGsm.println("AT+CIPSHUT=0");  //Close TCP Connection
  delay(300); 
  if(myGsm.read()=="ERROR")
  goto label2;
  //printSerialData();

 label3:
  myGsm.println("AT+CSTT=\"airtelgprs\"");
  delay(1000);
  if(myGsm.read()=="ERROR")
  goto label3;
  // printSerialData();
 
 label4:
  myGsm.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs\"");   //Set the APN
  delay(5000);
  if(myGsm.read()=="ERROR")
  goto label4;
  // printSerialData();

 label5:
  myGsm.println("AT+SAPBR =1,1");  //Enable the GPRS
  delay(1000);
  if(myGsm.read()=="ERROR")
  goto label5;
  //printSerialData();

 label6:
  myGsm.println("AT+SAPBR=2,1");   //we get back a IP address
  delay(1000);
  if(myGsm.read()=="ERROR")
  goto label6;
  //printSerialData();

 label7:
  myGsm.println("AT+HTTPINIT");
  delay(1000);
  if(myGsm.read()=="ERROR")
  goto label7;
  //printSerialData();

 label8:
  myGsm.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);
  if(myGsm.read()=="ERROR")
  goto label8;
  //printSerialData();

//JSON Formatting
 StaticJsonBuffer<200> jsonBuffer;
 JsonObject& root = jsonBuffer.createObject();
  root["imei"] = imei;
  root["time"] = t;
  
  JsonArray& batteries = jsonBuffer.createArray();
  //Run loop here
  batteries.add(0);
  batteries.add(1);

  JsonObject& batteriesObject = jsonBuffer.createObject();
  batteriesObject["batteries"] = batteries;
  batteriesObject["timestamp"] = "987654321234";
  
  JsonArray& measurement = root.createNestedArray("measurement");
  measurement.add(batteriesObject);

  // Step 4
  root.printTo(Serial);
  Serial.println();
  root.prettyPrintTo(Serial);
 
}


 void loop()
{
  sendToURL("http://geri.in:8001/write/", readData());
  delay(5000);
 }

String readData()
{
  String result = ""; 
  int address[3];
  for (count=0;count<8;count++)
  {
    address[0] = bitRead(count,0);
    
    address[1] = bitRead(count,1);
    address[2] = bitRead(count,2);

    digitalWrite(2, address[0]);
    digitalWrite(3, address[1]);
    digitalWrite(4, address[2]);

    int inputValue = analogRead(A0);
    BatteryVoltage[count] = inputValue*5/1023;
    result = result + BatteryVoltage[count]+",";
  }
  for (count=0;count<8;count++)
  {
    address[0] = bitRead(count,0);
    address[1] = bitRead(count,1);
    address[2] = bitRead(count,2);

    digitalWrite(2, address[0]);
    digitalWrite(3, address[1]);
    digitalWrite(4, address[2]);

    int inputValue = analogRead(A1);
    BatteryVoltage[count] = inputValue*5/1023;
    result = result + BatteryVoltage[count]+",";
  }
  return result;
}


void sendToURL(String url, String data)
{

 String completeURL = url+data;
 Serial.print(completeURL);  
 myGsm.println("AT+HTTPPARA=\"URL\",\""+completeURL+"\"");
 delay(1000);
 printSerialData();

 myGsm.println("AT+CIPSEND");//begin send data to remote server
 delay(1000);
 printSerialData();
  
 myGsm.println("AT+HTTPACTION=0");
 delay(1000);
 printSerialData();
}



