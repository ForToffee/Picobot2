/*
  Built on MotorTest by 4Tronix

  This example code is licensed under CC-BY-SA https://creativecommons.org/licenses/by-sa/3.0/
 */

#include <FastLED.h>
#include <SoftwareSerial.h>

#define L1 4  // Left motor pin1
#define L2 5  // Left motor pin2
#define R1 6  // Right motor pin1
#define R2 7  // Right motor pin2

#define LED 13  // Blue LED pin
#define DEBUG true
#
// WS2812B definitions
#define NUM_LEDS 2
#define DATA_PIN 9
CRGB leds[NUM_LEDS];
SoftwareSerial esp8266(2,3);


// the setup routine runs once when you press reset:
void setup()
{                
  esp8266.begin(9600); // your esp's baud rate might be different
  delay(100);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  allOff();
  // initialize the digital pins we will use as an output.
  pinMode(L1, OUTPUT);     
  pinMode(L2, OUTPUT);     
  pinMode(R1, OUTPUT);     
  pinMode(R2, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);

  sendData("AT+RST\r\n", "\r\nready",2000,DEBUG); // reset module
  sendData("AT+CWMODE=1\r\n",1000,DEBUG); // configure as station (client)
  /*uncomment following line to attach to known AP. Config is 
    stored between resets/power cycles*/
  sendData("AT+CWJAP=\"SSID\",\"PASSWORD\"\r\n",30000,DEBUG); // connect to AP
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=0\r\n",1000,DEBUG); // ensure single connection ONLY

}

// the loop routine runs over and over again forever:
String oldData;
void loop()
{
  String data = getCommand();
  if (oldData != data) {
    Serial.println("Data: " + data);
    String action = "";
    String duration;
    String colour;
    bool parseHashtag = false;  //have we found a hashtag, if so we want to skip it
    bool parseColour = false;
    for (unsigned int i = 0; i < data.length(); i++)
    {
      switch (data.charAt(i)) {
        case '#':{
            doAction(action, duration);
            action = "";
            parseHashtag = true;
            break;
          }
        case '[':{
          if (!parseHashtag){
            doAction(action, duration);
            action = "";
            parseColour = true;
          }
          break;
        }
        case ']':{
          parseColour = false;

          //http://stackoverflow.com/questions/23576827/arduino-convert-a-sting-hex-ffffff-into-3-int
          // convert colour to integer
          const char* hexColour = colour.c_str();
          long number = strtol(hexColour, NULL, 16);

          // Split them up into r, g, b values
          long r = number >> 16;
          long g = number >> 8 & 0xFF;
          long b = number & 0xFF;

          setAll(r,g,b);
        }
        case 'b':     // backward or hex value
        case 'f':{    //forward or hex value
          if (parseColour){
            colour += 'f';
          }
        }
        
        case 'l':
        case 'r':{
          if (!parseHashtag && !parseColour){
            doAction(action, duration);
            action =  data.charAt(i);
          }
          break;
        }
        case ' ':{
          parseHashtag = false;
          break;
        }
        case 'w':{
          //wander
          break;
        }
        default:{
          if (parseColour) {
            colour += data.charAt(i);
          }
          else if (isDigit(data.charAt(i))){
            duration += data.charAt(i);
          }
        }
      }
    }
    doAction(action, duration);
    oldData = data;
  }
  delay(9000);
}


void doAction(String action, String &durationVal){
  int duration = atoi(durationVal.c_str());
  Serial.println("Action: " + action);
  Serial.println("Duration: " + durationVal);

  if (durationVal != "" && action != ""){
    switch (action.charAt(0)) {
      case 'f':{
        forward(duration * 1000);
        halt(1);
        break;
      }
      case 'b':{
        reverse(duration * 1000);
        halt(1);
        break;
      }
      case 'l':{
        leftSpin(duration * 100);
        halt(1);
        break;
      }
      case 'r':{
        rightSpin(duration * 100);
        halt(1);
        break;
      }
    }
  
    durationVal = "";
  }
}


String getCommand()
{
  //connect to server
  sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 10000,DEBUG);

  //build request & command, don't forget \r\n terminator
  String request = "GET /channels/65715/field/2/last.txt\r\n";
  String cmd = "AT+CIPSEND=";
  cmd+= request.length();
  cmd += "\r\n";
  
  //send command
  sendData(cmd,1000,DEBUG);
  //send data
  sendData(request, "bytes",2000,DEBUG);
  
  delay(500);
  
  //check for response  "+IPD,[size],[data]"
  if(esp8266.find("+IPD,"))
  {
    char c;
    
    //get payload size
    String dataLen = "";
    while(esp8266.available())
      {
        c = esp8266.read();
        if (String(c) == ":")
          break;
        dataLen += String(c);
      }
    Serial.println("dataLen: " + dataLen);
    
    //read payload
    String response = "";
    for (int p=0; p < dataLen.toInt(); p++)
    {
       c = esp8266.read();
       response += String(c);
    }
    response.toLowerCase();
    return response;
  }
}



void ledOn()
{
  digitalWrite(LED, HIGH);
}

void ledOff()
{
  digitalWrite(LED, LOW);
}


// robMove routine switches the correct inputs to the L298N for the direction selected.
void robMove(int l1, int l2, int r1, int r2)
{
  digitalWrite(L1, l1);
  digitalWrite(L2, l2);
  digitalWrite(R1, r1);
  digitalWrite(R2, r2);  
}

void reverse(int wait)
{
  Serial.println("Moving backward");
  robMove(LOW, HIGH, HIGH, LOW);
  delay(wait);
}

void forward(int wait)
{
  Serial.println("Moving forward");
  robMove(HIGH, LOW, LOW, HIGH);
  delay(wait);
}

void rightSpin(int wait)
{
  Serial.println("Spinning right");
  robMove(HIGH, LOW, HIGH, LOW);
  delay(wait);
}

void leftSpin(int wait)
{
  Serial.println("Spinning left");
  robMove(LOW, HIGH, LOW, HIGH);
  delay(wait);
}

void halt(int wait)
{
  Serial.println("Stopping");
  robMove(HIGH, HIGH, HIGH, HIGH); // Brake
//  robMove(LOW, LOW, LOW, LOW); // Coast
  delay(wait);
}

void allOff()
{
  for (int i=0; i<NUM_LEDS; i++)
    leds[i] = 0;
  FastLED.show();
}

void setAll(int red, int green, int blue)
{
  for (int i=0; i<NUM_LEDS; i++)
  {
    leds[i].g = red;
    leds[i].r = green;
    leds[i].b = blue;
  }
  FastLED.show();
}

String sendData(String command, const int timeout, boolean debug)
  {
    return sendData(command, "OK", timeout, debug);
  }    

String sendData(String command, String waitFor, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    bool exit = false;
    while( ((time+timeout) > millis()) && (!exit))
    {
      while((esp8266.available()) && (!exit))
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
        if (response.indexOf(waitFor) > -1)
          exit = true;  //found data terminator
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}

