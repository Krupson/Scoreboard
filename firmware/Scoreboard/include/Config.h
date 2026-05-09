#pragma once

// Serial
#define SERIAL_BAUD_RATE        115200

// Network
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define SERVER_PORT             80
#define WIFI_SSID               "GruzowyScoreboard"
#define WIFI_PASSWORD           "GRUZ2137"

// Scoreboard LEDs
#define LEDS_PER_SEGMENT                    4
#define LEDS_PER_DIGIT                      (LEDS_PER_SEGMENT * 7)
#define NUM_LEDS                            (LEDS_PER_DIGIT * 4)

// Scoreboard timing
#define MIN_SCORE_CHANGE_TIME_MS            500UL
#define MIN_BRIGHTNESS_CHANGE_TIME_MS       20UL
