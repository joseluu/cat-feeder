/*
 * pin connections (may need to be changed if you have a different board)
 */

const int pinDetector1 = 25;
const int pinDetector2 = 26; 
const int pinDetector3 = 35;

int Pin1 = 13;//IN1 is connected to 13 
int Pin2 = 12;//IN2 is connected to 12  
int Pin3 = 14;//IN3 is connected to 14 
int Pin4 = 27;//IN4 is connected to 27 

String feedTimes[] ={"9:00:00", "13:00:00", "18:00:00", "23:00:00"};

String catName = "<your cat name>";

int feedQuantity = 2800;

const char *ssid = "<ssid of your wifi AP>";
const char *password = "<password of your wifi AP>";


#define dnsAddress "feed-cat"


#include <TimeLib.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


String buttonTitle1[] ={"Feed", "Reverse"};
String buttonTitle2[] ={"Feed", "Reverse"};
String argId[] ={"ccw", "cw"};

WebServer server(80);

#define MAX_LOG 100
String logData[MAX_LOG];
int logPtr=0;

// utility function for digital clock display: prints leading 0
String twoDigits(int digits){
  if(digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}
String getTimeNow(){
  return String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
}

/*
 * Controlling 28BYJ-48 Stepper Motor over WiFi using ESP32   
 * usng 2 push buttons: CW and CCW
 * 
 * Watch Video instrution for this code:https://youtu.be/n2oeT6RcU5Q
 * 
 * Full explanation of this code and wiring diagram is available at
 * my Arduino Course at Udemy.com here: http://robojax.com/L/?id=62

 * Written by Ahmad Shamshiri on April 19, 2020 at 17:58
 * in Ajax, Ontario, Canada. www.robojax.com
 * 

 *  * This code is "AS IS" without warranty or liability. Free to be used as long as you keep this note intact.* 
 * This code has been download from Robojax.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

*/
int  dirStatus = 0;// stores direction status 3= stop (do not change)

void handleRoot() {
   String timeNow = getTimeNow();
   //Robojax.com ESP32 Relay Motor Control
 String HTML ="<!DOCTYPE html>\
  <html>\
  <head>\
    <title>Cat feeder Control</title>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <meta http-equiv=\"refresh\" content=\"5; url=http://" dnsAddress "/\">\
    <style>\
      html,body{  \
      width:100%;\
      height:100%;\
      margin:0}\
      *{box-sizing:border-box}\
      .colorAll{\
        background-color:#90ee90}\
      .colorBtn{\
        background-color:#add8e6}\
      .angleButtdon,a{\
        font-:30px;\
      border:1px solid #ccc;\
      display:table-caption;\
      padding:7px 10px;\
      text-decoration:none;\
      cursor:pointer;\
      padding:5px 6px 7px 10px}a{\
        display:block}\
      .btn{\
        margin:5px;\
      border:none;\
      display:inline-block;\
      vertical-align:middle;\
      text-align:center;\
      white-space:nowrap}\
    </style>\
  </head>\
";
   
  HTML +="\
  <body>     <h1>"+ timeNow +" " dnsAddress " "+catName+" ?</h1>";

   if(dirStatus <0){
    HTML +="  <h2><span style=\"background-color: #FFFF00\">Motor Running in CW</span></h2>";    
   }else if(dirStatus >0){
    HTML +="  <h2><span style=\"background-color: #FFFF00\">Motor Running in CCW</span></h2>";      
   }else{
    HTML +="  <h2><span style=\"background-color: #FFFF00\">Motor OFF</span></h2>";    
   }
      if(dirStatus >0){
        HTML +="  <div class=\"btn\">    <a class=\"angleButton\" style=\"background-color:#f56464\"  href=\"/motor?";
        HTML += argId[0];
        HTML += "=off\">";
        HTML +=buttonTitle1[0]; //motor ON title
      }else{
        HTML +="  <div class=\"btn\">    <a class=\"angleButton \" style=\"background-color:#90ee90\"  href=\"/motor?";  
         HTML += argId[0];
        HTML += "=on\">";       
        HTML +=buttonTitle2[0];//motor OFF title   
      }   
     HTML +="</a>   </div>";  
           
      if(dirStatus <0){
        HTML +="  <div class=\"btn\">    <a class=\"angleButton\" style=\"background-color:#f56464\"  href=\"/motor?";
        HTML += argId[1];
        HTML += "=off\">";
        HTML +=buttonTitle1[1]; //motor ON title
      }else{
        HTML +="  <div class=\"btn\">    <a class=\"angleButton \" style=\"background-color:#90ee90\"  href=\"/motor?";  
         HTML += argId[1];
        HTML += "=on\">";       
        HTML +=buttonTitle2[1];//motor OFF title   
      }   
     HTML +="</a>   </div>";     
     HTML +=showLog();
  HTML +="  </body></html>";
  server.send(200, "text/html", HTML);  
}//handleRoot()

String showLog() {
  String result="";
  int ptr=logPtr;
  ptr--;
  while (ptr != logPtr) {
    if (ptr < 0) {
      ptr=MAX_LOG-1;
    }
    if (logData[ptr].length() > 0) {
      result += "<p>" + logData[ptr]+"</p>";
    } else {
      return result;
    }
    ptr--;
  }
}

void handleNotFound() {
  
  String message = "File Not Found";
  message += "URI: ";
  message += server.uri();
  message += "Method: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "Arguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void addLog(String newLog){
  logData[logPtr]=newLog;
  logPtr ++;
  if (logPtr >= MAX_LOG){
    logPtr=0;
  }
  if (logData[logPtr].length() >0){
     logData[logPtr];
  }
  logData[logPtr]="";
}

void resetLogData(){
  for (int i=0;i<MAX_LOG;i++) {
    logData[i]="";
  }
}

void timeSetup(void) {
  String formattedDate;
  String dayStamp;
  String timeStamp;
  timeClient.begin();
  timeClient.setTimeOffset(7200); // 3600 winter time
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedTime();
  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.println(timeStamp);
  setTime(timeClient.getEpochTime());
  //setSyncProvider((long(*)())timeClient.getEpochTime());
}

#define DEAD_TIME 10

void detectorTrigger_1(){
  static time_t previousTrigger;
  time_t timeNow=timeClient.getEpochTime();
  if (timeNow-previousTrigger > DEAD_TIME){
     addLog("detector 1 " + getTimeNow());
     previousTrigger = timeNow;
  }
}


void detectorTrigger_2(){
  static time_t previousTrigger;
  time_t timeNow=timeClient.getEpochTime();
  if (timeNow-previousTrigger > DEAD_TIME){
     addLog("detector 2 " + getTimeNow());
     previousTrigger = timeNow;
  }
}

void detectorTrigger_3(){
  static time_t previousTrigger;
  time_t timeNow=timeClient.getEpochTime();
  if (timeNow-previousTrigger > DEAD_TIME){
     addLog("detector 3 " + getTimeNow());
     previousTrigger = timeNow;
  }
}


void detectorSetup(){
  pinMode(pinDetector1,INPUT);
  pinMode(pinDetector2,INPUT);
  pinMode(pinDetector3,INPUT);
  attachInterrupt(pinDetector1, detectorTrigger_1, RISING);
  attachInterrupt(pinDetector2, detectorTrigger_2, RISING);
  attachInterrupt(pinDetector3, detectorTrigger_3, RISING);
}
void setup(void) {

    detectorSetup();
    
    pinMode(Pin1, OUTPUT);//define pin for ULN2003 in1 
    pinMode(Pin2, OUTPUT);//define pin for ULN2003 in2   
    pinMode(Pin3, OUTPUT);//define pin for ULN2003 in3   
    pinMode(Pin4, OUTPUT);//define pin for ULN2003 in4   
    
    Serial.begin(115200);//initialize the serial monitor
    Serial.println("cat feeder based on Robojax 28BYJ-48 Stepper Motor Control");

    resetLogData();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: http://");
    Serial.println(WiFi.localIP());

//multicast DNS   
    if (MDNS.begin(dnsAddress)) {
        Serial.println("MDNS responder started");
        Serial.println("access via http://" dnsAddress);
    }
    timeSetup();
    
    server.on("/", handleRoot);
    server.on("/motor", HTTP_GET, motorControl);           
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
    addLog("Started " + getTimeNow());
}//end of setup

 
int pole1[] ={0,0,0,0, 0,1,1,1, 0};//pole1, 8 step values
int pole2[] ={0,0,0,1, 1,1,0,0, 0};//pole2, 8 step values
int pole3[] ={0,1,1,1, 0,0,0,0, 0};//pole3, 8 step values
int pole4[] ={1,1,0,0, 0,0,0,1, 0};//pole4, 8 step values

int poleStep = 0; 

int feeds=0;
bool bFeeding;
void loop(void) {
  server.handleClient();
  if (dirStatus == 0) {
    driveStepper(8);   
  
    if (!bFeeding) {
      for (int i=0; i<sizeof(feedTimes)/sizeof(String); i++) {
        if (getTimeNow().compareTo(feedTimes[i])==0){
          feeds ++;
          doFeed("Auto ("+String(feeds)+")", true); // reverse first
          bFeeding=true;
        }
      }
    } else {
      doFeed("Auto ("+String(feeds)+")", false);
      bFeeding=false;
    }
  } else if(dirStatus >0 ){ 
    dirStatus --;
    poleStep++; 
    driveStepper(poleStep);    
  }else if(dirStatus <0){ 
    dirStatus ++;
    poleStep--; 
    driveStepper(poleStep);    
  }
  if(poleStep>7){ 
    poleStep=0; 
  } 
  if(poleStep<0){ 
    poleStep=7; 
  } 
  delay(1);
  //Robojax.com 28BYJ-48 Steper Motor Control
}//end of loop

void doFeed(String reason, int bReverse){
   if (bReverse) {
       addLog(reason + " feed at " + getTimeNow());
       dirStatus = - feedQuantity;// CW 
   } else {
      dirStatus = feedQuantity;// CCW 
   }

}

void motorControl() {
    if(server.arg(argId[0]) == "on") {
       addLog("Manual feed at " + getTimeNow());
       dirStatus = feedQuantity;// CCW 
    }else if(server.arg(argId[0]) == "off"){
      dirStatus = 0;  // motor OFF        
    }else if(server.arg(argId[1]) == "on"){
      dirStatus = -feedQuantity;  // CW          
    }else if(server.arg(argId[1]) == "off"){
      dirStatus = 0;  // motor OFF        
    }  
  handleRoot();
}//motorControl end



/*
 * @brief sends signal to the motor
 * @param "c" is integer representing the pol of motor
 * @return does not return anything
 * 
 * www.Robojax.com code June 2019
 */
void driveStepper(int c)
{
     digitalWrite(Pin1, pole1[c]);  
     digitalWrite(Pin2, pole2[c]); 
     digitalWrite(Pin3, pole3[c]); 
     digitalWrite(Pin4, pole4[c]);   
}//driveStepper end here
 
