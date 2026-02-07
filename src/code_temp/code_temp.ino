#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int sensorPin = A0;

// OLED parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32   // use 64 if you have a 128x64 display
#define OLED_RESET    -1   // reset not used on I2C OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // I2C address 0x3C is typical
    // If display is not found, stay here
    while (true) {
      Serial.println("SSD1306 allocation failed");
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Temp module start");
  display.display();
  delay(1000);
}

void loop() {
  // 1. Read ADC value
  int raw = analogRead(sensorPin);

  // 2. Convert to amplifier output voltage (3..5 V)
  float Vout = raw * (5.07 / 1023.0);

  // 3. Inverse amplifier formula: get original sensor voltage
  float Vsens = (Vout / 5.0) - 0.5;

  // 4. Sensor calibration: convert Vsens to temperature
  float temperature = (Vsens - 0.051) / 0.008;



  // ----- Serial output (for debugging) -----
  Serial.print("Vout = ");
  Serial.print(Vout, 3);
  Serial.print(" V | Temperature = ");
  Serial.print(temperature, 2);
  Serial.println(" C");

  // ----- OLED output -----
  display.clearDisplay();
  display.setTextSize(2);

  display.setCursor(0, 0);
  display.print("V:");
  display.print(Vout, 2);
  display.println("V");

  display.setCursor(0, 18); 
  display.print("T:");
  display.print(temperature, 1);
  display.println("C");

  display.display();

  delay(1000);  
}