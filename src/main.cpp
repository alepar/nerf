#include "WProgram.h"

#include "Adafruit_INA219.h"

Adafruit_INA219 ina219;

extern "C" int main(void)
{
	pinMode(13, OUTPUT);
	digitalWriteFast(13, HIGH);

	Serial.begin(115200);
	Serial.println("Hello, world!");

	ina219.begin();
	ina219.setCalibration_32V_20A();

	while(1) {
		float shuntvoltage = 0;
		float busvoltage = 0;
		float current_mA = 0;
		float loadvoltage = 0;

		shuntvoltage = ina219.getShuntVoltage_mV();
		busvoltage = ina219.getBusVoltage_V();
		current_mA = ina219.getCurrent_mA();
		loadvoltage = busvoltage + (shuntvoltage / 1000);

		Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
		Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
		Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
		Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
		Serial.println("");

		delay(1000);		
	}
}

