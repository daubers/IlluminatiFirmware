#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "";
const char* password = "";
const char* host = "illuminati";

// Update these with values suitable for your network.
IPAddress ip(192,168,0,128); 
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,255,255,0);

int led_red = 128;
int led_green = 128;
int led_blue = 128;

#define LED_PIN 14

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);


void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

bool isGoodValue(String inputValue){

  bool badDigit = false;

  for (int x=0; x<=inputValue.length()-1; x++){
    if (!isDigit(inputValue[x])){
      badDigit = true;
      returnFail(inputValue[x] + " is not a valid digit");
      break;
    }
  }
  
  return !badDigit;
}

void setColour(int red, int green, int blue){
  strip.setPixelColor(0, strip.Color(red,green,blue));
  strip.show();
}

void changeColour() {
  if(!server.hasArg("red")) return returnFail("BAD ARGS");
  if(!server.hasArg("green")) return returnFail("BAD ARGS");
  if(!server.hasArg("blue")) return returnFail("BAD ARGS");
  String red = server.arg("red");
  String green = server.arg("green");
  String blue = server.arg("blue");
  if (isGoodValue(red) && isGoodValue(green) && isGoodValue(blue)){
    led_red = red.toInt();
    led_green = green.toInt();
    led_blue = blue.toInt();
    setColour(led_red, led_green, led_blue);
  }
  returnOK();
}

void handleNotFound(){
  String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


void setup(void){
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {//wait 10 seconds
    delay(500);
  }
  if(i == 21){
    while(1) delay(500);
  }

  //MDNS probably won't work in the office, but it's nice
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
  }

  //initialise the neopixel
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //Set links from requests to functions, just add more server.on lines
  //for more links
  server.on("/update", HTTP_GET, changeColour);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop(void){
  server.handleClient();
}

