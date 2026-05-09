#include <Arduino.h>
#include <Config.h>
#include <OneButton.h>
#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include <common/scoreboard/scoreboard.h>

#define PIN_T1_ADD        D4
#define PIN_T1_SUB        D6
#define PIN_T2_ADD        D3
#define PIN_T2_SUB        D5
#define PIN_RESET_SCORE   D8
#define PIN_SWITCH_SIDES  D9

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

Scoreboard scoreboard;
OneButton team1AddButton;
OneButton team1SubButton;
OneButton team2AddButton;
OneButton team2SubButton;
OneButton resetScoreButton;
OneButton switchSidesButton;

NetworkServer server(SERVER_PORT);

void setupServer();
void handleServer();

void setup() {
    pinMode(WIFI_ENABLE, OUTPUT);
    digitalWrite(WIFI_ENABLE, LOW);
    delay(100);
    pinMode(WIFI_ANT_CONFIG, OUTPUT);
    digitalWrite(WIFI_ANT_CONFIG, HIGH);

    setupServer();

    team1AddButton.setup(PIN_T1_ADD, INPUT_PULLUP, true);
    team1SubButton.setup(PIN_T1_SUB, INPUT_PULLUP, true);
    team2AddButton.setup(PIN_T2_ADD, INPUT_PULLUP, true);
    team2SubButton.setup(PIN_T2_SUB, INPUT_PULLUP, true);
    resetScoreButton.setup(PIN_RESET_SCORE, INPUT_PULLUP, true);
    switchSidesButton.setup(PIN_SWITCH_SIDES, INPUT_PULLUP, true);

    team1AddButton.attachClick([]() { scoreboard.onT1Add(); });
    team1SubButton.attachClick([]() { scoreboard.onT1Sub(); });
    team2AddButton.attachClick([]() { scoreboard.onT2Add(); });
    team2SubButton.attachClick([]() { scoreboard.onT2Sub(); });

    resetScoreButton.attachDoubleClick([]() { scoreboard.onResetScore(); });
    switchSidesButton.attachDoubleClick([]() { scoreboard.onSwitchSides(); });

    switchSidesButton.attachLongPressStart([]() { scoreboard.onConfigToggle(); });

    team1AddButton.attachLongPressStart([]() { scoreboard.onBrightnessUp(); });
    team1AddButton.attachDuringLongPress([]() { scoreboard.onBrightnessUp(); });
    team1AddButton.attachLongPressStop([]() { scoreboard.onBrightnessSave(); });
    team1SubButton.attachLongPressStart([]() { scoreboard.onBrightnessDown(); });
    team1SubButton.attachDuringLongPress([]() { scoreboard.onBrightnessDown(); });
    team1SubButton.attachLongPressStop([]() { scoreboard.onBrightnessSave(); });

    scoreboard.begin();
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

void setupServer() {
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("Configuring access point...");

    if (!WiFi.softAP(ssid, password)) {
        log_e("Soft AP creation failed.");
        while (1);
    }
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    if (!MDNS.begin("scoreboard")) {
        Serial.println("Error setting up MDNS responder!");
        while (1) {
            delay(1000);
        }
    }
    MDNS.addService("_http", "_tcp", SERVER_PORT);
    Serial.println("mDNS responder started.");

    server.begin();
    Serial.println("Server started");
}

void handleServer() {
    NetworkClient client = server.accept();

    if (client) {
        Serial.println("New Client.");
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        client.print(R"(<meta name="viewport" content="width=device-width, initial-scale=1.0">)");
                        client.print(R"(<p style="text-align: center;"><a href="/T1" style="font-size: 100px;">T1</a></p>)");
                        client.print(R"(<br><br><br><br>)");
                        client.print(R"(<p style="text-align: center;"><a href="/T2" style="font-size: 100px;">T2</a></p>)");

                        client.println();
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }

                if (currentLine.endsWith("GET /T1")) {
                    scoreboard.onT1Add();
                }
                if (currentLine.endsWith("GET /T2")) {
                    scoreboard.onT2Add();
                }
            }
        }
        client.stop();
        Serial.println("Client Disconnected.");
    }
}
