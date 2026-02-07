#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C   // or 0x3D if needed

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
const int tempPin   = A0;
const int tempLedPin = 4;   // LED for temperature sensor
const int humPin    = 3;
const int humLedPin = 5;   // LED for humidity sensor
const int trigPin   = 9;
const int echoPin   = 10;
const int buzzerPin = 8;
const int ldrPin    = A1;   // same pin used for light sensor
const int lightLedPin = 6;   // LED for light sensor
const int resetBtnPin = 2;   // RESET button (active LOW, using INPUT_PULLUP)
const int powerBtnPin = 7;   // POWER button (active LOW, INPUT_PULLUP)

// Constants
const float Vref = 5.07; // temperature
const float TEMP_MIN_C = 15.0;
const float TEMP_MAX_C = 30.0;
const float SUDDEN_CHANGE_C = 3.0;
const float STUCK_DELTA_C   = 0.2;
const int   STUCK_COUNT_N   = 10;

const float M_ADC = -0.884784f;   // LDR , lux per ADC count
const float C_LUX = 919.134f;     // lux intercept
const float HUM_MIN_PCT = 20.0;
const float HUM_MAX_PCT = 90.0;
const float HUM_SUDDEN_DELTA = 10.0;  // % jump per sample
const float HUM_STUCK_DELTA  = 0.5;   // % minimal change considered "no change"
const int   HUM_STUCK_COUNT_N = 10;   // 10 cycles ~ 5 seconds (with delay(500))

const int   LIGHT_ADC_MIN = 5;        // near GND
const int   LIGHT_ADC_MAX = 1018;     // near VCC
const float LUX_MIN = 100.0;          // greenhouse lower limit
const float LUX_MAX = 9000.0;         // greenhouse upper limit
const int   LIGHT_SUDDEN_DELTA = 200; // ADC jump per sample
const int   LIGHT_STUCK_DELTA  = 3;   // minimal ADC change
const int   LIGHT_STUCK_COUNT_N = 10; // ~5 seconds

const float ULTRA_MIN_CM = 2.0; // MOTION
const float ULTRA_MAX_CM = 400.0;
const float ULTRA_SUDDEN_DELTA_CM = 15.0;
const float ULTRA_STUCK_DELTA_CM  = 0.5;   // tolerance for "no change"
const int   ULTRA_STUCK_COUNT_N   = 10;    // ~5 seconds (delay(500))

// Variables
float frequency = 0.0;
float humidity  = -1.0;
unsigned long highTime = 0;
unsigned long lowTime  = 0;
unsigned long period   = 0;
float prevTempC = NAN;
int stuckCounter = 0;
float prevHumidity = NAN;
int humStuckCounter = 0;
int prevRawL = -1;
int lightStuckCounter = 0;
float prevDistance = -1.0;
int ultraStuckCounter = 0;
bool ultrasonicError = false;
bool lastResetBtnState = HIGH;
unsigned long lastResetDebounceMs = 0;
const unsigned long RESET_DEBOUNCE_MS = 50;
bool systemEnabled = true;
bool lastPowerBtnState = HIGH;
unsigned long lastPowerDebounceMs = 0;
const unsigned long POWER_DEBOUNCE_MS = 50;

long duration = 0;
float distanceCm = -1.0;

// Function for TEMPERATURE error detection
bool checkTempErrors(float temperatureC) {
  bool errorDetected = false;

  // Out-of-range temperature (greenhouse limits)
  if (temperatureC < TEMP_MIN_C || temperatureC > TEMP_MAX_C || isnan(temperatureC)) {
    Serial.println("Error: Sensor value out of range!");
    errorDetected = true;
  }

  // Sudden drop or spike detection
  if (!errorDetected && !isnan(prevTempC)) {
    if (fabs(temperatureC - prevTempC) > SUDDEN_CHANGE_C) {
      Serial.println("Error: Sudden change in sensor value!");
      errorDetected = true;
    }
  }

  // Sensor stuck at constant value
  if (!errorDetected && !isnan(prevTempC)) {
    if (fabs(temperatureC - prevTempC) < STUCK_DELTA_C) {
      stuckCounter++;
    } else {
      stuckCounter = 0;
    }

    if (stuckCounter >= STUCK_COUNT_N) {
      Serial.println("Error: Sensor stuck at constant value!");
      errorDetected = true;
    }
  } else {
    stuckCounter = 0;
  }

  // LED indication for temperature sensor
  digitalWrite(tempLedPin, errorDetected ? HIGH : LOW);

  // Store previous temperature value
  prevTempC = temperatureC;

  return errorDetected;
}

