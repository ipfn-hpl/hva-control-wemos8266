/*
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

#ifndef __loop_functions_h__
#define __loop_functions_h__
#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <aREST.h>
//#include <aREST_UI.h>

#define ADDRESS 3

enum st_state
{
  stopped,
  moving_in,
  moving_out,
  fully_closed,
  fully_open,
  error
};
extern enum st_state state;
extern bool sensorLimIn, sensorLimOut;
extern bool switchIn, switchOut;
extern unsigned long holding;
extern char in_char;
//extern bool WifiConnect;
//extern WiFiServer server;
//extern aREST_UI rest;

const char* getStateName(enum st_state state);
void loop_print(void);
void printHelp(void);
//void printWifiStatus();
void connect_wifi_rest();
//int handle_rest_client();

#endif
