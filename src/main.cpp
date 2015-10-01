#include "WProgram.h"

#include "Snooze.h"
#include "Adafruit_NeoPixel.h"

#define PIN_LED_STATUS 			13

#define PIN_LED_NEOPIXEL 		23
#define NEOPIXEL_COUNT 			8
#define NEOPIXEL_MODE 			NEO_GRB+NEO_KHZ800

#define PIN_VOLTAGE_SENSOR		A0
#define PIN_JUDGE_SENSOR        12

const uint8_t GAMMAS[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_COUNT, PIN_LED_NEOPIXEL, NEOPIXEL_MODE);

SnoozeBlock config;

uint16_t curDispVal;
uint32_t lastUpdateMillis;

void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    pixels.setPixelColor(n, GAMMAS[r], GAMMAS[g], GAMMAS[b]);
}

#define MAX_SPEED 2
void recalcVal(uint16_t sensorVal) {
	uint32_t curMillis = millis();
	uint32_t delta = curMillis - lastUpdateMillis;

	if (delta) {
		int32_t sensorDelta = sensorVal - curDispVal;
		int32_t deltaVal = delta * MAX_SPEED;

		if (sensorDelta > deltaVal) {
			sensorDelta = deltaVal;
		}
		if (sensorDelta < -deltaVal) {
			sensorDelta = -deltaVal;
		}

		curDispVal += sensorDelta;
		lastUpdateMillis = curMillis;
	}
}

#define MAX_VOLT 277
#define VOLTS_PER_PIXEL (MAX_VOLT/NEOPIXEL_COUNT)
void setVoltDisplay(uint32_t val) {
	uint8_t i;

	for (i=0; i < val/VOLTS_PER_PIXEL; i++) {
		setPixelColor(i, 0xff, 0, 0);
	}

	setPixelColor(i, 255*(val%VOLTS_PER_PIXEL)/VOLTS_PER_PIXEL, 0, 0);
	for (; i < NEOPIXEL_COUNT; i++) {
		setPixelColor(i, 0, 0, 0);
	}
	pixels.show();
}

extern "C" int main(void) {
	delay(100); // let it settle

	pinMode(PIN_LED_STATUS, OUTPUT);
	config.pinMode(PIN_JUDGE_SENSOR, INPUT_PULLUP, LOW); //pinMode(PIN_JUDGE_SENSOR, INPUT_PULLUP);

	pixels.begin(); pixels.show(); // clear pixels

	curDispVal = 0;
	lastUpdateMillis = millis();

	//analogReadRes ?

	while(1) {
		Snooze.sleep(config);
	    digitalWriteFast(PIN_LED_STATUS, HIGH);

	    uint8_t judgePresent;
		while((judgePresent = !digitalRead(PIN_JUDGE_SENSOR)) || curDispVal) {
			int16_t dispVal = 0;
			if (judgePresent) {
				dispVal = analogRead(PIN_VOLTAGE_SENSOR) - 692;
			}

			if (dispVal < 0) dispVal = 0;
			if (dispVal > 277) dispVal = 277;

			recalcVal(dispVal);
			setVoltDisplay(curDispVal);

			Snooze.idle();
		}

		digitalWriteFast(PIN_LED_STATUS, LOW);
	}

}

