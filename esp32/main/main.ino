// === All Imports ===

#include <WiFiClientSecure.h>
#include <WiFiClient.h>

#include <SPI.h>
#include <math.h>
#include <string.h>

#include <mpu9255_esp32.h>
#include <compass.h>

#include "iir.hpp"

#include <ArduinoJson.h>

#include "ButtonClass.h"
#include "LedControl.h"
#include "binary.h"
#include "MatrixFunctions.h"


struct Coord {
  double lat;
  double lon;
  int error; // 0 for ok, 1 for error
};

// === Time Variables ===
// Note, these are all represented by their millisecond value, unless otherwise specified

const int ONE_SEC= 1000;

// === LED Matrix: Initiation, Variables, and Helper Functions ===

//(Moved, happens in MatrixFunctions.h)

// === System Variables ===

const int LOOP_PERIOD = 5; // in ms
uint32_t primary_timer=0;

 // (how often data should be collected and uploaded)
const int PING = 10*ONE_SEC;
uint32_t ping_timer = 0;

char main_user[60];
const int main_user_n = sprintf(main_user, "test1"); //  <------------------------------- USER MUST EDIT THIS!!!!!!

bool powered_off = true;

// === Button Initiation and Variables ===

int BUTTON_NUM = 45;
Button power_button(BUTTON_NUM);


// === Variables For Landmark Getting ===
int landmark_lat = 0;
int landmark_lon = 0;


// === WiFi Variables ===
const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 3500; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 3000;
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char request_mit[IN_BUFFER_SIZE];
char response_mit[OUT_BUFFER_SIZE];
char json_body[JSON_BODY_SIZE];

WiFiClientSecure client; //global WiFiClient Secure object
WiFiClient client2; //global WiFiClient Secure object



// === Variables For Geolocation Building ===

// Prefix to POST request:
const char PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
const char SUFFIX[] = "]}"; //suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //Google API key from 6.08 Staff

const int MAX_APS = 10; // max number of access points to collect

char* SERVER = "googleapis.com";  // Server URL



// === WiFi Credentials ===

// On Campus:
// const char NETWORK[] = "MIT GUEST";
// const char PASSWORD[] = "";

// const char NETWORK[] = "608_24G";
// const char PASSWORD[] = "608g2020";

// Hotspot:                                    //  <------------------------------- USER MUST EDIT THIS!!!!!!
// const char NETWORK[] = "SLP-F9FD71W0LMX0";
// const char PASSWORD[] = "bqd1nuuv3nd8d";

const char NETWORK[] = "EECS_Labs";
const char PASSWORD[] = "";



// === WiFi Extras ===

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

/*----------------------------------
   wifi_object_builder: generates a json-compatible entry for use with Google's geolocation API
   Arguments:
    * `char* object_string`: a char pointer to a location that can be used to build a c-string with a fully-contained JSON-compatible entry for one WiFi access point
    *  `uint32_t os_len`: a variable informing the function how much  space is available in the buffer
    * `uint8_t channel`: a value indicating the channel of WiFi operation (1 to 14)
    * `int signal_strength`: the value in dBm of the Access point
    * `uint8_t* mac_address`: a pointer to the six long array of `uint8_t` values that specifies the MAC address for the access point in question.

      Return value:
      the length of the object built. If not entry is written,
*/
int wifi_object_builder(char* object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t* mac_address) {
  char temp[300];//300 likely long enough for one wifi entry
  int len = sprintf(temp, "{\"macAddress\": \"%x:%x:%x:%x:%x:%x\",\"signalStrength\": %d,\"age\": 0,\"channel\": %d}",
                    mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5],
                    signal_strength, channel);
  if (len > os_len) {
    return 0;
  } else {
    return sprintf(object_string, "%s", temp);
  }
}

// More setup variables

MPU9255 imu; //imu object called, appropriately, imu

Compass compass(-14.75, imu); //For Cambridge, MA area

float beta = 0.90; // 0 <= beta <= 1, increase to emphasize new values more

float alpha = 1 - beta;
IIR filter(alpha);

double theta0 = -30;


