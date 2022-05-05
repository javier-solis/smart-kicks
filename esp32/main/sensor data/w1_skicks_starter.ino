#include <SPI.h>
#include <WiFi.h>
#include <mpu6050_esp32.h>
#include <stdio.h>
#include <string.h>
#include <Adafruit_BMP280.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

WiFiClientSecure client; 
WiFiClientSecure client2;

#include "extra.h"

Adafruit_BMP280 bmp; // I2C


float temperature; //variable for temperature
float pressure;    //variable for pressure
float altitude;    //variable for alititude, but tbh, this is pretty unreliable without calibration
const char USER[] = "waly";
uint32_t global_time = millis();

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 5000; //size of buffer to hold HTTP request

const uint16_t JSON_BODY_SIZE = 3000;
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char brequest[IN_BUFFER_SIZE];
char bresponse[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char json_body[JSON_BODY_SIZE];
char wapirequest[IN_BUFFER_SIZE];
char wapiresponse[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char wapijson_body[JSON_BODY_SIZE];

const char city[]="cambridge";
const char state[]="MA";
float altpressure = 1013.5;
const char WEATHER_API_KEY[] = "";  // paste your key here
const int MAX_APS = 5;


        //auto-increasing value that goes from 0 to 4095 (approx) in steps of 100


const char NETWORK[] = "MIT GUEST";
const char PASSWORD[] = "";

const int BUTTON_TIMEOUT = 500; //button timeout in milliseconds

//
//const char NETWORK[] = "EECS_Labs";
//const char PASSWORD[] = "";

int calibrated=0;

float altoffset;

float elevation = 0;
float old_elev = 0;

float older_alt_mag; 
float old_alt_mag; 
float alt_mag = 0; 
float avg_alt_mag = 0;
float delta = 0; 

const char* google_CERT = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
                      "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
                      "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
                      "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
                      "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
                      "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
                      "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
                      "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
                      "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
                      "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
                      "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
                      "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
                      "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
                      "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
                      "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
                      "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
                      "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
                      "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
                      "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
                      "-----END CERTIFICATE-----\n";

const char* Weather_CERT=\
                      "-----BEGIN CERTIFICATE-----\n"\
                      "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n" \
                      "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
                      "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n" \
                      "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n" \
                      "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n" \
                      "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n" \
                      "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n" \
                      "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n" \
                      "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n" \
                      "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n" \
                      "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n" \
                      "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n" \
                      "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n" \
                      "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n" \
                      "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n" \
                      "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n" \
                      "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n" \
                      "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n" \
                      "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" \
                      "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n" \
                      "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n" \
                      "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n" \
                      "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n" \
                      "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n" \
                      "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n" \
                      "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n" \
                      "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n" \
                      "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n" \
                      "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" \
                      "-----END CERTIFICATE-----\n";


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

  elevation = 8.0;
  
}

float alty = 0;
//main body of code
void loop() {

  temperature = bmp.readTemperature()*1.8 +32;
  pressure = bmp.readPressure();

  if(calibrated==0){// only run calibration if we haven't done it yet
    sprintf(wapirequest, "GET https://weather.visualcrossing.com/VisualCrossingWebServices/rest/services/timeline/cambridge, ma?unitGroup=metric&elements=pressure&include=current&key=QGXZBRL26UYTTTW9C67URDEFB&contentType=json HTTP/1.1\r\n");
    Serial.println("hi");
    strcat(wapirequest, "Host: weather.visualcrossing.com\r\n"); //add more to the end
    strcat(wapirequest, "\r\n"); //add blank line!
    Serial.println(wapirequest);
    //submit to function that performs GET.  It will return output using response_buffer char array
    Serial.print("check2");
    do_https_request("weather.visualcrossing.com", wapirequest, wapiresponse, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false, Weather_CERT);
    Serial.print("check");
    Serial.println(wapiresponse);

    DynamicJsonDocument doc1(1024);
    char* starting1 = strchr(wapiresponse, '{');
    char* ending1 = strrchr(wapiresponse, '}');
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
    altoffset = bmp.readAltitude(altpressure);
    wapiresponse[0] = 0;
        calibrated=1;
  }
  else{// regular height reporting every few seconds after calibration==1
    //Serial.println(altpressure);
    altitude = bmp.readAltitude(altpressure);
    delay(2000);
  }
  
  alt_mag = altitude;
  Serial.print(altitude);
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
  // sprintf(request, "GET http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py?hello=%s HTTP/1.1\r\n", num);
  // strcat(request, "Host: 608dev-2.net\r\n"); //add more to the end
  // strcat(request, "\r\n"); //add blank line!
  // //submit to function that performs GET.  It will return output using response_buffer char array
  // do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
  
  char body[100]; //for body
  sprintf(body,"user=%s&pressure=%f&altitude=%f&temperature=%f&hello=%s", USER, pressure, elevation, temperature,num);//generate body, posting to User, 1 step
  int body_len = strlen(body); //calculate body length (for header reporting)
  sprintf(brequest,"POST http://608dev-2.net/sandbox/sc/team44/w1_sk_server.py HTTP/1.1\r\n");
  strcat(brequest,"Host: 608dev-2.net\r\n");
  strcat(brequest,"Content-Type: application/x-www-form-urlencoded\r\n");
  sprintf(brequest+strlen(brequest),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
  strcat(brequest,"\r\n"); //new line from header to body
  strcat(brequest,body); //body
  strcat(brequest,"\r\n"); //new line
  do_http_request("608dev-2.net", brequest, bresponse, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);

  Serial.println(elevation);
}