// Function for HUMIDITY error detection
bool checkHumidityErrors(float humidityPct, unsigned long highTimeUs, unsigned long lowTimeUs) {
  bool errorDetected = false;

  // Error 1: No signal / disconnected (timeout)
  if (highTimeUs == 0 || lowTimeUs == 0 || isnan(humidityPct)) {
    Serial.println("Error: No humidity signal!");
    errorDetected = true;
  }

  // Error 2: Out of range humidity (greenhouse limits)
  if (!errorDetected) {
    if (humidityPct < HUM_MIN_PCT || humidityPct > HUM_MAX_PCT) {
      Serial.println("Error: Humidity value out of range!");
      errorDetected = true;
    }
  }

  // Error 3: Sudden change (noise / glitch)
  if (!errorDetected && !isnan(prevHumidity)) {
    if (fabs(humidityPct - prevHumidity) > HUM_SUDDEN_DELTA) {
      Serial.println("Error: Sudden change in humidity value!");
      errorDetected = true;
    }
  }

  // Error 4: Stuck at constant value
  if (!errorDetected && !isnan(prevHumidity)) {
    if (fabs(humidityPct - prevHumidity) < HUM_STUCK_DELTA) humStuckCounter++;
    else humStuckCounter = 0;

    if (humStuckCounter >= HUM_STUCK_COUNT_N) {
      Serial.println("Error: Humidity sensor stuck at constant value!");
      errorDetected = true;
    }
  } else {
    humStuckCounter = 0;
  }

  // LED indication for humidity sensor
  digitalWrite(humLedPin, errorDetected ? HIGH : LOW);

  // Store previous humidity value
  prevHumidity = humidityPct;

  return errorDetected;
}

// Function for LIGHT error detection
bool checkLightErrors(int rawL, float luxValue) {
  bool errorDetected = false;

  // Error 1: Disconnected or short circuit
  if (rawL <= LIGHT_ADC_MIN || rawL >= LIGHT_ADC_MAX) {
    Serial.println("Error: Light sensor disconnected or shorted!");
    errorDetected = true;
  }

  // Error 2: Out of expected light range
  if (!errorDetected) {
    if (luxValue < LUX_MIN || luxValue > LUX_MAX) {
      Serial.println("Error: Light level out of range!");
      errorDetected = true;
    }
  }

  // Error 3: Sudden change (noise)
  if (!errorDetected && prevRawL >= 0) {
    if (abs(rawL - prevRawL) > LIGHT_SUDDEN_DELTA) {
      Serial.println("Error: Sudden change in light level!");
      errorDetected = true;
    }
  }

  // Error 4: Sensor stuck
  if (!errorDetected && prevRawL >= 0) {
    if (abs(rawL - prevRawL) < LIGHT_STUCK_DELTA) {
      lightStuckCounter++;
    } else {
      lightStuckCounter = 0;
    }

    if (lightStuckCounter >= LIGHT_STUCK_COUNT_N) {
      Serial.println("Error: Light sensor stuck at constant value!");
      errorDetected = true;
    }
  } else {
    lightStuckCounter = 0;
  }

  // LED indication
  digitalWrite(lightLedPin, errorDetected ? HIGH : LOW);

  // Store previous ADC value
  prevRawL = rawL;

  return errorDetected;
}


// Function for MOTION error detection
bool checkUltrasonicErrors(float distanceCmValue) {
  bool errorDetected = false;

  // Error 2: Out of valid range (HC-SR04 typical range)
  if (distanceCmValue < ULTRA_MIN_CM || distanceCmValue > ULTRA_MAX_CM) {
    Serial.println("Error: Distance value out of valid range!");
    errorDetected = true;
  }

  // Error 3: Sudden spike detection
  if (!errorDetected && prevDistance > 0) {
    if (fabs(distanceCmValue - prevDistance) > ULTRA_SUDDEN_DELTA_CM) {
      Serial.println("Error: Sudden spike detected in ultrasonic reading!");
      errorDetected = true;
    }
  }

  // Error 4: Stuck / flat-line detection (with tolerance)
  if (!errorDetected && prevDistance > 0) {
    if (fabs(distanceCmValue - prevDistance) < ULTRA_STUCK_DELTA_CM) {
      ultraStuckCounter++;
      if (ultraStuckCounter >= ULTRA_STUCK_COUNT_N) {
        Serial.println("Error: Ultrasonic sensor flat-line detected!");
        errorDetected = true;
      }
    } else {
      ultraStuckCounter = 0;
    }
  } else {
    ultraStuckCounter = 0;
  }

  // Store previous value
  prevDistance = distanceCmValue;

  return errorDetected;
}

// Function for RST button
void resetAllErrors() {
  // Turn off all error LEDs
  digitalWrite(tempLedPin, LOW);
  digitalWrite(humLedPin, LOW);
  digitalWrite(lightLedPin, LOW);

  // Clear error flags
  ultrasonicError = false;

  // Reset history values (prevents false sudden/stuck after reset)
  prevTempC = NAN;
  prevHumidity = NAN;
  prevRawL = -1;
  prevDistance = -1.0;

  // Reset counters
  stuckCounter = 0;
  humStuckCounter = 0;
  lightStuckCounter = 0;
  ultraStuckCounter = 0;

  // Safety: turn off buzzer
  digitalWrite(buzzerPin, LOW);
}

