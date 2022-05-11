// === All Imports ===

// = External =

// - WiFi Related -
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

// - Sensor Related -
#include <mpu9255_esp32.h>
#include <Adafruit_BMP280.h>
#include <compass.h>

// - Other -
#include <SPI.h>
#include <math.h>
#include <ArduinoJson.h>

// TODO: Remove these?
#include "binary.h"
#include <stdio.h>
#include <string.h>

// = Internal =
#include "AveragingFilters.h"
#include "ButtonClass.h"
#include "LedControl.h"
#include "MatrixFunctions.h"
#include "HTTPS_Certficates.h"
#include "WiFi_Extras.h"


// === Initializing Sensors ===

// - BMP -
Adafruit_BMP280 bmp;

// - Magnetometer and IMU -
MPU9255 imu;
Compass compass(-14.75, imu); //For Cambridge, MA area

// - Button -
int BUTTON_NUM = 45;
Button power_button(BUTTON_NUM);

// === Sensor Variables ===

// - General -
float temperature;
float pressure;
float altitude;

// - Magnetometer -

float beta = 0.5; // 0 <= beta <= 1, increase to emphasize new values more

float alpha = 1 - beta;
IIR mag_filter(alpha);

double theta0 = 180; // Offset to account for positioning of the magnetometer on the board, etc.

uint32_t led_timer;


// - Step Counter -
float x, y, z;
float old_acc_mag;
float older_acc_mag;
int steps = 0;


// Audio variables
uint8_t AUDIO_TRANSDUCER = 14;
uint8_t AUDIO_PWM = 1;

enum PLAY_STATE {IDLE, PLAY};
PLAY_STATE audio_state = IDLE;

uint32_t play_timer;
double prox_threshold = 25.0;

bool play_flag = false;



// - Pressure and Altitude -

float altpressure = 1013.5; // starting value
int calibrated = 0;
float elevation = 0;
float old_elev = 0;
float older_alt_mag;
float old_alt_mag;
float alt_mag = 0;
float avg_alt_mag = 0;
float delta = 0;


// === Other Helpful Things ===
struct Coord {
  double lat;
  double lon;
  int error; // 0 for ok, 1 for error
};


// === Time Variables ===
// Note, these are all represented by their millisecond value, unless otherwise specified

const int ONE_SEC = 1000;


// === System Variables ===

const int LOOP_PERIOD = 5; // in ms
uint32_t primary_timer=0;

 // (how often data should be collected and uploaded)
const int PING = 10 * ONE_SEC;
uint32_t ping_timer;

char main_user[60];
const int main_user_n = sprintf(main_user, "julia"); //  <------------------------------- USER MUST EDIT THIS!!!!!!

bool powered_off = true;



// === Variables For Landmark Getting, and angle calculations ===
Coord destination;
Coord current_location;
double forward_azimuth;
double proximity = prox_threshold + 1.0;

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
WiFiClient client2; //global WiFiClient object



// === Variables For Geolocation Building ===

// Prefix to POST request:
const char PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
const char SUFFIX[] = "]}"; //suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //Google API key from 6.08 Staff

const int MAX_APS = 10; // max number of access points to collect

char* SERVER = "googleapis.com";  // Server URL


// === WiFi Credentials ===

// On Campus:
// const char NETWORK[] = "MIT";
// const char PASSWORD[] = "";

// const char NETWORK[] = "EECS_Labs";
// const char PASSWORD[] = "";

// const char NETWORK[] = "18_62";
// const char PASSWORD[] = "";

// const char NETWORK[] = "608_24G";
// const char PASSWORD[] = "608g2020";

// Hotspot:                                    //  <------------------------------- USER MUST EDIT THIS!!!!!!
const char NETWORK[] = "SLP-F9FD71W0LMX0";
const char PASSWORD[] = "bqd1nuuv3nd8d";

// const char NETWORK[] = "806net";
// const char PASSWORD[] = "kfhe94jcmshfkr";

// === WiFi Extras ===

// For more information, refer to WiFi_Extras.cpp

