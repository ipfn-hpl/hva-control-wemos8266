/*
 * vim: sta:et:sw=4:ts=4:sts=4
 * @file loop_functions.cpp
 * @brief Arduino software to control MST12 Arm
 * @author Bernardo Carvalho / IPFN
 * @date 14/10/2021
 *
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

#include "loop_functions.h"

//=== function to print the command list:  ===========================
void printHelp(void){
    Serial.println("--- Command list: ---");
    Serial.println("? -> Print this HELP");
    Serial.println("o -> OPEN Valve  \"open\"");
    Serial.println("d -> CLOSE Valve \"close\"");
    Serial.println("s -> Valve     \"status\"");
}

const char* getStateName(enum st_state state)
{
    switch (state)
    {
        case stopped: return    "Stopped";
        case moving_in: return  "MovingIn";
        case moving_out: return "MovingOut";
        case fully_closed: return "Closed";
        case fully_open: return  "Open";
        case error: return  "Error";
        default: return "";
    }
}

void loop_print(void) {

    static unsigned long lastTime = 0;
    const long interval = 2000;
    static bool led_state = 0;

    unsigned long now = millis();

    if ( now - lastTime > interval ) {
        //state = 1;
        lastTime = now;
        //Serial.print(state);
        Serial.print(ADDRESS, DEC);
        Serial.print(F(", "));
        Serial.print(getStateName(state));
        Serial.print(F(", Char:"));
        Serial.print(in_char);
        Serial.print(F(", SwIN:"));
        Serial.print(switchIn, DEC);
        Serial.print(F(", SwOUT:"));
        Serial.print(switchOut, DEC);
        Serial.print(F(", LimIN:"));
        Serial.print(sensorLimIn, DEC);
        Serial.print(F(", LimOUT:"));
        Serial.print(sensorLimOut, DEC);
        Serial.print(F(", Millis:"));
        Serial.print(now, DEC);
        Serial.print(F(", Holding:"));
        Serial.println(holding, DEC);
        /* Then, later in main: */
        //printf("%s", getDayName(TheDay));
        //Serial.println(F(" end"));
        if (led_state ) {
            digitalWrite(LED_BUILTIN, LOW);
            led_state = 0;
        }
        else {
            digitalWrite(LED_BUILTIN, HIGH);
            led_state = 1;
        }
    }
    // printf("%s", getDayName(TheDay));
}
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

