#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <mpu9255_esp32.h>
#include <compass.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <math.h>
#include <string.h> //this is the line of code you are missing

#include <iir.hpp>

#include "LedControl.h"
#include "binary.h"

// #define CALIBRATE 1

// #define DO_COMPASS 1
// #define DO_RESULTS 0

// const uint16_t RESPONSE_TIMEOUT = 6000;
// const uint16_t IN_BUFFER_SIZE = 3500; //size of buffer to hold HTTP request
// const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
// const uint16_t JSON_BODY_SIZE = 3000;
// char request[IN_BUFFER_SIZE];
// char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
// char json_body[JSON_BODY_SIZE];

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 800; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 800; //size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 800;
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char json_body[JSON_BODY_SIZE];

/* CONSTANTS */
//Prefix to POST request:
const char PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
const char SUFFIX[] = "]}"; //suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //don't change this and don't share this

const int MAX_APS = 5;

WiFiClientSecure client; //global WiFiClient Secure object
WiFiClient client2; //global WiFiClient Secure object

// const char NETWORK[] = "MIT";
// const char PASSWORD[] = "";

const char NETWORK[] = "EECS_Labs";
const char PASSWORD[] = "";

/* Having network issues since there are 50 MIT and MIT_GUEST networks?. Do the following:
    When the access points are printed out at the start, find a particularly strong one that you're targeting.
    Let's say it is an MIT one and it has the following entry:
   . 4: MIT, Ch:1 (-51dBm)  4:95:E6:AE:DB:41
   Do the following...set the variable channel below to be the channel shown (1 in this example)
   and then copy the MAC address into the byte array below like shown.  Note the values are rendered in hexadecimal
   That is specified by putting a leading 0x in front of the number. We need to specify six pairs of hex values so:
   a 4 turns into a 0x04 (put a leading 0 if only one printed)
   a 95 becomes a 0x95, etc...
   see starting values below that match the example above. Change for your use:
   Finally where you connect to the network, comment out
     WiFi.begin(network, password);
   and uncomment out:
     WiFi.begin(network, password, channel, bssid);
   This will allow you target a specific router rather than a random one!
*/
uint8_t channel = 6; //network channel on 2.4 GHz
byte bssid[] = {0xD4, 0x20, 0xB0, 0xCC, 0xDF, 0x44}; //6 byte MAC address of AP you're targeting.

char* SERVER = "googleapis.com";  // Server URL

// TFT_eSPI tft = TFT_eSPI();
MPU9255 imu; //imu object called, appropriately, imu

Compass compass(-14.75, imu); //For Cambridge, MA area

float beta = 0.90; // 0 <= beta <= 1, increase to emphasize new values more

float alpha = 1 - beta;
IIR filter(alpha);

float theta0 = -30;

LedControl lc = LedControl(35,36,15,1);
byte north[9]= {B00000001,B00000001,B00000001,B00011001,B00011001,B00000001,B00000001,B00000001};
byte south[9]= {B10000000,B10000000,B10000000,B10011000,B10011000,B10000000,B10000000,B10000000};
byte east[9]= {B00000000,B00000000,B00000000,B00011000,B00011000,B00000000,B00000000,B11111111};
byte west[9]= {B11111111,B00000000,B00000000,B00011000,B00011000,B00000000,B00000000,B00000000};
byte northwest[9]= {B11111111,B00000001,B00000001,B00011001,B00011001,B00000001,B00000001,B00000001};
byte southwest[9]= {B11111111,B10000000,B10000000,B10011000,B10011000,B10000000,B10000000,B10000000};
byte northeast[9]= {B00000001,B00000001,B00000001,B00011001,B00011001,B00000001,B00000001,B11111111};
byte southeast[9]= {B10000000,B10000000,B10000000,B10011000,B10011000,B10000000,B10000000,B11111111};

