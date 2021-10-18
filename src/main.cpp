/*
* @file main.c
 * @brief Arduino software to control MST12 Arm 
 * @author Bernardo Carvalho / IPFN
 * @date 14/10/2021
 *
 * https://github.com/Seeed-Studio/Wio_Link/wiki/Advanced%20User%20Guide
 * 
 * @copyright Copyright 2016 - 2021 IPFN-Instituto Superior Tecnico, Portugal
 * Licensed under the EUPL, Version 1.2 only (the "Licence");
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence, available in 23 official languages of
 * the European Union, at:
 * https://joinup.ec.europa.eu/community/eupl/og_page/eupl-text-11-12
 *
 * @warning Unless required by applicable law or agreed to in writing, software
 * distributed under the Licence is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the Licence for the specific language governing permissions and
 * limitations under the Licence.
 *
 * @details version Seeduino Wio Link
https://seeeddoc.github.io/Wio_Link/
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <aREST.h>
#include <aREST_UI.h>


// Create aREST instance
aREST_UI rest = aREST_UI();


// Create an instance of the server

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80
WiFiServer server(LISTEN_PORT);
void printWifiStatus();

int handle_rest_client() {
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return 0;
  }
  while (!client.available()) {
    delay(1);
  }
  rest.handle(client);
  return 0;
  // Serial.print("~");
}

//#define LED_OUT 2 // PD2  // e.g. blue LED
//#define BLUE_SWITCH PIN_WIRE_SDA
//#define BLUE_SWITCH 3 // PD3
//#define LED_IN 4   // e.g. Red LED
//#define RED_SWITCH 5 // PD5
//#define LED_IN 13   // e.g. Yelow LED
#define LED_YELLOW 12 //13   // e.g. Yelow LED
#define SWITCH_YELLOW 13// 2 // PD5
//#define LED_IN 14   // e.g. Yelow LED

#define LIMIT_IN  4   // SDA
#define LIMIT_OUT 5 // SCL

#define RELAY_OPEN 14
//#define LED_RED 12  // temp

//#define RELAY_OUT SCL // PB0

bool sensorLimIn, sensorLimOpen;
unsigned long holding ;
//int state_rest;
void loop_led();
void loop_rest();
int ledControl(String command);

enum arm_state
{                                                                                                                                                                                                                             
// stopped,
  moving_in,
  moving_out,
  limit_in,
  limit_open
} state;

//#define LED_PIN 13

const char* getStateName(enum arm_state state) 
{                                                                                                                
   switch (state) 
   {
   //   case stopped: return    "Stopped";
      case moving_in: return  "Moving In";
      case moving_out: return "Moving Out";
      case limit_in: return   "Limit In";
      case limit_open: return  "Limit Open";
      default: return "";
   }
}
const char* ssid     = "Lab. Plasmas Hipersonicos";
const char* password = "";

//const char* host = "data.sparkfun.com";

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup() {

  static const char * name_esp = "esp8266"; 

  pinMode(LED_YELLOW, OUTPUT);  //yellow
  pinMode(SWITCH_YELLOW,  INPUT_PULLUP);
  //pinMode(LED_BUILTIN, OUTPUT);

  pinMode(LIMIT_IN, INPUT_PULLUP);
  pinMode(LIMIT_OUT, INPUT_PULLUP);
  pinMode(RELAY_OPEN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // RED LED

  // 
  //pinMode(PIN_GROVE_POWER, OUTPUT);
  //digitalWrite(PIN_GROVE_POWER, 1);
  // ou must pull up GPIO 15 in your Arduino sketch to power on the Grove system:
  pinMode(PIN_GROVE_POWER,  INPUT_PULLUP);

  // Start serial port
  Serial.begin(115200);
  // Create UI
  rest.title(F("Relay Control"));
  rest.button(LED_BUILTIN); // 
  //rest.button(12); // 
  //rest.variable("state_rest", &state_rest);
  //rest.label("state_rest");
  rest.variable("state", &state);
  rest.label("state");
  //rest.variable("temperature",&state_rest);
  // Function to be exposed
  rest.function("led", ledControl);

  // Give name and ID to device
  rest.set_id(F("1"));
  rest.set_name(name_esp);// "esp8266");

   /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println(F("WiFi connected"));  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  // Serial.println(WiFi.localIP());
  // Print out the status
  printWifiStatus();
  digitalWrite(LED_BUILTIN, HIGH);
}
void loop_led() {

  static unsigned long nextTime = 0;
  const long interval = 200;
  static bool led_state = LOW;

  unsigned long now = millis();

  if ( now > nextTime ) {  
    nextTime = now + interval; 
    led_state = !led_state;   
    switch (state)
    {
      case moving_out:
          digitalWrite(LED_YELLOW, led_state);
      break;    
      case moving_in:
      case limit_in:  
          digitalWrite(LED_YELLOW, HIGH);
        break;          
      case limit_open:  
          digitalWrite(LED_YELLOW, LOW);
      default:;
    }
  }
}

void loop_sm() {
  static const long debounce = 2000; 
  static const long debounce_short = 500; 
  sensorLimIn =  false; // ! digitalRead(LIMIT_IN);
  sensorLimOpen = !digitalRead(LED_BUILTIN); //false; //!digitalRead(LIMIT_OUT);
  
  unsigned long now = millis();

  switch (state)
  {
    case moving_in:
       if (sensorLimIn){
        Serial.println(F("Moving->limit_in"));
        state = limit_in;
      }
    case limit_in:
    //case stopped:
      digitalWrite(RELAY_OPEN, LOW);
      if (now > holding) {
        if (!digitalRead(SWITCH_YELLOW) ){ //&& !sensorLimIn ){
            holding = now + debounce;
            state = moving_out;
            Serial.println(F("STOP->OUT"));
        }
      }
    break;

    case moving_out:
      digitalWrite(RELAY_OPEN, HIGH);
      //digitalWrite(LED_OUT, HIGH);
      if (sensorLimOpen){
        holding = now + debounce;
        Serial.println(F("Moving->limit_open"));
        state = limit_open;
      }
      if (now > holding) {    
        if (!digitalRead(SWITCH_YELLOW)) {
          holding = now + debounce_short;
          state = moving_in;
          Serial.println(F("OUT->IN"));
        }
      }      
    break;   

    case limit_open:
        digitalWrite(RELAY_OPEN, HIGH);
      if (!digitalRead(SWITCH_YELLOW)) {
        holding = now + debounce_short;
        state = moving_in;
      }
    break;
  default:;
  }
}

void loop() {
  loop_sm();
  loop_led();
  loop_rest();
}


void loop_rest() {
  //static bool led_state = LOW;
  static unsigned long nextTime = 0;
  static const unsigned long REFRESH_INTERVAL = 1000; // ms
  //static const long debounce = 1000; 
  //static const long debounce_short = 500;   unsigned long now = millis();

  unsigned long now = millis();
  //state_rest = state;

  if ( now > nextTime ) {  
    nextTime = now + REFRESH_INTERVAL; 
    //state = (state != stopped)? stopped:  moving_in;

    //if ( now > nextTime ) {  
    //nextTime = now + interval; 
    Serial.print(getStateName(state));
    //Serial.print(state_res);
    Serial.print(", LIM IN ");
    Serial.print(digitalRead(LIMIT_IN)); 
    Serial.print(", "); 
    Serial.print(", LIM OPN ");
    Serial.print(digitalRead(LED_BUILTIN)); 
    Serial.print(", "); 
    //Serial.print(digitalRead(led_state));  
    Serial.print(", SW: "); 
    Serial.print(digitalRead(SWITCH_YELLOW));
    Serial.print(", RLY: ");
    Serial.print(digitalRead(RELAY_OPEN)); 
    Serial.print(", ");
    Serial.print(now- holding);
    //Serial.print(digitalRead(led_state));  
    //Serial.print(digitalRead(SWITCH_YELLOW));

    //digitalWrite(LED_IN, state);  
    //digitalWrite(LED_IN, led_state);  
    //digitalWrite(LED_RED, state);  
    //digitalWrite(LED_BUILTIN, led_state);  
    //digitalWrite(RELAY_OPEN, state);  
    //digitalWrite(SDA, led_state);  
    //digitalWrite(SCL, led_state);  


    Serial.println(F("."));
    //delay(500);
    // put your main code here, to run repeatedly:

  }

  handle_rest_client();
}


int ledControl(String command) {
  // Print command
  Serial.println(command);

  // Get state from command
  int statec = command.toInt();
  Serial.println(F("ledControl"));
  digitalWrite(LED_BUILTIN, statec);
  return 1;
}