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



#define LED_YELLOW 12   // e.g. Yelow LED
#define SWITCH_YELLOW 13 // PD5

#define LIMIT_IN  4   // SDA
#define LIMIT_OUT 5 // SCL

#define RELAY_OPEN 14

bool sensorLimIn, sensorLimOpen;
bool switchYellow;
bool WifiConnect;
unsigned long holding ;
//int state_rest;
void loop_led();
void loop_status();

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
  int i;
  static const char * name_esp = "esp8266"; 

  pinMode(LED_YELLOW, OUTPUT);  
  pinMode(SWITCH_YELLOW,  INPUT_PULLUP);

  pinMode(LIMIT_IN, INPUT_PULLUP);
  pinMode(LIMIT_OUT, INPUT_PULLUP);
  pinMode(RELAY_OPEN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // BLUE LED
 
  //pinMode(PIN_GROVE_POWER, OUTPUT);
  //digitalWrite(PIN_GROVE_POWER, 1);
  // ou must pull up GPIO 15 in your Arduino sketch to power on the Grove system:
  pinMode(PIN_GROVE_POWER,  INPUT_PULLUP);

  // Start serial port
  Serial.begin(115200);
  // Create UI
  rest.title(F("Relay Control"));
  rest.button(LED_BUILTIN); // BLUE LED
  //rest.variable("state_rest", &state_rest);
  //rest.label("state_rest");
  rest.variable("state", &state);
  rest.label("state");
  // Function to be exposed
  //rest.function("led", ledControl);

  // Give name and ID to device
  rest.set_id(F("1"));
  rest.set_name(name_esp);// "esp8266");

   /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WifiConnect = false;
  for (i=0; i< 20; i++){
  //while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECTED){
      WifiConnect = true;
      //Serial.println("");
      break;  
    }
    delay(500);
    Serial.print(".");
  }
  if (WifiConnect == 0 ){
    //Serial.println("");
    Serial.println(F("WiFi Not connected"));
    for (i=0; i< 6; i++){
        digitalWrite(LED_BUILTIN, (i & 0x01) );
        delay(200);
    }
  }
  else {
    Serial.println(F("WiFi connected"));
    Serial.print("IP address: ");
    for (i=0; i< 10; i++){
        digitalWrite(LED_BUILTIN, (i & 0x01) );
        delay(200);
    }
    Serial.println(WiFi.localIP());
    // Start the server
    server.begin();
    Serial.println(F("Server started"));

    // Print out the status
    printWifiStatus();
  }

}
void loop_led() {

  static unsigned long nextTime = 0;
  //const long slow= 500;
  const long fast = 200;
  static int led_count = 0;
  bool led_slow;
  bool led_fast;

  unsigned long now = millis();

  led_slow = ((led_count & 0x4)==0x4);
  led_fast = ((led_count & 0x1)==0x1);
  
  if ( now > nextTime ) {  
    nextTime = now + fast; 
    led_count++;  // +=  1; //!led_state;   

    switch (state)
    {
      case moving_in:
          digitalWrite(LED_YELLOW, led_slow);
      break;    
      case moving_out:
          digitalWrite(LED_YELLOW, led_fast);
      break;    
      case limit_in:  
          digitalWrite(LED_YELLOW, LOW);
        break;          
      case limit_open:  
          digitalWrite(LED_YELLOW, HIGH);
      default:;
    }
  }
}

void loop_sm() {
  static const long debounce = 2000; 
  static const long debounce_short = 500; 
  sensorLimIn =  !digitalRead(LIMIT_IN);
  sensorLimOpen = !digitalRead(LIMIT_OUT); //false; //!digitalRead(LIMIT_OUT);
  switchYellow =  !digitalRead(SWITCH_YELLOW);

  
  unsigned long now = millis();

  switch (state)
  {
    case moving_in:
       if (sensorLimIn){
        Serial.println(F("moving_in->limit_in"));
        state = limit_in;
      }
    case limit_in:
      digitalWrite(RELAY_OPEN, LOW);
      if (now > holding) {
        if (switchYellow){  //&& !sensorLimIn ){
            holding = now + debounce;
            state = moving_out;
            Serial.println(F("moving_in->moving_out"));
        }
      }
    break;

    case moving_out:
      digitalWrite(RELAY_OPEN, HIGH);
      //digitalWrite(LED_OUT, HIGH);
      if (sensorLimOpen){
        holding = now + debounce;
        Serial.println(F("moving_out->limit_open"));
        state = limit_open;
      }
      else if (now > holding) {    
        if (switchYellow) {
          holding = now + debounce_short;
          state = moving_in;
          Serial.println(F("moving_out->moving_in"));
        }
      }      
    break;   

    case limit_open:
      digitalWrite(RELAY_OPEN, HIGH);
      if (switchYellow && (now > holding)) {
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
  loop_status();
}


void loop_status() {
  static unsigned long nextTime = 0;
  static const unsigned long REFRESH_INTERVAL = 1000; // ms

  unsigned long now = millis();

  if ( now > nextTime ) {  
    nextTime = now + REFRESH_INTERVAL; 
    Serial.print(getStateName(state));
    Serial.print(", LIM IN ");
    Serial.print(sensorLimIn); 
    Serial.print(", LIM OPN ");
    Serial.print(sensorLimOpen); 
    //Serial.print(digitalRead(led_state));  
    Serial.print(", SW: "); 
    Serial.print(switchYellow);
    Serial.print(", RLY: ");
    Serial.print(digitalRead(RELAY_OPEN)); 
    Serial.print(", ");
    Serial.print(now - holding);
    Serial.print(", Wifi: "); 
    Serial.print(WifiConnect);
    Serial.println(F("."));
  }
  if(WifiConnect)
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