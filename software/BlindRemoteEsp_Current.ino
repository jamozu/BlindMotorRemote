/** **********************************************************************************************
   COVER'S REMOTE CONTROL 
*********************************************************************************************** */
/*
  This software may be modified and distributed under the terms of the MIT license.

  LICENSE
  <https://mit-license.org/>.
  ------------------------------------------------------------------------------------------------
  Copyright © 2023 Jesus Amozurrutia
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
  and associated documentation files (the “Software”), to deal in the Software without 
  restriction, including without limitation the rights to use, copy, modify, merge, publish, 
  distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the 
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or 
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  
  ------------------------------------------------------------------------------------------------
  RRoble Covers Remote Control
  ------------------------------------------------------------------------------------------------
  
  This project consists of a remote contol for the RollerHouse Rolling Covers (Blinds / Shades) 
  that use 433.92MHz frequency with a BF-305 equivalent protocol with 41 bit command.

  ------------------------------------------------------------------------------------------------
  Protocol Description
  ------------------------------------------------------------------------------------------------
  
  Each command has 3 segments:
    * 4 AGC sync. bits
    * 41 command tribits
    * Radio silence

  The pulse width is 23us. 

  The AGC sync. bits are:
    * HIGH 216 pulses
    * LOW 108 pulses
    * HIGH 75 pulses
    * LOW 15 pulses

  The command bits are:
    * Data 0 = HIGH (15 pulses), LOW (30 pulses)
    * Data 1 = HIGH (30 pulses), LOW (15 pulses)

  Command is as follows:
    * 16 bits for (unique) remote control ID, hard coded in remotes
    * 4 bits for channel ID: 1 - 15
    * 4 bits for command: DOWN = 1000, UP = 0011, STOP = 1010, CONFIRM/PAIR = 0010, LIMITS = 0100, 
      ROTATION DIRECTION = 0001
    * 8 bits for remote control mode: step by step = 10000000, continuous mode = 10001000
    * 8 bits for checksum This is the sum of the first 4 bytes, inverted (xored) + 2.
      Other remote brands may use a diferent constant for the checksum. 
    * 1 bit = 1

    41 bits in total

  Radio silence of 223 pulses
  
  ------------------------------------------------------------------------------------------------
  Definitions
  ------------------------------------------------------------------------------------------------

  - Device: Refers to the component as a whole. It is used to identify the status of the device 
    and to set the configuration.
  - Instance: It refers to each of the covers (blinds/shades).
  - Command: An instruction that is transmited to the covers via the 433MHz signal.
  - Action: The instruction for the covers.
  - OTA: Over The Air updates. Allow the firmware to be updated over WiFi.
  
  ------------------------------------------------------------------------------------------------
  Pinout ESP-12E (D1 Mini)
  ------------------------------------------------------------------------------------------------

                    /------------------------\
                    | RST               TXD0 | 
                    | ADC  (A0)         RXD0 | 
                    | EN           (D1) IO5  |-----> 
                    | IO16 (D0)    (D2) IO4  |-----> 
  Receiver    ----->| IO14 (D5)    (D3) IO0  |-----> Buzzer
  Button      ----->| IO12 (D6)    (D4) IO2  |-----> LED
  Transmitter ----->| IO13 (D7)    (D8) IO15 |-----> 
                    | VCC               GND  |
                    \------------------------/

  #define PIN_RF_TX                       D7      // RF Transmitter pin (13)
  #define PIN_RF_RX                       D5      // RF Receiver pin    (14)
  #define PIN_BUTTON                      D6      // Button             (12)
  #define PIN_LED1                        D4      // LED indicator      (2)
  #define PIN_BUZZER                      D3      // Buzzer             (0)
  ------------------------------------------------------------------------------------------------
  EEPROM Map
  ------------------------------------------------------------------------------------------------
  
  000 - 009       Initialization values.
  030 - 039       LED indicator state
  110 - 255       Boot information.
  
  ------------------------------------------------------------------------------------------------
  Flashing (This should be adjusted for your hardware)
  ------------------------------------------------------------------------------------------------
  Board: "Generic ESP8266 Module"
  Built in LED: 2
  Upload Speed: 115200
  CPU Frequency: 80 MHz
  Crystal Frequency: 26 MHz
  Flash Size: 4M (1M LittleFS)
  Flash Mode: QIO
  Flash Frequency: 40 MHz
  Reset Method: ck
  Debug Port: Disabled
  Debug Level: None
  IwIP Varaint: v2 Lower Memory
  
  
  ------------------------------------------------------------------------------------------------
  Changes
  ------------------------------------------------------------------------------------------------

  ----------------
  0.1.03
  
  - Split the cycle to subscribe to the entities topics from the discovery.

  
  ----------------
  0.1.04
  
  - Split the cycle that sets the entities to "online" from the subscribe cycle.
  - Set a timer to repeat the "online" cycle periodically.

  ----------------
  0.1.05

  - Fixed overflow issue. Changed conflicting variables that can be called inside interrupts.
  
  ----------------
  0.1.06

  - Set buzzer frequency as a configuration parameter.
  
  ------------------------------------------------------------------------------------------------
  Todo
  ------------------------------------------------------------------------------------------------

  
  ------------------------------------------------------------------------------------------------
  ------------------------------------------------------------------------------------------------
  @author Jesus Amozurrutia Elizalde <jamozu@gmail.com>
  @version 0.1.06
  @date 2023/05/14
  @since Friday February 03 2023, 22:45:00
  @copyright MIT license
  @pre Device: ESP-12E (ESP8266) / WeMOS D1 Mini
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sketch configuration starts here
//
//              v            v            v            v            v 

/* --------------------------------------------------------------------------------------------
   Device identification
   -------------------------------------------------------------------------------------------- */
// Hardware version
#define HVERSION                          "01"
#define HVERSION_NUM                      1

// Board (ESP8266 / D1 / NodeMCU)
//#define BOARD_ESP8266
#define BOARD_D1MINI
//#define BOARD_NODEMCU

// Software version
#define SVERSION                          "0.1.06"

// Title for the application
#define DEVICE_TITLE                      "RRoble X Remote"

// Title for WiFi/MQTT Setup Portal
#define PORTAL_PREFIX                     "RXRemote_"


/* --------------------------------------------------------------------------------------------
   Features
   -------------------------------------------------------------------------------------------- */
// Enable multiple clicks to trigger the Setup Portal
#define MULT_CLICK

// Enable MQTT discovery (compatible with HomeAssistant)
#define MQTT_DISCOVERY

// MQTT configuration enabled. Allows the device to be configured remotely trough MQTT
#define MQTT_CONFIG

// Allow reset through MQTT message
#define MQTT_RESET

// Enable OTA Updates
#define OTA_UPDATES

// Save staus information on reset (useful for debugging network issues)
#define COLLECT_STATS

// Number of boot cycles stats are collected
#define STATS_NUM                         3

// Allow clearing saved status via MQTT message
#define CLEAR_STATS


/* -------------------------------------------------------------------------------------------- 
   Device definitions
   -------------------------------------------------------------------------------------------- */
// Button debounce timer in ms
#define BTN_DEBOUNCE                      50

// Button long press in ms
#define BTN_LONG_PRESS                    2000

// Button double click timer in ms
#define DBL_CLICK_INTERVAL                600

// Number of double clicks to trigger the Setup Portal
#define MULT_CLICK_COUNT                  6

// LED indicator flash period
#define LED_FLASH                         600

// Beep period for Receiver
#define BEEP_PERIOD                       600000

// Beep period for AKN
#define BEEP_PERIOD_AKN                   200000

// Number of Beeps for AKN
#define BEEP_NUM                          3


/* -------------------------------------------------------------------------------------------- 
   Network
   -------------------------------------------------------------------------------------------- */
// WiFi re-connect timer (ms)
#define WIFI_RECONNECT                    8900

// Reboot after N consecutive WiFi connect attemps
#define WIFI_CONNECT_REBOOT               10

// Reboot after WiFi is restated N times without connectivity to the broker
#define WIFI_RESTART_REBOOT               3

// Network re-connect timer (ms)
#define NET_RECONNECT                     2200

// MQTT subscription QOS [0, 1]
#define MQTT_SUB_QOS                      1

// MQTT publish QOS [0, 1]
#define MQTT_PUB_QOS                      1

// MQTT maximum number of unanswered events
#define MQTT_MAX_EVENTS                   6

// MQTT Signature (Not implemented yet)
//#define MQTT_SIGN

// MQTT Buffer size (10 - 90)
#define BUFFER_SIZE                       8

// MQTT Subscription Busy Time Out (ms)
#define MQTT_BUSY_TO                      5000

// MQTT Buffer message timer (ms)
#define MQTT_BUFFER_TMR                   350

// Re-subscribe to MQTT Topics (sec)
#define MQTT_REFRESH                      90

// Re-subscribe to MQTT Topics (sec)
#define MQTT_RESUBSCRIBE                  1200

// Max. lenght for an incomming MQTT payload 
#define MQTT_MAX_MSG_LEN                  300

// MQTT Topic max size
#define TOPIC_SIZE                        81

// MQTT Buffer Payload max size
#define PAYLOAD_SIZE                      201


/* -------------------------------------------------------------------------------------------- 
   Messages
   -------------------------------------------------------------------------------------------- */
// Get state of linked light
#define MQTT_MSG_GET_STATE                "{\"action\":\"getState\"}"

// Alive message
#define MQTT_MSG_ALIVE                    "{\"state\":\"online\"}"

// Go live sequence start message to hub/broker
#define MQTT_MSG_RESET                    "{\"state\":\"reset\"}"

// Configuration saved feedback message
#define MQTT_MSG_SAVE                     "{\"feedback\":\"configuration saved\"}"

// Configuration saved feedback message
#define MQTT_MSG_SAVE_DATA                "{\"feedback\":\"remotes data saved\"}"

// Configuration saved feedback message
#define MQTT_MSG_SAVE_DATA1               "{\"feedback\":\"covers data saved\"}"

// Configuration save failed feedback message
#define MQTT_MSG_SAVE_FAIL                "{\"feedback\":\"configuration not saved\"}"

// System stats cleared feedback message
#define MQTT_MSG_SC                       "{\"feedback\":\"stats cleared\"}"

// System stats NOT cleared feedback message
#define MQTT_MSG_SNC                      "{\"feedback\":\"stats not cleared\"}"

// Capture mode enabled feedback message
//#define MQTT_MSG_CAP_ON                   "{\"capture\":\"capture_mode_on\"}"
#define MQTT_MSG_CAP_ON                   "ON"

// Capture mode disabled feedback message
//#define MQTT_MSG_CAP_OFF                  "{\"capture\":\"capture_mode_off\"}"
#define MQTT_MSG_CAP_OFF                  "OFF"

// Capture mode stopped with new data feedback message
#define MQTT_MSG_CAP_STOP                 "{\"capture\":\"new_remote_captured\"}"

// New remote saved feedback message
#define MQTT_MSG_CAP_NEW                  "{\"capture\":\"new_remote_saved\"}"

// Capture mode disabled feedback message
#define MQTT_MSG_CAP_CLEAR                "{\"capture\":\"capture_cleared\"}"

// New cover saved feedback message
#define MQTT_MSG_CAP_NODATA               "{\"capture\":\"no_data\"}"

// New remote name is empty
#define MQTT_MSG_CAP_NONAME               "{\"capture\":\"empty_name\"}"

// New remote ID is duplicated
#define MQTT_MSG_CAP_DUP                  "{\"capture\":\"duplicated_id\"}"

// New remote name is duplicated
#define MQTT_MSG_CAP_DUPNAME              "{\"capture\":\"duplicated_name\"}"

// Remote registry is full feedback message
#define MQTT_MSG_CAP_FULL                 "{\"capture\":\"registry_full\"}"

// New cover saved feedback message
#define MQTT_MSG_COV_NEW                  "{\"cover\":\"new_cover_saved\"}"

// Remote name not found feedback message
#define MQTT_MSG_COV_NOREM                "{\"cover\":\"remote_no_found\"}"

// Cover name is empty
#define MQTT_MSG_COV_NONAME               "{\"cover\":\"empty_name\"}"

// Remote registry is full feedback message
#define MQTT_MSG_COV_FULL                 "{\"cover\":\"registry_full\"}"

// Remote registry is full feedback message
#define MQTT_MSG_COV_AVAIL                "online"

// Remote registry is full feedback message
#define MQTT_MSG_COV_UNAVAIL              "offline"

// Configuration secret
#define MQTT_SECRET                       "secret"


/* -------------------------------------------------------------------------------------------- 
   Default configuration values
   -------------------------------------------------------------------------------------------- */
#define CONFIG_TIMEOUT        300                 // Setup Portal timeout
#define CONFIG_KEEPALIVE      57                  // "keepAlive" configuration parameter default
#define CONFIG_MQTT_PORT      1883                // "mqttport" configuration parameter default
#define CONFIG_MQTT_BROKER    ""                  // "mqttbroker" configuration parameter default
#define CONFIG_MQTT_USER      ""                  // "mqttuser" configuration parameter default
#define CONFIG_MQTT_PASS      ""                  // "mqttpass" configuration parameter default
#define CONFIG_MQTT_NAMESPACE "namespc"           // "namespc" configuration parameter default
#define CONFIG_MQTT_KEY       ""                  // "mqttkey" configuration parameter default
#define CONFIG_MPOS_TIME      800                 // "mposTime" configuration parameter default
#define CONFIG_BUZZER_FREC    2500                // "buzzerFrec" configuration parameter default 

/* -------------------------------------------------------------------------------------------- 
   Double Reset
   -------------------------------------------------------------------------------------------- */
// Time between resets to detect a double reset
#define DRD_TIMEOUT                       1.5

// Hardware address
#define DRD_ADDRESS                       0x00


/* -------------------------------------------------------------------------------------------- 
    Direct OTA Updates
   -------------------------------------------------------------------------------------------- */
// OTA Port
//#define OTA_AR_PORT 8266

// MD5 Hash for secure OTA Arduino Updates
//#define OTA_AR_HASH "266ba2d990fc2ff1ef34c571e4ea49de"

// Password for secure OTA Arduino Updates (not recommended)
//#define OTA_AR_PASS "myOtaPass"


/* -------------------------------------------------------------------------------------------- 
    OTA through HTTP Server (Overrides Direct OTA Updates)
   -------------------------------------------------------------------------------------------- */
// Define the next 4 parameters to update from an HTTP Server
#define OTA_HTTP_SERVER                   "192.168.100.92"
#define OTA_HTTP_PORT                     80
#define OTA_SCRIPT_NAME                   "/devices/xremoteUpdate.php"
                  
// Another option is to define the Http server in a single line
//#define OTA_HTTP_URL                    "http://192.168.100.92/devices/remoteUpdate.php"


/* -------------------------------------------------------------------------------------------- 
    Debug
   -------------------------------------------------------------------------------------------- */
// Enable debug messages to serial port
//#define DEBUG_MODE

// Serial port speed
#define SERIAL_SPEED                      74880


//              ^            ^            ^            ^            ^ 
//
// Sketch configuration ends here
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

/* --------------------------------------------------------------------------------------------
   Libraries
   -------------------------------------------------------------------------------------------- */
#include "LittleFS.h"           // This needs to be first
#include <ESP8266WiFi.h>        // Ver: 2.3.0 @ https://github.com/esp8266/Arduino
//                                 https://wiki.wemos.cc/tutorials:get_started:get_started_in_arduino
//                                 a) Start Arduino and open File -> Preferences window.
//                                 b) Enter the following URL into "Additional Board Manager URLs" field.
//                                    You can add multiple URLs, separating them with commas.
//                                    http://arduino.esp8266.com/stable/package_esp8266com_index.json
//                                    OLD: http://arduino.esp8266.com/versions/2.3.0/package_esp8266com_index.json
//                                 c) Open "Boards Manager" from Tools -> Board menu and install "esp8266" platform.
//                                 d) Patch the files @ https://github.com/esp8266/Arduino/commit/4dc4e75216dfe21ca1bc0ba4eebc2ec94d7b6be5
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>        // Ver: 0.14.0 @ https://github.com/tzapu/WiFiManager
#include <DoubleResetDetect.h>  // Ver: 1.0.0 @ https://github.com/jenscski/DoubleResetDetect
#include <ArduinoJson.h>        // Ver: 5.13.3 @ https://github.com/bblanchon/ArduinoJson/  
//                                 https://arduinojson.org
//                                 The Arduino Library Manager installs the ArduinoJson version 6 by default.
//                                 However, using version 5 is highly recommended because version 6 is still in beta stage.
#include <AsyncMqttClient.h>    // Ver: 0.8.1 @ https://github.com/marvinroger/async-mqtt-client/releases/tag/v0.8.1
                                // https://github.com/marvinroger/async-mqtt-client/blob/master/docs/1.-Getting-started.md
                                // Download the ZIP file
                                // Load the .zip with Sketch → Include Library → Add .ZIP Library
                                // Dependency: https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip
#include <Ticker.h>
#include <EEPROM.h>


/* --------------------------------------------------------------------------------------------
   Compilation messages
   -------------------------------------------------------------------------------------------- */
#warning "Image for Covers (blinds) Remote Control"
#warning "Hardware version XM01"
#if defined(BOARD_NODEMCU)
  #if defined(BOARD_D1MINI)
    #error "BOARD_NODEMCU and BOARD_D1MINI can not be defined together"
  #endif
  #if defined(BOARD_ESP8266)
    #error "BOARD_NODEMCU and BBOARD_ESP8266 can not be defined together"
  #endif
  #warning "Board: Node MCU"
  #define BOARD_PF ".XN"
#elif defined(BOARD_D1MINI)
  #if defined(BOARD_ESP8266)
    #error "BOARD_D1MINI and BOARD_ESP8266 can not be defined together"
  #endif
  #warning "Board: D1 Mini"
  #define BOARD_PF ".XM"
#else
  #ifndef BOARD_ESP8266
    #define BOARD_ESP8266
  #endif
  #warning "Board: ESP8266 Generic"
  #define BOARD_PF ".XG"
#endif


/* -------------------------------------------------------------------------------------------- 
   General definitions
   -------------------------------------------------------------------------------------------- */
// System timer (us)
#define SYS_TIMER                         1000

// PACKETS IN TRANSIT
#define IN_TRANSIT                        3

// TRANSIT TIMEOUT
#define TRANSIT_TO                        5

// MQTT instance type. Used by the IoT Hub to identify the device properties and to construct
// the MQTT topics
#define INSTANCE_TYPE                     "switch"

