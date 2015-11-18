/*  Requires FastLED from http://fastled.io/ for pixels
    
    Cheerlights client that reads the lastest colour every 0.5 seconds
    and updates both the OLED and pixel accordingly
    
    By Carl Monk (@ForToffee)
*/
#include <SoftwareSerial.h>
#include <FastLED.h>

#define DEBUG true
#define NUM_LEDS 2
#define DATA_PIN 9

//global objects
CRGB leds[NUM_LEDS];
SoftwareSerial esp8266(2,3);

void setup()
{
  esp8266.begin(9600); // your esp's baud rate might be different
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  delay(100);
  allOff();

  sendData("AT+RST\r\n", "\r\nready",2000,DEBUG); // reset module
  sendData("AT+CWMODE=1\r\n",1000,DEBUG); // configure as station (client)
  /*uncomment following line to attach to known AP. Config is 
    stored between resets/power cycles*/
  //sendData("AT+CWJAP=\"SSID\",\"Password\"\r\n",30000,DEBUG); // connect to AP
  sendData("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  sendData("AT+CIPMUX=0\r\n",1000,DEBUG); // ensure single connection ONLY
}

void loop()
{
  getColour();

  if(esp8266.available()) // check if the esp is sending a message 
  {
    esp8266.flush();  //purge closed message
  }
  delay(5000);
}

void getColour()
{
  //connect to server
  sendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n", 10000,DEBUG);

  //build request & command, don't forget \r\n terminator
  String request = "GET /channels/1417/field/1/last.txt\r\n";
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
    String colour = "";
    for (int p=0; p < dataLen.toInt(); p++)
    {
       c = esp8266.read();
       colour += String(c);
    }
    setColour(colour);
    Serial.println("colour: " + colour);
  }
}

void setColour(String c)
{
  uint32_t pc = -1;
  if (c == "white")
  {  
      setAll(0xFF,0xFF,0xFF);
  }
  else if (c == "red")
  {  
      setAll(0xFF,0,0);
  }
  else if (c == "green")
  {  
      setAll(0,0x80,0);
  }
  else if (c == "blue")
  {  
      setAll(0,0,0xFF);
  }
  else if (c == "cyan")
  {  
      setAll(0,0xFF,0xFF);
  }
  else if (c == "magenta")
  {  
      setAll(0xFF,0,0xFF);
  }
  else if (c == "yellow")
  {  
      setAll(0xFF,0xFF,0);
  }
  else if (c == "purple")
  {  
      setAll(0x80,0,0x80);
  }
  else if (c == "orange")
  {  
      setAll(0xFF,0xA5,0);
  }
  else if (c == "pink")
  {  
      setAll(0xFF,0xC0,0xCB);
  }
  else if ((c == "warmwhite") || (c = "oldlace"))
  {  
      setAll(0xFD,0xF5,0xE6);
  }

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

// WS2812B support functions
void allOff()
{
  setAll(0,0,0);
}
void setAll(uint8_t red, uint8_t green, uint8_t blue)
{
  for (uint8_t i=0; i<NUM_LEDS; i++)
  {
    leds[i].g = red;
    leds[i].r = green;
    leds[i].b = blue;
  }
  FastLED.show();
}

