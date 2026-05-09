#include "scoreboard.h"

#define PIN_DATA D10

static const uint8_t DIGITS[] = {
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

static const CRGB COLORS[] = {
    CRGB::Red,
    CRGB::DarkOrange,
    CRGB::Green,
    CRGB::Cyan,
    CRGB::Blue,
    CRGB::Purple,
    CRGB::BlueViolet,
    CRGB::White,
};

void Scoreboard::begin() {
    FastLED.addLeds<WS2812B, PIN_DATA, GRB>(leds, NUM_LEDS);
    loadViewState();
    updateScoreBoard();
}

bool Scoreboard::canChangeScore() {
    return millis() - lastScoreChange > MIN_SCORE_CHANGE_TIME_MS;
}

bool Scoreboard::canChangeBrightness() {
    return millis() - lastBrightnessChange > MIN_BRIGHTNESS_CHANGE_TIME_MS;
}

void Scoreboard::afterScoreChange() {
    viewState.crc = calcViewStateChecksum();
    saveViewState();
    lastScoreChange = millis();
    updateScoreBoard();
}

void Scoreboard::onT1Add() {
    if (inConfigMode) {
        onT1ColorNext();
        return;
    }
    if (!canChangeScore()) return;
    if (viewState.team1 < 99) viewState.team1++;
    afterScoreChange();
}

void Scoreboard::onT1Sub() {
    if (inConfigMode) {
        onT1ColorPrev();
        return;
    }
    if (!canChangeScore()) return;
    if (viewState.team1 > 0) viewState.team1--;
    afterScoreChange();
}

void Scoreboard::onT2Add() {
    if (inConfigMode) {
        onT2ColorNext();
        return;
    }
    if (!canChangeScore()) return;
    if (viewState.team2 < 99) viewState.team2++;
    afterScoreChange();
}

void Scoreboard::onT2Sub() {
    if (inConfigMode) {
        onT2ColorPrev();
        return;
    }
    if (!canChangeScore()) return;
    if (viewState.team2 > 0) viewState.team2--;
    afterScoreChange();
}

void Scoreboard::onResetScore() {
    if (inConfigMode) return;
    if (!canChangeScore()) return;
    viewState.team1 = 0;
    viewState.team2 = 0;
    afterScoreChange();
}

void Scoreboard::onSwitchSides() {
    if (inConfigMode) return;
    if (!canChangeScore()) return;
    uint8_t t1Temp = viewState.team1;
    viewState.team1 = viewState.team2;
    viewState.team2 = t1Temp;

    uint8_t t1ColorTemp = viewState.team1Color;
    viewState.team1Color = viewState.team2Color;
    viewState.team2Color = t1ColorTemp;

    afterScoreChange();
}

void Scoreboard::onConfigToggle() {
    inConfigMode = !inConfigMode;
    updateScoreBoard();
}

void Scoreboard::onBrightnessUp() {
    if (!inConfigMode) return;
    if (!canChangeBrightness()) return;
    int newBrightness = viewState.brightness;
    newBrightness = min(newBrightness + 1, 255);
    lastBrightnessChange = millis();
    viewState.brightness = newBrightness;
    viewState.crc = calcViewStateChecksum();
    updateScoreBoard();
}

void Scoreboard::onBrightnessDown() {
    if (!inConfigMode) return;
    if (!canChangeBrightness()) return;
    int newBrightness = viewState.brightness;
    newBrightness = max(newBrightness - 1, 10);
    lastBrightnessChange = millis();
    viewState.brightness = newBrightness;
    viewState.crc = calcViewStateChecksum();
    updateScoreBoard();
}

void Scoreboard::onBrightnessSave() {
    viewState.crc = calcViewStateChecksum();
    saveViewState();
    updateScoreBoard();
}

void Scoreboard::onT1ColorNext() {
    if (!inConfigMode) return;
    uint8_t colorsLen = sizeof(COLORS) / sizeof(CRGB);
    int newColorIndex = (viewState.team1Color + 1) % colorsLen;
    if (newColorIndex == viewState.team2Color) {
        newColorIndex = (newColorIndex + 1) % colorsLen;
    }
    viewState.team1Color = newColorIndex;
    saveViewState();
    updateScoreBoard();
}

void Scoreboard::onT1ColorPrev() {
    if (!inConfigMode) return;
    uint8_t colorsLen = sizeof(COLORS) / sizeof(CRGB);
    int newColorIndex = (viewState.team1Color - 1);
    if (newColorIndex < 0) newColorIndex = colorsLen - 1;
    if (newColorIndex == viewState.team2Color) {
        newColorIndex = (newColorIndex - 1);
        if (newColorIndex < 0) newColorIndex = colorsLen - 1;
    }
    viewState.team1Color = newColorIndex;
    saveViewState();
    updateScoreBoard();
}

void Scoreboard::onT2ColorNext() {
    if (!inConfigMode) return;
    uint8_t colorsLen = sizeof(COLORS) / sizeof(CRGB);
    int newColorIndex = (viewState.team2Color + 1) % colorsLen;
    if (newColorIndex == viewState.team1Color) {
        newColorIndex = (newColorIndex + 1) % colorsLen;
    }
    viewState.team2Color = newColorIndex;
    saveViewState();
    updateScoreBoard();
}

void Scoreboard::onT2ColorPrev() {
    if (!inConfigMode) return;
    uint8_t colorsLen = sizeof(COLORS) / sizeof(CRGB);
    int newColorIndex = (viewState.team2Color - 1);
    if (newColorIndex < 0) newColorIndex = colorsLen - 1;
    if (newColorIndex == viewState.team1Color) {
        newColorIndex = (newColorIndex - 1);
    }
    if (newColorIndex < 0) newColorIndex = colorsLen - 1;
    viewState.team2Color = newColorIndex;
    saveViewState();
    updateScoreBoard();
}

void Scoreboard::updateScoreBoard() {
    if (inConfigMode) {
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
    uint8_t t1d1 = (team1Score / 10) % 10;
    uint8_t t2d0 = team2Score % 10;
    uint8_t t2d1 = (team2Score / 10) % 10;

    setDigit(0, t1d0);
    if (t1d1 == 0) {
        clearDigit(1);
    } else {
        setDigit(1, t1d1);
    }

    setDigit(2, t2d0);
    if (t2d1 == 0) {
        clearDigit(3);
    } else {
        setDigit(3, t2d1);
    }

    FastLED.setBrightness(viewState.brightness);
    FastLED.show();
}

void Scoreboard::setDigit(uint8_t index, uint8_t digit) {
    uint8_t colorsLen = sizeof(COLORS) / sizeof(CRGB);
    CRGB color;
    if (index <= 1) {
        color = COLORS[viewState.team1Color % colorsLen];
    } else {
        color = COLORS[viewState.team2Color % colorsLen];
    }

    int start = index * LEDS_PER_DIGIT;
    for (int segment = 0; segment < 7; segment++) {
        CRGB segmentColor;
        bool segmentState = (DIGITS[digit] >> segment) & 1;
        if (segmentState) {
            segmentColor = color;
        } else {
            segmentColor = CRGB::Black;
        }
        for (int ledOfSegment = 0; ledOfSegment < LEDS_PER_SEGMENT; ledOfSegment++) {
            int ledIndex = start + (segment * LEDS_PER_SEGMENT) + ledOfSegment;
            leds[ledIndex] = segmentColor;
        }
    }
}

void Scoreboard::clearDigit(uint8_t index) {
    int start = index * LEDS_PER_DIGIT;
    for (int x = start; x < start + LEDS_PER_DIGIT; x++) {
        leds[x] = CRGB::Black;
    }
}

uint8_t Scoreboard::calcViewStateChecksum() {
    return viewState.team1 ^ viewState.team2 ^ viewState.team1Color ^ viewState.team2Color ^ viewState.brightness ^ 0xAA;
}

void Scoreboard::saveViewState() {
    viewState.crc = calcViewStateChecksum();
    prefs.begin("scoreboard", false);
    prefs.putBytes("viewState", &viewState, sizeof(PersistentViewState));
    prefs.end();
}

void Scoreboard::loadViewState() {
    prefs.begin("scoreboard", true);
    size_t len = prefs.getBytes("viewState", &viewState, sizeof(PersistentViewState));
    prefs.end();

    bool failed = len != sizeof(PersistentViewState);
    failed = failed || viewState.crc != calcViewStateChecksum();

    if (failed) {
        viewState.team1 = 0;
        viewState.team2 = 0;
        viewState.team1Color = 0;
        viewState.team2Color = 2;
        viewState.brightness = 255;
    }
    viewState.crc = calcViewStateChecksum();
}
