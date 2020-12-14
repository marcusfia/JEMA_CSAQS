//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                             Citizen Science Air Quality Sensor - Device Firmware  (v0.1)
//
//                                                       by
//
//                                                   JEMA Designs
//
//
//                               University of Iowa - Electrical and Computer Engineering
//                                         Senior Design Project - Fall 2020
// 
//
//
//
//                                               in conjunction with -
//
//
//
//
//
//
//                                       Dr. Jun Wang & ARROMA Lab (arroma.uiowa.edu)
//
//   NOTES  ----------------------------------------------------------------------------------------------------------------------
//   9/12/2020 (AW)  - This is a sample note. Start each note with the date, and your initials in paretheses.  Do no remove notes
//   , instead update with a new note, stating that the previous issue is now resolved or changed (reference to the date to make it
//   simple to find what matches to what.  Use multiple lines if needed for a given note, like this one.
//   9/33/2020 (AW)  - This would be the next note......
//   9/04/2020 (AW)  - Added code to read push button to be used for forcing device into Access Point mode. Push button will be a pull
//   -down resistor in order to consume the least amount of power.
//   10/20/2020 (AW) - Contributions by Jarrett for polling of sensors and reading of analog inputs
//   10/20/2020 (AW) - Merged firmware code with Marcus's wifi transmission firmware. Includes MQTT setup, AWS connection & WiFi connection
//   10/21/2020 (AW) - Added code to read & translate NOx sensor
//   10/21/2020 (AW) - Built out deep sleep mode and alpha device firmware --NOTE Deep Sleep not currently active,falls through to main loop
//   10/23/2020 (AW) - Major re-organization of code; usage of Arduino file tabs to separate out code and make more legible
//   10/23/2020 (AW) - Added new debug flag - DEBUG_OUT, for turning on and off the Serial data.  Can be used during normal or debug modes
//   10/23/2020 (AW) - Added enumerations for checking the power supply values, added logic points for dealing with power supply
//   10/24/2020 (AW) - Added changes Marcus made to JSON payload, added Jarrett's SIV code, added Marcus's device ID code
//   10/29/2020 (JH) - Added WifiManager and made it so AP can get a wifi and be resest with a buttonPressed Commented out because it suppresses
//   - the Serial output
//   10/30/2020 (MF) - Updated JSON to not prepend mac address to clientID. This was causing conflicts in API Gateway.
//   10/31/2020 (MF) - Updated TTL in JSON to 30 from 600 to decrease burden on DB and updated delays for loading certs from 1s to 100ms.
//   11/01/2020 (AW) - Fixed battery level calculation.
//   11/09/2020 (AW) - Implemented sleep mode for GPS, cleaned up & re-organized code
//   11/12/2020 (AW) - Implemented sampling protocol for analog input - uses an averaging method, variable re-naming
//   11/22/2020 (AW) - Added check against the fix statue of the GPS.  If no good GPS, wait 5 seconds, try again. Do this up to 10 times, then move on.
//   11/22/2020 (AW) - Added else checks to power supply if block to avoid unneeded checks
//   11/23/2020 (AW) - Adjusted power supply value enumerations based on battery life testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DEBUG FLAGS    
boolean DEBUG_MODE = false;            // SET DEBUG MODE to TRUE to disable WIFI/AWS/MQTT and Payload Transmit

boolean DEBUG_OUT = true;             //Turns on and off the serial output messages

boolean SENSOR_CAPTURE_MODE = false;  //Uses AWS transmission, but different sampling frequencies and no enabled sleep mode

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Include Headers

#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <EEPROM.h>                             //EEPROM storage in Flash used for UBLOX-GPS

#include <WiFiManager.h>                        //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include "Zanshin_BME680.h"                     //BME680 by Zanshin

#include "SparkFun_Ublox_Arduino_Library.h"     //u-blox NEO-M8U

#include "heartRate.h"                          //MAX 30105
#include "spo2_algorithm.h"                     //MAX 30105
#include "MAX30105.h"                           //MAX 30105

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Enumerations
enum PowerSupplyState 
{
  PWR_UNKNOWN  = 0,
  BATT_AT_DEAD = 1,
  BATT_AT_10   = 2,
  BATT_AT_50   = 3,
  BATT_AT_75   = 4,
  BATT_AT_MAX  = 5,
  SOLAR_PWR    = 6
};