#if (defined(OTA_HTTP_SERVER) && defined(OTA_HTTP_PORT) && defined(OTA_SCRIPT_NAME)) || (defined(OTA_HTTP_URL))
  #define OTA_HTTP
  #define OTA_IMAGE_VERSION (SVERSION BOARD_PF HVERSION)
#endif


/* --------------------------------------------------------------------------------------------
   Remote parameters
   -------------------------------------------------------------------------------------------- */
// Number of remotes
#define NUM_REMOTES                       20
// Number of covers
#define NUM_COVERS                        50


/* -------------------------------------------------------------------------------------------- 
   RF Transmitter parameters.
   -------------------------------------------------------------------------------------------- */
// Sync bit 1 duration (tics)
#define RF_TIC_SYNC1                      24840
// Sync bit 2 duration (tics)
#define RF_TIC_SYNC2                      12420
// Sync bit 3 duration (tics)
#define RF_TIC_SYNC3                      8625
// Short pulse duration (tics)
#define RF_TIC_SHORT                      1725
// Long pulse duration (tics)
#define RF_TIC_LONG                       3450
// Transmitter packet buffer size
#define RF_TX_BUFFER                      50
// Remote Checksum adjustment
#define RF_CS_ADJ                         2
// Remote packet repeat
#define RF_TX_REPEAT                      6

// Remote Stop action 
#define ACTION_STOP                       80
// Remote Open action 
#define ACTION_OPEN                       192
// Remote Close action 
#define ACTION_CLOSE                      16
// Remote Confirm/Configure action 
#define ACTION_CONF                       64
// Remote Change Direction action
#define ACTION_DIRECTION                  128
// Remote Limit Mode action
#define ACTION_LIMIT                      32
// Remote Limit Mode action
#define ACTION_LIMIT_UP                   144
// Remote Limit Mode action
#define ACTION_LIMIT_DN                   160


/* -------------------------------------------------------------------------------------------- 
   RF Receiver parameters.
   -------------------------------------------------------------------------------------------- */
// Maximum time for a complete packet
#define RF_TIME_PACKET                    60000
// Maximum time for sync bits
#define RF_TIME_SYNC                      15800
// Sync bit 1 duration (us) range start
#define RF_TIME_SYNC1_F                   4222
// Sync bit 1 duration (us) range end
#define RF_TIME_SYNC1_T                   6000
// Sync bit 2 duration (us) range start
#define RF_TIME_SYNC2_F                   2100
// Sync bit 2 duration (us) range end
#define RF_TIME_SYNC2_T                   2856
// Sync bit 3 duration (us) range start
#define RF_TIME_SYNC3_F                   1100
// Sync bit 3 duration (us) range end
#define RF_TIME_SYNC3_T                   1980
// Short pulse duration (us) range start
#define RF_TIME_SHORT_F                   200
// Short pulse duration (us) range end
#define RF_TIME_SHORT_T                   400
// Long pulse duration (us) range start
#define RF_TIME_LONG_F                    500
// Long pulse duration (us) range end
#define RF_TIME_LONG_T                    800
// Pulse debounce time
#define RF_TIME_DEBOUNCE                  30
// Receiver packet sequence length
#define RF_SEQUENCE                       86
// Receiver number of command bits
#define RF_CMD_BITS                       40
// Receiver timeout (sec)
#define RF_RX_TIMEOUT                     90


/* -------------------------------------------------------------------------------------------- 
   EEPROM Addresses
   
    000 - 009       Initialization values.
    010 - 029       AC Configuration.
    030 - 109       Instance state.
    110 - 255       System Stats.
   -------------------------------------------------------------------------------------------- */
// EEPROM Address for Initialization
#define EADDR_INIT                        1

// EEPROM Initialization value
#define E_VAL_INIT                        22119

// EEPROM Address for device parameters
#define EADDR_STATE                       30

#ifdef COLLECT_STATS
  // EEPROM Address for Boot info
  #define EADDR_BOOT_INFO                 110
#endif


/* -------------------------------------------------------------------------------------------- 
   Hardware
   -------------------------------------------------------------------------------------------- */
// Pins
#ifdef BOARD_ESP8266
  #define PIN_RF_TX                       13      // RF Transmitter pin
  #define PIN_RF_RX                       14      // RF Receiver pin
  #define PIN_BUTTON                      12      // Button
  #define PIN_LED1                        2       // LED indicator
  #define PIN_BUZZER                      0       // Buzzer
#endif
#if defined(BOARD_D1MINI) || defined(BOARD_NODEMCU)
  #define PIN_RF_TX                       D7      // RF Transmitter pin (13)
  #define PIN_RF_RX                       D5      // RF Receiver pin    (14)
  #define PIN_BUTTON                      D6      // Button             (12)
  #define PIN_LED1                        D4      // LED indicator      (2)
  #define PIN_BUZZER                      D3      // Buzzer             (0)
#endif


/* -------------------------------------------------------------------------------------------- 
   Conditional libraries
   -------------------------------------------------------------------------------------------- */
#ifdef OTA_UPDATES
  #ifdef OTA_HTTP
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
  #else
    #include <ESP8266mDNS.h>
    #include <WiFiUdp.h>
    #include <ArduinoOTA.h>
  #endif
#endif


/* --------------------------------------------------------------------------------------------
   Device identification constants
   -------------------------------------------------------------------------------------------- */
// Title for the application
const char*     TITLE = DEVICE_TITLE " Ver. " SVERSION;

// Title for WiFi/MQTT configuration portal
const char*     PORTAL_SSID = PORTAL_PREFIX;

// MQTT Device type. Used by the IoT Hub to identify the device properties and to construct
// the MQTT topics
const char*     INST_TYPE = INSTANCE_TYPE;


/* -------------------------------------------------------------------------------------------- 
   Remote registry.
   -------------------------------------------------------------------------------------------- */
struct registyRemote {
  uint16_t      id;                       // Remote Id
  char          name[41];                 // Remote name
};
registyRemote   remotes[NUM_REMOTES];


/* -------------------------------------------------------------------------------------------- 
   Covers registry.
   -------------------------------------------------------------------------------------------- */
struct registyCovers {
  uint8_t       remote;                   // Remote Index
  uint8_t       chann;                    // Channel
  uint8_t       oTime;                    // Time from closed to open
  uint8_t       cTime;                    // Time from open to closed
  uint8_t       feedb;                    // Position feedback [0=no cover; 1=none; 2=time; 3=full]
  uint8_t       mpos;                     // Steps from the closed position
  char          name[41];                 // Instance Name
  uint8_t       len;                      // Name length
};
registyCovers   covers[NUM_COVERS];

/* -------------------------------------------------------------------------------------------- 
   RF Transmitter parameters.
   -------------------------------------------------------------------------------------------- */
// Transmitter channel bussy
bool            rfTxBussy;
// Transmitter packet sequence 
unsigned long   rfTxSeq[RF_SEQUENCE];
// Transmitter sequencer state
bool            rfTxSeqStt = false;
// Transmitter sequencer index (current position)
int             rfTxSeqIdx = 0;
// Transmitter Buffer pointer
int             rfTxBuffPtr = 0;
// Transmitter Buffer publish pointer
int             rfTxBuffPub = RF_TX_BUFFER;
// Transmitter Buffer Remote ID cache
uint16_t        rfTxBuffId[RF_TX_BUFFER];
// Transmitter Buffer Action cache
uint8_t         rfTxBuffAc[RF_TX_BUFFER];
// Transmitter Buffer Mode cache
bool            rfTxBuffMd[RF_TX_BUFFER];
// Transmitter Buffer Timestamp cache
unsigned long   rfTxBuffTs[RF_TX_BUFFER];
// Transmitter packet repetition
int             rfTxSeqRep = 0;

/* -------------------------------------------------------------------------------------------- 
   RF Receiver parameters.
   -------------------------------------------------------------------------------------------- */
// Receiver command sequencer index 
int             rfRxSeq = 0;
// Receiver sync bits sequencer index 
int             rfRxSeqSync = 0;
// Receiver current pulse state
bool            rfRxPulse = false;
// Receiver pulse length timestamp meter
unsigned long   rfRxPulseT0;
// Receiver packet length timestamp meter
unsigned long   rfRxPulseTp;
// Receiver buffer
char            rfRxBuff[42];
// Receiver last Remote ID
uint16_t        rfRxRemId;
// Receiver last Channel
uint8_t         rfRxRemChan;
// Receiver last Action
uint8_t         rfRxRemAcc;
// Receiver last Mode
bool            rfRxRemMode = false;
// Receiver last CheckSum
bool            rfRxRemCs = false;
// Receiver last Command flag
bool            rfRxGotCommand = false;
// Receiver Packet Buffer full indicator
bool            rfRxBuffFull = false;
// Receiver enabled flag
bool            rfRxEnable = false;
// Receiver channel bussy flag
bool            rfRxBussy = false;
// Receiver enabled timestamp (seconds)
unsigned long   rfRxStarted = 0;

/* -------------------------------------------------------------------------------------------- 
   Configuration parameters. Stored in FS (File System)
   -------------------------------------------------------------------------------------------- */
struct iotConfig {
  int           mqttport;                 // MQTT port
  char          mqttbroker[65];           // Broker domain or IP address
  char          mqttuser[41];             // MQTT User
  char          mqttpass[41];             // MQTT Password
  char          namespc[41];              // MQTT Namespace
  char          mqttkey[41];              // MQTT signature key (not implemented yet)
  int           keepAlive;                // MQTT Keepalive interval
  char          myId[21];                 // MQTT namespace Unique ID
  int           mposTime;                 // Time between micro adjustments
  int           buzzerFrec;               // Buzzer frequency
};
// Global configuration object
iotConfig       dconfig;


/* -------------------------------------------------------------------------------------------- 
   Topics
   -------------------------------------------------------------------------------------------- */
// MQTT topic to subscribe for incomming SET messages: "NAMESPACE/INSTANCE_TYPE/DEVICE_ID/set"
char            topicSet[TOPIC_SIZE];
// MQTT topic to publish device status: "NAMESPACE/INSTANCE_TYPE/DEVICE_ID/state"
char            topicState[TOPIC_SIZE];
// MQTT base topic for the covers: "NAMESPACE/INSTANCE_TYPE/DEVICE_ID/[s|c|p|q]"
char            topicCover[TOPIC_SIZE];
// Length of the preix for cover's topics
int             topicCoverPref;

/* -------------------------------------------------------------------------------------------- 
   Button definitions
   -------------------------------------------------------------------------------------------- */
struct pressButton {
  int           pinId;
  int           eventState;
  int           features;
  int           state;
  int           lastState;
  int           clkCtr;
  bool          lngFlag;
  unsigned int  repInterval;
  unsigned long btnTimer;
  unsigned long repTimer;
  unsigned long dblTimer;
};
pressButton     myButtons[1];


/* -------------------------------------------------------------------------------------------- 
   Configuration and loop variables
   -------------------------------------------------------------------------------------------- */
// Flag for saving configuration data
bool            shouldSaveConfig = false;
// Flag for indicating the file system is mounted
bool            mounted = false;
// Millisecond counter for periodic system calls
unsigned long   sysMillis = 0;
// Overflow detector  for system timer
unsigned long   sysOvf = 0;
// Flag for system overflow
bool            sysOvfFlg = false;
// Keep alive counter
int             keepAlive = 0;
// Second counter
unsigned long   aliveCounter = 0;
// Temp variables to create messages and compound strings
char            tmpMsg[341];
char            tmpMsg0[101];
char            tmpMsg1[41];
char            tmpMsg2[41];
char            tmpMsg3[41];
#ifdef DEBUG_MODE
  char          tmpMsgD[101];
#endif
#ifdef OTA_UPDATES
  // Enable OTA if WiFi is available on boot. Otherwise, disable OTA and continue
  // with normal operation
  bool          otaEnabled = false;
  // Update flag
  bool          otaUpdating = false;
#endif
#ifdef MQTT_DISCOVERY
  // Auto discovery sent flag
  bool          discovered = false;
  // Auto discovery Instance pointer
  int           discInst = 0;
  // Auto discovery Action pointer
  int           discActn = 0;
#endif


/* -------------------------------------------------------------------------------------------- 
   Network variables
   -------------------------------------------------------------------------------------------- */
// Flag for controling first connect cycle
bool            netFirst = true;
// Enable system reboot only if a full network connection is detected and then goes down
bool            canReboot = false;
// Flag for controling WiFi setup
bool            wifiSetup = false;
// Flag for controling WiFi connectivity
bool            wifiStatus = false;
// WiFi re-connect/timeout timer
unsigned long   wifiTimer = 0;
// Count consecutive connection attemps
uint16_t        wifiConnCtr = 0;
// WiFi restart counter (counts the number of times WiFi is forced to reset between successfull 
//  Broker connections)
uint16_t        wifiRstCtr = 0;
// Flag for controling MQTT setup
bool            mqttSetup = false;
// Flag for controling MQTT connectivity
bool            mqttStatus = false;
// Flag indicating we are waiting connection response from the MQTT broker
bool            mqttConnecting = false;
// MQTT event counter. Messages and connection attemps. Resets the count on callback functions
int             mqttEventCtr = 0;
// Pending MQTT events
unsigned int    mqttBusy0 = 0;
unsigned long   mqttTmr0 = 0;
// Re-subscribe counter
/////
int             reSubscribe = 0;
// Re-connect counter
int             mqttReConn = 0;
// Re-connect retry counter
int             mqttReTries = 0;
// Subscription cycle in progress
bool            mqttSubscribing = false;
// Subscription cycle in progress
int             mqttSsIdx = 0;
// Index for "online" cycle
int             mqttOnIdx = 0;
// "Online" cycle timer
int             mqttOnTimer = MQTT_REFRESH;
// "Online" cycle flag
bool            mqttSetOnline = false;
// Millisecond counter for net connect retry
unsigned long   netMillis = 0;
// Millisecond counter for net connect retry
unsigned long   pubMillis = 0;
// Device ID (MAC Address)
char            deviceId[21];
// Buffer to publish offline MQTT messages
char            buffTop[BUFFER_SIZE][TOPIC_SIZE];
char            buffMsg[BUFFER_SIZE][PAYLOAD_SIZE];
int             buffStat = 0;
int             buffPub = 0;
// Messages in transit
uint16_t        transit[IN_TRANSIT];
int             transitTic[IN_TRANSIT];
bool            transitBussy = false;


/* -------------------------------------------------------------------------------------------- 
   LED Control
   -------------------------------------------------------------------------------------------- */
// LEDs status
bool            ledsState = false;
// LED flash control
unsigned long   LedPwm = 0;


/* -------------------------------------------------------------------------------------------- 
   Buzzer Control
   -------------------------------------------------------------------------------------------- */
// Buzzer repetitions
int             buzzRep = 0;
// Buzzer beep period
unsigned long   buzzPer = 0;
// Buzzer on/off
bool            buzzState = false;
// Buzzer pin state
bool            buzzPinStt = false;
// Buzzer period control
unsigned long   buzzPerTmr = 0;
// Buzzer frecuency
unsigned long   buzzPeriod = 200;
// Buzzer frecuency control
unsigned long   buzzPwm = 0;

/* -------------------------------------------------------------------------------------------- 
   System stats
   -------------------------------------------------------------------------------------------- */
#ifdef COLLECT_STATS
  // Count number of times the system boots
  uint16_t      statBoot = 0;
  // Count number of forced resets
  int16_t       statRst = -1;
  // Count the number of seconds without full network connectivity
  uint16_t      statNoNet = 0;
  // WiFi total restarts counter (between boot)
  uint16_t      statWifiRstTtl = 0;
  // Loop counter
  uint16_t      statLoopCnt = 0;
  // Loop count per second
  uint16_t      statLoopPS = 0;
  // Button use counter
  uint16_t      statBtnCnt = 0;
  // Button detect counter
  uint16_t      statBtnDet = 0;
  // Timer overflow counter
  uint16_t      statTmrOvf = 0;
  // Counts the number of times a restar state is met due to network failure, but canReboot is 
  //  FALSE
  uint16_t      statNetRstStt = 0;
  // Count the number of transmit packets
  uint16_t      statTxPks = 0;
  // Count the number of received packets
  uint16_t      statRxPks = 0;
  // Count the number of invalid packets
  uint16_t      statRxInvPks = 0;
  // Count the number of captured packets
  uint16_t      statRxCap = 0;
  // Count the number of times capure mode is started
  uint16_t      statRxStart = 0;
  // Count the number of times capure mode is stopped
  uint16_t      statRxStop = 0;
  // Saved information between boots
  struct statStruct {
    uint16_t    loopPS;                   // statLoopPS
    uint16_t    btnCnt;                   // statBtnCnt
    uint16_t    tmrOvf;                   // statTmrOvf
    uint16_t    noNet;                    // statNoNet
    uint16_t    wifiRstTtl;               // statWifiRstTtl
    uint16_t    wifiRstCtr;               // wifiRstCtr
    uint16_t    wifiConnCtr;              // wifiConnCtr
    unsigned long aliveCounter;           // aliveCounter
  };
  statStruct    sStats[STATS_NUM];
#endif


/* -------------------------------------------------------------------------------------------- 
   Objects
   -------------------------------------------------------------------------------------------- */
// Reset detector
DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

// Network Objects
AsyncMqttClient mqttClient;

// Timers
Ticker ticker;


/* --------------------------------------------------------------------------------------------
   Function definitions
   -------------------------------------------------------------------------------------------- */
#ifdef DEBUG_MODE
  void writeDebug (const char* message, int scope = 0, bool keepLine = false);
#endif

void mqttPublish (int tIndex, int instance, const char* payload, bool useBuffer = false);

/** **********************************************************************************************
  Radio Transmitter sequencer
  
  Sends the command in the output sequence.
*********************************************************************************************** */
void IRAM_ATTR rfTxPulseSeq () {
  // Increase the sequence
  ++rfTxSeqIdx;
  if (rfTxSeqIdx >= RF_SEQUENCE || rfTxSeqIdx < 0) {
    ++rfTxSeqRep;
    if (rfTxSeqRep >= RF_TX_REPEAT) {
      rfTxBussy = false;
      #ifdef COLLECT_STATS
        ++statTxPks;
      #endif
      return;
    }
    // Repeat sequence
    rfTxSeqStt = false;
    rfTxSeqIdx = 0;
  }
  // Set the pin state for this pulse
  rfTxSeqStt = !rfTxSeqStt;
  int pv = (rfTxSeqStt) ? HIGH : LOW;
  digitalWrite(PIN_RF_TX, pv);
  // Set the timer with the duration of this pulse
  timer1_write(rfTxSeq[rfTxSeqIdx]);
}