Coord getLocation() {
  int offset = sprintf(json_body, "%s", PREFIX);
  int n = WiFi.scanNetworks(); //run a new scan
  Serial.println("scan done");
  if (n == 0) { // this should never happen, maybe a differenth thing has to happen or else below code will crash
    Serial.println("no networks found");

    return make_error_coord();

    // FIX!! We need to return something in this case, not sure what yet

  }
  else {
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

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));

    // Make a HTTPS request:
    Serial.println("GET geolocation");
    // request[0] = '\0'; //set 0th byte to null
    offset = 0; //reset offset variable for sprintf-ing
    offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
    offset += sprintf(request + offset, "Host: googleapis.com\r\n");
    offset += sprintf(request + offset, "Content-Type: application/json\r\n");
    offset += sprintf(request + offset, "cache-control: no-cache\r\n");
    offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
    offset += sprintf(request + offset, "%s\r\n", json_body);
    bool succeed = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

    while(!succeed){ // also add a check to wait every 4 seconds before sending out again
      succeed = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    }

    if (!*response) {
      Serial.println("Empty response");
      return make_error_coord();
    }

    Serial.println("finished GET of geolocation");

    char* start = strchr(response,'{');
    char* end = strrchr(response,'}');

    if (!start || !end) {
      Serial.println("Invalid response: does not include necessary braces");
      return make_error_coord();
    }

    *(end + 1) = NULL;

    DynamicJsonDocument doc(5000);
    DeserializationError error = deserializeJson(doc, start);

    if (error) {
      Serial.print(F("deserializeJson failed"));
      return make_error_coord();
    }

    double latitude = doc["location"]["lat"];
    double longitude = doc["location"]["lng"];

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));

    Coord location;
    location.lat = latitude;
    location.lon = longitude;
    location.error = 0;

    return location;
  }
}


// ======

/**
 *Used to get your current location.
 */
void pingLocation(Coord current_location) {

  // int offset = sprintf(json_body, "%s", PREFIX);
  // int n = WiFi.scanNetworks(); //run a new scan
  // Serial.println("scan done");
  // if (n == 0) { // this should never happen, maybe a differenth thing has to happen or else below code will crash
  //   Serial.println("no networks found");
  // } else {
  //   int max_aps = max(min(MAX_APS, n), 1);
  //   for (int i = 0; i < max_aps; ++i) { //for each valid access point
  //     uint8_t* mac = WiFi.BSSID(i); //get the MAC Address
  //     offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE-offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); //generate the query
  //     if(i!=max_aps-1){
  //       offset +=sprintf(json_body+offset,",");//add comma between entries except trailing.
  //     }
  //   }

  //   sprintf(json_body + offset, "%s", SUFFIX);

  //   int len = strlen(json_body);

  //   // request[0] = '\0';
  //   // response[0] = '\0';

  //   memset(request, 0, sizeof(request));
  //   memset(response, 0, sizeof(response));

  //   // Make a HTTPS request:
  //   Serial.println("GET geolocation");
  //   // request[0] = '\0'; //set 0th byte to null
  //   offset = 0; //reset offset variable for sprintf-ing
  //   offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
  //   offset += sprintf(request + offset, "Host: googleapis.com\r\n");
  //   offset += sprintf(request + offset, "Content-Type: application/json\r\n");
  //   offset += sprintf(request + offset, "cache-control: no-cache\r\n");
  //   offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
  //   offset += sprintf(request + offset, "%s\r\n", json_body);
  //   bool succeed = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

  //   while(!succeed){ // also add a check to wait every 4 seconds before sending out again
  //     succeed = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  //   }

  //   if (!*response) {
  //     Serial.println("Empty response");
  //     return;
  //   }

  //   Serial.println("finished GET of geolocation");

  //   char* start = strchr(response,'{');
  //   char* end = strrchr(response,'}');

  //   if (!start || !end) {
  //     Serial.println("Invalid response: does not include necessary braces");
  //     return;
  //   }

  //   *(end + 1) = NULL;

  //   DynamicJsonDocument doc(5000);
  //   DeserializationError error = deserializeJson(doc, start);

  //   if (error) {
  //     Serial.print(F("deserializeJson failed"));
  //     return;
  //   }

  //   double latitude = doc["location"]["lat"];
  //   double longitude = doc["location"]["lng"];

  //   memset(request, 0, sizeof(request));
  //   memset(response, 0, sizeof(response));

  // Make a POST request:

  double latitude = current_location.lat;
  double longitude = current_location.lon;

  char json_body[300]; // Should be plenty
  sprintf(json_body, "user=%s&lat=%f&lon=%f", main_user, latitude, longitude);

  char buffer[1000];
  int n = sprintf(buffer, "POSTing this to our server: %s", json_body);
  Serial.println(buffer);

  // Clearing it just for good practice
  // request[0] = '\0';
  // response[0] = '\0';

  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  int offset = 0;

  offset += sprintf(request + offset, "POST /sandbox/sc/team44/map/main.py HTTP/1.1\r\n");
  offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
  offset += sprintf(request + offset, "Content-Type: application/x-www-form-urlencoded\r\n");
  offset += sprintf(request + offset, "Content-Length: %d\r\n", strlen(json_body));
  offset += sprintf(request + offset, "\r\n");
  offset += sprintf(request + offset, "%s", json_body);
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  Serial.println("finished POSTing to our server");

}