void setup() {
  Serial.begin(115200); //for debugging if needed.
  delay(50); //pause to make sure comms get set up
  setup_imu();

  // tft.init(); //initialize the screen
  // tft.setRotation(3); //set rotation for our layout
  // tft.fillScreen(TFT_BLACK);
  // tft.setTextSize(1); //default font size
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);



  lc.shutdown(0,false);
  // Set brightness to a medium value
  lc.setIntensity(0,8);
  // Clear the display
  lc.clearDisplay(0);



  //PRINT OUT WIFI NETWORKS NEARBY
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
      uint8_t* cc = WiFi.BSSID(i);
      for (int k = 0; k < 6; k++) {
        Serial.print(*cc, HEX);
        if (k != 5) Serial.print(":");
        cc++;
      }
      Serial.println("");
    }
  }
  delay(100); //wait a bit (100 ms)

  // Wifi stuff below

  //if using regular connection use line below:
  WiFi.begin(NETWORK, PASSWORD);
  //if using channel/mac specification for crowded bands use the following:
  //WiFi.begin(network, password, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }



  //Perform calibration routine

  Serial.printf("Compass calibration will begin in 3s\r\n");
  delay(3000);

  Serial.printf("Calibrating: spin system in 360deg motions for 15s\r\n");

  // tft.setCursor(0, 0);
  // tft.println("Calibrating: spin system in 360deg motions for 15s");

  compass.calibrate(); //Calibrate for a set # of milliseconds

  Serial.printf("Compass is calibrated\r\n");

  // tft.fillScreen(TFT_BLACK);

  // tft.setCursor(0, 0);
  // tft.println("Compass is calibrated                       ");

  // delay(1500);
  // tft.fillScreen(TFT_BLACK);


  filter.set(update_compass());

}

void loop() {
  
  // Get current lat, lon

  double current_lat;
  double current_lon;

  int offset = sprintf(json_body, "%s", PREFIX);
  int n = WiFi.scanNetworks(); //run a new scan. could also modify to use original scan from setup so quicker (though older info)
  // Serial.println("scan done");
  if (n == 0) {
    // Serial.println("no networks found");
  } else {
    int max_aps = max(min(MAX_APS, n), 1);
    for (int i = 0; i < max_aps; ++i) { //for each valid access point
      uint8_t* mac = WiFi.BSSID(i); //get the MAC Address
      offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE-offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); //generate the query
      if(i!=max_aps-1){
        offset +=sprintf(json_body+offset,",");//add comma between entries except trailing.
      }
    }

    sprintf(json_body + offset, "%s", SUFFIX);
    
    int len = strlen(json_body);

    // request[0] = '\0';
    // response[0] = '\0';
    // Make a HTTP request:
    // Serial.println("GETing geolocation");
    // request[0] = '\0'; //set 0th byte to null

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));

    offset = 0; //reset offset variable for sprintf-ing
    offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
    offset += sprintf(request + offset, "Host: googleapis.com\r\n");
    offset += sprintf(request + offset, "Content-Type: application/json\r\n");
    offset += sprintf(request + offset, "cache-control: no-cache\r\n");
    offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
    offset += sprintf(request + offset, "%s\r\n", json_body);
    // do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    // Serial.println("finished GETing geolocation");

    char* start = strchr(response,'{');
    char* end = strrchr(response,'}');
    *(end + 1) = NULL;

    // DynamicJsonDocument doc(5000);
    DynamicJsonDocument doc(950);
    DeserializationError error = deserializeJson(doc, start);

    if (error) {
      Serial.print(F("deserializeJson failed"));
      return;
    }

    current_lat = doc["location"]["lat"];
    current_lon = doc["location"]["lng"];

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));
  }

  // Serial.printf("lat: %f, lon: %f\r\n", current_lat, current_lon);

  double dest_lat = 42.3597118;
  double dest_lon = -71.0941475;


  offset = 0;
  offset += sprintf(request + offset, "GET https://608dev-2.net/sandbox/sc/team44/compute_angle.py?current_lat=%f&current_lon=%f&dest_lat=%f&dest_lon=%f HTTP/1.1\r\n", current_lat, current_lon, dest_lat, dest_lon);
  offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
  offset += sprintf(request + offset, "\r\n");
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  double forward_azimuth = atof(response);

  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  // Serial.printf("%s\r\n", response);

  double heading = update_compass();
  double filtered_heading = filter.step(heading);

  Serial.printf("Heading is %f\r\n", filtered_heading);

  // // tft.setCursor(0, 0);
  // // tft.printf("Heading: %.3f deg   ", heading);

  Serial.printf("Forward azimuth is %f\r\n", forward_azimuth);
  int calc_angle = forward_azimuth - filtered_heading + theta0;
  int actual_angle = angle_in_range(calc_angle);

  Serial.printf("Actual angle: %d\r\n\n", actual_angle);

  set_LED_direction(actual_angle);
}