/** **********************************************************************************************
  Radio Receiver sequencer

  Detects changes in the RF input and measures the length of high and low values.
  It also calculates the individual bits received and saves them in the input sequencer.
*********************************************************************************************** */
void IRAM_ATTR rfRxPulseState () {
  //////////////////////////////////////////////////////////////////////////////////////
  // Get timer and pin state
  //////////////////////////////////////////////////////////////////////////////////////
  unsigned long zcT = micros();
  int rfEdge = digitalRead(PIN_RF_RX);
  //////////////////////////////////////////////////////////////////////////////////////
  // End detection when we have all 41 bits
  //////////////////////////////////////////////////////////////////////////////////////
  if (rfRxSeq >= RF_CMD_BITS) {
    rfReceiverStop();
    rfRxBuffFull = true;
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Check the sequence of RF edges and calculate discard debounce
  //////////////////////////////////////////////////////////////////////////////////////
  if (rfEdge == HIGH) {
    if (!rfRxPulse) return;
    rfRxPulse = false;  // Measure the LOW state
  } else {
    if (rfRxPulse) return;
    rfRxPulse = true; // Measure the HIGH state
  }
  // Calculate the time since the last event
  unsigned long evtTmr = zcT - rfRxPulseT0;
  rfRxPulseT0 = zcT;
  // Debounce
  if (evtTmr < RF_TIME_DEBOUNCE) return;
  // Stop other interruptions during Timer calculations
  noInterrupts();
  //////////////////////////////////////////////////////////////////////////////////////
  // Detect the SYNC sequence
  //////////////////////////////////////////////////////////////////////////////////////
  if (rfRxSeqSync < 3) {
    // Long pulse resets the packet sequence
    if (rfRxPulse && evtTmr > RF_TIME_SYNC1_F && evtTmr < RF_TIME_SYNC1_T) {
      rfRxPulseTp = zcT;
      rfRxSeqSync = 1;
      interrupts();
      return;
    }
    if (rfRxSeqSync == 1) {
      if (rfRxPulse || evtTmr < RF_TIME_SYNC2_F || evtTmr > RF_TIME_SYNC2_T) {
        interrupts();
        return;
      }
    } else if (rfRxSeqSync == 2) {
      if (!rfRxPulse || evtTmr < RF_TIME_SYNC3_F || evtTmr > RF_TIME_SYNC3_T) {
        interrupts();
        return;
      }
    }
    if ((zcT - rfRxPulseTp) > RF_TIME_SYNC) {
      rfRxReset();
      interrupts();
      return;
    }
    ++rfRxSeqSync;
    #ifdef COLLECT_STATS
      ++statRxPks;
    #endif
    interrupts();
    return;
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Detect bit sequence
  //////////////////////////////////////////////////////////////////////////////////////
  // Bits are detected at the end of the HIGH pulse
  if (rfRxPulse) {
    // Detect a Zero "0"
    if (evtTmr > RF_TIME_SHORT_F && evtTmr < RF_TIME_SHORT_T) {
      rfRxBuff[rfRxSeq] = 48;
    }
    // Detect a One "1"
    else if (evtTmr > RF_TIME_LONG_F && evtTmr < RF_TIME_LONG_T) {
      rfRxBuff[rfRxSeq] = 49;
    }
    // Unrecognized bit
    else {
      rfRxReset();
      #ifdef COLLECT_STATS
        ++statRxInvPks;
      #endif
      interrupts();
      return;
    } 
    // Increment the bit sequence
    ++rfRxSeq;
    // End detection when we have all 41 bits
    if (rfRxSeq >= RF_CMD_BITS) {
      rfReceiverStop();
      rfRxBuffFull = true;
    }
  }
  // Check the max. time for a packet
  if ((zcT - rfRxPulseTp) > RF_TIME_PACKET) rfRxReset();
  // Restart interrupts
  interrupts();
}

#ifdef DEBUG_MODE
/** **********************************************************************************************
  Write a message to the Serial port

  @param message Message
  @param scope Scope.
    - 0 = Plain (Default)
    - 1 = System debug
    - 2 = Network
    - 3 = Remote
    - 4 = MQTT
  @param keepLine Set to "true" to print on the same line
*********************************************************************************************** */
void writeDebug (const char* message, int scope, bool keepLine) {
  if (scope == 1) {
    Serial.print("SYS: ");
  } else if (scope == 2) {
    Serial.print("NET: ");
  } else if (scope == 3) {
    Serial.print("REM: ");
  } else if (scope == 4) {
    Serial.print("MQTT: ");
  }
  if (keepLine) {
    Serial.print(message);
  } else {
    Serial.println(message);
  }
}
#endif

/** **********************************************************************************************
  System timer

  Keep track of running time and execute periodic activities every second (aprox.)
*********************************************************************************************** */
void systemTimer () {
  //////////////////////////////////////////////////////////////////////////////////////
  // Validate second timer and overflow states
  //////////////////////////////////////////////////////////////////////////////////////
  unsigned long cTmr = millis();
  if (sysOvfFlg && cTmr >= sysOvf) return;
  sysOvfFlg = false;
  if (cTmr < sysMillis) {
    sysOvf = cTmr;
    return;
  }
  sysMillis += SYS_TIMER;
  // Check overflow
  if (sysMillis < cTmr && sysOvf > 2000000) {
    sysOvfFlg = true;
    #ifdef COLLECT_STATS
      ++statTmrOvf;
    #endif
  }
  sysOvf = cTmr;
  //////////////////////////////////////////////////////////////////////////////////////
  // One second passed
  //////////////////////////////////////////////////////////////////////////////////////
  ++aliveCounter;
  #ifdef COLLECT_STATS
    //////////////////////////////////////////////////////////////////////////////////////
    // Count seconds without network connection
    //////////////////////////////////////////////////////////////////////////////////////
    ++statNoNet;
  #endif
  //////////////////////////////////////////////////////////////////////////////////////
  // Keep alive message to broker
  //////////////////////////////////////////////////////////////////////////////////////
  ++keepAlive;
  if (keepAlive >= dconfig.keepAlive) {
    keepAlive = 0;
    mqttPublish(0, 0, MQTT_MSG_ALIVE);
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Re-subscribe at MQTT_RESUBSCRIBE intervals (6 min. arox.)
  //////////////////////////////////////////////////////////////////////////////////////
  if (mqttReConn > 0) --mqttReConn;
  if (mqttOnTimer > 0) {
    --mqttOnTimer;
    if (mqttOnTimer == 0) mqttSetOnline = true;
  }
  /*
  ++reSubscribe;
  if (reSubscribe >= MQTT_RESUBSCRIBE) {
    #ifdef DEBUG_MODE
      writeDebug("Start periodic re-subscription", 1);
    #endif
    mqttSubscribe();
  }
  */
  //////////////////////////////////////////////////////////////////////////////////////
  // Tic packets in transit
  //////////////////////////////////////////////////////////////////////////////////////
  mqttTicTransit();
  #ifdef MQTT_DISCOVERY
    //////////////////////////////////////////////////////////////////////////////////////
    // Trigger Auto-discovery
    //////////////////////////////////////////////////////////////////////////////////////
    mqttDiscovery();
  #endif
  //////////////////////////////////////////////////////////////////////////////////////
  // Timeout receiver
  //////////////////////////////////////////////////////////////////////////////////////
  if ((aliveCounter - rfRxStarted) > RF_RX_TIMEOUT) {
    rfReceiverClear();
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Check LED status
  //////////////////////////////////////////////////////////////////////////////////////
  if (!rfRxBussy && !ledsState) {
    flashLed();
  }
  #ifdef COLLECT_STATS
    //////////////////////////////////////////////////////////////////////////////////////
    // Reset the loop counter to get per second count
    //////////////////////////////////////////////////////////////////////////////////////
    statLoopPS = statLoopCnt;
    statLoopCnt = 0;
  #endif

}

/** **********************************************************************************************
  Define the device ID
*********************************************************************************************** */
void setDeviceId () {
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(deviceId, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "Device setup: %s / %s", INST_TYPE, deviceId);
    writeDebug(tmpMsgD, 1);
  #endif
}

/** **********************************************************************************************
  Flash LED indicators
*********************************************************************************************** */
void flashLed () {
  ledsState = !ledsState;
  uint8_t pv = (ledsState) ? HIGH : LOW;
  digitalWrite(PIN_LED1, pv);
}

/** **********************************************************************************************
  Handle LED indicators
*********************************************************************************************** */
void handleLed () {
  if (!rfRxBussy) return;
  unsigned long lTmr = millis();
  if ((unsigned long)(lTmr - LedPwm) > LED_FLASH) {
    flashLed();
    LedPwm = lTmr;
  }
}

/** **********************************************************************************************
  Buzz
*********************************************************************************************** */
void doBuzz () {
  buzzPinStt = !buzzPinStt;
  uint8_t pv = (buzzPinStt) ? HIGH : LOW;
  digitalWrite(PIN_BUZZER, pv);
}

/** **********************************************************************************************
  Turn on Buzzer
  
  @param period Buzzer period
  @param rep Number of buzzes
*********************************************************************************************** */
void buzzerOn (unsigned long period, int rep) {
  buzzPer = period;
  buzzRep = rep;
  buzzState = true;
  buzzPerTmr = micros();
}

/** **********************************************************************************************
  Turn off Buzzer
*********************************************************************************************** */
void buzzerOff () {
  buzzPer = 0;
  buzzRep = 0;
  buzzState = false;
  buzzPinStt = true;
  doBuzz();
}

/** **********************************************************************************************
  Handle Buzzer
*********************************************************************************************** */
void handleBuzzer () {
  if (buzzRep == 0) return;
  unsigned long lTmr = micros();
  if ((unsigned long)(lTmr - buzzPerTmr) > buzzPer) {
    buzzState = !buzzState;
    buzzPerTmr = lTmr;
    if (buzzRep > 0) {
      if (buzzState) {
        --buzzRep;
        if (buzzRep <= 0) buzzerOff();
      }
    }
  }
  if (!buzzState) return;
  if ((unsigned long)(lTmr - buzzPwm) > buzzPeriod) {
    doBuzz();
    buzzPwm = lTmr;
  }
}

/** **********************************************************************************************
  Set Buzzer Frequency
*********************************************************************************************** */
void setBuzzerFrequency (int frec) {
  buzzPeriod = 1000000 / (unsigned long)(frec) / 2;
}

/** **********************************************************************************************
  Set a RF command into the buffer to be transmitted

  Commands are composed of: remoteID / Channel / Command / Mode / (checksum)
  
  @param coverIdx Index for the cover
  @param action Cover action [ACTION_STOP | ACTION_OPEN | ACTION_CLOSE | ...]
  @param tMode Step mode [false = go to limit | true = step by step]
  @param tStamp Timstap to send the command, Zero indicates now
*********************************************************************************************** */
void rfSendCommand (int coverIdx, uint8_t action, bool tMode, unsigned long tStamp) {
  #ifdef DEBUG_MODE
    char tmpMsgL[61];
  #endif
  //////////////////////////////////////////////////////////////////////////////////////
  // Get the ID for the remote and set the action
  //////////////////////////////////////////////////////////////////////////////////////
  uint8_t channAction = (covers[coverIdx].chann | action);
  uint16_t remoteID = remotes[covers[coverIdx].remote].id;
  //////////////////////////////////////////////////////////////////////////////////////
  // Find the next available position in the buffer
  //////////////////////////////////////////////////////////////////////////////////////
  bool go = false;
  int ptr = 0;
  unsigned long vMin = rfTxBuffTs[0];
  for (int i = 0; i < RF_TX_BUFFER; i++) {
    // Get the oldest value in case of overflow
    if (rfTxBuffTs[rfTxBuffPtr] < vMin) {
      vMin = rfTxBuffTs[rfTxBuffPtr];
      ptr = rfTxBuffPtr;
    }
    // If the Timestamp is zero the position in the buffer is available
    if (rfTxBuffTs[rfTxBuffPtr] == 0) {
      go = true;
      break;
    }
    ++rfTxBuffPtr;
  }
  if (!go) {
    // Overflow situation
    rfTxBuffPtr = ptr;
    #ifdef DEBUG_MODE
      writeDebug("Transmitter buffer overflow", 3);
    #endif
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Save the command in the buffer
  //////////////////////////////////////////////////////////////////////////////////////
  rfTxBuffId[rfTxBuffPtr] = remoteID;
  rfTxBuffAc[rfTxBuffPtr] = channAction;
  rfTxBuffMd[rfTxBuffPtr] = tMode;
  if (tStamp == 0) tStamp = millis();
  rfTxBuffTs[rfTxBuffPtr] = tStamp;
  #ifdef DEBUG_MODE
    // Debug message
    sprintf(tmpMsgL, "Buffer command ID=%d CM=%d MD=%d [%d]", remoteID, channAction, tMode, rfTxBuffPtr);
    writeDebug(tmpMsgL, 3);
  #endif
  //////////////////////////////////////////////////////////////////////////////////////
  // Set the pointer to the next position in the buffer
  //////////////////////////////////////////////////////////////////////////////////////
  rfTxBuffPtr++;
  if (rfTxBuffPtr > RF_TX_BUFFER) {
    rfTxBuffPtr = 0;
  }
}

/** **********************************************************************************************
  Set the commands to move to a position

  @param coverIdx Index for the cover
  @param position Position for the cover [100=open|0=close]
*********************************************************************************************** */
void rfMove (int coverIdx, uint8_t position) {
  // Get current timestamp
  unsigned long tStamp = millis();
  // If the position is close to the top, OPEN then set the position
  // For position 1, set the micro adjstments
  if (position == 1) {
    #ifdef DEBUG_MODE
      writeDebug("Move to position 1 (mpos)", 3);
    #endif
    // Close, then open in micro-steps
    rfSendCommand(coverIdx, ACTION_CLOSE, false, tStamp);
    tStamp += ((covers[coverIdx].cTime + 1) * 1000);
    for (int i = 0; i < covers[coverIdx].mpos; i++) {
      rfSendCommand(coverIdx, ACTION_OPEN, true, tStamp);
      tStamp += dconfig.mposTime;
    }
  }
  else if (position > 65) {
    #ifdef DEBUG_MODE
      writeDebug("Move to open postion > 65", 3);
    #endif
    rfSendCommand(coverIdx, ACTION_OPEN, false, tStamp);
    tStamp += (covers[coverIdx].oTime + 1) * 1000;
    rfSendCommand(coverIdx, ACTION_CLOSE, false, tStamp);
    tStamp += (covers[coverIdx].cTime * 10) * (100 - position);
    rfSendCommand(coverIdx, ACTION_STOP, false, tStamp);
    tStamp += 100;
    rfSendCommand(coverIdx, ACTION_STOP, false, tStamp);
    tStamp += 100;
    rfSendCommand(coverIdx, ACTION_STOP, false, tStamp);
  }
  // For the rest of the positions, CLOSE then set the position
  else {
    #ifdef DEBUG_MODE
      writeDebug("Move to closed postion < 65", 3);
    #endif
    rfSendCommand(coverIdx, ACTION_CLOSE, false, tStamp);
    tStamp += ((covers[coverIdx].cTime + 1) * 1000);
    rfSendCommand(coverIdx, ACTION_OPEN, false, tStamp);
    tStamp += (covers[coverIdx].oTime * 10) * position;
    rfSendCommand(coverIdx, ACTION_STOP, false, tStamp);
    tStamp += 100;
    rfSendCommand(coverIdx, ACTION_STOP, false, tStamp);
    tStamp += 100;
    rfSendCommand(coverIdx, ACTION_STOP, false, tStamp);
  }
}

/** **********************************************************************************************
  Transmit the next RF command in the buffer
*********************************************************************************************** */
void rfTxRun () {
  //////////////////////////////////////////////////////////////////////////////////////
  // If the channel is bussy or the receiver is enabled, end here
  //////////////////////////////////////////////////////////////////////////////////////
  if (rfTxBussy || rfRxEnable) return;
  //////////////////////////////////////////////////////////////////////////////////////
  // Find the next item scheduled to go out
  //////////////////////////////////////////////////////////////////////////////////////
  unsigned long tNow = millis();
  bool go = false;
  int i;
  for (i = 0; i < RF_TX_BUFFER; i++) {
    ++rfTxBuffPub;
    if (rfTxBuffPub >= RF_TX_BUFFER) rfTxBuffPub = 0;
    if (rfTxBuffTs[rfTxBuffPub] == 0) continue;
    if (tNow > rfTxBuffTs[rfTxBuffPub] && (tNow - rfTxBuffTs[rfTxBuffPub]) < 800000) {
      go = true;
      break;
    }
  }
  // Nothig to transmit
  if (!go) return;
  //////////////////////////////////////////////////////////////////////////////////////
  // Transmit the next packet
  //////////////////////////////////////////////////////////////////////////////////////
  // Set channel as bussy
  rfTxBussy = true;
  // Define word variables
  int j;
  int idx = 0;
  uint8_t wrd;
  uint8_t wds[5];
  // Define the 5 bytes of the command
  wds[0] = (rfTxBuffId[rfTxBuffPub] & 255);
  wds[1] = (rfTxBuffId[rfTxBuffPub] >> 8);
  wds[2] = rfTxBuffAc[rfTxBuffPub];
  wds[3] = 1 + ((rfTxBuffMd[rfTxBuffPub]) ? 16 : 0);
  wds[4] = ((wds[0] + wds[1] + wds[2] + wds[3]) ^ 255) + RF_CS_ADJ;
  // Calculate radio pulses
  rfTxSeq[idx++] = RF_TIC_SYNC1;
  rfTxSeq[idx++] = RF_TIC_SYNC2;
  rfTxSeq[idx++] = RF_TIC_SYNC3;
  rfTxSeq[idx++] = RF_TIC_SHORT;
  for (i = 0; i < 5; i++) {
    wrd = wds[i];
    for (j = 0; j < 8; j++) {
      if (wrd & 1) {
        rfTxSeq[idx++] = RF_TIC_LONG;
        rfTxSeq[idx++] = RF_TIC_SHORT;
      } else {
        rfTxSeq[idx++] = RF_TIC_SHORT;
        rfTxSeq[idx++] = RF_TIC_LONG;
      }
      wrd = wrd >> 1;
    }
  }
  rfTxSeq[idx++] = RF_TIC_LONG;
  rfTxSeq[idx] = RF_TIC_SHORT;
  #ifdef DEBUG_MODE
    // Show the command
    sprintf(tmpMsgD, "Transmit command ID=%d CM=%d MD=CM=%d [%d]", rfTxBuffId[rfTxBuffPub], rfTxBuffAc[rfTxBuffPub], rfTxBuffMd[rfTxBuffPub], rfTxBuffPub);
    writeDebug(tmpMsgD, 3);
  #endif
  // Clear the position in the buffer
  rfTxBuffTs[rfTxBuffPub] = 0;
  // Start transmission
  rfTxSeqStt = false;
  rfTxSeqIdx = -1;
  rfTxSeqRep = 0;
  timer1_write(RF_TIC_SHORT);
}

/** **********************************************************************************************
  Enable receiver mode
*********************************************************************************************** */
void rfReceiverStart () {
  // Get the timestamp (seconds) when the receiver is enabled
  rfRxStarted = aliveCounter;
  // If the receiver is already enabled, end here.
  if (rfRxEnable) return;
  // Enable receiver mode
  rfRxEnable = true;
  // Turn on Buzzer
  buzzerOn(BEEP_PERIOD, -1);
  // Inform
  mqttPublish(0, 0, MQTT_MSG_CAP_ON, true);
  #ifdef DEBUG_MODE
    writeDebug("Receiver started", 3);
  #endif
  #ifdef COLLECT_STATS
    ++statRxStart;
  #endif
}

/** **********************************************************************************************
  Disable receiver mode
*********************************************************************************************** */
void rfReceiverStop () {
  if (!rfRxEnable && !rfRxBussy) return;
  // Stop the receiver mode and clear the channel
  rfRxEnable = rfRxBussy = false;
  // Stop receiver sequencer
  detachInterrupt(digitalPinToInterrupt(PIN_RF_RX));
  // Turn off Buzzer
  buzzerOff();
  // Inform
  mqttPublish(0, 0, MQTT_MSG_CAP_OFF, true);
  #ifdef DEBUG_MODE
    writeDebug("Receiver stopped", 3);
  #endif
  #ifdef COLLECT_STATS
    ++statRxStop;
  #endif
}

/** **********************************************************************************************
  Clear receiver buffers
*********************************************************************************************** */
void rfReceiverClear () {
  // Stop receiver mode
  rfReceiverStop();
  //
  if (!rfRxGotCommand) return;
  // Clear receiver data
  rfRxGotCommand = false;
  rfRxRemId = 0;
  rfRxRemChan = 0;
  rfRxRemAcc = 0;
  rfRxRemMode = false;
  rfRxRemCs = 0;
  // Inform
  mqttPublish(0, 0, MQTT_MSG_CAP_CLEAR, true);
  #ifdef DEBUG_MODE
    writeDebug("Clear captured remote", 3);
  #endif
}

/** **********************************************************************************************
  Start receiver sequencer
  
  Waits for transmitter to finish its current packet and then starts the receiver
*********************************************************************************************** */
void rfRxRun () {
  // Wait if receiver is not enabled or transmitter channel is bussy.
  if (!rfRxEnable || rfRxBussy) return;
  // Set receiver as bussy and start sequencer
  rfRxBussy = true;
  attachInterrupt(digitalPinToInterrupt(PIN_RF_RX), rfRxPulseState, CHANGE);
  #ifdef DEBUG_MODE
    writeDebug("Receiver channel set as bussy", 3);
  #endif
}

/** **********************************************************************************************
  Process the receiver buffer
*********************************************************************************************** */
void rfRxProcessBuffer () {
  // Wait until the receiver buffer gets a full packet
  if (!rfRxBuffFull) return;
  // Alert user of Remote OK
  buzzerOn(BEEP_PERIOD_AKN, BEEP_NUM);
  // Define variables to process command items
  uint16_t idb;
  int wdIdx;
  int i;
  uint8_t wbit = 1;
  uint8_t cs = 0;
  uint8_t wds[6];
  // Break the buffer into 5 eight bit words and do the sum for the checksum
  for (i = 0; i < 6; i++) wds[i] = 0;
  for (i = 0; i <= RF_CMD_BITS; i++) {
    wdIdx = i >> 3;                               // Divide by 8
    if (rfRxBuff[i] == 49) wds[wdIdx] |= wbit;    // Turn on ones
    wbit = wbit << 1;                             // Move the bit
    if (wbit == 0) {                              // Break the words
      wbit = 1;
      if (wdIdx <= 3) cs += wds[wdIdx];           // Sum the first 4 words
    }
  }
  // Calculate checksum
  cs = (cs ^ 255) + RF_CS_ADJ;
  // Get the remote ID
  rfRxRemId = wds[1];
  rfRxRemId = rfRxRemId << 8;
  rfRxRemId |= wds[0];
  // Get the channel
  rfRxRemChan = wds[2] & 15;
  // Get the action
  rfRxRemAcc = wds[2] & 240;
  // Get the mode
  rfRxRemMode = ((wds[3] & 16) > 0) ? true : false;
  // Validate checksum
  rfRxRemCs = (wds[4] == cs) ? true : false;
  // Set flag indicating capture ready
  rfRxGotCommand = true;
  // Reset and stop receiver
  rfRxReset();
  rfReceiverStop();
  // Publish receiver results
  char acc[6];
  if (rfRxRemAcc == ACTION_STOP) strlcpy(acc, "STOP", sizeof(acc));
  else if (rfRxRemAcc == ACTION_OPEN) strlcpy(acc, "OPEN", sizeof(acc));
  else if (rfRxRemAcc == ACTION_CLOSE) strlcpy(acc, "CLOSE", sizeof(acc));
  else if (rfRxRemAcc == ACTION_CONF) strlcpy(acc, "CONF", sizeof(acc));
  else if (rfRxRemAcc == ACTION_DIRECTION) strlcpy(acc, "DIR", sizeof(acc));
  else if (rfRxRemAcc == ACTION_LIMIT) strlcpy(acc, "LIMIT", sizeof(acc));
  else if (rfRxRemAcc == ACTION_LIMIT_UP) strlcpy(acc, "LIMUP", sizeof(acc));
  else if (rfRxRemAcc == ACTION_LIMIT_DN) strlcpy(acc, "LIMDN", sizeof(acc));
  sprintf(tmpMsg, "{\"state\":\"capture\",\"id\":%d,\"chan\":%d,\"action\":\"%s\",\"cs\":\"%d\"}", rfRxRemId, rfRxRemChan, acc, rfRxRemCs);
  mqttPublish(0, 0, tmpMsg, true);
  // Reset the clear timer
  rfRxStarted = aliveCounter;
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "New capture ID=%d CHAN=%d ACTION=%s CS=%d PK=%s", rfRxRemId, rfRxRemChan, acc, rfRxRemCs, rfRxBuff);
    writeDebug(tmpMsgD, 3);
  #endif
  #ifdef COLLECT_STATS
    ++statRxCap;
  #endif
}

/** **********************************************************************************************
  Reset the receiver sequencer
*********************************************************************************************** */
void rfRxReset () {
  rfRxSeqSync = 0;
  rfRxSeq = 0;
  rfRxPulse = false;
  rfRxBuffFull = false;
}

/** **********************************************************************************************
  Start WiFi
*********************************************************************************************** */
void wifiBegin () {
  if (!wifiSetup || (unsigned long)(millis() - wifiTimer) > WIFI_RECONNECT) {
    #ifdef OTA_UPDATES
      WiFi.mode(WIFI_STA);
    #endif
    WiFi.begin();
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    wifiTimer = millis();
    #ifdef DEBUG_MODE
      writeDebug("Connect to WiFi: ", 2, true);
      String dvSsid = WiFi.SSID();
      writeDebug(dvSsid.c_str());
    #endif
  }
  wifiStatus = false;
  wifiSetup = true;
}

/** **********************************************************************************************
  Check network connectivity
*********************************************************************************************** */
bool netCheck () {
  if (!wifiSetup) return false;
  //////////////////////////////////////////////////////////////////////////////////////
  // Check WiFi fail counters
  //////////////////////////////////////////////////////////////////////////////////////
  if (wifiRstCtr > WIFI_RESTART_REBOOT || wifiConnCtr > WIFI_CONNECT_REBOOT) {
    // Reboot only if a network connection has been detected since boot and the lights are off
    if (canReboot) {
      #ifdef COLLECT_STATS
        // Save the system stats that triggered the reset process
        bool ss = saveStatus(true);
        ss = false;
      #endif
      // Reboot
      ESP.restart();
    }
    #ifdef COLLECT_STATS
      else {
        ++statNetRstStt;
      }
    #endif
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Check MQTT fail counters
  //////////////////////////////////////////////////////////////////////////////////////
  if (mqttEventCtr > MQTT_MAX_EVENTS) {
    #ifdef DEBUG_MODE
      #ifdef COLLECT_STATS
        sprintf(tmpMsgD, "Broker is not responding. Reset network connections. %d / %d", wifiRstCtr, statWifiRstTtl);
      #else
        sprintf(tmpMsgD, "Broker is not responding. Reset network connections. %d ", wifiRstCtr);
      #endif
      writeDebug(tmpMsgD, 2);
    #endif
    // Disconnect MQTT
    mqttDisconnect();
    // Restart WiFi connection
    wifiStatus = false;
    WiFi.reconnect();
    ++wifiRstCtr;
    #ifdef COLLECT_STATS
      ++statWifiRstTtl;
    #endif
    return false;
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Check WiFi Connection
  //////////////////////////////////////////////////////////////////////////////////////
  // Not connected
  if (!WiFi.isConnected()) {
    // Status is Up, so connection just fell down
    if (wifiStatus) {
      // Change status
      wifiStatus = false;
      // Disconnect MQTT
      mqttDisconnect();
      #ifdef DEBUG_MODE
        writeDebug("WiFi is disconnected", 2);
      #endif
    }
    // Connection is down
    return false;
  }
  // Connected, but status is Down
  if (!wifiStatus) {
    // Change status
    wifiStatus = true;
    // Force a MQTT re-connection
    mqttDisconnect();
    // Force an immediate visit to netConnect()
    netFirst = true;
  }
  // Reset consecutive connection counter
  wifiConnCtr = 0;
  //////////////////////////////////////////////////////////////////////////////////////
  // Check MQTT connectivity
  //////////////////////////////////////////////////////////////////////////////////////
  // Not connected
  if (!mqttClient.connected()) {
    // Status is Up, so connection just fell down
    if (mqttStatus) {
      // Change status
      mqttStatus = false;
      // Clear timers/counters
      mqttTmr0 = 0;
      mqttEventCtr = 0;
      #ifdef DEBUG_MODE
        writeDebug("MQTT is disconnected", 4);
      #endif
    }
    // MQTT Connection is down
    return false;
  }
  // MQTT is connected, but status is Down
  if (!mqttStatus) {
    // Status is set on the Connect Callback function, so this staus is not expected
    ++mqttEventCtr;
    #ifdef DEBUG_MODE
      writeDebug("Ilegal MQTT Status", 4);
    #endif
    return false;
  }
  // Check MQTT busy timeout
  if (mqttBusy0 > 0) {
    // If busy for long, clear counters
    if ((unsigned long)(millis() - mqttTmr0) > MQTT_BUSY_TO) {
      mqttBusy0 = 0;
      ++mqttEventCtr;
    }
  }
  // All connections are OK
  wifiRstCtr = 0;
  // Once WiFi and MQTT connections are detected once, we allow a forced reboot
  canReboot = true;
  return true;
}

/** **********************************************************************************************
  Connect to the MQTT Broker
*********************************************************************************************** */
void netConnect () {
  if (!wifiSetup) return;
  if (!wifiStatus) {
    // Make sure connection is down before calling WiFi connector
    if (WiFi.status() != WL_CONNECTED) {
      #ifdef DEBUG_MODE
        #ifdef COLLECT_STATS
          sprintf(tmpMsgD, "Call WiFi Setup: %d, ERR: %d, RST: %d", WiFi.status(), wifiRstCtr, statWifiRstTtl);
        #else
          sprintf(tmpMsgD, "Call WiFi Setup: %d, ERR: %d", WiFi.status(), wifiRstCtr);
        #endif
        writeDebug(tmpMsgD, 2);
      #endif
      wifiBegin();
      // Count the number of simultaneous connection attemps
      ++wifiConnCtr;
    }
    return;
  }
  if (mqttConnecting) {
    mqttConnecting = false;
    return;
  }
  if (mqttReConn > 0) return;
  mqttBegin();
  #ifdef DEBUG_MODE
    if (!mqttStatus) {
      writeDebug("Connecting to Broker: ", 4, true);
      writeDebug(dconfig.myId, 0, true);
      writeDebug(" / ", 0, true);
      writeDebug(dconfig.mqttuser, 0, true);
      writeDebug(" @ ", 0, true);
      writeDebug(dconfig.mqttbroker, 0, true);
      sprintf(tmpMsgD, " :: %d ", mqttEventCtr);
      writeDebug(tmpMsgD);
    } else {
      writeDebug("Call broker connection", 4);
    }
  #endif
  // Count the number of consecutive connection attemps
  ++mqttEventCtr;
  ++mqttReTries;
  mqttConnecting = true;
  mqttClient.connect();
}

/** **********************************************************************************************
  MQTT Topics

  Define MQTT topics
*********************************************************************************************** */
void mqttDefineTopics () {
  // Device STATE topic
  sprintf(topicState, "%s/%s/%s/state", dconfig.namespc, INST_TYPE, dconfig.myId);
  // Device SET topic
  sprintf(topicSet, "%s/%s/%s/set", dconfig.namespc, INST_TYPE, dconfig.myId);
  // Covers
  sprintf(topicCover, "%s/cover/", dconfig.namespc);
  // Calculate the length of the topic prefix
  topicCoverPref = strlen(topicCover);
}

/** **********************************************************************************************
  MQTT Setup
*********************************************************************************************** */
void mqttBegin () {
  if (mqttSetup) return;
  for (int i = 0; i < IN_TRANSIT; i++) {
    transit[i] = 0;
    transitTic[i] = 0;
  }
  #ifdef MQTT_DISCOVERY
    mqttReDiscover();
  #endif
  mqttClient.onConnect(mqttConnectCallback);
  mqttClient.onDisconnect(mqttDisconnectCallback);
  mqttClient.onSubscribe(mqttSubscribeCallback);
  mqttClient.onUnsubscribe(mqttUnsubscribeCallback);
  mqttClient.onMessage(mqttMessageCallback);
  mqttClient.onPublish(mqttPublishCallback);
  mqttClient.setServer(dconfig.mqttbroker, dconfig.mqttport);
  mqttClient.setCredentials(dconfig.mqttuser, dconfig.mqttpass);
  mqttClient.setClientId(dconfig.myId);
  mqttSetup = true;
  #ifdef DEBUG_MODE
    writeDebug("Setup Broker: ", 4, true);
    writeDebug(dconfig.mqttbroker, 0, true);
    writeDebug(" / ", 0, true);
    char prtnm[13];
    sprintf(prtnm, "%d", dconfig.mqttport);
    writeDebug(prtnm);
  #endif
}

/** **********************************************************************************************
  MQTT connection completed
   
  @param sessionPresent Flag indicating a session is present.
*********************************************************************************************** */
void mqttConnectCallback (bool sessionPresent) {
  if (mqttConnecting) {
    mqttStatus = true;
  }
  mqttConnecting = false;
  mqttEventCtr = 0;
  mqttReTries = 0;
  #ifdef DEBUG_MODE
    if (sessionPresent) {
      writeDebug("Connected to MQTT, with session", 4);
    } else {
      writeDebug("Connected to MQTT", 4);
    }
  #endif
  mqttSubscribe();
}

/** **********************************************************************************************
  MQTT disconection detected
   
  @param reason Disconnect reason
*********************************************************************************************** */
void mqttDisconnectCallback (AsyncMqttClientDisconnectReason reason) {
  mqttConnecting = false;
  mqttStatus = false;
  #ifdef DEBUG_MODE
    writeDebug("MQTT Disconnected. Reason:", 4);
    Serial.println((uint8_t)reason);
  #endif
  if (mqttReTries < 3) {
    mqttReConn = 1;
  } else if (mqttReTries < 5) {
    mqttReConn = 2;
  } else if (mqttReTries < 10) {
    mqttReConn = 15;
  } else if (mqttReTries < 20) {
    mqttReConn = 30;
  } else {
    mqttReConn = 60;
  }
}

/** **********************************************************************************************
  MQTT Subscription completed
   
  @param packetId Id number of the message
  @param qos QOS obtained from broker
*********************************************************************************************** */
void mqttSubscribeCallback (uint16_t packetId, uint8_t qos) {
  if (mqttBusy0 > 0) {
    --mqttBusy0;
  }
  #ifdef DEBUG_MODE
    writeDebug("Subscribed", 4);
  #endif
}

/** **********************************************************************************************
  MQTT Un-Subscription detected
   
  @param packetId Id number of the published message
*********************************************************************************************** */
void mqttUnsubscribeCallback (uint16_t packetId) {
  mqttSubscribe();
}

/** **********************************************************************************************
  MQTT Publish completed
   
  @param packetId Id number of the published message
*********************************************************************************************** */
void mqttPublishCallback (uint16_t packetId) {
  #ifdef DEBUG_MODE
    writeDebug("Published  ", 4, true);
    Serial.println(packetId);
  #endif
  mqttUnTransit(packetId);
  mqttEventCtr = 0;
}

/** **********************************************************************************************
  MQTT Register a packet in transit
   
  @param pId Packet ID.
*********************************************************************************************** */
void mqttTransit (uint16_t pId) {
  int cnt = 0;
  for (int i = 0; i < IN_TRANSIT; i++) {
    if (transit[i] > 0) {
      ++cnt;
      continue;
    }
    transit[i] = pId;
    transitTic[i] = 0;
    ++cnt;
    break;
  }
  if (cnt >= IN_TRANSIT) {
    transitBussy = true;
  }
}

/** **********************************************************************************************
  MQTT Un-register a packet in transit
   
  @param pId Packet ID.
*********************************************************************************************** */
void mqttUnTransit (uint16_t pId) {
  for (int i = 0; i < IN_TRANSIT; i++) {
    if (transit[i] != pId) continue;
    transit[i] = 0;
    transitTic[i] = 0;
    transitBussy = false;
  }
}

/** **********************************************************************************************
  MQTT Tic packets in transit
*********************************************************************************************** */
void mqttTicTransit () {
  for (int i = 0; i < IN_TRANSIT; i++) {
    if (transit[i] == 0) continue;
    ++transitTic[i];
    if (transitTic[i] > TRANSIT_TO) {
      transit[i] = 0;
      transitTic[i] = 0;
      transitBussy = false;
    }
  }
}

/** **********************************************************************************************
  Force a disconnect from MQTT services
*********************************************************************************************** */
void mqttDisconnect () {
  if (mqttClient.connected()) {
    mqttClient.disconnect(true);
  }
  mqttStatus = false;
  mqttConnecting = false;
  mqttTmr0 = 0;
  mqttEventCtr = 0;
}

/** **********************************************************************************************
  MQTT subscribe to topics
*********************************************************************************************** */
void mqttSubscribe () {
  if (!mqttStatus) return;
  unsigned long cTmr = millis();
  reSubscribe = 0;
  mqttClient.subscribe(topicSet, MQTT_SUB_QOS);
  ++mqttBusy0;
  mqttTmr0 = cTmr;
  #ifdef DEBUG_MODE
    writeDebug("Subscribe to Device SET topic: ", 4, true);
    writeDebug(topicSet);
  #endif
  mqttSubscribing = true;
  mqttSsIdx = 0;
}

/** **********************************************************************************************
  Handle MQTT subscriptions
*********************************************************************************************** */
void mqttSubscriptions () {
  if (!discovered || !mqttSubscribing || mqttBusy0 > 0) return;
  // Look for the next enabled cover
  if (covers[mqttSsIdx].feedb > 0) {
    // Clear buffer
    mqttPublishBuffer();
    // Subscribe to Cover
    mqttSubscribeCover(mqttSsIdx);
    // Reset resubscription timer
    mqttTmr0 = millis();
  }
  ++mqttSsIdx;
  // Check if we hit the end of Covers registry
  if (mqttSsIdx >= NUM_COVERS) {
    mqttSubscribing = false;
    // Start "online" cycle
    mqttOnIdx = 0;
    mqttSetOnline = true;
  }
}

/** **********************************************************************************************
  Subscribe to a Cover topics
*********************************************************************************************** */
void mqttSubscribeCover (int idx) {
  // Define variables
  char topic[TOPIC_SIZE];
  int pos = topicCoverPref;
  unsigned long cTmr = millis();
  // Get the prefix of the Cover topic
  strlcpy(topic, topicCover, sizeof(topic));
  // Append the Cover name to the topic
  for (int i = 0; i < covers[idx].len; i++) {
    topic[pos++] = covers[idx].name[i];
  }
  topic[pos++] = 47;                          // "/"
  topic[(pos + 1)] = 0;
  // Subscribe to the COMMAND topic
  topic[pos] = 99;                            // "c"
  mqttClient.subscribe(topic, MQTT_SUB_QOS);
  ++mqttBusy0;
  // Subscribe to the POSITION topic
  if (covers[idx].feedb > 1) {
    topic[pos] = 113;                         // "q"
    mqttClient.subscribe(topic, MQTT_SUB_QOS);
    ++mqttBusy0;
  }
  #ifdef DEBUG_MODE
    char myVal[11];
    itoa(idx, myVal, 10);
    writeDebug("Subscribe to Cover: ", 4, true);
    writeDebug(myVal);
  #endif
}

/** **********************************************************************************************
  Set Covers to Online state
*********************************************************************************************** */
void mqttOnline () {
  if (!mqttSetOnline) return;
  if (transitBussy || mqttBusy0 > 0) return;
  // After subscription, set the cover to available
  if (covers[mqttOnIdx].feedb > 0) {
    mqttPublish(3, mqttOnIdx, MQTT_MSG_COV_AVAIL, true);
  }
  // Increment index
  ++mqttOnIdx;
  // Reset timers
  mqttOnTimer = MQTT_REFRESH;
  // Check if we hit the end of Covers registry
  if (mqttOnIdx >= NUM_COVERS) {
    mqttOnIdx = 0;
    mqttSetOnline = false;
  }
}

/** **********************************************************************************************
  Publish a message to MQTT Broker

  @param tIndex Topic index [0=device state | 1=cover state | 2=cover position | 3=cover available]
  @param cIndex Cover index
  @param payload MQTT Payload
  @param useBuffer Use Buffer, if MQTT is ofline in non-blocking mode
*********************************************************************************************** */
void mqttPublish (int tIndex, int cIndex, const char* payload, bool useBuffer) {
  #ifdef DEBUG_MODE
     char tmpMsgL[61];
  #endif
  char topic[TOPIC_SIZE];
  if (tIndex == 0) {
    strlcpy(topic, topicState, sizeof(topic));
  }
  else {
    int pos = topicCoverPref;
    strlcpy(topic, topicCover, sizeof(topic));
    // Append the name to the topic
    for (int i = 0; i < covers[cIndex].len; i++) {
      topic[pos++] = covers[cIndex].name[i];
    }
    topic[pos++] = 47;                          // "/"
    if (tIndex == 1) topic[pos++] = 115;        // "s"
    else if (tIndex == 2) topic[pos++] = 112;   // "p"
    else if (tIndex == 3) topic[pos++] = 97;    // "a"
    topic[pos] = 0;
  }
  if (mqttStatus && !transitBussy && mqttBusy0 == 0) {
    mqttTransit(mqttClient.publish(topic, MQTT_PUB_QOS, false, payload));
    ++mqttEventCtr;
    #ifdef DEBUG_MODE
      writeDebug("Publish on topic: ", 4, true);
      writeDebug(topic, 0, true);
      sprintf(tmpMsgL, " :: %d ", mqttEventCtr);
      writeDebug(tmpMsgL);
      writeDebug(payload);
    #endif
    useBuffer = false;
  }
  if (!useBuffer) return;
  // Messages larger than the buffer are not saved
  if (strlen(payload) >= PAYLOAD_SIZE) return;
  // Save message in Buffer for later processing
  strlcpy(buffTop[buffStat], topic, sizeof(topic));
  strlcpy(buffMsg[buffStat], payload, sizeof(buffMsg[buffStat]));
  #ifdef DEBUG_MODE
    sprintf(tmpMsgL, "Buffer message %d for topic ", buffStat);
    writeDebug(tmpMsgL, 4, true);
    writeDebug(topic);
    writeDebug(payload);
  #endif
  // Buffer overflow... overwrite the oldest message
  if ((buffPub == (buffStat + 1)) || (buffStat == BUFFER_SIZE && buffPub == 0)) {
    buffPub++;
    if (buffPub > BUFFER_SIZE) {
      buffPub = 0;
    }
  }
  // Increment buffer 
  buffStat++;
  if (buffStat > BUFFER_SIZE) {
    buffStat = 0;
  }
}

/** **********************************************************************************************
  Publish pending messages in the buffer
*********************************************************************************************** */
void mqttPublishBuffer () {
  if (buffStat == buffPub) return;
  if (!mqttStatus || transitBussy) return;
  mqttTransit(mqttClient.publish(buffTop[buffPub], MQTT_PUB_QOS, false, buffMsg[buffPub]));
  ++mqttEventCtr;
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "Publish buffer %d for topic: ", buffPub);
    writeDebug(tmpMsgD, 4, true);
    writeDebug(buffTop[buffPub], 0, true);
    sprintf(tmpMsgD, " :: %d ", mqttEventCtr);
    writeDebug(tmpMsgD);
    writeDebug(buffMsg[buffPub]);
  #endif
  buffPub++;
  if (buffPub > BUFFER_SIZE) {
    buffPub = 0;
  }
}

#ifdef MQTT_DISCOVERY
/** **********************************************************************************************
  Reset Discovery
*********************************************************************************************** */
void mqttReDiscover () {
  discovered = false;
  discInst = 0;
  discActn = 0;
}

/** **********************************************************************************************
  Publish auto discovery message
  https://www.home-assistant.io/integrations/device_trigger.mqtt/#configuration-variables
*********************************************************************************************** */
void mqttDiscovery () {
  if (discovered) return;
  // Check network bussy status
  if (!mqttStatus || transitBussy || mqttBusy0 > 0) return;
  // Finished!
  if (discInst > NUM_COVERS) {
    discovered = true;
    return;
  }
  //////////////////////////////////////////
  // Send discovery packet for device
  //////////////////////////////////////////
  if (discInst == 0) {
    char tmpMsgS[101];
    char tmpMsgN[471];
    sprintf(tmpMsgS, "homeassistant/%s/%s/config", INST_TYPE, dconfig.myId);
    sprintf(tmpMsgN, "{\"name\":\"%s\",\"uniq_id\":\"%s\",\"cmd_t\":\"%s\",\"stat_t\":\"%s\",\"opt\":false,\"dev\":{\"ids\":[\"X%s\"],\"name\":\"%s\",\"mf\":\"JAE\",\"mdl\":\"RXm%s\",\"sw\":\"%s\"}}", dconfig.myId, deviceId, topicSet, topicState, deviceId, dconfig.myId, HVERSION, SVERSION);
    ++mqttEventCtr;
    mqttTransit(mqttClient.publish(tmpMsgS, MQTT_PUB_QOS, true, tmpMsgN));
    #ifdef DEBUG_MODE
      writeDebug("Discovery on topic: ", 4, true);
      writeDebug(tmpMsgS, 0, true);
      writeDebug(" ", 0, true);
      writeDebug(tmpMsgN);
    #endif
    ++discInst;
    return;
  }
  //////////////////////////////////////////
  // Send discovery packets for covers
  //////////////////////////////////////////
  // Find the next cover
  int idx = discInst - 1;
  while (covers[idx].feedb == 0 && discInst <= NUM_COVERS) {
    ++discInst;
    // Check if we hit the end of Covers registry
    if (discInst > NUM_COVERS) {
      discovered = true;
      return;
    }
  }
  mqttDiscoverCover(idx);
  // Increment discovery counters
  ++discInst;
}

/** **********************************************************************************************
  Publish auto discovery message for a Cover
*********************************************************************************************** */
void mqttDiscoverCover (int idx) {
  // Define variables
  char tmpMsgS[101];
  char tmpMsgN[471];
  char topic[TOPIC_SIZE];
  int pos = topicCoverPref;
  // Send discovery message on the COVER config topic
  sprintf(tmpMsgS, "homeassistant/cover/%s/config", covers[idx].name);
  // Append the name to the cover topic prefix
  strlcpy(topic, topicCover, sizeof(topic));
  for (int i = 0; i < covers[idx].len; i++) {
    topic[pos++] = covers[idx].name[i];
  }
  topic[pos++] = 47;                            // "/"
  topic[pos] = 0;
  // Create the cover discovery packets
  if (covers[idx].feedb > 1) {
    sprintf(tmpMsgN, "{\"name\":\"%s\",\"uniq_id\":\"_%s\",\"dev_cla\":\"blind\",\"cmd_t\":\"%sc\",\"pos_t\":\"%sp\",\"set_pos_t\":\"%sq\",\"avty_t\":\"%sa\",\"dev\":{\"ids\":[\"X%s\"],\"name\":\"%s\",\"mf\":\"JAE\",\"mdl\":\"RXm%s\",\"sw\":\"%s\"}}", covers[idx].name, covers[idx].name, topic, topic, topic, topic, deviceId, dconfig.myId, HVERSION, SVERSION);
  } else {
    sprintf(tmpMsgN, "{\"name\":\"%s\",\"uniq_id\":\"_%s\",\"dev_cla\":\"blind\",\"stat_t\":\"%ss\",\"cmd_t\":\"%sc\",\"avty_t\":\"%sa\",\"dev\":{\"ids\":[\"X%s\"],\"name\":\"%s\",\"mf\":\"JAE\",\"mdl\":\"RXm%s\",\"sw\":\"%s\"}}", covers[idx].name, covers[idx].name, topic, topic, topic, deviceId, dconfig.myId, HVERSION, SVERSION);
  }
  // Send discovery message
  mqttTransit(mqttClient.publish(tmpMsgS, MQTT_PUB_QOS, true, tmpMsgN));
  #ifdef DEBUG_MODE
    writeDebug("Discover on topic: ", 4, true);
    writeDebug(tmpMsgS, 0, true);
    writeDebug(" ", 0, true);
    writeDebug(tmpMsgN);
  #endif
}
#endif

/** **********************************************************************************************
  MQTT Message received

  Decode MQTT messages obtained from the subscription channels
  
  @param topic Topic the message was received on
  @param payload Message contents
  @param properties Message properties
  @param len Size of the message segment
  @param index Index of the segment
  @param total Total message size
*********************************************************************************************** */
void mqttMessageCallback (char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  #ifdef DEBUG_MODE
     char tmpMsgL[151];
  #endif
  char lclMsg[201];
  // Set a limit to the message length
  if (len > MQTT_MAX_MSG_LEN) {
    #ifdef DEBUG_MODE
      sprintf(tmpMsgL, "Incoming MQTT message: too long: %d", len);
      writeDebug(tmpMsgL, 4);
    #endif
    return;
  }
  // Set termination character to payload
  payload[len] = 0;
  #ifdef DEBUG_MODE
    sprintf(tmpMsgL, "Incoming MQTT message: %s, %d, %d, %d", topic, len, index, total);
    writeDebug(tmpMsgL, 4);
    writeDebug(payload, 4);
  #endif
  //////////////////////////////////////////////////////////////////////////////////////
  // Basic validations
  //////////////////////////////////////////////////////////////////////////////////////
  if (len != total) {
    #ifdef DEBUG_MODE
      writeDebug("Ignore MQTT segmented message", 4);
    #endif
    return;
  }
  bool gotJson = true;
  DynamicJsonDocument json(750);
  if (payload[0] == '{') {
    auto error = deserializeJson(json, payload);
    if (error) {
      #ifdef DEBUG_MODE
        writeDebug("Invalid JSON Structure", 4);
      #endif
      return;
    }
    #ifdef DEBUG_MODE
      writeDebug("JSON payload parsed", 4);
    #endif
  } else {
    gotJson = false;
    payload[len] = 0;
    #ifdef DEBUG_MODE
      writeDebug("Got plain message", 4);
    #endif
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Set topic for the device
  //////////////////////////////////////////////////////////////////////////////////////
  if (strcmp(topic, topicSet) == 0) {
    // Discard retained messages
    if (properties.retain) {
      #ifdef DEBUG_MODE
        writeDebug("Device SET topics do not accept retained messages", 4);
      #endif
      return;
    }
    if (!gotJson) {
      if (len > 5) return;
      // Check for state change request..
      if (strcmp(payload, "ON") == 0) {
        // Start capture mode
        rfReceiverStart();
      }
      else if (strcmp(payload, "OFF") == 0) {
        // End capture mode
        rfReceiverClear();
      }
      return;
    }
    // No ACTION requested...
    if (!json.containsKey("action") || !json["action"].is<char*>()) {
      #ifdef DEBUG_MODE
        writeDebug("Invalid configuration message; No action defined", 4);
      #endif
      return;
    }
    // Return system information
    if (strcmp(json["action"], "getInfo") == 0) {
      //const char* ipadd1 = WiFi.localIP().toString().c_str();
      IPAddress dip = WiFi.localIP();
      unsigned long act = aliveCounter;
      #ifdef COLLECT_STATS
        sprintf(lclMsg, "{\"dev\":\"%s\",\"id\":\"%s\",\"ip\":\"%d.%d.%d.%d\",\"al\":%ld,\"rc\":%d,\"cc\":%d,\"boot\":%d,\"rst\":%d}", TITLE, dconfig.myId, dip[0], dip[1], dip[2], dip[3], act, wifiRstCtr, wifiConnCtr, statBoot, statRst);
      #else
        sprintf(lclMsg, "{\"dev\":\"%s\",\"id\":\"%s\",\"ip\":\"%d.%d.%d.%d\",\"al\":%ld,\"rc\":%d,\"cc\":%d}", TITLE, dconfig.myId, dip[0], dip[1], dip[2], dip[3], act, wifiRstCtr, wifiConnCtr);
      #endif
      mqttPublish(0, 0, lclMsg, true);
      #ifdef COLLECT_STATS
        sprintf(lclMsg, "  Loop=%d; Btns=%d (%d); Ovf=%d; NoNet=%d; WiFiRst=%d;  WiFiFail=%d; WiFiReconn=%d; TX=%d; RX=%d; Cap=%d; Stt=%d; Stp=%d", statLoopPS, statBtnCnt, statBtnDet, statTmrOvf, statNoNet, statWifiRstTtl, wifiRstCtr, wifiConnCtr, statTxPks, statRxPks, statRxCap, statRxStart, statRxStop);
        mqttPublish(0, 0, lclMsg, true);
        if (json.containsKey("verbose")) {
          for (int i = 0; i < STATS_NUM; i++) {
            sprintf(lclMsg, "  (%d) LoopPS=%d; Btns=%d; Ovf=%d; NoNet=%d; WiFiRst=%d;  WiFiFail=%d; WiFiReconn=%d; Alive=%d", i, sStats[i].loopPS, sStats[i].btnCnt, sStats[i].tmrOvf, sStats[i].noNet, sStats[i].wifiRstTtl, sStats[i].wifiRstCtr, sStats[i].wifiConnCtr, sStats[i].aliveCounter);
            mqttPublish(0, 0, lclMsg, true);
          }
        }
      #endif
      return;
    }
    // Return Remotes information
    if (strcmp(json["action"], "getRemotes") == 0) {
      for (int i = 0; i < NUM_REMOTES; i++) {
        if (remotes[i].id == 0) continue;
        sprintf(lclMsg, "  (%d) Name=%s; ID=%d", i, remotes[i].name, remotes[i].id);
        mqttPublish(0, 0, lclMsg, true);
      }
      return;
    }
    // Return Covers information
    if (strcmp(json["action"], "getCovers") == 0) {
      for (int i = 0; i < NUM_COVERS; i++) {
        if (covers[i].feedb == 0) continue;
        sprintf(lclMsg, "  (%d) Name=%s; Rem=%d; Chan=%d; Feedb=%d; oTime=%d; cTime=%d; mPos=%d", i, covers[i].name, covers[i].remote, covers[i].chann, covers[i].feedb, covers[i].oTime, covers[i].cTime, covers[i].mpos);
        mqttPublish(0, 0, lclMsg, true);
      }
      return;
    }
    // Save the last remote captured
    if (strcmp(json["action"], "saveRemote") == 0) {
      if (!json.containsKey("name") || !json["name"].is<char*>()) {
        #ifdef DEBUG_MODE
          writeDebug("Name not set for the new remote", 4);
        #endif
        tmpMsg2[0] = 0; // ""
      } else {
        strlcpy(tmpMsg2, json["name"], sizeof(tmpMsg2));
      }
      saveCapturedRemote(tmpMsg2);
      return;
    }
    // Save cover
    if (strcmp(json["action"], "saveCover") == 0) {
      if (!json.containsKey("name") || !json["name"].is<char*>()) {
        #ifdef DEBUG_MODE
          writeDebug("Name not indicated for the cover", 4);
        #endif
        tmpMsg3[0] = 0; // ""
        return;
      } else {
        strlcpy(tmpMsg3, json["name"], sizeof(tmpMsg3));
      }
      if (json.containsKey("newName") && json["newName"].is<char*>()) {
        strlcpy(tmpMsg1, json["newName"], sizeof(tmpMsg1));
      } else {
        tmpMsg1[0] = 0; // ""
      }
      if (json.containsKey("remote") && json["remote"].is<char*>()) {
        strlcpy(tmpMsg2, json["remote"], sizeof(tmpMsg2));
      } else {
        tmpMsg2[0] = 0; // ""
      }
      uint8_t chan = 0;
      uint8_t oTime = 0;
      uint8_t cTime = 0;
      uint8_t feedback = 10;
      uint8_t mpos = 0;
      if (json.containsKey("channel") && (json["channel"].is<signed int>() || json["channel"].is<unsigned int>())) {
        chan = json["channel"];
      }
      if (json.containsKey("oTime") && (json["oTime"].is<signed int>() || json["oTime"].is<unsigned int>())) {
        oTime = json["oTime"];
      }
      if (json.containsKey("cTime") && (json["cTime"].is<signed int>() || json["cTime"].is<unsigned int>())) {
        cTime = json["cTime"];
      }
      if (json.containsKey("feedback") && (json["feedback"].is<signed int>() || json["feedback"].is<unsigned int>())) {
        feedback = json["feedback"];
      }
      if (json.containsKey("mpos") && (json["mpos"].is<signed int>() || json["mpos"].is<unsigned int>())) {
        mpos = json["mpos"];
      }
      updateCover(tmpMsg3, tmpMsg1, tmpMsg2, chan, oTime, cTime, feedback, mpos);
      return;
    }
    if (strcmp(json["action"], "getConfig") == 0) {
      #ifndef MQTT_CONFIG
        #ifdef DEBUG_MODE
          writeDebug("Remote configuration disabled", 1);
        #endif
      #else
        // Get the device configuration
        File configFile = LittleFS.open("/config.json", "r");
        if (configFile) {
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          // Publish config string
          mqttPublish(0, 0, buf.get());
          configFile.close();
        }
      #endif
      return;
    }
    if (strcmp(json["action"], "config") == 0) {
      #ifndef MQTT_CONFIG
        #ifdef DEBUG_MODE
          writeDebug("Remote configuration disabled", 1);
        #endif
      #else
        // Configure
        if (!json.containsKey("secret") || !json["secret"].is<char*>()) {
          #ifdef DEBUG_MODE
            writeDebug("Missing key 'secret'", 4);
          #endif
          return;
        }
        if (strcmp(json["secret"], MQTT_SECRET) != 0) {
          #ifdef DEBUG_MODE
            writeDebug("Invalid 'secret'", 4);
          #endif
          return;
        }
        bool configChanged = false;
        if (json.containsKey("keepAlive") && (json["keepAlive"].is<signed int>() || json["keepAlive"].is<unsigned int>())) {
          dconfig.keepAlive = json["keepAlive"];
          configChanged = true;
        }
        if (json.containsKey("myId") && json["myId"].is<char*>()) {
          strlcpy(dconfig.myId, json["myId"], sizeof(dconfig.myId));
          configChanged = true;
          #ifdef MQTT_DISCOVERY
            mqttReDiscover();
          #endif
        }
        if (json.containsKey("mposTime") && (json["mposTime"].is<signed int>() || json["mposTime"].is<unsigned int>())) {
          dconfig.mposTime = json["mposTime"];
          configChanged = true;
        }
        if (json.containsKey("buzzerFrec") && (json["buzzerFrec"].is<signed int>() || json["buzzerFrec"].is<unsigned int>())) {
          dconfig.buzzerFrec = json["buzzerFrec"];
          configChanged = true;
          setBuzzerFrequency(dconfig.buzzerFrec);
        }
        // Clear registry
        if (json.containsKey("clearMem") && json["clearMem"].is<char*>()) {
          if (strcmp(json["clearMem"], "remotes") == 0) {
            for (int i = 0; i < NUM_REMOTES; i++) {
              remotes[i].id = 0;
              remotes[i].name[0] = 0;
            }
            saveRemoteData();
          } else
          if (strcmp(json["clearMem"], "covers") == 0) {
            for (int i = 0; i < NUM_COVERS; i++) {
              covers[i].feedb = 0;
              covers[i].name[0] = 0;
            }
            saveCoversData();
            // Alert user of Saved Data
            buzzerOn(BEEP_PERIOD_AKN, BEEP_NUM);
          }
        }
        /* --------------------------------------------------------------------------------------------  */
        // Save configuration changes
        if (configChanged) {
          saveConfig();
        }
        #ifdef DEBUG_MODE
          else {
            writeDebug("No changes in the configuration", 1);
          }
        #endif
      #endif
      return;
    }
    #ifdef MQTT_RESET
      // Reset the Device
      if (strcmp(json["action"], "reset") == 0) {
        mqttPublish(0, 0, MQTT_MSG_RESET);
        #ifdef DEBUG_MODE
          writeDebug("Reset requested", 1);
        #endif
        #ifdef COLLECT_STATS
          // Save system stats
          bool ss = saveStatus(false);
          ss = false;
        #endif
        ESP.restart();
        return;
      }
    #endif
    #if defined(COLLECT_STATS) && defined(CLEAR_STATS)
      // Clear status from EEPROM
      if (strcmp(json["action"], "clear_stats") == 0) {
        clearStatus();
        return;
      }
    #endif
    #ifdef DEBUG_MODE
      writeDebug("Invalid action", 4);
    #endif
    return;
  }
  //////////////////////////////////////////////////////////////////////////////////////
  // Cover topics
  //////////////////////////////////////////////////////////////////////////////////////
  // Check topic length (prefix + name + "/x)"
  int tlen = strlen(topic);
  if (tlen < (topicCoverPref + 3) || tlen > (topicCoverPref + 42)) {
    #ifdef DEBUG_MODE
      writeDebug("Invalid cover topic length: ", 4, true);
      writeDebug(topic);
    #endif
    return;
  }
  // Compare topic prefix portion
  int i;
  for (i = 0; i < topicCoverPref; i++) {
    if (topicCover[i] != topic[i]) {
      #ifdef DEBUG_MODE
        writeDebug("Invalid cover topic: ", 4, true);
        writeDebug(topic);
      #endif
      return;
    }
  }
  // Get the topic type from the last letter
  bool actnPos = false;
  char tLtr = topic[(tlen - 1)];
  if (tLtr == 113) {
    actnPos = true;
  } else if (tLtr != 99) {
    #ifdef DEBUG_MODE
      writeDebug("Invalid cover topic type", 4, true);
      writeDebug(topic);
    #endif
    return;
  }
  // Copy name portion of the topic
  int j = 0;
  for (i = topicCoverPref; i < (tlen - 2); i++) {
    tmpMsg1[j++] = topic[i];
  }
  tmpMsg1[j] = 0;
  // Search for matching Cover name
  for (i = 0; i < NUM_COVERS; i++) {
    if (covers[i].feedb == 0) continue;
    if (strcmp(covers[i].name, tmpMsg1) == 0) {
      // Process the position
      if (actnPos) {
        // Validate the correct position type in the cover
        if (covers[i].feedb < 2) {
          #ifdef DEBUG_MODE
            writeDebug("Invalid topic", 4, true);
            writeDebug(topic);
          #endif
          return;
        }
        // Process position value
        uint8_t val = atoi(payload);
        if (val < 0) val = 0;
        else if (val > 100) val = 100;
        // Send action to the cover
        if (covers[i].feedb == 2) {
          rfMove(i, val);
        //} else {
          // TBD
        }
        // Inform
        char myPos[11];
        if (val < 2) val = 2;
        else if (val > 98) val = 98;
        itoa(val, myPos, 10);
        mqttPublish(2, i, myPos, true);
      }
      // Process the action
      else {
        char myPos[11];
        if (strcmp(payload, "OPEN") == 0) {
          rfSendCommand(i, ACTION_OPEN, false, 0);
          // Inform
          itoa(98, myPos, 10);
          mqttPublish(2, i, myPos, true);
        } else if (strcmp(payload, "CLOSE") == 0) {
          rfSendCommand(i, ACTION_CLOSE, false, 0);
          // Inform
          itoa(2, myPos, 10);
          mqttPublish(2, i, myPos, true);
        } else if (strcmp(payload, "STOP") == 0) {
          rfSendCommand(i, ACTION_STOP, false, 0);
        }
      }
      //break;
      return;
    }
  }
  #ifdef DEBUG_MODE
    writeDebug("Invalid cover", 4, true);
    writeDebug(topic);
  #endif
}

/** **********************************************************************************************
  Check if EEPROM has been initialized
  
  @return Returns TRUE if EEPROM has been initialized
*********************************************************************************************** */
bool isEepromInit () {
  int vale;
  int addr = EADDR_INIT;
  addr += EEPROM_readAnything(addr, vale);
  if (vale == E_VAL_INIT) {
    #ifdef DEBUG_MODE
      writeDebug("EEPROM has been initialized", 1);
    #endif
    return true; 
  }
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "EEPROM is NOT initialized = %ld / %d", vale, addr);
    writeDebug(tmpMsgD, 1);
  #endif
  return false;
}

/** **********************************************************************************************
  Initialize EEPROM
*********************************************************************************************** */
void setEepromInit () {
  int vale = E_VAL_INIT;
  int addr = EADDR_INIT;
  // Save values 
  addr += EEPROM_writeAnything(addr, vale);
  #ifdef DEBUG_MODE
    if (EEPROM.commit()) {
      #ifdef DEBUG_MODE
        sprintf(tmpMsgD, "EEPROM initialized = %ld / %d", vale, addr);
        writeDebug(tmpMsgD, 1);
      #endif
      //writeDebug("EEPROM initialized", 1);
    } else {
      writeDebug("Failed initializing EEPROM", 1);
    }
  #else
    EEPROM.commit();
  #endif
}

/** **********************************************************************************************
  Callback notifying that we need to save WiFi Manager config
*********************************************************************************************** */
void saveConfigCallback () {
  shouldSaveConfig = true;
  #ifdef DEBUG_MODE
    writeDebug("Should save config", 1);
  #endif
}

/** **********************************************************************************************
  Set default values for configuration parameters

  If you set additional parameters in the WiFiManager, define initial values here
*********************************************************************************************** */
void configDefault () {
  dconfig.keepAlive = CONFIG_KEEPALIVE;
  dconfig.mqttport = CONFIG_MQTT_PORT;
  strcpy(dconfig.mqttbroker, CONFIG_MQTT_BROKER);
  strcpy(dconfig.mqttuser, CONFIG_MQTT_USER);
  strcpy(dconfig.mqttpass, CONFIG_MQTT_PASS);
  strcpy(dconfig.namespc, CONFIG_MQTT_NAMESPACE);
  strcpy(dconfig.mqttkey, CONFIG_MQTT_KEY);
  sprintf(tmpMsg0, "RCovers_%s", deviceId);
  strcpy(dconfig.myId, tmpMsg0);
  dconfig.mposTime = CONFIG_MPOS_TIME;
  dconfig.buzzerFrec = CONFIG_BUZZER_FREC;
}

/** **********************************************************************************************
   Load the configuration
*********************************************************************************************** */
void loadConfig () {
  // Read configuration from FS json
  #ifdef DEBUG_MODE
    writeDebug("Mounting FS...", 1);
  #endif
  mounted = LittleFS.begin();
  if (!mounted) {
    #ifdef DEBUG_MODE
      writeDebug("Failed to mount FS", 1);
    #endif
    return;
  }
  #ifdef DEBUG_MODE
    writeDebug("Mounted file system", 1);
  #endif
  // Check Config file exists
  if (!LittleFS.exists("/config.json")) {
    #ifdef DEBUG_MODE
      writeDebug("Config file does not exist, create a new one...", 1);
    #endif
    saveConfig();
    return;
  }
  // File exists, read and load configuration
  #ifdef DEBUG_MODE
    writeDebug("Reading Config file", 1);
  #endif
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    #ifdef DEBUG_MODE
    writeDebug("Failed to open configuration file", 1);
    #endif
    return;
  }
  // Allocate a buffer to store contents of the Config file.
  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  // Read the file contents
  configFile.readBytes(buf.get(), size);
  // Create the JSON document and parse Config file data
  DynamicJsonDocument json(800);
  auto error = deserializeJson(json, buf.get());
  if (error) {
    #ifdef DEBUG_MODE
      writeDebug("Failed to parse configuration file", 1);
    #endif
    return;
  }
  #ifdef DEBUG_MODE
    writeDebug("Parsed config file", 1);
  #endif
  // Move JSON data into the Config variable, preserve defaults if a parameter is missing
  dconfig.mqttport = json["mqttport"] | dconfig.mqttport;
  strlcpy(dconfig.mqttbroker, json["mqttbroker"] | dconfig.mqttbroker, sizeof(dconfig.mqttbroker));
  strlcpy(dconfig.mqttuser, json["mqttuser"] | dconfig.mqttuser, sizeof(dconfig.mqttuser));
  strlcpy(dconfig.mqttpass, json["mqttpass"] | dconfig.mqttpass, sizeof(dconfig.mqttpass));
  strlcpy(dconfig.namespc, json["namespc"] | dconfig.namespc, sizeof(dconfig.namespc));
  strlcpy(dconfig.mqttkey, json["mqttkey"] | dconfig.mqttkey, sizeof(dconfig.mqttkey));
  dconfig.keepAlive = json["keepAlive"] | dconfig.keepAlive;
  strlcpy(dconfig.myId, json["myId"] | dconfig.myId, sizeof(dconfig.myId));
  if (strlen(dconfig.myId) == 0) {
    strlcpy(dconfig.myId, deviceId, sizeof(dconfig.myId));
  }
  dconfig.mposTime = json["mposTime"] | dconfig.mposTime;
  dconfig.buzzerFrec = json["buzzerFrec"] | dconfig.buzzerFrec;
  // Close file
  configFile.close();
  // Inform
  #ifdef DEBUG_MODE
    serializeJson(json, Serial);
    writeDebug(" ");
  #endif
  setBuzzerFrequency(dconfig.buzzerFrec);
}

/** **********************************************************************************************
  Save the configuration
*********************************************************************************************** */
void saveConfig () {
  // Check filesystem
  if (!mounted) {
    #ifdef DEBUG_MODE
      writeDebug("FS not mounted", 1);
    #endif
    return;
  }
  #ifdef DEBUG_MODE
    writeDebug("Saving config", 1);
  #endif
  // Create JSON document
  DynamicJsonDocument json(800);
  // Move Config data into JSON object
  json["mqttport"] = dconfig.mqttport;
  json["mqttbroker"] = dconfig.mqttbroker;
  json["mqttuser"] = dconfig.mqttuser;
  json["mqttpass"] = dconfig.mqttpass;
  json["namespc"] = dconfig.namespc;
  json["mqttkey"] = dconfig.mqttkey;
  json["keepAlive"] = dconfig.keepAlive;
  json["myId"] = dconfig.myId;
  json["mposTime"] = dconfig.mposTime;
  json["buzzerFrec"] = dconfig.buzzerFrec;
  // Open file and print JSON Data into it
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    // Inform error
    #ifdef DEBUG_MODE
      writeDebug("Failed to open config file for writing", 1);
    #endif
    mqttPublish(0, 0, MQTT_MSG_SAVE_FAIL);
    return;
  }
  serializeJson(json, configFile);
  // Close file
  configFile.close();
  // Inform
  mqttPublish(0, 0, MQTT_MSG_SAVE);
  #ifdef DEBUG_MODE
    serializeJson(json, Serial);
    writeDebug("");
  #endif
}

/** **********************************************************************************************
  Clear the Remotes/Covers registry
*********************************************************************************************** */
void clearRegistry () {
  int i;
  for (i = 0; i < NUM_REMOTES; i++) {
    remotes[i].id = 0;
  }
  for (i = 0; i < NUM_COVERS; i++) {
    covers[i].feedb = 0;
    covers[i].len = 0;
    covers[i].name[0] = 0;
  }
}

/** **********************************************************************************************
  Save the data of the last Remote captured
  
  @param rName Name for the new remote
*********************************************************************************************** */
void saveCapturedRemote (char* rName) {
  if (!rfRxGotCommand) {
    // Inform
    mqttPublish(0, 0, MQTT_MSG_CAP_NODATA);
    #ifdef DEBUG_MODE
      writeDebug("No capture data available", 3);
    #endif
    return;
  }
  // Validate Name
  if (strlen(rName) == 0) {
      // Inform
      mqttPublish(0, 0, MQTT_MSG_CAP_NONAME);
      #ifdef DEBUG_MODE
        writeDebug("Empty Remote name", 3);
      #endif
      return;
  }
  // Validate repeated ID / Name
  int canSave = -1;
  bool dup1 = false;
  bool dup2 = false;
  char lclMsg[101];
  #ifdef DEBUG_MODE
     char tmpMsgL[101];
  #endif
  for (int i = 0; i < NUM_REMOTES; i++) {
    // Dup ID
    if (!dup1 && (remotes[i].id == rfRxRemId || rfRxRemId == 0)) {
      rfRxGotCommand = false;
      dup1 = true;
      canSave = i;
      // Inform
      mqttPublish(0, 0, MQTT_MSG_CAP_DUP);
      #ifdef DEBUG_MODE
        writeDebug("Duplicated Remote ID", 3);
      #endif
    }
    // Dup name
    if (!dup2 && remotes[i].id != 0 && strcmp(remotes[i].name, rName) == 0) {
      dup2 = true;
      // Inform
      mqttPublish(0, 0, MQTT_MSG_CAP_DUPNAME);
      #ifdef DEBUG_MODE
        writeDebug("Duplicated Remote name", 3);
      #endif
    }
    if (canSave < 0 && remotes[i].id == 0) canSave = i;
  }
  if (dup2) return;
  // Check if there is space to save
  if (canSave < 0) {
    rfRxGotCommand = false;
    mqttPublish(0, 0, MQTT_MSG_CAP_FULL);
    // Inform
    #ifdef DEBUG_MODE
      writeDebug("Can not save Remote. Registry is full", 3);
    #endif
    return;
  }
  // Save the new Remote in the available position
  remotes[canSave].id = rfRxRemId;
  strlcpy(remotes[canSave].name, rName, sizeof(remotes[canSave].name));
  // Inform
  if (dup1) sprintf(lclMsg, "{\"state\":\"update\",\"id\":%d,\"idx\":%d,\"name\":\"%s\"}", rfRxRemId, canSave, rName);
  else sprintf(lclMsg, "{\"state\":\"add\",\"id\":%d,\"idx\":%d,\"name\":\"%s\"}", rfRxRemId, canSave, rName);
  mqttPublish(0, 0, lclMsg, true);
  #ifdef DEBUG_MODE
    if (dup1) sprintf(tmpMsgL, "Change Remote ID=%d (%d) '%s'", rfRxRemId, canSave, rName);
    else sprintf(tmpMsgL, "New Remote ID=%d (%d) '%s'", rfRxRemId, canSave, rName);
    writeDebug(tmpMsgL, 3);
  #endif
  // Save data to File system
  saveRemoteData();
}

/** **********************************************************************************************
  Load the Remotes data
*********************************************************************************************** */
void loadRemotes () {
  if (!mounted) return;
  // Check Remotes file exists
  if (!LittleFS.exists("/remotes.json")) {
    #ifdef DEBUG_MODE
      writeDebug("Remotes file does not exist, create a new one...", 1);
    #endif
    saveRemoteData();
    return;
  }
  // File exists, read and load Remotes
  #ifdef DEBUG_MODE
    writeDebug("Reading Remotes file", 1);
  #endif
  File dataFile = LittleFS.open("/remotes.json", "r");
  if (!dataFile) {
    #ifdef DEBUG_MODE
      writeDebug("Failed to open Remotes file", 1);
    #endif
    return;
  }
  // Allocate a buffer to store contents of the Remotes file.
  size_t size = dataFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  // Read the file contents
  dataFile.readBytes(buf.get(), size);
  // Create the JSON document and parse file data
  DynamicJsonDocument json(1200);
  auto error = deserializeJson(json, buf.get());
  if (error) {
    #ifdef DEBUG_MODE
      writeDebug("Failed to parse Remotes file", 1);
    #endif
    return;
  }
  #ifdef DEBUG_MODE
    writeDebug("Parsed Remotes file", 1);
  #endif
  // Move JSON data into the registry
  uint8_t idx = 0;
  for (uint8_t i = 0; i < NUM_REMOTES; i++) {
    remotes[i].id = json[idx++];
    strlcpy(remotes[i].name, json[idx] | remotes[i].name, sizeof(remotes[i].name));
    idx++;
  }
  // Close file
  dataFile.close();
  // Inform
  #ifdef DEBUG_MODE
    serializeJson(json, Serial);
    writeDebug(" ");
  #endif
}

/** **********************************************************************************************
  Save the Remotes data
*********************************************************************************************** */
void saveRemoteData () {
  if (!mounted) return;
  #ifdef DEBUG_MODE
    writeDebug("Saving Remotes data", 1);
  #endif
  // Create JSON document
  DynamicJsonDocument json(1200);
  // Move registry data into JSON object
  uint8_t idx = 0;
  for (uint8_t i = 0; i < NUM_REMOTES; i++) {
    json[idx++] = remotes[i].id;
    json[idx++] = remotes[i].name;
  }
  // Open file and print JSON Data into it
  File dataFile = LittleFS.open("/remotes.json", "w");
  if (!dataFile) {
    // Inform error
    #ifdef DEBUG_MODE
      writeDebug("Failed to open Remotes file for writing", 1);
    #endif
    mqttPublish(0, 0, MQTT_MSG_SAVE_FAIL);
    return;
  }
  serializeJson(json, dataFile);
  // Close file
  dataFile.close();
  // Alert user of Saved Data
  buzzerOn(BEEP_PERIOD_AKN, BEEP_NUM);
  // Inform
  mqttPublish(0, 0, MQTT_MSG_SAVE_DATA);
  #ifdef DEBUG_MODE
    serializeJson(json, Serial);
    writeDebug("");
  #endif
}

/** **********************************************************************************************
  Update / add a Cover
  
  @param cName Name for the Cover
  @param cNewName New name for the Cover
  @param cRemote Name of the remote
  @param chan Channel
  @param oTime Time to open
  @param cTime Time to close
  @param feedback Feedback type [0=delete cover; 1=none; 2=time; 3=feedback; 10=keep_same]
  @param mpos Micro steps from the closed position
*********************************************************************************************** */
void updateCover (char* cName, char* cNewName, char* cRemote, uint8_t chan, uint8_t oTime, uint8_t cTime, uint8_t feedback, uint8_t mpos) {
  int i;
  // Validate Cover Name
  if (strlen(cName) == 0) {
    // Inform
    mqttPublish(0, 0, MQTT_MSG_COV_NONAME);
    #ifdef DEBUG_MODE
      writeDebug("Empty Cover name", 3);
    #endif
    return;
  }
  // Find the remote ID
  int8_t remIdx = -1;
  for (i = 0; i < NUM_REMOTES; i++) {
    if (strcmp(remotes[i].name, cRemote) == 0) {
      remIdx = i;
      break;
    }
  }
  char lclMsg[151];
  #ifdef DEBUG_MODE
     char tmpMsgL[151];
  #endif
  // Find the cover name
  int canSave = -1;
  bool updt = false;
  for (i = 0; i < NUM_COVERS; i++) {
    if (covers[i].feedb > 0 && strcmp(covers[i].name, cName) == 0) {
      canSave = i;
      updt = true;
      // Inform
      #ifdef DEBUG_MODE
        writeDebug("Overwrite Cover", 3);
      #endif
    }
    if (canSave < 0 && covers[i].feedb == 0) canSave = i;
  }
  // Check if there is space to save
  if (canSave < 0) {
    mqttPublish(0, 0, MQTT_MSG_COV_FULL);
    // Inform
    #ifdef DEBUG_MODE
      writeDebug("Can not save Cover. Registry is full", 3);
    #endif
    return;
  }
  // If the remote ID is empty and not updating
  if (remIdx < 0) {
    if (!updt) {
      // Inform
      mqttPublish(0, 0, MQTT_MSG_COV_NOREM);
      #ifdef DEBUG_MODE
        writeDebug("No Remote found", 3);
      #endif
      return;
    }
  } else {
    covers[canSave].remote = remIdx;
  }
  // Save cover data
  if (chan > 0) covers[canSave].chann = chan;
  if (oTime > 0) covers[canSave].oTime = oTime;
  if (cTime > 0) covers[canSave].cTime = cTime;
  if (feedback < 10) covers[canSave].feedb = feedback;
  if (mpos > 0) covers[canSave].mpos = mpos;
  if (feedback > 0 && strlen(cNewName) > 0) {
    strlcpy(covers[canSave].name, cNewName, sizeof(covers[canSave].name));
  } else {
    strlcpy(covers[canSave].name, cName, sizeof(covers[canSave].name));
  }
  covers[canSave].len = strlen(covers[canSave].name);
  // Inform
  sprintf(lclMsg, "{\"state\":\"add_cover\",\"name\":\"%s\",\"remote\":%d,\"chan\":%d}", covers[canSave].name, remIdx, chan);
  mqttPublish(0, 0, lclMsg, true);
  #ifdef DEBUG_MODE
    sprintf(tmpMsgL, "Cover Name='%s' remote=%d chan=%d oTime=%d cTime=%d feedback=%d mpos=%d) ", covers[canSave].name, remIdx, chan, oTime, cTime, feedback, mpos);
    writeDebug(tmpMsgL, 3);
  #endif
  // Save data to File system
  saveCoversData();
  // Alert user of Saved Data
  buzzerOn(BEEP_PERIOD_AKN, BEEP_NUM);
  #ifdef MQTT_DISCOVERY
    // Send the discovery packet
    mqttDiscoverCover(canSave);
  #endif
  // Subscribe to the cover
  mqttSubscribeCover(canSave);
}

/** **********************************************************************************************
   Load the Covers data
*********************************************************************************************** */
void loadCovers () {
  if (!mounted) return;
  // Check Covers file exists
  if (!LittleFS.exists("/covers.json")) {
    #ifdef DEBUG_MODE
      writeDebug("Covers file does not exist, create a new one...", 1);
    #endif
    saveCoversData();
    return;
  }
  // File exists, read and load Covers
  #ifdef DEBUG_MODE
    writeDebug("Reading Covers file", 1);
  #endif
  File dataFile = LittleFS.open("/covers.json", "r");
  if (!dataFile) {
    #ifdef DEBUG_MODE
    writeDebug("Failed to open Covers file", 1);
    #endif
    return;
  }
  // Allocate a buffer to store contents of the Covers file.
  size_t size = dataFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  // Read the file contents
  dataFile.readBytes(buf.get(), size);
  // Create the JSON document and parse file data
  DynamicJsonDocument json(4000);
  auto error = deserializeJson(json, buf.get());
  if (error) {
    #ifdef DEBUG_MODE
      writeDebug("Failed to parse Covers file", 1);
    #endif
    return;
  }
  #ifdef DEBUG_MODE
    writeDebug("Parsed covers file", 1);
  #endif
  // Move JSON data into the registry
  uint16_t idx = 0;
  for (uint8_t i = 0; i < NUM_COVERS; i++) {
    covers[i].remote = json[idx++];
    covers[i].chann = json[idx++];
    covers[i].oTime = json[idx++];
    covers[i].cTime = json[idx++];
    covers[i].feedb = json[idx++];
    covers[i].mpos = json[idx++];
    strlcpy(covers[i].name, json[idx] | covers[i].name, sizeof(covers[i].name));
    idx++;
    // Calculate name length and define if Cover is active
    covers[i].len = ((covers[i].feedb > 0) ? strlen(covers[i].name) : 0);
    if (covers[i].len == 0) covers[i].feedb = 0;
  }
  for (uint8_t i = 0; i < NUM_COVERS; i++) {
    if (covers[i].feedb == 0) continue;
  }
  // Close file
  dataFile.close();
  #ifdef DEBUG_MODE
    serializeJson(json, Serial);
    writeDebug(" ");
  #endif
}

/** **********************************************************************************************
  Save the Covers data
*********************************************************************************************** */
void saveCoversData () {
  if (!mounted) return;
  #ifdef DEBUG_MODE
    writeDebug("Saving Covers data", 1);
  #endif
  // Create document
  DynamicJsonDocument json(4000);
  // Move registry data into JSON object
  uint16_t idx = 0;
  for (uint8_t i = 0; i < NUM_COVERS; i++) {
    json[idx++] = covers[i].remote;
    json[idx++] = covers[i].chann;
    json[idx++] = covers[i].oTime;
    json[idx++] = covers[i].cTime;
    json[idx++] = covers[i].feedb;
    json[idx++] = covers[i].mpos;
    json[idx++] = covers[i].name;
  }
  // Open file and print JSON Data into it
  File dataFile = LittleFS.open("/covers.json", "w");
  if (!dataFile) {
    #ifdef DEBUG_MODE
      writeDebug("Failed to open Covers file for writing", 1);
    #endif
    mqttPublish(0, 0, MQTT_MSG_SAVE_FAIL);
    return;
  }
  // Write file contents
  serializeJson(json, dataFile);
  // Close file
  dataFile.close();
  // Inform
  mqttPublish(0, 0, MQTT_MSG_SAVE_DATA1);
  #ifdef DEBUG_MODE
    serializeJson(json, Serial);
    writeDebug("");
  #endif
}

#ifdef COLLECT_STATS
/** **********************************************************************************************
  Load status from before reset
*********************************************************************************************** */
void loadLastStatus () {
  uint16_t iv;
  int addr = EADDR_BOOT_INFO;
  #ifdef DEBUG_MODE
    writeDebug("Reading system stats", 1);
  #endif
  addr += EEPROM_readAnything(addr, statBoot);
  addr += EEPROM_readAnything(addr, statRst);
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "System Stats: Boot=%d; Reset=%d", statBoot, statRst);
    writeDebug(tmpMsg, 1);
  #endif
  for (int i = 0; i < STATS_NUM; i++) {
    addr += EEPROM_readAnything(addr, iv);
    sStats[i].loopPS = iv;
    addr += EEPROM_readAnything(addr, sStats[i].btnCnt);
    addr += EEPROM_readAnything(addr, sStats[i].tmrOvf);
    addr += EEPROM_readAnything(addr, sStats[i].noNet);
    addr += EEPROM_readAnything(addr, sStats[i].wifiRstTtl);
    addr += EEPROM_readAnything(addr, sStats[i].wifiRstCtr);
    addr += EEPROM_readAnything(addr, sStats[i].wifiConnCtr);
    addr += EEPROM_readAnything(addr, sStats[i].aliveCounter);
    #ifdef DEBUG_MODE
      sprintf(tmpMsgD, "  (%d) LoopPS=%d; Btns=%d; Ovf=%d; NoNet=%d; WiFiRst=%d;  WiFiFail=%d; WiFiReconn=%d; Alive=%d", i, sStats[i].loopPS, sStats[i].btnCnt, sStats[i].tmrOvf, sStats[i].noNet, sStats[i].wifiRstTtl, sStats[i].wifiRstCtr, sStats[i].wifiConnCtr, sStats[i].aliveCounter);
      writeDebug(tmpMsg, 1);
    #endif
  }
  // Increase Boot Count
  if (statBoot < 0) {
    statBoot = 0;
  }
  statBoot += 1;
  // Save Boot Count
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "Save Boot counter: %d", statBoot);
    writeDebug(tmpMsgD, 1);
  #endif
  addr = EADDR_BOOT_INFO;
  EEPROM_writeAnything(addr, statBoot);
  #ifdef DEBUG_MODE
    if (EEPROM.commit()) {
      writeDebug("Boot counter saved", 1);
    } else {
      writeDebug("Failed saving boot counter", 1);
    }
  #else
    EEPROM.commit();
  #endif
}

/** **********************************************************************************************
  Save system stats
   
  @param forced Forced reset
  @return Returns TRUE if saved successfully
*********************************************************************************************** */
bool saveStatus (bool forced) {
  int addr = EADDR_BOOT_INFO;
  // Increase the reboot counter
  if (forced) statRst += 1;
  // Save values 
  addr += EEPROM_writeAnything(addr, statBoot);
  addr += EEPROM_writeAnything(addr, statRst);
  addr += EEPROM_writeAnything(addr, statLoopPS);
  addr += EEPROM_writeAnything(addr, statBtnCnt);
  addr += EEPROM_writeAnything(addr, statTmrOvf);
  addr += EEPROM_writeAnything(addr, statNoNet);
  addr += EEPROM_writeAnything(addr, statWifiRstTtl);
  addr += EEPROM_writeAnything(addr, wifiRstCtr);
  addr += EEPROM_writeAnything(addr, wifiConnCtr);
  addr += EEPROM_writeAnything(addr, aliveCounter);
  for (int i = 0; i < (STATS_NUM - 1); i++) {
    addr += EEPROM_writeAnything(addr, sStats[i].loopPS);
    addr += EEPROM_writeAnything(addr, sStats[i].btnCnt);
    addr += EEPROM_writeAnything(addr, sStats[i].tmrOvf);
    addr += EEPROM_writeAnything(addr, sStats[i].noNet);
    addr += EEPROM_writeAnything(addr, sStats[i].wifiRstTtl);
    addr += EEPROM_writeAnything(addr, sStats[i].wifiRstCtr);
    addr += EEPROM_writeAnything(addr, sStats[i].wifiConnCtr);
    addr += EEPROM_writeAnything(addr, sStats[i].aliveCounter);
  }
  bool ret = EEPROM.commit();
  #ifdef DEBUG_MODE
    if (ret) {
      writeDebug("System stats saved", 1);
    } else {
      writeDebug("Failed saving System stats", 1);
    }
  #endif
  return ret;
}

#ifdef CLEAR_STATS
/** **********************************************************************************************
  Clear saved status
*********************************************************************************************** */
void clearStatus () {
  int addr = EADDR_BOOT_INFO;
  #ifdef DEBUG_MODE
    writeDebug("Clear EEPROM", 1);
  #endif
  statBoot = 0;
  statRst = 0;
  statLoopPS = 0;
  statBtnCnt = 0;
  statTmrOvf = 0;
  statWifiRstTtl = 0;
  for (int i = 0; i < STATS_NUM; i++) {
    sStats[i].loopPS = 0;
    sStats[i].btnCnt = 0;
    sStats[i].tmrOvf = 0;
    sStats[i].noNet = 0;
    sStats[i].wifiRstTtl = 0;
    sStats[i].wifiRstCtr = 0;
    sStats[i].wifiConnCtr = 0;
    sStats[i].aliveCounter = 0;
  }
  if (saveStatus(false)) {
    #ifdef DEBUG_MODE
      writeDebug("Cleared system stats", 1);
    #endif
    mqttPublish(0, 0, MQTT_MSG_SC);
    return;
  }
  #ifdef DEBUG_MODE
    writeDebug("Failed deleting system stats", 1);
  #endif
  mqttPublish(0, 0, MQTT_MSG_SNC);
}
#endif
#endif

/** **********************************************************************************************
  Write a value to EEPROM

  @param ee EEPROM position [1 - 512]
  @param value Value variable
  @return Returns the number of bytes written to EEPROM
*********************************************************************************************** */
template <class T> int EEPROM_writeAnything (int ee, const T& value) {
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++) {
    /*
    #ifdef DEBUG_MODE
      sprintf(tmpMsg, "EEPROM WRITE: %d @ %d", *p, ee);
      writeDebug(tmpMsg, 1);
    #endif
    */
    EEPROM.write(ee++, *p++);
  }
  return i;
}

