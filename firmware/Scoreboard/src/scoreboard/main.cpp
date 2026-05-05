#include <Arduino.h>
#include <Config.h>
#include <FastLED.h>
#include <OneButton.h>
#include <Preferences.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>

#define LEDS_PER_SEGMENT  4
#define LEDS_PER_DIGIT    (LEDS_PER_SEGMENT * 7)
#define NUM_LEDS          (LEDS_PER_DIGIT * 4)

#define PIN_DATA          D10
#define PIN_T1_ADD        D4
#define PIN_T1_SUB        D6
#define PIN_T2_ADD        D3
#define PIN_T2_SUB        D5
#define PIN_RESET_SCORE   D8
#define PIN_SWITCH_SIDES  D9

const unsigned long MIN_SCORE_CHANGE_TIME_MS = 500;
const unsigned long MIN_BRIGHTNESS_CHANGE_TIME_MS = 20;

const uint8_t DIGITS[] = {
  0b1110111,
  0b0010001,
  0b0111110,
  0b0111011,
  0b1011001,
  0b1101011,
  0b1101111,
  0b0110001,
  0b1111111,
  0b1111011,
};

const CRGB COLORS[] = {
  CRGB::Red,
  CRGB::DarkOrange,
  CRGB::Green,
  CRGB::Cyan,
  CRGB::Blue,
  CRGB::Purple,
  CRGB::BlueViolet,
  CRGB::White,
};

CRGB leds[NUM_LEDS];
OneButton team1AddButton;
OneButton team1SubButton;
OneButton team2AddButton;
OneButton team2SubButton;
OneButton resetScoreButton;
OneButton switchSidesButton;

unsigned long lastScoreChange = 0;
unsigned long lastBrightnessChange = 0;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

bool inConfigMode = false;

struct PersistentViewState {
  uint8_t team1 = 0;
  uint8_t team2 = 0;
  uint8_t team1Color = 0;
  uint8_t team2Color = 3;
  uint8_t brightness = 255;
  uint8_t crc = 0;
};

Preferences prefs;
PersistentViewState viewState;
NetworkServer server(SERVER_PORT);

void setupServer();
void handleServer();
void onT1Add();
void onT1Sub();
void onT2Add();
void onT2Sub();
void onResetScore();
void onSwitchSides();
void onConfigToggle();
void onBrightnessUp();
void onBrightnessDown();
void onBrightnessSave();
void loadViewState();
void updateScoreBoard();
void afterScoreChange();
void onT1ColorNext();
void onT1ColorPrev();
void onT2ColorNext();
void onT2ColorPrev();
uint8_t calcViewStateChecksum();
void saveViewState();
void setDigit(uint8_t index, uint8_t digit);
void clearDigit(uint8_t index);

void setup() {
  // switch to external antenna
  pinMode(WIFI_ENABLE, OUTPUT);
  digitalWrite(WIFI_ENABLE, LOW);
  delay(100);
  pinMode(WIFI_ANT_CONFIG, OUTPUT);
  digitalWrite(WIFI_ANT_CONFIG, HIGH);

  setupServer();

  FastLED.addLeds<WS2812B, PIN_DATA, GRB>(leds, NUM_LEDS);

  team1AddButton.setup(PIN_T1_ADD, INPUT_PULLUP, true);
  team1SubButton.setup(PIN_T1_SUB, INPUT_PULLUP, true);
  team2AddButton.setup(PIN_T2_ADD, INPUT_PULLUP, true);
  team2SubButton.setup(PIN_T2_SUB, INPUT_PULLUP, true);
  resetScoreButton.setup(PIN_RESET_SCORE, INPUT_PULLUP, true);
  switchSidesButton.setup(PIN_SWITCH_SIDES, INPUT_PULLUP, true);

  team1AddButton.attachClick(onT1Add);
  team1SubButton.attachClick(onT1Sub);
  team2AddButton.attachClick(onT2Add);
  team2SubButton.attachClick(onT2Sub);

  resetScoreButton.attachDoubleClick(onResetScore);
  switchSidesButton.attachDoubleClick(onSwitchSides);

  switchSidesButton.attachLongPressStart(onConfigToggle);

  team1AddButton.attachLongPressStart(onBrightnessUp);
  team1AddButton.attachDuringLongPress(onBrightnessUp);
  team1AddButton.attachLongPressStop(onBrightnessSave);
  team1SubButton.attachLongPressStart(onBrightnessDown);
  team1SubButton.attachDuringLongPress(onBrightnessDown);
  team1SubButton.attachLongPressStop(onBrightnessSave);

  loadViewState();

  // Test
  //viewState.team1 = 98;
  //viewState.team2 = 98;

  updateScoreBoard();
}

