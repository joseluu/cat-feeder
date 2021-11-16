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

//String feedTimes[] ={"9:00:00", "13:00:00", "18:00:00", "23:00:00"};
String feedTimes[] ={"4:00:00", "9:00:00", "13:00:00", "18:00:00", "23:00:00"};

String catName = "Satoshi";

int feedQuantity = 2800;

// Place for your network credentials
#include "network_creds.h"
//const char* ssid     = "wifi_name";
//const char* password = "wifi_passwd";

#define dnsAddress "feed-cat"

#include <SSD1306Wire.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#define CONFIG_MDNS_STRICT_MODE y
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

#define D3 4
#define D5 15
// Initialize the OLED display using Wire library
SSD1306Wire  oled(0x3c, D3, D5);
bool wifiIsConnected=false;



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
    <meta http-equiv=\"refresh\" content=\"10\">\
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

void oledSetup(void) {
  // reset OLED
  pinMode(16,OUTPUT); 
  digitalWrite(16,LOW); 
  delay(50); 
  digitalWrite(16,HIGH); 
  
  oled.init();
  oled.clear();
  oled.flipScreenVertically();
  oled.setFont(ArialMT_Plain_10);
  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.drawString(0 , 0, "START" );
  oled.display();
}

void displayPoint(int at,OLEDDISPLAY_COLOR color) {
    oled.setColor(color);
    oled.fillRect(at, 20, 1, 1 );
}

void timeSetup(void) {
  String formattedTime;
  String dayStamp;
  String timeStamp;
  timeClient.begin();
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedTime = timeClient.getFormattedTime();
  Serial.println(formattedTime);

    // Central European Time Zone (Paris)
  TimeChangeRule winterRule = {"CET", Last, Sun, Oct, 2, +60};    //Winter time = UTC + 1 hours
  TimeChangeRule summerRule = {"CST", Last, Sun, Mar, 2, +120};  //Summer time = UTC + 2 hours
  Timezone CET(summerRule,winterRule);
  
  setTime(CET.toLocal(timeClient.getEpochTime()));
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

void displaySignalLevel() {

    oled.setFont(ArialMT_Plain_10);
    oled.setTextAlignment(TEXT_ALIGN_LEFT);
    char levelStr[30];
    sprintf(levelStr,"@%hhddb",WiFi.RSSI());
    //Serial.println(levelStr);
    oled.setColor(BLACK);
    oled.fillRect(90,10,30,10);
    oled.setColor(WHITE);
    oled.drawString(90,10,levelStr);
    oled.display();
}
bool doWifiWait(){
      // Wait for connection
    unsigned int count=0;
    for (int i=0;i<128;i++) {
      displayPoint(count,BLACK);
    }
    Serial.println("");
    Serial.print("trying to connect to:");
    Serial.print(ssid);
    Serial.print(" ");
    Serial.print(password);
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED && count < 128) {
        delay(500);
        Serial.print(".");
        displayPoint(count,WHITE);
        oled.display();
        count ++;
    }

    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: http://");
    Serial.print(WiFi.localIP());
    oled.setColor(BLACK);
    oled.fillRect(0,20,128,1);
    oled.setColor(WHITE);
    oled.drawString(0,10,WiFi.localIP().toString());
    displaySignalLevel();
    return true;
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

    oledSetup();
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
    String macStr=WiFi.macAddress();
    oled.drawString(36,0,macStr);
    wifiIsConnected=doWifiWait();
    if (!wifiIsConnected)
        esp_restart();

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

void drawTime(){
  String timenow = String(hour())+":"+twoDigits(minute())+":"+twoDigits(second());
  oled.setTextAlignment(TEXT_ALIGN_CENTER);
  oled.setFont(ArialMT_Plain_24);
  oled.setColor(BLACK);
  oled.fillRect(0,40,128,24);
  oled.setColor(WHITE);
  oled.drawString(64 , 40, timenow );
}

 
int pole1[] ={0,0,0,0, 0,1,1,1, 0};//pole1, 8 step values
int pole2[] ={0,0,0,1, 1,1,0,0, 0};//pole2, 8 step values
int pole3[] ={0,1,1,1, 0,0,0,0, 0};//pole3, 8 step values
int pole4[] ={1,1,0,0, 0,0,0,1, 0};//pole4, 8 step values

int poleStep = 0; 

int feeds=0;
bool bFeeding;
void loop(void) {
  server.handleClient();
  displaySignalLevel();
  drawTime();
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
 