float update_compass() {
  compass.update();
  float compass_heading = compass.heading;
  float clockwise_heading = 360.0 - compass_heading;
  return clockwise_heading;
}

int real_mod(int x, int m) {
  return (x % m + m) % m;
}

int angle_in_range(float angle) {
  int rounded_angle = (int)round(angle);
  return real_mod(rounded_angle, 360);
}

void northled(){
  lc.setRow(0,0,north[0]);
  lc.setRow(0,1,north[1]);
  lc.setRow(0,2,north[2]);
  lc.setRow(0,3,north[3]);
  lc.setRow(0,4,north[4]);
  lc.setRow(0,5,north[5]);
  lc.setRow(0,6,north[6]);
  lc.setRow(0,7,north[7]);
}

void southled(){
  lc.setRow(0,0,south[0]);
  lc.setRow(0,1,south[1]);
  lc.setRow(0,2,south[2]);
  lc.setRow(0,3,south[3]);
  lc.setRow(0,4,south[4]);
  lc.setRow(0,5,south[5]);
  lc.setRow(0,6,south[6]);
  lc.setRow(0,7,south[7]);
}

void westled(){
  lc.setRow(0,0,west[0]);
  lc.setRow(0,1,west[1]);
  lc.setRow(0,2,west[2]);
  lc.setRow(0,3,west[3]);
  lc.setRow(0,4,west[4]);
  lc.setRow(0,5,west[5]);
  lc.setRow(0,6,west[6]);
  lc.setRow(0,7,west[7]);
}

void eastled(){
  lc.setRow(0,0,east[0]);
  lc.setRow(0,1,east[1]);
  lc.setRow(0,2,east[2]);
  lc.setRow(0,3,east[3]);
  lc.setRow(0,4,east[4]);
  lc.setRow(0,5,east[5]);
  lc.setRow(0,6,east[6]);
  lc.setRow(0,7,east[7]);
  
}

void neled(){
  lc.setRow(0,0,northeast[0]);
  lc.setRow(0,1,northeast[1]);
  lc.setRow(0,2,northeast[2]);
  lc.setRow(0,3,northeast[3]);
  lc.setRow(0,4,northeast[4]);
  lc.setRow(0,5,northeast[5]);
  lc.setRow(0,6,northeast[6]);
  lc.setRow(0,7,northeast[7]);
}

void seled(){
  lc.setRow(0,0,southeast[0]);
  lc.setRow(0,1,southeast[1]);
  lc.setRow(0,2,southeast[2]);
  lc.setRow(0,3,southeast[3]);
  lc.setRow(0,4,southeast[4]);
  lc.setRow(0,5,southeast[5]);
  lc.setRow(0,6,southeast[6]);
  lc.setRow(0,7,southeast[7]);
}

void nwled(){
  lc.setRow(0,0,northwest[0]);
  lc.setRow(0,1,northwest[1]);
  lc.setRow(0,2,northwest[2]);
  lc.setRow(0,3,northwest[3]);
  lc.setRow(0,4,northwest[4]);
  lc.setRow(0,5,northwest[5]);
  lc.setRow(0,6,northwest[6]);
  lc.setRow(0,7,northwest[7]);
}

void swled(){
  lc.setRow(0,0,southwest[0]);
  lc.setRow(0,1,southwest[1]);
  lc.setRow(0,2,southwest[2]);
  lc.setRow(0,3,southwest[3]);
  lc.setRow(0,4,southwest[4]);
  lc.setRow(0,5,southwest[5]);
  lc.setRow(0,6,southwest[6]);
  lc.setRow(0,7,southwest[7]);

}

void set_LED_direction(int angle) {
  if ((angle <= 22.5) || (angle > 337.5)){
    northled();
  }
  else if ((angle > 22.5) && (angle <= 67.5)){
    neled();
  }
  else if ((angle > 67.5) && (angle <= 112.5)){
    eastled();
  }
  else if ((angle > 112.5) && (angle <= 157.5)){
    seled();
  }
  else if ((angle > 157.5) && (angle <= 202.5)){
    southled();
  }
  else if ((angle > 202.5) && (angle <= 247.5)){
    swled();
  }
  else if ((angle > 247.5) && (angle <= 292.5)){
    westled();
  }
  else{
    nwled();
  }
}