/** **********************************************************************************************
  Read a value from EEPROM

  @param ee MySensors EEPROM position [1 - 255]
  @param value Value variable
  @return Returns the number of bytes read from EEPROM
*********************************************************************************************** */
template <class T> int EEPROM_readAnything (int ee, T& value) {
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++) {
    *p++ = EEPROM.read(ee++);
    /*
    #ifdef DEBUG_MODE
      sprintf(tmpMsg, "EEPROM read: %d @ %d", *p, (ee - 1));
      writeDebug(tmpMsg, 1);
    #endif
    */
  }
  return i;
}

/** **********************************************************************************************
  Initialize button variables
   
  @param button Button Index
  @param pinId Hardware Pin
  @param eventState Active state
  @param features Button features
    - 1 = Long Press
    - 2 = Repeat (Requires "Long Press")
    - 4 = Multiple click
  @param repInterval Repeat interval (ms)
*********************************************************************************************** */
void initButton (int button, int pinId, int eventState, int features, int repInterval) {
  myButtons[button].pinId = pinId;
  myButtons[button].eventState = eventState;
  myButtons[button].features = features;
  myButtons[button].repInterval = repInterval;
  myButtons[button].state = !eventState;
  myButtons[button].lastState = !eventState;
}

/** **********************************************************************************************
   Get the status of a button

  @param btnIndex Button Index
  @return Returns:
    - 0 = No action
    - 1 = Button Down
    - 2 = Button Up
    - 3 = Button Up after long press
    - 4 = Button Up after repeat
    - 5 = Repeat
    - 6 = Double click
    - 7 = Triple click
    - 8 = Multi click
*********************************************************************************************** */
int getButtonStatus (int btnIndex) {
  // Init timer
  unsigned long cTmr = millis();
  // Read the state of the button
  int reading = digitalRead(myButtons[btnIndex].pinId);
  // State changes reset the debounce timer
  if (reading != myButtons[btnIndex].lastState) {
    myButtons[btnIndex].btnTimer = cTmr;
    myButtons[btnIndex].lastState = reading;
  }
  #ifdef COLLECT_STATS
    ++statBtnDet;
  #endif
  // Button sate changed
  if (reading != myButtons[btnIndex].state) {
    // Skip the rest of checking during debouncing
    if ((unsigned long)(cTmr - myButtons[btnIndex].btnTimer) < BTN_DEBOUNCE) {
      return 0;
    }
    // Whatever the reading is at, it's been there for longer than the debounce
    //  delay, so change the actual state
    myButtons[btnIndex].state = reading;
    // Button DOWN...
    if (myButtons[btnIndex].state == myButtons[btnIndex].eventState) {
      // ... Reset the repetition flag and timer
      myButtons[btnIndex].lngFlag = false;
      myButtons[btnIndex].repTimer = cTmr;
      return 1;
    }
    // Button UP...
    //  Long press/repeat enabled
    if (myButtons[btnIndex].lngFlag) {
      myButtons[btnIndex].clkCtr = 0;
    // ... Repeat or Long press
      if ((myButtons[btnIndex].features & 2) > 0) return 4;
      return 3;
    }
    // If double/multi click is enabled...
    if ((myButtons[btnIndex].features & 12) > 0) {
      // ... count the number of "clicks"
      myButtons[btnIndex].clkCtr++;
      myButtons[btnIndex].dblTimer = cTmr;
      return 0;
    }
    // Reset count and return "button up" value
    myButtons[btnIndex].clkCtr = 0;
    return 2;
  }
  // Button is down
  if (myButtons[btnIndex].state == myButtons[btnIndex].eventState) {
    if (myButtons[btnIndex].lngFlag) {
      if ((myButtons[btnIndex].features & 2) > 0 && (unsigned long)(cTmr - myButtons[btnIndex].repTimer) > myButtons[btnIndex].repInterval) {
        myButtons[btnIndex].repTimer = cTmr;
        return 5;
      }
    }
    else if ((myButtons[btnIndex].features & 1) > 0) {
      if ((unsigned long)(cTmr - myButtons[btnIndex].repTimer) > BTN_LONG_PRESS) {
        myButtons[btnIndex].repTimer = cTmr;
        // Set the repetition flag...
        myButtons[btnIndex].lngFlag = true;
        // If repetition is on, start repeat cycle
        if ((myButtons[btnIndex].features & 2) > 0) return 5;
      }
    }
    return 0;
  }
  // Button is up
  if ((myButtons[btnIndex].features & 4) > 0 && myButtons[btnIndex].clkCtr > 0) {
    // Check if the double click timer has expired
    if ((unsigned long)(cTmr - myButtons[btnIndex].dblTimer) < DBL_CLICK_INTERVAL) return 0;
    int cntr = myButtons[btnIndex].clkCtr;
    myButtons[btnIndex].clkCtr = 0;
    // Multi-click
    if (cntr >= MULT_CLICK_COUNT) {
      return 8;
    }
    // Triple-click
    else if (cntr >= 3) {
      return 7;
    }
    // Double-click
    else if (cntr >= 2) {
      return 6;
    }
    // Button UP
    return 2;
  }
  return 0;
}

