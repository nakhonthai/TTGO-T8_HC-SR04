// Ultrasonic Sensor HC-SR04 With TTGO-T8
// 
/*
    Name:       Ultrasonic.ino
    Created:	15/11/2561 10:13:17
    Author:     SOMKIAT\somkiat
*/

// Define User Types below here or use a .h file
//
#include <OLEDDisplayUi.h>
#include <OLEDDisplayFonts.h>
#include <OLEDDisplay.h>
#include <Wire.h>
#include <SH1106Wire.h>
#include <SH1106.h>

// Define Function Prototypes that use User Types below here or use a .h file
//

#ifdef __cplusplus
extern "C" {
#endif
	uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

// Define Functions below here or use other .ino or cpp files
//

// Initialize the OLED display using Wire library
// SSD1306  display(0x3c, 5, 4);
SH1106 display(0x3c, 5, 4);//5,4 //D1,D2

// defines pins numbers
const int trigPin = 25;
const int echoPin = 26;

// defines variables
long duration;
float distance;
float temperature=29.0;
float v;

// Pins
#define THERMISTORPIN 32

// Series resistor value
#define SERIESRESISTOR 10000
// Number of samples to average
#define SAMPLERATE 10
// Nominal resistance at 25C
#define THERMISTORNOMINAL 10000
// Nominal temperature in degrees
#define TEMPERATURENOMINAL 25
// Beta coefficient
//B57861S103F40 NTC 10K B=3988 +-1%
#define BCOEFFICIENT 3988


float getTemp() {
	uint16_t thermalSamples[SAMPLERATE],tmp16;
	double average,kelvin, resistance, celsius;
	int i;

	// Collect SAMPLERATE (default 5) samples
		for (i = 0; i<SAMPLERATE; i++) {
			thermalSamples[i] = analogRead(THERMISTORPIN);
			delay(10);
		}

	// Calculate the average value of the samples
	tmp16 = 0;
	for (i = 0; i<SAMPLERATE; i++) {
		tmp16 += thermalSamples[i];
	}
	average = (double)tmp16/SAMPLERATE;

		// Convert to resistance
		resistance = (double)(4095 / average - 1);
		resistance = (double)(SERIESRESISTOR / resistance);


		/*
		* Use Steinhart equation (simplified B parameter equation) to convert resistance to kelvin
		* B param eq: T = 1/( 1/To + 1/B * ln(R/Ro) )
		* T  = Temperature in Kelvin
		* R  = Resistance measured
		* Ro = Resistance at nominal temperature
		* B  = Coefficent of the thermistor
		* To = Nominal temperature in kelvin
		*/
		kelvin = resistance / THERMISTORNOMINAL;                              // R/Ro
	kelvin = log(kelvin);                                                                   // ln(R/Ro)
	kelvin = (1.0 / BCOEFFICIENT) * kelvin;                                   // 1/B * ln(R/Ro)
	kelvin = (1.0 / (TEMPERATURENOMINAL + 273.15)) + kelvin;  // 1/To + 1/B * ln(R/Ro)
	kelvin = 1.0 / kelvin;	 // 1/( 1/To + 1/B * ln(R/Ro)
		// Convert Kelvin to Celsius
		celsius = kelvin - 273.15;

		// Send the value back to be displayed
		return (float)celsius;
}

float avgDistanceRead(uint16_t samples = 8) {
	float sum = 0.0F;
	int sampleErr = 0;
	long sumDuration = 0;
	// Calculating the speed of sound in temperature from v=331m/s +0.6m/s/C*T
	v = 331.0F + (float)(0.6*temperature);
	for (int x = 0; x<samples; x++) {
		// Clears the trigPin
		digitalWrite(trigPin, LOW);
		delayMicroseconds(2);
		// Sets the trigPin on HIGH state for 10 micro seconds
		digitalWrite(trigPin, HIGH);
		delayMicroseconds(10);
		digitalWrite(trigPin, LOW);
		// Reads the echoPin, returns the sound wave travel time in microseconds
		duration = pulseIn(echoPin, HIGH, 30000);
		if (duration > 0) {
			sumDuration += duration;
		}
		else {
			sampleErr++;
		}
	}
	// Calculating the distance
	sumDuration /= (samples-sampleErr);
	distance = (float)sumDuration*(v / 10000);
	distance /= 2;
	return distance;
}

// The setup() function runs once each time the micro-controller starts
void setup()
{
	analogReadResolution(12); //12 bits
	analogSetAttenuation(ADC_11db);  //For all pins
	//analogSetPinAttenuation(A18, ADC_0db); //0db attenuation on pin A18
	//analogSetPinAttenuation(A19, ADC_2_5db); //2.5db attenuation on pin A19
	//analogSetPinAttenuation(A6, ADC_6db); //6db attenuation on pin A6
	//analogSetPinAttenuation(A7, ADC_11db); //11db attenuation on pin A7

	pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
	pinMode(echoPin, INPUT); // Sets the echoPin as an Input

	Serial.begin(9600);
	Serial.println();

	// Initialising the UI will init the display too.
	display.init();

	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_16);
}

// Add the main program code into the continuous loop() function
void loop()
{
	temperature= getTemp();
	distance = avgDistanceRead(SAMPLERATE);

	// Prints the distance on the Serial Monitor
	Serial.print("Distance: ");
	Serial.println(distance);

	display.clear();
	display.setTextAlignment(TEXT_ALIGN_CENTER);
	display.setFont(ArialMT_Plain_10);
	display.drawString(64, 2, "DISTANCE");
	display.drawString(32, 37, "Temperature");
	display.drawString(32, 48, String(temperature,2)+"C");
	display.drawString(96, 37, "Snd Speed");
	display.drawString(96, 48, String(v,1)+"m/s");
	display.setFont(ArialMT_Plain_24);
	display.drawRect(0, 0, 128, 64);
	display.drawLine(0, 35, 128, 35);
	display.drawLine(64, 35, 64, 64);
	if (distance >= 0) {
		display.drawString(64, 10, String(distance, 1) + " cm");
	}
	else {
		display.drawString(64, 10, "Error!");
	}
	display.display();
	delay(100);
}