void setupServer() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);


  // Set mDNS
  if (!MDNS.begin("scoreboard")) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  // Add service to MDNS-SD
  MDNS.addService("_http", "_tcp", SERVER_PORT);
  Serial.println("mDNS responder started.");


  server.begin();

  Serial.println("Server started");
}

void handleServer() {
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print(R"(<meta name="viewport" content="width=device-width, initial-scale=1.0">)");
            client.print(R"(<p style="text-align: center;"><a href="/T1" style="font-size: 100px;">T1</a></p>)");
            client.print(R"(<br><br><br><br>)");
            client.print(R"(<p style="text-align: center;"><a href="/T2" style="font-size: 100px;">T2</a></p>)");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /T1")) {
          onT1Add();
        }
        if (currentLine.endsWith("GET /T2")) {
          onT2Add();
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void loop() {
  handleServer();
  team1AddButton.tick();
  team1SubButton.tick();
  team2AddButton.tick();
  team2SubButton.tick();
  resetScoreButton.tick();
  switchSidesButton.tick();
}

bool canChangeScore() {
  return millis() - lastScoreChange > MIN_SCORE_CHANGE_TIME_MS;
}

void onT1Add() {
  if(inConfigMode) {
    onT1ColorNext();
    return;
  }
  if(!canChangeScore()) return;
  if(viewState.team1 < 99) viewState.team1++;
  afterScoreChange();
}

void onT1Sub() {
  if(inConfigMode) {
    onT1ColorPrev();
    return;
  }
  if(!canChangeScore()) return;
  if(viewState.team1 > 0) viewState.team1--;
  afterScoreChange();
}

void onT2Add() {
  if(inConfigMode) {
    onT2ColorNext();
    return;
  }
  if(!canChangeScore()) return;
  if(viewState.team2 < 99) viewState.team2++;
  afterScoreChange();
}

void onT2Sub() {
  if(inConfigMode) {
    onT2ColorPrev();
    return;
  }
  if(!canChangeScore()) return;
  if(viewState.team2 > 0) viewState.team2--;
  afterScoreChange();
}

void onResetScore() {
  if(inConfigMode) return;
  if(!canChangeScore()) return;
  viewState.team1 = 0;
  viewState.team2 = 0;
  afterScoreChange();
}

void onSwitchSides() {
  if(inConfigMode) return;
  if(!canChangeScore()) return;
  uint8_t t1Temp = viewState.team1;
  viewState.team1 = viewState.team2;
  viewState.team2 = t1Temp;

  uint8_t t1ColorTemp = viewState.team1Color;
  viewState.team1Color = viewState.team2Color;
  viewState.team2Color = t1ColorTemp;

  afterScoreChange();
}

void afterScoreChange() {
  viewState.crc = calcViewStateChecksum();
  saveViewState();
  lastScoreChange = millis();
  updateScoreBoard();
}

bool canChangeBrightness() {
  return millis() - lastBrightnessChange > MIN_BRIGHTNESS_CHANGE_TIME_MS;
}

void onConfigToggle() {
  inConfigMode = !inConfigMode;
  updateScoreBoard();
}

void onBrightnessUp() {
  if(!inConfigMode) return;
  if(!canChangeBrightness()) return;
  int newBrightness = viewState.brightness;
  newBrightness = min(newBrightness + 1, 255);
  lastBrightnessChange = millis();
  viewState.brightness = newBrightness;
  viewState.crc = calcViewStateChecksum();
  updateScoreBoard();
}

void onBrightnessDown() {
  if(!inConfigMode) return;
  if(!canChangeBrightness()) return;
  int newBrightness = viewState.brightness;
  newBrightness = max(newBrightness - 1, 10);
  lastBrightnessChange = millis();
  viewState.brightness = newBrightness;
  viewState.crc = calcViewStateChecksum();
  updateScoreBoard();
}

void onBrightnessSave() {
  viewState.crc = calcViewStateChecksum();
  saveViewState();
  updateScoreBoard();
}

void onT1ColorNext() {
  if(!inConfigMode) return;
  uint8_t colorsLen = sizeof(COLORS)/sizeof(CRGB);
  int newColorIndex = (viewState.team1Color + 1) % colorsLen;
  if(newColorIndex == viewState.team2Color) {
    newColorIndex = (newColorIndex + 1) % colorsLen;
  }
  viewState.team1Color = newColorIndex;
  saveViewState();
  updateScoreBoard();
}

void onT1ColorPrev() {
  if(!inConfigMode) return;
  uint8_t colorsLen = sizeof(COLORS)/sizeof(CRGB);
  int newColorIndex = (viewState.team1Color - 1);
  if(newColorIndex < 0) newColorIndex = colorsLen - 1;
  if(newColorIndex == viewState.team2Color) {
    newColorIndex = (newColorIndex - 1);
    if(newColorIndex < 0) newColorIndex = colorsLen - 1;
  }
  viewState.team1Color = newColorIndex;
  saveViewState();
  updateScoreBoard();
}

void onT2ColorNext() {
  if(!inConfigMode) return;
  uint8_t colorsLen = sizeof(COLORS)/sizeof(CRGB);
  int newColorIndex = (viewState.team2Color + 1) % colorsLen;
  if(newColorIndex == viewState.team1Color) {
    newColorIndex = (newColorIndex + 1) % colorsLen;
  }
  viewState.team2Color = newColorIndex;
  saveViewState();
  updateScoreBoard();
}

void onT2ColorPrev() {
  if(!inConfigMode) return;
  uint8_t colorsLen = sizeof(COLORS)/sizeof(CRGB);
  int newColorIndex = (viewState.team2Color - 1);
  if(newColorIndex < 0) newColorIndex = colorsLen - 1;
  if(newColorIndex == viewState.team1Color) {
    newColorIndex = (newColorIndex - 1);
  }
  if(newColorIndex < 0) newColorIndex = colorsLen - 1;
  viewState.team2Color = newColorIndex;
  saveViewState();
  updateScoreBoard();
}

void updateScoreBoard() {
  if(inConfigMode) {
    setDigit(0, 0);
    setDigit(1, 0);
    setDigit(2, 0);
    setDigit(3, 0);
    FastLED.setBrightness(viewState.brightness);
    FastLED.show();
    return;
  }

  uint8_t team1Score = viewState.team1;
  uint8_t team2Score = viewState.team2;

  uint8_t t1d0 = team1Score % 10;
  uint8_t t1d1 = (team1Score/10) % 10;
  uint8_t t2d0 = team2Score % 10;
  uint8_t t2d1 = (team2Score/10) % 10;

  setDigit(0, t1d0);
  if(t1d1 == 0) {
    clearDigit(1);
  } else {
    setDigit(1, t1d1);
  }

  setDigit(2, t2d0);
  if(t2d1 == 0) {
    clearDigit(3);
  } else {
    setDigit(3, t2d1);
  }

  FastLED.setBrightness(viewState.brightness);
  FastLED.show();
}

void setDigit(uint8_t index, uint8_t digit) {
  CRGB color;
  uint8_t colorsLen = sizeof(COLORS)/sizeof(CRGB);
  if(index <= 1) {
    color = COLORS[viewState.team1Color % colorsLen];
  } else {
    color = COLORS[viewState.team2Color % colorsLen];
  }

  int start = index * LEDS_PER_DIGIT;
  for(int segment = 0; segment < 7; segment++) {
    CRGB segmentColor;
    bool segmentState = (DIGITS[digit] >> segment) & 1;
    if(segmentState) {
      segmentColor = color;
    } else {
      segmentColor = CRGB::Black;
    }
    for(int ledOfSegment = 0; ledOfSegment < LEDS_PER_SEGMENT; ledOfSegment++) {
      int ledIndex = start + (segment * LEDS_PER_SEGMENT) + ledOfSegment;
      leds[ledIndex] = segmentColor;
    }
  }
}

void clearDigit(uint8_t index) {
  int start = index * LEDS_PER_DIGIT;
  for(int x = start; x < start + LEDS_PER_DIGIT; x++) {
    leds[x] = CRGB::Black;
  }
}

uint8_t calcViewStateChecksum() {
    return viewState.team1 ^ viewState.team2 ^ viewState.team1Color ^ viewState.team2Color ^ viewState.brightness ^ 0xAA;
}

void saveViewState() {
    viewState.crc = calcViewStateChecksum();
    prefs.begin("scoreboard", false);
    prefs.putBytes("viewState", &viewState, sizeof(PersistentViewState));
    prefs.end();
}

void loadViewState() {
    prefs.begin("scoreboard", true);
    size_t len = prefs.getBytes("viewState", &viewState, sizeof(PersistentViewState));
    prefs.end();

    bool failed = len != sizeof(PersistentViewState);
    failed = failed || viewState.crc != calcViewStateChecksum();

    if(failed) {
      viewState.team1 = 0;
      viewState.team2 = 0;
      viewState.team1Color = 0;
      viewState.team2Color = 2;
      viewState.brightness = 255;
    }
    viewState.crc = calcViewStateChecksum();
}