/** **********************************************************************************************
  Go to setup mode by simulating a double reset
*********************************************************************************************** */
void goToSetupMode () {
  #ifdef DEBUG_MODE
    writeDebug("Multi-click detected. Boot in setup mode...", 1);
  #endif
  // Boot in MULT_CLICK_COUNT
  uint32_t data = 0xAAAAAAAA;
  ESP.rtcUserMemoryWrite(DRD_ADDRESS, &data, sizeof(data));
  ESP.reset();
}

#ifdef OTA_HTTP
/** **********************************************************************************************
  HTTP Update started
*********************************************************************************************** */
void http_update_started () {
  #ifdef DEBUG_MODE
    writeDebug("HTTP update process started", 1);
  #endif
}

/** **********************************************************************************************
  HTTP Update finished
*********************************************************************************************** */
void http_update_finished () {
  #ifdef DEBUG_MODE
    writeDebug("HTTP update process finished", 1);
  #endif
}

/** **********************************************************************************************
  HTTP Update progress
*********************************************************************************************** */
void http_update_progress (int cur, int total) {
  yield();
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "HTTP update process at %d of %d bytes...", cur, total);
    writeDebug(tmpMsg, 1);
  #endif
}

/** **********************************************************************************************
  HTTP Update error
*********************************************************************************************** */
void http_update_error (int err) {
  #ifdef DEBUG_MODE
    sprintf(tmpMsgD, "HTTP update fatal error code %d", err);
    writeDebug(tmpMsg, 1);
  #endif
}
#endif