uint8_t channel = 6; //network channel on 2.4 GHz
byte bssid[] = {0xD4, 0x20, 0xB0, 0xCC, 0xDF, 0x44}; //6 byte MAC address of AP you're targeting.

void print_WiFi_networks(){
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
}



// === WiFi Related Functions ====

void connectToWiFi(){
  //if using regular connection use line below:
  WiFi.begin(NETWORK, PASSWORD);
  //if using channel/mac specification for crowded bands use the following:
  // WiFi.begin(NETWORK, PASSWORD, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 600) {
    throbber_animation();
    delay(100);
    Serial.print(".");
    if(count%10==0 && count!=0){
      Serial.print("\n");
    }
    count++;
  }

  clear_matrix();

  delay(500);

  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    // Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
    //               WiFi.localIP()[1], WiFi.localIP()[0],
    //               WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    setup();
    Serial.println("Failed to Connect after 1 minute. Going to restart.");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
}


// === All Other Functions ===

/**
 * Used to get your current location.
 */
Coord getLocation() {
  int offset = sprintf(json_body, "%s", PREFIX);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    // if this happens, return a error coord
    return make_error_coord();
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

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));

    offset = 0; //reset offset variable for sprintf-ing
    offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
    offset += sprintf(request + offset, "Host: googleapis.com\r\n");
    offset += sprintf(request + offset, "Content-Type: application/json\r\n");
    offset += sprintf(request + offset, "cache-control: no-cache\r\n");
    offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
    offset += sprintf(request + offset, "%s\r\n", json_body);
    bool succeed = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true, CA_CERT);

    while(!succeed){ // also add a check to wait every 4 seconds before sending out again
      succeed = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true, CA_CERT);
    }

    if (!*response) {
      Serial.println("Empty response");
      return make_error_coord();
    }

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

    Coord location;
    location.lat = latitude;
    location.lon = longitude;
    location.error = 0;

    return location;
  }
}



/**
 * Used to send your current location to the server
 */
void sendLocation(Coord current_location) {

  double latitude = current_location.lat;
  double longitude = current_location.lon;

  char json_body[300]; // Should be plenty
  sprintf(json_body, "user=%s&lat=%f&lon=%f", main_user, latitude, longitude);

  char buffer[1000];
  int n = sprintf(buffer, "POSTing this to our server: %s", json_body);

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
}


/**
* Get the last landmark that the user selected on the website.
*/
Coord get_destination() {


  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  int offset = 0;
  offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team44/map/main.py?destination=%s HTTP/1.1\r\n", main_user);
  offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
  offset += sprintf(request + offset, "\r\n");
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

  if (!*response) {
    Serial.println("Empty response");
    destination = make_error_coord();

    return destination;
  }

  char* ptr = strtok(response, ",");
  double dest_lat = atof(ptr);
  ptr = strtok(NULL, ",");
  double dest_lon = atof(ptr);

  Coord destination;
  destination.lat = dest_lat;
  destination.lon = dest_lon;
  destination.error = 0;
  return destination;

}


