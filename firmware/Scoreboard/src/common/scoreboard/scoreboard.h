#pragma once
#include <FastLED.h>
#include <Preferences.h>
#include "Config.h"


class Scoreboard {
public:
    void begin();
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

private:
    struct PersistentViewState {
        uint8_t team1 = 0;
        uint8_t team2 = 0;
        uint8_t team1Color = 0;
        uint8_t team2Color = 3;
        uint8_t brightness = 255;
        uint8_t crc = 0;
    };

    CRGB leds[NUM_LEDS];
    bool inConfigMode = false;
    unsigned long lastScoreChange = 0;
    unsigned long lastBrightnessChange = 0;
    Preferences prefs;
    PersistentViewState viewState;

    bool canChangeScore();
    bool canChangeBrightness();
    void afterScoreChange();
    void onT1ColorNext();
    void onT1ColorPrev();
    void onT2ColorNext();
    void onT2ColorPrev();
    void updateScoreBoard();
    void setDigit(uint8_t index, uint8_t digit);
    void clearDigit(uint8_t index);
    uint8_t calcViewStateChecksum();
    void saveViewState();
    void loadViewState();
};
