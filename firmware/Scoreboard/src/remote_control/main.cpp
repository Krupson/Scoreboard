#include <Arduino.h>
#include <Config.h>
#include <OneButton.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define PIN_T1_ADD D8
#define PIN_T2_ADD D7

OneButton team1AddButton;
OneButton team2AddButton;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
//String baseUrl = "http://192.168.4.1";
String baseUrl = "http://scoreboard.local";

void onT1Add();

void onT2Add();

void reconnectWiFi();

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    WiFi.begin(ssid, password);
    WiFi.enableLongRange(true);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, HIGH);

    team1AddButton.setup(PIN_T1_ADD, INPUT_PULLUP, true);
    team2AddButton.setup(PIN_T2_ADD, INPUT_PULLUP, true);

    team1AddButton.attachClick(onT1Add);
    team2AddButton.attachClick(onT2Add);
}

void loop() {
    team1AddButton.tick();
    team2AddButton.tick();
    if (WiFi.status() != WL_CONNECTED) {
        reconnectWiFi();
    }
}

void reconnectWiFi() {
    Serial.println("Reconnecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.print(".");
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void callServer(const String &path) {
    if (WiFi.status() == WL_CONNECTED) {
        digitalWrite(LED_BUILTIN, LOW);
        HTTPClient http;

        String serverPath = baseUrl + path;

        Serial.print("Calling endpoint: ");
        Serial.println(serverPath);

        // Your Domain name with URL path or IP address with path
        http.begin(serverPath.c_str());

        // If you need Node-RED/server authentication, insert user and password below
        //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

        // Send HTTP GET request
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);
        } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
            for (int x = 0; x < 10; x++) {
                digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                delay(60);
            }
        }
        // Free resources
        http.end();
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
        Serial.println("WiFi Disconnected");
        reconnectWiFi();
    }
}

void onT1Add() {
    Serial.println("onT1Add");
    callServer("/T1");
}

void onT2Add() {
    Serial.println("onT2Add");
    callServer("/T2");
}