bool on_off_check(){
  int curr_state = power_button.update();

  if(powered_off){

    blinking_center_animation();

    if(curr_state==1){
      powered_off=false;

      show_center();

      destination = get_destination();

      play_flag = true;

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


void set_azimuth_and_proximity(Coord current_location, Coord destination) {
  double current_lat = current_location.lat;
  double current_lon = current_location.lon;

  double dest_lat = destination.lat;
  double dest_lon = destination.lon;

  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  int offset = 0;
  offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team44/map/compute_angle.py?current_lat=%f&current_lon=%f&dest_lat=%f&dest_lon=%f HTTP/1.1\r\n", current_lat, current_lon, dest_lat, dest_lon);
  offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
  offset += sprintf(request + offset, "\r\n");
  do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

  char* ptr = strtok(response, ",");
  forward_azimuth = atof(ptr);
  ptr = strtok(NULL, ",");
  proximity = atof(ptr);
}



//Post reporting state machine

void post_reporter_fsm() {

  if(millis() - ping_timer > PING) {
      char body[100];


      memset(request, 0, sizeof(request));
      memset(response, 0, sizeof(response));

      int offset = 0;
      sprintf(body,"user=%s&steps=%d",main_user,steps);
      int body_len = strlen(body);
      offset += sprintf(request + offset,"POST http://608dev-2.net/sandbox/sc/team44/steps.py HTTP/1.1\r\n");
      offset += sprintf(request + offset,"Host: 608dev.net\r\n");
      offset += sprintf(request + offset,"Content-Type: application/x-www-form-urlencoded\r\n");
      offset += sprintf(request + offset,"Content-Length: %d\r\n", body_len);
      offset += sprintf(request + offset,"\r\n");
      offset += sprintf(request + offset, "%s", body);
      offset += sprintf(request + offset,"\r\n");

      do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);

      char output[80];
      ping_timer = millis();
  }
}

void step_reporter_fsm(float avg_acc_mag) {
  if(avg_acc_mag < 13.5/9.81 && avg_acc_mag > 12.5/9.81 && avg_acc_mag > older_acc_mag)
    {
      steps++;
    }
}



void run_instrument() {
    bool within_proximity = proximity <= prox_threshold;
    switch(audio_state) {
      case(IDLE):
        if (within_proximity && play_flag) {
          play_timer = millis();
          audio_state = PLAY;
          Serial.printf("Entered playing state\r\n");
        }
        break;
      
      case(PLAY):
        if (millis() - play_timer <= 400) {
            Serial.printf("Played first tone\r\n");
            ledcWriteTone(AUDIO_PWM, 1567.98);
          }
        else if (millis() - play_timer <= 800 && millis() - play_timer > 400) {
          ledcWriteTone(AUDIO_PWM, 1046.50);
        }
        else if (millis() - play_timer <= 1200 && millis() - play_timer > 800) {
          ledcWriteTone(AUDIO_PWM, 311.13);
        }
        else {
          ledcWriteTone(AUDIO_PWM, 0);
          audio_state = IDLE;
          play_flag = false;
        }
        break;
    }
}



// === Compass Stuff ===

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

void set_geo_values() {
  current_location = getLocation();
  sendLocation(current_location);
  set_azimuth_and_proximity(current_location, destination);
  Serial.printf("Proximity: %f\r\n", proximity);
}



void setup() {
  Serial.begin(115200);

  // === Sensor Stuff ===

  setup_imu();

  initialize_matrix();

  pinMode(BUTTON_NUM, INPUT_PULLUP);

  // - BMP Stuff -
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
    esp_restart();
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  delay(100);

  // === Other ===

  connectToWiFi();
  Serial.println("Powered off.");

  // === Get current pressure conditions ===

  memset(request, 0, sizeof(request));
  memset(response, 0, sizeof(response));

  int offset = 0;
  offset += sprintf(request + offset, "GET https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/cambridge, ma?unitGroup=metric&elements=pressure&include=current&key=QGXZBRL26UYTTTW9C67URDEFB&contentType=json HTTP/1.1\r\n");
  offset += sprintf(request + offset, "Host: weather.visualcrossing.com\r\n");
  offset += sprintf(request + offset, "\r\n");

  bool succeed3 = do_https_request("weather.visualcrossing.com", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false, WEATHER_CERT);

  while(!succeed3){
    succeed3 = do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true, WEATHER_CERT);
  }

  if (!*response) {
    Serial.println("Empty response.");
    elevation = 8;

  } else {

    DynamicJsonDocument doc1(1024);
    char* starting1 = strchr(response, '{');
    char* ending1 = strrchr(response, '}');
    *(ending1 + 1) = NULL;
    DeserializationError error = deserializeJson(doc1, starting1);
    // Test if parsing succeeds.
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
    }
    else {
        altpressure = doc1["currentConditions"]["pressure"];
    }
    response[0] = 0;

    elevation = 8.0;

  }

  // Audio setup

  pinMode(AUDIO_TRANSDUCER, OUTPUT);

  //set up AUDIO_PWM which we will control in this lab for music:
  ledcSetup(AUDIO_PWM, 0, 12);//12 bits of PWM precision
  // ledcWrite(AUDIO_PWM, 0); //0 is a 0% duty cycle for the NFET
  ledcAttachPin(AUDIO_TRANSDUCER, AUDIO_PWM);


  set_geo_values();
  ping_timer = millis();

  // === Calibration For Compass ===

  show_border();

  Serial.printf("Compass calibration will begin in 3s\r\n");
  delay(3000);

  Serial.printf("Calibrating: spin system in 360deg motions for 15s\r\n");

  compass.calibrate(); //Calibrate for a set # of milliseconds

  Serial.printf("Compass is calibrated\r\n");

  mag_filter.set(update_compass());


}