/** **********************************************************************************************
  Setup function
*********************************************************************************************** */
void setup () {

  /* ------------------------------------------------------------------------------
     Service variables
     ------------------------------------------------------------------------------ */
  int i;
  
  /* ------------------------------------------------------------------------------
     Hardware configuration
     ------------------------------------------------------------------------------ */
  // Define pins
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_RF_TX, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_RF_RX, INPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  delay(10);
  
  /* ------------------------------------------------------------------------------
     Enable serial port
     ------------------------------------------------------------------------------ */
  Serial.begin(SERIAL_SPEED);
  delay(200);
  
  /* ------------------------------------------------------------------------------
     Define a Unique ID for the device (MAC Address)
     ------------------------------------------------------------------------------ */
  setDeviceId();

  Serial.println("////////////////////////////////////////////////////////////");
  Serial.println("//");
  Serial.print("//   ");
  Serial.println(TITLE);
  Serial.print("//   Type: ");
  Serial.println(INST_TYPE);
  Serial.print("//   Hardware: ");
  Serial.println(HVERSION);
  Serial.print("//   Device ID: ");
  Serial.println(deviceId);
  Serial.println("//");
  Serial.println("////////////////////////////////////////////////////////////");

  /* ------------------------------------------------------------------------------
     Read device configuration
     ------------------------------------------------------------------------------ */
  // Clean FS - Uncomment to re-format during testing
  //LittleFS.format();

  // Clear the devices registry
  clearRegistry();

  // Set default values for configuration
  configDefault();

  // Load configuration
  loadConfig();

  /* ------------------------------------------------------------------------------
     Enter Setup Portal on double reset
     ------------------------------------------------------------------------------ */
  if (drd.detect()) {
    #ifdef DEBUG_MODE
      writeDebug("Double reset detected. Starting Setup Portal through WiFi...", 1);
    #endif
    // Show config mode by flashing the LEDs at a 1 sec. interval
    ticker.attach(0.5, flashLed);
    //
    // WiFiManager
    //
    // Configure MQTT Parameters for the WiFi Manager
    WiFiManagerParameter custom_device_id("deviceId", "Device Id", dconfig.myId, 40);
    char prtnm[13];
    sprintf(prtnm, "%d", dconfig.mqttport);
    WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT port", prtnm, 6);
    WiFiManagerParameter custom_mqtt_broker("mqtt_broker", "MQTT broker", dconfig.mqttbroker, 64);
    WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT user", dconfig.mqttuser, 40);
    WiFiManagerParameter custom_mqtt_pass("mqtt_pass", "MQTT password", dconfig.mqttpass, 40);
    WiFiManagerParameter custom_mqtt_ns("mqtt_ns", "MQTT namespace", dconfig.namespc, 40);
    #ifdef MQTT_SIGN
      WiFiManagerParameter custom_mqtt_key("mqtt_key", "MQTT shared key", dconfig.mqttkey, 40);
    #endif
    // Create the instance of WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //
    // Set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    //
    // Timeout
    #ifdef CONFIG_TIMEOUT
      wifiManager.setConfigPortalTimeout(CONFIG_TIMEOUT);
    #endif
    //
    // Set static ip;
    //wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
    //
    // Add the MQTT parameters to the WiFi manager
    wifiManager.addParameter(&custom_device_id);
    wifiManager.addParameter(&custom_mqtt_broker);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);
    wifiManager.addParameter(&custom_mqtt_ns);
    #ifdef MQTT_SIGN
      wifiManager.addParameter(&custom_mqtt_key);
    #endif
    //
    // Reset settings - Uncomment to clean parameters during testing
    //wifiManager.resetSettings();
    //
    // Fetches SSID and PASS and tries to connect...
    // If it does not connect it starts an access point with the specified name
    // here TITLE and goes into a blocking loop awaiting configuration
    strcpy(tmpMsg0, PORTAL_SSID);
    strcat(tmpMsg0, deviceId);
    if (!wifiManager.startConfigPortal(tmpMsg0)) {
      #ifdef DEBUG_MODE
        writeDebug("Failed to connect and hit timeout; Reset in 3 sec.", 1);
      #endif
      delay(3000);
      // Reset and go into operation mode
      ESP.restart();
      delay(3000);
    }
    //
    // Save User Configuration to FS
    //
    if (shouldSaveConfig) {
      // Read Additional parameters
      strcpy(dconfig.myId, custom_device_id.getValue());
      strcpy(prtnm, custom_mqtt_port.getValue());
      dconfig.mqttport = atoi(prtnm);
      strcpy(dconfig.mqttbroker, custom_mqtt_broker.getValue());
      strcpy(dconfig.mqttuser, custom_mqtt_user.getValue());
      strcpy(dconfig.mqttpass, custom_mqtt_pass.getValue());
      strcpy(dconfig.namespc, custom_mqtt_ns.getValue());
      #ifdef MQTT_SIGN
        strcpy(dconfig.mqttkey, custom_mqtt_key.getValue());
      #endif
      // Save the configuration
      saveConfig();
    }
    delay(2000);
    ticker.detach();
    #ifdef DEBUG_MODE
      writeDebug("WiFi configurator done; Restart", 1);
    #endif
    ESP.restart();
  }
  
  /* ------------------------------------------------------------------------------
     Initialization
     ------------------------------------------------------------------------------ */
     
  // Define button
  initButton(0, PIN_BUTTON, LOW, 5, 0);
  
  // Start EEPROM
  #ifdef DEBUG_MODE
    writeDebug("Init EEPROM", 1);
  #endif
  EEPROM.begin(512);
  
  #ifdef COLLECT_STATS
    // Load last known status
    loadLastStatus();
  #endif
  
  // Start Network
  if (WiFi.SSID()) {
    wifiBegin();
  }
  #ifdef DEBUG_MODE
    else {
      writeDebug("Starting without WiFi", 1);
    }
  #endif
    
  /* ------------------------------------------------------------------------------
     Start OTA Updates
     ------------------------------------------------------------------------------ */
  #ifdef OTA_UPDATES
    otaEnabled = false;
    if (wifiSetup) {
      //int wt = 0;
      unsigned long wt = millis() + 8000;
      // Wait up to 8 seconds for WiFi... otherwise start without OTA
      do {
        #ifdef DEBUG_MODE
          writeDebug("Wait for WiFi for OTA...", 2);
        #endif
        delay(500);
        //++wt;
        otaEnabled = (WiFi.waitForConnectResult() == WL_CONNECTED) ? true : false;
        //} while (wt < 16 && !otaEnabled);
      } while (millis() < wt && !otaEnabled);
    }
    // If WiFi is available, configure OTA updates
    if (otaEnabled) {
      #ifdef OTA_HTTP
        // Web server update
        #ifdef DEBUG_MODE
          writeDebug("Request update to remote HTTP server", 1);
        #endif
        // Request for the latest image
        WiFiClient client;
        ESPhttpUpdate.setLedPin(PIN_LED1, HIGH);
        ESPhttpUpdate.onStart(http_update_started);
        ESPhttpUpdate.onEnd(http_update_finished);
        ESPhttpUpdate.onProgress(http_update_progress);
        ESPhttpUpdate.onError(http_update_error);
        #ifdef OTA_HTTP_URL
          t_httpUpdate_return ret = ESPhttpUpdate.update(client, OTA_HTTP_URL, OTA_IMAGE_VERSION);
        #else
          t_httpUpdate_return ret = ESPhttpUpdate.update(client, OTA_HTTP_SERVER, OTA_HTTP_PORT, OTA_SCRIPT_NAME, OTA_IMAGE_VERSION);
        #endif
        #ifdef DEBUG_MODE
          // Report results
          switch(ret) {
            case HTTP_UPDATE_FAILED:
              sprintf(tmpMsg, "OTA HTTP Update failed (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
              writeDebug(tmpMsg, 1);
              break;
            case HTTP_UPDATE_NO_UPDATES:
              writeDebug("OTA HTTP up to date", 1);
              break;
            case HTTP_UPDATE_OK:
              writeDebug("OTA HTTP update ok", 1); // may not be called since we reboot the ESP
              break;
          }
        #endif
        otaEnabled = false;
      #else
        // Setup Arduino updates
        #ifdef DEBUG_MODE
          writeDebug("Configure OTA port...", 2);
        #endif
        ArduinoOTA.setHostname("RREMOTE");
        #ifdef OTA_AR_PORT
          ArduinoOTA.setPort(OTA_AR_PORT);
        #endif
        #if defined(OTA_AR_HASH)
          ArduinoOTA.setPasswordHash(OTA_AR_HASH);
        #elif defined(OTA_AR_PASS)
          ArduinoOTA.setPassword(OTA_AR_PASS);
        #endif
        ArduinoOTA.onStart([]() {
          #ifdef DEBUG_MODE
            if (ArduinoOTA.getCommand() == U_FLASH) {
              writeDebug("Start updating sketch", 1);
            } else { // U_FS
              writeDebug("Start updating filesystem", 1);
            }
          #endif
          delay(300);
          // Disconnect MQTT
          mqttDisconnect();
          // Un-mount File System
          if (mounted) {
            LittleFS.end();
          }
          // Set the flag to start 
          otaUpdating = true;
        });
        ArduinoOTA.onEnd([]() {
          #ifdef DEBUG_MODE
            writeDebug("End OTA", 1);
          #endif
          otaUpdating = false;
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
          #ifdef DEBUG_MODE
            sprintf(tmpMsgD, "Progress: %u%%", (progress / (total / 100)));
            writeDebug(tmpMsgD, 1);
          #endif
        });
        ArduinoOTA.onError([](ota_error_t error) {
          #ifdef DEBUG_MODE
            sprintf(tmpMsgD, "Error[%u]: ", error);
            writeDebug(tmpMsgD, 1);
            if (error == OTA_AUTH_ERROR) {
              writeDebug("Auth Failed", 1);
            } else if (error == OTA_BEGIN_ERROR) {
              writeDebug("Begin Failed", 1);
            } else if (error == OTA_CONNECT_ERROR) {
              writeDebug("Connect Failed", 1);
            } else if (error == OTA_RECEIVE_ERROR) {
              writeDebug("Receive Failed", 1);
            } else if (error == OTA_END_ERROR) {
              writeDebug("End Failed", 1);
            }
          #endif
          otaUpdating = false;
        });
        ArduinoOTA.begin();
      #endif
    }
  #endif

  /* ------------------------------------------------------------------------------
     Remote setup
     ------------------------------------------------------------------------------ */
  #ifdef DEBUG_MODE
    writeDebug("Normal Boot", 1);
  #endif

  // Load data
  loadRemotes();
  loadCovers();

  // Define MQTT Topics
  mqttDefineTopics();
  
  if (wifiSetup) {
    mqttBegin();
  }
  netFirst = true;
  
  /* ------------------------------------------------------------------------------
     Start Operation
     ------------------------------------------------------------------------------ */
  // Start Remote Control
  delay(200);
  #ifdef DEBUG_MODE
    writeDebug("Start X remote", 3);
  #endif
  if (isEepromInit()) {
    //readLedState();
  } else {
    setEepromInit();
    ledsState = true;
    //saveLedState();
  }
  
  // Start timer interruptions for RF transmitter
  timer1_isr_init();
  timer1_attachInterrupt(rfTxPulseSeq);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
    
  // Start system timer
  sysMillis = millis();
}

