/*
 * vim: sta:et:sw=4:ts=4:sts=4
 * @file main.cpp
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
//#include <ESP8266WiFi.h>
#include "loop_functions.h"

enum st_state state;
// Create aREST instance
//aREST_UI rest = aREST_UI();


// Create an instance of the server

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80
//WiFiServer server(LISTEN_PORT);
//void printWifiStatus();


/**  Seeduino Wio specific **/
#define LED_YELLOW 12   // e.g. Yelow LED
//#define LED_OUT 12   // e.g. Yelow LED
//#define SWITCH_YELLOW 13 // PD5
#define SWITCH_OUT 13 // PD5 YELLOW

//#define LIMIT_CLSD  5   // SCL   Limit CLOSED Yellow cable
//#define LIMIT_OPN   4  // SDA    Limit OPEN Blue cable
#define LIMIT_IN  5   // SCL   Limit CLOSED Yellow cable
#define LIMIT_OUT   4  // SDA    Limit OPEN Blue cable

//#define RELAY_OPEN 14
#define RELAY_OUT 14

#define RELAY_OFF LOW
#define RELAY_ON HIGH
/**  Seeduino Wio specific **/

//bool sensorLimClsd, sensorLimOpen;
bool switchYellow;
bool WifiConnect;
//unsigned long holding ;
//char rxChar= 0;         // RXcHAR holds the received command.

bool sensorLimIn, sensorLimOut;
bool switchIn, switchOut;
unsigned long holding;
char in_char;

//int state_rest;
void loop_led();
//void loop_status();

void setup() {

    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    //pinMode(SWITCH_YELLOW,  INPUT_PULLUP);
    pinMode(SWITCH_OUT,  INPUT_PULLUP);

    //pinMode(LIMIT_CLSD, INPUT_PULLUP);
    //pinMode(LIMIT_OPN, INPUT_PULLUP);
    pinMode(LIMIT_OUT, INPUT_PULLUP);
    pinMode(LIMIT_IN,  INPUT_PULLUP);

    //pinMode(RELAY_OPEN, OUTPUT);

    //pinMode(LED_OUT, OUTPUT);
    pinMode(RELAY_OUT, OUTPUT);

    //pinMode(PIN_GROVE_POWER, OUTPUT);
    //digitalWrite(PIN_GROVE_POWER, 1);
    // ou must pull up GPIO 15 in your Arduino sketch to power on the Grove system:
    pinMode(PIN_GROVE_POWER,  INPUT_PULLUP);

    // Start serial port
    Serial.begin(115200);
    state = stopped;

    //sensorLimClsd =  !digitalRead(LIMIT_CLSD);
    //sensorLimClsd =  !digitalRead(LIMIT_CLSD);
    //sensorLimOpen = !digitalRead(LIMIT_OPN); //false; //!digitalRead(LIMIT_OPN);
    switchIn = false;

    in_char = 'x';
    //if (sensorLimClsd && sensorLimOpen)
    //state = error;
    //else if (sensorLimOpen)
    //state = fully_open;
    //else
    //state = moving_in;
}

// YELLOW blink loop
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

void loop_sm() {    //fast Loop

    int iByte; // for incoming serial data
    const long debounce = 1000; // 0.2 s
    const long debounce2 = 500; // 0.2 s
    //const long in_extend = 1000;

    while (Serial.available() > 0) {
        // read incoming bytes, store last:
        iByte = Serial.read();
//        if (iByte == 'i' || iByte == 'I')
//            in_char = 'i';
//        else
        if (iByte == 'o' || iByte == 'O')
            in_char = 'o';
        //else if (iByte == 's' || iByte == 'S')
        //    in_char = 's';
       // else
       //     in_char = 'x';
    }

    sensorLimIn = !digitalRead(LIMIT_IN);
    sensorLimOut = !digitalRead(LIMIT_OUT);
    //switchIn = !digitalRead(RED_SWITCH) || (in_char=='i') || (in_char=='I'); // red
    //switchOut = !digitalRead(BLUE_SWITCH) || (in_char=='o') || (in_char=='O');  // blue
    switchOut = !digitalRead(SWITCH_OUT) || (in_char=='o') || (in_char=='O');  // blue

    unsigned long now = millis();

    switch (state)
    {
        case stopped:
            digitalWrite(RELAY_OUT, RELAY_OFF);
            if (sensorLimIn && sensorLimOut )
                state = error;
            else if (sensorLimIn && !sensorLimOut )
                state = fully_closed;
            else if (!sensorLimIn && sensorLimOut )
                state = fully_open;
            else if (now > holding) {
                if (switchOut && !sensorLimOut){
                    holding = now + debounce;
                    in_char = 'x';
                    state = moving_out;
                }
            }
            break;
        case moving_in:
            if (sensorLimIn && sensorLimOut )
                state = error;
            else {
                digitalWrite(RELAY_OUT, RELAY_OFF);
                if (sensorLimIn){
                    state = fully_closed;
                }
            }
            break;
        case moving_out:
            if (sensorLimIn && sensorLimOut )
                state = error;
            else {
                digitalWrite(RELAY_OUT, RELAY_ON);
                if (sensorLimOut){
                    //Serial.println(F("Moving->fully_open"));
                    state = fully_open;
                }

                if (switchOut && (now > holding) ){
                    holding = now + debounce2;
                    state = stopped;
                    in_char = 'x';
                    //Serial.println(F("OUT->STOP"));
                }
            }
            break;
        case fully_closed:
            if (sensorLimIn && sensorLimOut )
                state = error;
            else {
                digitalWrite(RELAY_OUT, RELAY_OFF);
                if (switchOut && (now > holding) ){
                    holding = now + debounce2;
                    state = moving_out;
                    in_char = 'x';
                }
            }
            break;
        case fully_open:
            if (sensorLimIn && sensorLimOut )
                state = error;
            else {
                digitalWrite(RELAY_OUT, RELAY_ON);
                if (switchOut && (now > holding) ){
                    holding = now + debounce2;
                    state = moving_in;
                    in_char = 'x';
                }
            }
            break;
        case error:
            digitalWrite(RELAY_OUT, RELAY_OFF);
            if (sensorLimIn && !sensorLimOut )
                state = fully_closed;
            else if ( !sensorLimIn && sensorLimOut )
                state = fully_open;
            else if ( !sensorLimIn && !sensorLimOut )
                state = stopped;
            break;
        default:;
    }

}
void loop() {
    loop_sm();
    loop_led();
    //loop_serial();
    //loop_status();
    loop_print();
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
