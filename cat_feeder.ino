 /*
 * pin connections (may need to be changed if you have a different board)
 */

const int pinDetector1 = 25;
const int pinDetector2 = 26; 
const int pinDetector3 = 35;

static const int servoPin = 13;
#define ZERO 91
#define REVERSE 0
#define FORWARD 180


String feedTimes[] ={"6:00:00", "14:00:00", "22:00:00"};
//String feedTimes[] ={"4:00:00", "9:00:00", "13:00:00", "18:00:1", "21:59:00"};

int feedQuantity = 300;
#define REVERSE_QUANTITY 50

String catName = "Satoshi";


// Place for your network credentials
#include "network_creds.h"
//const char* ssid     = "wifi_name";
//const char* password = "wifi_passwd";

#define dnsAddress "feed-cat"

#include <Servo.h>
#include <SSD1306Wire.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#define CONFIG_MDNS_STRICT_MODE y
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);



Servo servo1;

void doServo(int position){
  static int previousServoPosition=0;
  servo1.write(position);
  if (position != previousServoPosition) {
       Serial.print("servoPosition: ");
       Serial.println(position);
       previousServoPosition = position;
  }
}

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

int  dirStatus = 0;// stores direction status 3= stop (do not change)

void handleRoot() {
   String timeNow = getTimeNow();
   //Robojax.com ESP32 Relay Motor Control
   if (!server.authenticate(http_username, http_password)) {
        return server.requestAuthentication();
   }
 String HTML ="<!DOCTYPE html>\
  <html>\
  <head>\
    <title>Cat feeder Control</title>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
    <meta http-equiv=\"refresh\" content=\"10; URL='http://" + server.hostHeader() + "/'\"/=>\
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

void OTAdelay(int ms) {
  uint32_t moment = millis();
  ArduinoOTA.handle();
  while (millis() - moment < ms) {
    ArduinoOTA.handle();
    yield();
  }
}
void setupOTA() {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(dnsAddress);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
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
void servoSetup(){
    servo1.attach(servoPin);
    doServo(ZERO);
}
void setup(void) {

    oledSetup();
    detectorSetup();
    servoSetup();
    

    Serial.begin(115200);//initialize the serial monitor
    Serial.println("cat feeder based on continuous modded servo motor");

    resetLogData();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
    String macStr=WiFi.macAddress();
    oled.drawString(36,0,macStr);
    wifiIsConnected=doWifiWait();
    if (!wifiIsConnected)
        esp_restart();


    timeSetup();
    setupOTA();
    server.on("/", handleRoot);
    server.on("/motor", HTTP_GET, handleMotorControl);           
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

int feeds=0;
bool bFeeding;
void loop(void) {
    server.handleClient();
    displaySignalLevel();
    drawTime();
    if (dirStatus == 0) {
        doServo(ZERO);
        if (!bFeeding) {
            for (int i=0; i<sizeof(feedTimes)/sizeof(String); i++) {
                if (getTimeNow().compareTo(feedTimes[i])==0){
                    feeds ++;
                    doFeed("Auto ("+String(feeds)+")", true); // reverse
                    bFeeding=true;
                }
            }
        } else {
            doFeed("Auto ("+String(feeds)+")", false);
            bFeeding=false;
        }
    } else if(dirStatus >0 ){ 
        dirStatus --; 
        if (dirStatus == 0) 
             addLog("zero " + getTimeNow());
    }else if(dirStatus <0){ 
        dirStatus ++;
        if (dirStatus == 0) 
             addLog("zero " + getTimeNow());
    }
    OTAdelay(2);
}//end of loop

void doFeed(String reason, int bReverse){
  doFeedInternal(reason, bReverse,0);
}

void doFeedInternal(String reason, int bReverse, int quantity){
   int actual_quantity;
   if (bReverse) {
       if (quantity == 0) {
          actual_quantity = REVERSE_QUANTITY;
       } else {
          actual_quantity = quantity;
       }
       addLog(reason + " -feed at " + getTimeNow());
       dirStatus = -actual_quantity;
       doServo(REVERSE);
   } else {
       if (quantity == 0) {
          actual_quantity = feedQuantity;
       } else {
          actual_quantity = quantity;
       }
      addLog(reason + " +feed at " + getTimeNow());
      dirStatus = actual_quantity;// CCW 
      doServo(FORWARD);
   }
}

void handleMotorControl() {
    if(server.arg(argId[0]) == "on") {
        feeds ++;
        doFeedInternal("Manual ("+String(feeds)+")", false,1000);
    }else if(server.arg(argId[0]) == "off"){
        dirStatus = 0;  // motor OFF        
    }else if(server.arg(argId[1]) == "on"){
        doFeedInternal("Manual reverse at " + getTimeNow(),true,50);
    }else if(server.arg(argId[1]) == "off"){
        dirStatus = 0;  // motor OFF        
    }  
    handleRoot();
}




 
