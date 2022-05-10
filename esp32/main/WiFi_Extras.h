#ifndef WiFi_Extras_h
#define WiFi_Extras_h
#include "Arduino.h"

int wifi_object_builder(char* object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t* mac_address);
void print_WiFi_networks();

#endif
