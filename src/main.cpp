#include "WProgram.h"

#include "Adafruit_NeoPixel.h"

#define PIN_LED_STATUS 			13

#define PIN_LED_NEOPIXEL 		23
#define NEOPIXEL_COUNT 			8
#define NEOPIXEL_MODE 			NEO_GRB+NEO_KHZ800

#define PIN_JUDGE_SENSOR        22

#define PIN_RELAY_ON 			20
#define PIN_RELAY_OFF 			21

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

uint16_t curDispVal;
uint32_t lastUpdateMillis;
uint32_t lastWriteBucket = 0;

void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
    pixels.setPixelColor(n, GAMMAS[r], GAMMAS[g], GAMMAS[b]);
}

#define MAX_SPEED 1
void recalcVal(int16_t sensorVal) {
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

#define MIN_SENSOR 500  // 3.1v on 3s
#define MAX_SENSOR 708  // 4.2v on 3s
#define MAX_VOLT (MAX_SENSOR-MIN_SENSOR)
#define VOLTS_PER_PIXEL (MAX_VOLT/NEOPIXEL_COUNT)
void setVoltDisplay(uint32_t val) {
	uint8_t i;

	for (i=0; i < val/VOLTS_PER_PIXEL; i++) {
		setPixelColor(i, 0xff, 0, 0);
	}

	setPixelColor(i, 255*(val%VOLTS_PER_PIXEL)/VOLTS_PER_PIXEL, 0, 0);
	for (i++; i < NEOPIXEL_COUNT; i++) {
		setPixelColor(i, 0, 0, 0);
	}
	pixels.show();
}

extern "C" int main(void) {
	delay(200); // let it settle

	Serial.begin(115200);

	pinMode(PIN_LED_STATUS, OUTPUT);
    digitalWriteFast(PIN_LED_STATUS, HIGH);
	
	pinMode(PIN_JUDGE_SENSOR, INPUT);

	pinMode(PIN_RELAY_OFF, OUTPUT);
	digitalWrite(PIN_RELAY_OFF, 0);
	pinMode(PIN_RELAY_ON, OUTPUT);
	digitalWrite(PIN_RELAY_ON, 1);

	pixels.begin(); pixels.show(); // clear pixels

	curDispVal = 0;
	lastUpdateMillis = millis();

	while(1) {
		int16_t dispVal = analogRead(PIN_JUDGE_SENSOR) - MIN_SENSOR;
		uint32_t curBucket = millis() / 500;
		if (curBucket != lastWriteBucket) {
			lastWriteBucket = curBucket;
			Serial.println(analogRead(PIN_JUDGE_SENSOR));
		}

		if (dispVal < 0) dispVal = 0;
		if (dispVal > MAX_VOLT) dispVal = MAX_VOLT;

		recalcVal(dispVal);
		setVoltDisplay(curDispVal);

		if (!dispVal && !curDispVal) {
			digitalWrite(PIN_RELAY_ON, 0);
			digitalWrite(PIN_RELAY_OFF, 1);
		}
	}

}
