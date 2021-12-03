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

#define LIMIT_CLSD  5   // SCL   Limit CLOSED Yellow cable 
#define LIMIT_OPN   4  // SDA    Limit OPEN Blue cable 

#define RELAY_OPEN 14

bool sensorLimClsd, sensorLimOpen;
bool switchYellow;
bool WifiConnect;
unsigned long holding ;
char rxChar= 0;         // RXcHAR holds the received command.

//int state_rest;
void loop_led();
void loop_status();

enum arm_state
{                                                                                                                                                                                                                             
// stopped,
  moving_in,
  moving_out,
  fully_closed,
  fully_open,
  error
} state;

//#define LED_PIN 13

const char* getStateName(enum arm_state state) 
{                                                                                                                
   switch (state) 
   {
   //   case stopped: return    "Stopped";
      case moving_in: return  "Moving In";
      case moving_out: return "Moving Out";
      case fully_closed: return   "Fully Closed";
      case fully_open: return  "Fully Open";
      case error: return  "error";
      default: return "";
   }
}
const char* ssid     = "Lab. Plasmas Hipersonicos";
const char* password = "";

//=== function to print the command list:  ===========================
void printHelp(void){
  Serial.println("--- Command list: ---");
  Serial.println("? -> Print this HELP");  
  Serial.println("o -> OPEN Valve  \"open\"");
  Serial.println("d -> CLOSE Valve \"close\"");
  Serial.println("s -> Valve     \"status\"");  
}

void loop_serial(){
  if (Serial.available() >0){          // Check receive buffer.
    rxChar = Serial.read();            // Save character received. 
    Serial.flush();                    // Clear receive buffer.
  
  switch (rxChar) {
    
    case 'o':
    case 'O':
        if (state != fully_open && state != error  ){
          state = moving_out;                          // If received 'o' or 'O':
          Serial.println(F("STOP->OUT"));        
	      }
        else 
            Serial.println(getStateName(state));
        break;
    case 'c':
    case 'C':                          // If received 'd' or 'D':
        if (state != fully_closed && state != error  ){
          state = moving_in;                          // If received 'o' or 'O':
          Serial.println(F("OUT-> IN"));        
	      }
        else 
            Serial.println(getStateName(state));
        break;
    case 's':
    case 'S':  
        Serial.println(getStateName(state));  
        break;
    case '?':                          // If received a ?:
        printHelp();                   // print the command list.
        break;
        
    default:                           
      Serial.print("'");
      Serial.print((char)rxChar);
      Serial.println("' is not a command!");
    }
  }
}

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

  pinMode(LIMIT_CLSD, INPUT_PULLUP);
  pinMode(LIMIT_OPN, INPUT_PULLUP);
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
    Serial.print(F("WiFi connected "));
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
  sensorLimClsd =  !digitalRead(LIMIT_CLSD);
  sensorLimOpen = !digitalRead(LIMIT_OPN); //false; //!digitalRead(LIMIT_OPN);

  if (sensorLimClsd && sensorLimOpen)
    state = error;
  else if (sensorLimOpen)
    state = fully_open;
  else
    state = moving_in;
}
void loop_led() {

  static unsigned long nextTime = 0;
  //const long slow= 500;
  const long fast = 200;
  static int led_count = 0;
  bool led_slow;
  bool led_normal;
  bool led_fast;

  unsigned long now = millis();

  led_slow = ((led_count & 0x4)==0x4);
  led_normal = ((led_count & 0x2)==0x2);
  //led_normal = ((led_count & 0x2)==0x2);
  led_fast = ((led_count & 0x3)==0x3);
  
  if ( now > nextTime ) {  
    nextTime = now + fast; 
    led_count++;  // +=  1; //!led_state;   

    switch (state)
    {
      case moving_in:
          digitalWrite(LED_YELLOW, led_slow);
      break;    
      case moving_out:
          digitalWrite(LED_YELLOW, led_normal);
      break;    
      case fully_closed:  
          digitalWrite(LED_YELLOW, LOW);
        break;          
      case fully_open:  
          digitalWrite(LED_YELLOW, HIGH);
        break;          
      case error:  
          digitalWrite(LED_YELLOW, led_fast);
      default:;
    }
  }
}

void loop_sm() {
  static const long debounce = 2000; 
  static const long debounce_short = 500; 
  sensorLimClsd =  !digitalRead(LIMIT_CLSD);
  sensorLimOpen = !digitalRead(LIMIT_OPN); //false; //!digitalRead(LIMIT_OPN);
  switchYellow =  !digitalRead(SWITCH_YELLOW);

  
  unsigned long now = millis();

  switch (state)
  {
    case moving_in:
      digitalWrite(RELAY_OPEN, LOW);
      if (sensorLimClsd && sensorLimOpen)
        state = error;
      else if (sensorLimClsd){
        Serial.println(F("moving_in->fully_closed"));
        state = fully_closed;
      }
    break;

    case fully_closed:
      digitalWrite(RELAY_OPEN, LOW);
      if (sensorLimClsd && sensorLimOpen)
        state = error;
      else  {
            if ((now > holding) && (switchYellow)) {  //&& !sensorLimClsd ){
              holding = now + debounce;
              state = moving_out;
              Serial.println(F("moving_in->moving_out"));
            }

      } 
    break;

    case moving_out:
      if (sensorLimClsd && sensorLimOpen)
        state = error;
      else {
        digitalWrite(RELAY_OPEN, HIGH);
        if (sensorLimOpen){
          holding = now + debounce;
          Serial.println(F("moving_out->fully_open"));
          state = fully_open;
        }
        else if (now > holding) {    
          if (switchYellow) {
            holding = now + debounce_short;
            state = moving_in;
            Serial.println(F("moving_out->moving_in"));
          }
        } 
      }      
    break;   

    case fully_open:
      if (sensorLimClsd && sensorLimOpen)
        state = error;
      else {    
        digitalWrite(RELAY_OPEN, HIGH);
        if (switchYellow && (now > holding)) {
          holding = now + debounce_short;
          state = moving_in;
        }
      }
    break;

    case error:
      digitalWrite(RELAY_OPEN, LOW);
      if (sensorLimClsd && !sensorLimOpen ) 
        state = fully_closed;         
      else if ( !sensorLimClsd && sensorLimOpen ) 
        state = fully_open;      
      else if ( !sensorLimClsd && !sensorLimOpen ) 
        state = moving_in;      

    break;
  default:;
  }
}

void loop() {
  loop_sm();
  loop_led();
  loop_serial();
  loop_status();
}


void loop_status() {
  static unsigned long nextTime = 0;
  static const unsigned long REFRESH_INTERVAL = 1000; // ms

  unsigned long now = millis();

  if ( now > nextTime ) {  
    nextTime = now + REFRESH_INTERVAL; 
    Serial.print(getStateName(state));
    Serial.print(", LIM CLSD ");
    Serial.print(sensorLimClsd); 
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