void loop() {

  if(WiFi.status() != WL_CONNECTED){
    connectToWiFi();
  }

  if (on_off_check()){
    return;
  }

  // === Get IMU information ===

  imu.readAccelData(imu.accelCount);


  float x, y, z;


  x = imu.accelCount[0] * imu.aRes;


  y = imu.accelCount[1] * imu.aRes;


  z = imu.accelCount[2] * imu.aRes;


  float acc_mag = sqrt(x * x + y * y + z * z);


  float avg_acc_mag = 1.0 / 3.0 * (acc_mag + old_acc_mag + older_acc_mag);


  older_acc_mag = old_acc_mag;
  old_acc_mag = acc_mag;


  step_reporter_fsm(avg_acc_mag);

  // === Pressure and Altitude Stuff ===

  temperature = bmp.readTemperature()*1.8 +32;
  pressure = bmp.readPressure();

  if(calibrated==0){ // only run calibration if we haven't done it yet
    calibrated=1;
  }
  else {// regular height reporting every few seconds after calibration==1
    if(millis() - ping_timer > PING) {
    altitude = bmp.readAltitude(altpressure);
    }

  }

  alt_mag = altitude;
  avg_alt_mag = (old_alt_mag + older_alt_mag)/2;
  delta = (alt_mag - avg_alt_mag);
  if (delta > 1){
    delta = 0;
  }
  else if (delta <= .035 && delta > 0){
    delta = 0;
  }
  else if (delta >= -.035 && delta < 0){
    delta = 0;
  }
  older_alt_mag = old_alt_mag;
  old_alt_mag = alt_mag;
  old_elev = elevation;

  if (old_elev-(elevation + delta) > .5){
    delta = 0;
  }
  elevation = elevation + delta;

  char num[] = "7";


  if (millis() - ping_timer > PING){


    post_reporter_fsm();

    char body[100]; //for body

    memset(request, 0, sizeof(request));
    memset(response, 0, sizeof(response));

    int offset = 0;
    sprintf(body,"user=%s&pressure=%f&altitude=%f&temperature=%f&hello=%s", main_user, pressure, elevation, temperature, num);//generate body, posting to User, 1 step
    int body_len = strlen(body); //calculate body length (for header reporting)
    offset += sprintf(request + offset,"POST http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py HTTP/1.1\r\n");
    offset += sprintf(request + offset,"Host: 608dev-2.net\r\n");
    offset += sprintf(request + offset,"Content-Type: application/x-www-form-urlencoded\r\n");
    offset += sprintf(request + offset,"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
    offset += sprintf(request + offset,"\r\n"); //new line from header to body
    offset += sprintf(request + offset, "%s", body); //body
    offset += sprintf(request + offset,"\r\n"); //new line
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);











    // === Compass and Geolocation Stuff ===

    set_geo_values();
    if (current_location.error) {
      return;
    }
    ping_timer = millis();
  }

  double heading = update_compass();
  double filtered_heading = mag_filter.step(heading);

  if (millis() - led_timer >= 1000) {
    double calc_angle = forward_azimuth - filtered_heading + theta0;
    int actual_angle = angle_in_range(calc_angle);

    set_LED_direction(actual_angle);
    led_timer = millis();
  }

  run_instrument();


  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}
