#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <string.h> //this is the line of code you are missing
#include <ButtonClass.h>

// Variables I declared:

const int LOOP_PERIOD = 5; // how often the void loop() should refresh, in ms
uint32_t primary_timer=0;

const int PING = 10*1000; // how often the pingLocation should refresh (every 10 sec)
uint32_t ping_timer=0;

char mainUser[60];
int n = sprintf(mainUser, "Javier");

bool powered_off = true;
int BUTTON = 45;
Button power_button(BUTTON);

TFT_eSPI tft = TFT_eSPI();

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

/* CONSTANTS */
//Prefix to POST request:
const char PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
const char SUFFIX[] = "]}"; //suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //don't change this and don't share this

const int MAX_APS = 5;

/* Global variables*/
WiFiClientSecure client; //global WiFiClient Secure object
WiFiClient client2; //global WiFiClient Secure object


// const char NETWORK[] = "MIT GUEST";
// const char PASSWORD[] = "";

// const char NETWORK[] = "608_24G";
// const char PASSWORD[] = "608g2020";

// using a hotspot here...
const char NETWORK[] = "SLP-F9FD71W0LMX0";
const char PASSWORD[] = "bqd1nuuv3nd8d";

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


char* SERVER = "googleapis.com";  // Server URL

void pingLocation() {
  int offset = sprintf(json_body, "%s", PREFIX);
  int n = WiFi.scanNetworks(); //run a new scan. could also modify to use original scan from setup so quicker (though older info)
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
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

    request[0] = '\0';
    response[0] = '\0';
    // Make a HTTP request:
    Serial.println("GETing geolocation");
    request[0] = '\0'; //set 0th byte to null
    offset = 0; //reset offset variable for sprintf-ing
    offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
    offset += sprintf(request + offset, "Host: googleapis.com\r\n");
    offset += sprintf(request + offset, "Content-Type: application/json\r\n");
    offset += sprintf(request + offset, "cache-control: no-cache\r\n");
    offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
    offset += sprintf(request + offset, "%s\r\n", json_body);
    do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    Serial.println("finished GETing geolocation");

    char* start = strchr(response,'{');
    char* end = strrchr(response,'}');
    *(end + 1) = NULL;

    DynamicJsonDocument doc(5000);
    DeserializationError error = deserializeJson(doc, start);

    if (error) {
      Serial.print(F("deserializeJson failed"));
      return;
    }

    double latitude = doc["location"]["lat"];
    double longitude = doc["location"]["lng"];

    // Make a POST request:

    char json_body[300]; // Should be plenty
    sprintf(json_body, "user=%s&lat=%f&lon=%f", mainUser, latitude, longitude);

    char buffer[1000];
    int n = sprintf(buffer, "POSTing this to our server: %s", json_body);
    Serial.println(buffer);

    // Clearing it just for good practice
    request[0] = '\0';
    response[0] = '\0';
    offset = 0;

    offset += sprintf(request + offset, "POST /sandbox/sc/javsolis/final-project/main.py HTTP/1.1\r\n");
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
    offset += sprintf(request + offset, "Content-Type: application/x-www-form-urlencoded\r\n");
    offset += sprintf(request + offset, "Content-Length: %d\r\n", strlen(json_body));
    offset += sprintf(request + offset, "\r\n");
    offset += sprintf(request + offset, "%s", json_body);
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    Serial.println("finished POSTing to our server");
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(BUTTON, INPUT_PULLUP);

  //SET UP SCREEN:
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size, change if you want
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_PINK, TFT_BLACK); //set color of font to hot pink foreground, black background

  delay(100); //wait a bit (100 ms)

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
  delay(500); //wait a bit (100 ms)

  //if using regular connection use line below:
  WiFi.begin(NETWORK, PASSWORD);
  //if using channel/mac specification for crowded bands use the following:
  // WiFi.begin(NETWORK, PASSWORD, channel, bssid);

  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count < 10) {
    delay(1000);
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
}

//main body of code
void loop() {
  if(powered_off){
    if(power_button.read()==1){
      powered_off==false;
      Serial.println("Powered on");
    }
    // trying out some light sleep function here?????
    return;
  }

  if(power_button.read()==2){
    powered_off=true;

    Serial.println("Powered off");
  }

  if (millis() - ping_timer > PING){
    pingLocation();
    ping_timer=millis();
  } 
    
  while (millis() - primary_timer < LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}