/** **********************************************************************************************
  Loop function
*********************************************************************************************** */
void loop () {
  /* ------------------------------------------------------------------------------
     Handle OTA updates
     ------------------------------------------------------------------------------ */
  #if defined(OTA_UPDATES) && !defined(OTA_HTTP)
    if (otaEnabled) {
      ArduinoOTA.handle();
    }
  #endif

  /* ------------------------------------------------------------------------------
     Validate network connections
     ------------------------------------------------------------------------------ */
  unsigned long cTmr = millis();
  if (netCheck()) {
    // Process pending messages
    if ((unsigned long)(cTmr - pubMillis) > MQTT_BUFFER_TMR) {
      // Process pending messages
      mqttPublishBuffer();
      pubMillis = cTmr;
    }
    // Refresh net check timer
    netMillis = cTmr;
    #ifdef COLLECT_STATS
      // Reset net fail second couter
      statNoNet = 0;
    #endif
  } else {
    // Attemp to reconnect every NET_RECONNECT milliseconds (2.5 sec. aprox), peventing conection 
    // overlap and hang times...
    if (netFirst || (unsigned long)(cTmr - netMillis) > NET_RECONNECT) {
      netFirst = false;
      netConnect();
      netMillis = cTmr;
    }
    // Refresh the publish timer
    pubMillis = cTmr;
  }
  
  /* ------------------------------------------------------------------------------
     Keep track of running time
     ------------------------------------------------------------------------------ */
  systemTimer();
  
  /* ------------------------------------------------------------------------------
     Set action for the button depending on status
     ------------------------------------------------------------------------------ */
  int btn = getButtonStatus(0);
  if (btn == 3 || btn == 4) {
    #ifdef COLLECT_STATS
      ++statBtnCnt;
    #endif
    // Start radio receiver
    rfReceiverStart();
  }
  #ifdef MULT_CLICK
    else if (btn == 8) {
      #ifdef DEBUG_MODE
        writeDebug("Multi-click button", 1);
      #endif
      #ifdef COLLECT_STATS
        ++statBtnCnt;
      #endif
      goToSetupMode();
    }
  #endif
  
  // Monitor buffers
  rfTxRun();
  rfRxProcessBuffer();
  rfRxRun();
  
  // MQTT housekeeping
  mqttSubscriptions();
  mqttOnline();
  
  // LED handler
  handleLed();
  
  // Buzzer handler
  handleBuzzer();
  
  #ifdef COLLECT_STATS
    // Count the number of times the loop is executed
    ++statLoopCnt;
  #endif
}
