#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <mpu6050_esp32.h>
#include <stdio.h>
#include <string.h>
#include <Adafruit_BMP280.h>
#include "extra.h"


TFT_eSPI tft = TFT_eSPI();

Adafruit_BMP280 bmp; // I2C


float temperature; //variable for temperature
float pressure;    //variable for pressure
float altitude;    //variable for alititude, but tbh, this is pretty unreliable without calibration
const char USER[] = "waly";
uint32_t global_time = millis();

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 3500; //size of buffer to hold HTTP request

char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char brequest[IN_BUFFER_SIZE];
char bresponse[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request



        //auto-increasing value that goes from 0 to 4095 (approx) in steps of 100


//const char NETWORK[] = "MIT GUEST";
//const char PASSWORD[] = "";

const int BUTTON_TIMEOUT = 500; //button timeout in milliseconds

//
const char NETWORK[] = "MIT Guest";
const char PASSWORD[] = "";

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

int wifi_object_builder(char* object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t* mac_address) {
  char temp[300];//300 likely long enough for one wifi entry
  int len = sprintf(temp, "{\"macAddress\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"signalStrength\": %d,\"age\": 0,\"channel\": %d}",
                 mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5],
                 signal_strength, channel);
  if (len>os_len){
    return 0;
  }else{
    return sprintf(object_string,"%s",temp);
  }
}


uint32_t timer;

void setup() {
  
  Serial.begin(115200);               // Set up serial port

  //SET UP SCREEN:
  tft.init();  //init screen
  tft.setRotation(2); //adjust rotation
  tft.setTextSize(1); //default font size, change if you want
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_PINK, TFT_BLACK); //set color of font to hot pink foreground, black background

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

  while (!Serial);
  delay(1000);
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(), 16);
    esp_restart();
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  Serial.println("setup done");
  timer = millis();
  
}


//main body of code
void loop() {

  temperature = bmp.readTemperature()*1.8 +32;
  pressure = bmp.readPressure();
  altitude = bmp.readAltitude();
  
  char num[] = "7";
  // sprintf(request, "GET http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py?hello=%s HTTP/1.1\r\n", num);
  // strcat(request, "Host: 608dev-2.net\r\n"); //add more to the end
  // strcat(request, "\r\n"); //add blank line!
  // //submit to function that performs GET.  It will return output using response_buffer char array
  // do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  
  char body[100]; //for body
  sprintf(body,"user=%s&pressure=%f&altitude=%f&temperature=%f&hello=%s", USER, pressure, altitude, temperature,num);//generate body, posting to User, 1 step
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(brequest,"POST http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py HTTP/1.1\r\n");
  strcat(brequest,"Host: 608dev-2.net\r\n");
  strcat(brequest,"Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(brequest+strlen(brequest),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(brequest,"\r\n"); //new line from header to body
  strcat(brequest,body); //body
  strcat(brequest,"\r\n"); //new line
  do_http_request("608dev-2.net", brequest, bresponse, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);

  Serial.println(response);
  Serial.println(bresponse);
}