/**
* Get the last landmark that the user selected on the website.
* Returns an array of floats: [lat, lon].
*/
void getLandmarkLatLon(){
  request[0] = '\0';
  response[0] = '\0';
  int offset = 0;

  // TODO, finalize URL and stuff:

  // offset += sprintf(request + offset, "GET https://608dev-2.net/sandbox/sc/team44/map/landmarks.py?getLandmark=%s HTTP/1.1\r\n", main_user);
  // offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
  // offset += sprintf(request + offset, "\r\n");
  // do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

  // TODO: now with this response, do some delimiter stuff to get the actual lat and lon numbers

  landmark_lat = 0;
  landmark_lon = 0;

}

bool on_off_check(){
  int curr_state = power_button.update();

  if(powered_off){

    blinking_center_animation();

    if(curr_state==1){
      powered_off=false;
      show_center();

      getLandmarkLatLon();

      Serial.println("Powered on");
    }
    return true;
  }

  if(curr_state==2){
    powered_off=true;
    Serial.println("Powered off");
  }
  return false;
}

double get_azimuth(Coord current_location) {
  double current_lat = current_location.lat;
  double current_lon = current_location.lon;
  
  double dest_lat = 42.3597118;
  double dest_lon = -71.0941475;

  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  int offset = 0;
  offset += sprintf(request + offset, "GET https://608dev-2.net/sandbox/sc/team44/compute_angle.py?current_lat=%f&current_lon=%f&dest_lat=%f&dest_lon=%f HTTP/1.1\r\n", current_lat, current_lon, dest_lat, dest_lon);
  offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
  offset += sprintf(request + offset, "\r\n");
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  double forward_azimuth = atof(response);

  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  return forward_azimuth;
}


void setup() {
  Serial.begin(115200);

  initialize_matrix();

  pinMode(BUTTON_NUM, INPUT_PULLUP);

  delay(100); //wait a bit (100 ms)

  //PRINT OUT WIFI NETWORKS NEARBY
  // TODO this may be removed ?????? Not entirely sure if the next part uses it
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
  delay(500); //wait a bit (100 ms)


  //if using regular connection use line below:
  WiFi.begin(NETWORK, PASSWORD);
  //if using channel/mac specification for crowded bands use the following:
  // WiFi.begin(NETWORK, PASSWORD, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 100) {
    throbber_animation();
    delay(100);
    Serial.print(".");
    if(count%10==0){
      Serial.print("\n");
    }
    count++;
  }

  delay(2000);

  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    // Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
    //               WiFi.localIP()[1], WiFi.localIP()[0],
    //               WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    setup();
    Serial.println("Failed to Connect. Going to restart.");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }


  Serial.println("Powered off");


  Serial.printf("Compass calibration will begin in 3s\r\n");
  delay(3000);

  Serial.printf("Calibrating: spin system in 360deg motions for 15s\r\n");

  compass.calibrate(); //Calibrate for a set # of milliseconds

  Serial.printf("Compass is calibrated\r\n");

  filter.set(update_compass());

}

void loop() {

  if(WiFi.status() != WL_CONNECTED){
    powered_off=true;
  }

  // check that wifi is still connected
  // if not, go into wifi search mode without crashing

  if (on_off_check()){
    return;
  }



  if (millis() - ping_timer > PING){


    // sensor readings go in here






    // other get and posts go here



    Coord current_location = getLocation();

    pingLocation(current_location);
    double forward_azimuth = get_azimuth(current_location);


    double heading = update_compass();
    double filtered_heading = filter.step(heading);

    double calc_angle = forward_azimuth - filtered_heading + theta0;

    int actual_angle = angle_in_range(calc_angle);
    set_LED_direction(actual_angle);

    ping_timer = millis();
  }

  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}

// double rad_to_deg(double rad) {
//   return rad * 180.0 / PI;
// }

float update_compass() {
  compass.update();
  float compass_heading = compass.heading;
  float clockwise_heading = 360.0 - compass_heading;
  return clockwise_heading;
}

int real_mod(int x, int m) {
  return (x % m + m) % m;
}

int angle_in_range(double angle) {
  int rounded_angle = (int)round(angle);
  return real_mod(rounded_angle, 360);
}

Coord make_error_coord() {
  Coord error_coord;
  error_coord.lat = 0.0;
  error_coord.lon = 0.0;
  error_coord.error = 1;

  return error_coord;
}