// Function for ON PWR button
void powerOffSystem() {
  systemEnabled = false;

  // Turn off outputs
  digitalWrite(tempLedPin, LOW);
  digitalWrite(humLedPin, LOW);
  digitalWrite(lightLedPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // Turn off OLED
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

// Function for OFF PWR button
void powerOnSystem() {
  systemEnabled = true;

  // Turn on OLED
  display.ssd1306_command(SSD1306_DISPLAYON);

  // Reset error states on power-up
  resetAllErrors();
}

void setup() {
  Serial.begin(9600);

  pinMode(humPin, INPUT);
  pinMode(tempLedPin, OUTPUT);
  digitalWrite(tempLedPin, LOW);
  pinMode(trigPin, OUTPUT);
  pinMode(humLedPin, OUTPUT);
  digitalWrite(humLedPin, LOW);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  pinMode(lightLedPin, OUTPUT);
  digitalWrite(lightLedPin, LOW);
  pinMode(resetBtnPin, INPUT_PULLUP);
  pinMode(powerBtnPin, INPUT_PULLUP);
  
  

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    while (1) {
      Serial.println("OLED fail");
      delay(1000);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("System ready");
  display.display();
  delay(1000);
}

void loop() {
  bool reading = digitalRead(resetBtnPin);
if (reading != lastResetBtnState) {
  lastResetDebounceMs = millis();
}
if ((millis() - lastResetDebounceMs) > RESET_DEBOUNCE_MS) {
  // Button pressed (active LOW)
  if (lastResetBtnState == HIGH && reading == LOW) {
    resetAllErrors();
  }
}
lastResetBtnState = reading;

bool powerReading = digitalRead(powerBtnPin);
if (powerReading != lastPowerBtnState) {
  lastPowerDebounceMs = millis();
}
if ((millis() - lastPowerDebounceMs) > POWER_DEBOUNCE_MS) {
  // Button pressed (active LOW)
  if (lastPowerBtnState == HIGH && powerReading == LOW) {
    if (systemEnabled) {
      powerOffSystem();
    } else {
      powerOnSystem();
    }
  }
}
lastPowerBtnState = powerReading;

if (!systemEnabled) {
  delay(100);
  return;
}


  // -------- Temperature ----------
  int rawT = analogRead(tempPin);
  float Vout = rawT * (Vref / 1023.0);
  float Vsens = (Vout / 5.0) - 0.5;
  float temperature = (Vsens - 0.051) / 0.008;
  checkTempErrors(temperature);

  // -------- Humidity ----------
  highTime = pulseIn(humPin, HIGH, 200000);
  lowTime  = pulseIn(humPin, LOW, 200000);
  if (highTime > 0 && lowTime > 0) {
    period = highTime + lowTime;
    frequency = 1000000.0 / period;
    humidity  = -0.0689655 * frequency + 503.96575;
  } else {
    humidity = -1.0;
  }
  checkHumidityErrors(humidity, highTime, lowTime);

  // -------- Distance ----------
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

duration = pulseIn(echoPin, HIGH, 30000UL);

// Error 1: No echo received (timeout)
if (duration == 0) {
  ultrasonicError = true;
  distanceCm = -1.0;
  Serial.println("Error: No echo received (sensor disconnected or out of range)");
} else {
  distanceCm = duration * 0.034f / 2.0f;
  ultrasonicError = checkUltrasonicErrors(distanceCm);
}

// Buzzer warning only when there is NO sensor error
if (!ultrasonicError && distanceCm > 0 && distanceCm <= 5.0) {
  digitalWrite(buzzerPin, HIGH);
} else {
  digitalWrite(buzzerPin, LOW);
}

  // -------- Light Sensor (ADC + Lux) ----------
  int rawL = analogRead(ldrPin);
  float lightLevel = rawL;
  float lux = M_ADC * rawL + C_LUX;
  checkLightErrors(rawL, lux);
  if (lux < 0) lux = 0;
  if (lux > 9999) lux = 9999;

  // -------- Serial Output ----------
  Serial.print(humidity,1);    Serial.print(",");
  Serial.print(temperature,1); Serial.print(",");
  Serial.print(distanceCm,1);  Serial.print(",");
  Serial.print(lightLevel);    Serial.print(",");
  Serial.println(lux,1);

  // -------- Display ----------
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print("Hum: ");
  if (humidity < 0) display.print("N/A");
  else { display.print(humidity, 1); display.print("%"); }

  display.setCursor(0, 10);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.print("C");

  display.setCursor(0, 20);
  display.print("Dist: ");
  if (distanceCm < 0) display.print("N/A");
  else display.print(distanceCm, 1);
  display.print("cm");

  display.setCursor(70, 20);
  display.print("Lux:");
  display.print((int)lux);

  display.display();
  delay(500);
}

