#pragma once

// Serial
#define SERIAL_BAUD_RATE        115200

// Network
#define WIFI_CONNECT_TIMEOUT_MS 10000
#define SERVER_PORT             80
#define WIFI_SSID               "GruzowyScoreboard"
#define WIFI_PASSWORD           "GRUZ2137"

// Scoreboard hardware
#define PIN_DATA          D10
#define PIN_T1_ADD        D4
#define PIN_T1_SUB        D6
#define PIN_T2_ADD        D3
#define PIN_T2_SUB        D5
#define PIN_RESET_SCORE   D8
#define PIN_SWITCH_SIDES  D9

// Scoreboard LEDs
#define LEDS_PER_SEGMENT                    4
#define LEDS_PER_DIGIT                      (LEDS_PER_SEGMENT * 7)
#define NUM_LEDS                            (LEDS_PER_DIGIT * 4)

// Scoreboard timing
#define MIN_SCORE_CHANGE_TIME_MS            500UL
#define MIN_BRIGHTNESS_CHANGE_TIME_MS       20UL