enum PowerSupplyValues //Correlate the voltage x 100 (e.g. 360 = 3.60 volts x 100)
{
  BATTERY_DEAD = 355,
  BATTERY_10   = 358,
  BATTERY_20   = 366,
  BATTERY_50   = 390,
  BATTERY_75   = 410,
  BATTERY_MAX  = 430
};

enum DeepSleepDuration
{
  DUR_1HR = 3600,
  DUR_5MIN = 300,
  DUR_10MIN = 600,
  DUR_10SEC = 10,
  DUR_30SEC = 30
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Hardware Defines

#define UART_SPD    115200         //Serial communication speed setting

// Wemos board pin definitions to ESP8266 definitions
#define D0          16         //Used for deep sleep functions - DO NOT USE
#define D1          5          //SCL for I2C
#define D2          4          //SDA for I2C
#define D3          0          //Used for programming chip
#define D4          2          //Used for programming chip
#define D5          14         //Used for push button
#define D6          12         //Used for multiplexer select
#define D7          13
#define D8          15

//Device specific pin/logic defines
#define MUX_SELECT   12        //Multiplexer Select pin
#define SCL          5         //SCL pin
#define SDA          4         //SDA pin
#define PUSH_BUTTON  14        //Push button input pin
#define PWR_SUPPLY   0         //Logic level for MUX_SELECT to read power supply input from analog read
#define NOX_SENSOR   1         //Logic level for MUX_SELECT to read NOX sensor from analog read

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Objects and Variable

// AWS Credentials
const char* AWS_ENDPOINT  = "a3rqtvxgwfds5w-ats.iot.us-west-2.amazonaws.com";
char* AWS_IOT_CORE_CLIENT_ID = "trappist";
char* AWS_IOT_CORE_PUBLISH_TOPIC = "trappist/pub"; // AWS topic to publish to
const int MQTT_PAYLOAD_SIZE = 512;                 // Max size for buffer for payload
String payload;                                    // Stores the payload information
String db_device_ID;                               // Creates unique ID for queries
char msg[MQTT_PAYLOAD_SIZE];                       // stores msg to publish


//for receving messages IF the device is subscribed to something (not applicable currently)
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
}


// Instance Objects
WiFiClientSecure espClient;
PubSubClient client(AWS_ENDPOINT, 8883, callback, espClient); 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
BME680_Class BME680;        //BME60 I2C address = 0x77
MAX30105 particleSensor;    //MAX30105 I2C address = 0x57
SFE_UBLOX_GPS ubloxGPSObj;  //GPS I2C address = 0x42
WiFiManager wifiManager;

//BME680 variables
static int32_t temp;
static int32_t humidity;
static int32_t pressure;
static int32_t gas;
String tempStr;
String humidityStr;
String pressureStr;
String gasStr;

//Sensor enabled check declarations
bool gpsEnabled      = false;
bool bme680Enabled   = false;
bool max30105Enabled = false;

//Analog read & power supply variables
float voltage_divider_scalar = 19.936;      // power supply divison coeffecient 
float NOx_voltage_divider_scalar = 3.21056; //coefficient for NOx sensor reading
float ADC_resolution = 1024;                // 10 bit ADC
int read_A0;                                // Pin A0 on ESP8266 used for analog read of power
int currentPowerState;                      // Stores enumeration of current status of power supply
float vcc;
float noxSensorRead;

//MAX30105 variables
String red;  
String green;
String IR;

//u-blox variables
String latitude;
String longitude;
String altitude;
byte SIV;

//Deep Sleep Duration (in seconds)
static int32_t deepSleepDuration;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup

void setup()
{
  if(DEBUG_OUT)  //If using our serial output, set it up
  {
    Serial.begin(UART_SPD);
    Serial.setDebugOutput(true);
    delay(1000);
  }
  deepSleepDuration = DUR_5MIN;     //Default sampling rate is set here
  
  // setup pins and multiplexer
  pinMode(MUX_SELECT, OUTPUT);
  pinMode(PUSH_BUTTON, INPUT);
  digitalWrite(MUX_SELECT, PWR_SUPPLY);
  Wire.begin();

  db_device_ID = transmit_createClientID();
  CheckPowerSupply();               //Checks reading from power supply, determines if we should move on to rest of program or deal with low supply power
  CheckForPushButtonPresses();      //Check to see if we are holding the button at startup

  if(DEBUG_MODE)  //If in debug mode, set duration to 10 seconds, will ues delay loop
  {
    deepSleepDuration = DUR_10SEC;
    if(DEBUG_OUT) Serial.println("*DEBUG_MODE ACTIVE* Not attempting to connect to WiFi");
  }
  else
  {
    SetupWifiConnection();
  }

  delay(1000);
  
  if (!SPIFFS.begin())
  {
    if(DEBUG_OUT) Serial.println("Failed to mount file system"); 
    return;
  }

  delay(1000);
    
  InitializeBME680();
  InitialzieMAX30105();
  InitializeNEOM8U();

  /* START Time Client   ***************************/
  timeClient.begin();
  
  LoadAWSCertificate();

  //    New 'main loop'. Executes once each start up.  Deep sleep takes device back to reset, so only setup loop is run if we
  // are using deepsleep (debug_mode == false). If not, then proceeds to loop() where it uses a delay rather than deepsleep.  This 
  // delay will consume power, whereas deepsleep does not. In loop(), debug_mode == true, and some functionality is turned off during
  // debug mode. Review proceeding code to determine what functionality is turned off.

  //Main loop
  int counter = 0;
  if(gpsEnabled)
  {
      while(ubloxGPSObj.getFixType() < 2 && counter < 10) //If we don't have good fix, wait a bit, check again. Don't do this more than 10 times
      {
        delay(5000); //give it some time to try again
        counter++;
      }
  }

  ConnectToMQTT();
  PollSensors();
  BuildJSONPayload();
  SendJSONPayload();
  // End of Main loop

  
  if(!DEBUG_MODE) EnterDeepSleepMode(deepSleepDuration); //If we are in normal operation, go to sleep, else go to loop()

  if(SENSOR_CAPTURE_MODE)  //If using our serial output, set it up
  {
    Serial.begin(UART_SPD);
    Serial.setDebugOutput(true);
    delay(1000);
    Serial.println("NOx,VOC");
  }
  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Loop
//This is only entered if the device is set to debug mode (DEBUG_MODE = true);
void loop()
{
    ConnectToMQTT();
    PollSensors();
    BuildJSONPayload();
    SendJSONPayload();
    if(SENSOR_CAPTURE_MODE)
    {
      Serial.println(String(noxSensorRead) + "," + gasStr);
      delay(10*1000);  //ten second delay
    }
    else
    {
      delay(deepSleepDuration * 1000); 
    }
    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods and Functions
// Groups of functions are also split out into the tabs in the Arduino IDE

//This function will contain our logic for determing what to do when based on our current estimates of power left in battery
void CheckPowerSupply()
{
  currentPowerState = PWR_UNKNOWN;
  int voltageReading = SampleAnalogInput();
  int convertedVoltageReading =(int)(((float)voltageReading * (voltage_divider_scalar / ADC_resolution)) * 100); //converts from x.xx to xxx

  if(convertedVoltageReading > BATTERY_MAX)                                                   currentPowerState = SOLAR_PWR;
  else if(convertedVoltageReading <= BATTERY_MAX && convertedVoltageReading >= (BATTERY_MAX - 10)) currentPowerState = BATT_AT_MAX;
  else if(convertedVoltageReading < (BATTERY_MAX - 10) && convertedVoltageReading >= BATTERY_75)   currentPowerState = BATT_AT_75;
  else if(convertedVoltageReading < BATTERY_75 && convertedVoltageReading >= BATTERY_20)           currentPowerState = BATT_AT_50;
  else if(convertedVoltageReading < BATTERY_20 && convertedVoltageReading >= BATTERY_10)           currentPowerState = BATT_AT_10;
  else if(convertedVoltageReading < BATTERY_10 && convertedVoltageReading >= BATTERY_DEAD)         currentPowerState = BATT_AT_DEAD;

  switch(currentPowerState)  //What to do based on our current power situation
  {
    case SOLAR_PWR:
    if(DEBUG_OUT) Serial.println("CheckPowerSupply status = SOLAR_PWR");
    break;
    case BATT_AT_MAX:
    if(DEBUG_OUT) Serial.println("CheckPowerSupply status = BATT_AT_MAX");
    break;
    case BATT_AT_75:
    if(DEBUG_OUT) Serial.println("CheckPowerSupply status = BATT_AT_75");
    break;
    case BATT_AT_50:
    if(DEBUG_OUT) Serial.println("CheckPowerSupply status = BATT_AT_50");
    break;    
    case BATT_AT_10:
    {
      if(DEBUG_OUT) Serial.println("CheckPowerSupply status = BATT_AT_10, entering deep sleep");
      //EnterDeepSleepMode(deepSleepDuration);
    }
    break;
    case BATT_AT_DEAD:
    {
      if(DEBUG_OUT) Serial.println("CheckPowerSupply status = BATT_AT_DEAD, entering deep sleep");
      //EnterDeepSleepMode(deepSleepDuration);
    }
    break;
    default:   //PWR_UNKNOWN or non-handled #s
    if(DEBUG_OUT) Serial.println("CheckPowerSupply status = PWR_KNOWNN");
    break;
  }
}

void CheckForPushButtonPresses()
{
  int numberOfPresses = 0;
  bool pressed = digitalRead(PUSH_BUTTON);
  numberOfPresses++;
  while(pressed)
  {
    numberOfPresses++;
    if(numberOfPresses > 20)             //20 is an abritrary number.  Based on the delay below and the approx. amount of time we want to hold the button. Setting a low delay # means it leaves this while loop faster
    {
      //Go to AP mode start
      BeginAPMode();
      numberOfPresses = 0;
    }
    pressed = digitalRead(PUSH_BUTTON);
    delay(100);                         //100 is a small duration.  If we let go, it should move on from this function fast.  
  }
}

void BeginAPMode()
{
  wifiManager.resetSettings();
  wifiManager.autoConnect("Jema");
}

//MQTT reconnection attempts to reconnect every 5 seconds until it makes a connection
void reconnect() 
{
  while (!client.connected()) 
  {
    if(DEBUG_OUT) Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(AWS_IOT_CORE_CLIENT_ID)) 
    {
      if(DEBUG_OUT) Serial.println("connected");
    } 
    else 
    {
      if(DEBUG_OUT) 
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      
        char buf[256];
        espClient.getLastSSLError(buf,256);
        Serial.print("WiFiClientSecure SSL error: ");
        Serial.println(buf);
      }
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Checks if we are connected to the MQTT server, attempts to establish a connection if not.  
void ConnectToMQTT()
{
  if(!DEBUG_MODE)
  {
      if (!client.connected()) 
      {
        if(DEBUG_OUT) Serial.println("reconnect()");
        reconnect();
      }
      client.loop();
  }
  else if(DEBUG_OUT) Serial.println("*DEBUG_MODE ACTIVE* No MQTT connection Attempted");
}

int SampleAnalogInput()
{
  int analogInputReading = 0;
  for(int i = 0; i < 10; i++)
  {
    analogInputReading += analogRead(A0);
  }
  return analogInputReading /= 10;  //sample analog input 10 times and average the result
}

//One method call to poll all the current sensors
void PollSensors()
{
  client.setBufferSize(MQTT_PAYLOAD_SIZE);  //Ensure we have enough space in the buffer for all our data
  PollPowerSupply();
  PollNOxSensor();
  PollBME680();
  PollMAX30105();
  PollNEOM8U();
}

//Send the JSON payload. Just call BuildJSONPayload first or else it will be an empty payload
void SendJSONPayload()
{
   payload.toCharArray(msg, MQTT_PAYLOAD_SIZE);
   if(!DEBUG_MODE)
   {
     // publish msg to AWS topic:
     int payLoadLength = payload.length();
     int result = client.publish(AWS_IOT_CORE_PUBLISH_TOPIC, msg);
     if(DEBUG_OUT) Serial.println("Client id = " + db_device_ID + ", Payload size = " + String(payLoadLength) + ",client.publish = " + String(result) + "\r\n");
   }
   else if(DEBUG_OUT) Serial.println("*DEBUG_MODE ACTIVE* No Payload Sent!\r\n");
   payload = "";                  //empty String payload buffer
   memset(msg, 0, sizeof(msg));   //empty msg buffer
}

// Turns off all devices to save power, sets deepsleep on ESP8266 for duration seconds
void EnterDeepSleepMode(uint32_t duration)
{
  //put MAX to sleep
  particleSensor.shutDown();
  //Put NEO M8U to sleep
  bool gpsConfirmSleep = ubloxGPSObj.powerOff((duration * 900), 100);  //Duration of sleep in ms (zero for infinite), maximum timeout to wait for resposne from GPS module 
  //Put anything else to sleep
  if(DEBUG_OUT) Serial.println("u-blox powerOff response = " + String(gpsConfirmSleep));
  if(DEBUG_OUT) Serial.println("Entering sleep mode.... ");
  ESP.deepSleep(duration * 1000000);  //Deep sleep is in microseconds (1,000,000 (10^6) microseconds = 1 second)
}
