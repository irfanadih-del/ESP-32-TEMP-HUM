/*
  ============================================================
  Title       : ESP32 DHT11 Temperature & Humidity Monitor
  Description : Read temperature and humidity from a DHT11
                sensor and display the values on a 16x2
                I2C LCD.
  Components  :
                - ESP32
                - DHT11 Temperature & Humidity Sensor
                - 16x2 I2C LCD
                - Jumper Wires

  Wiring:
  ------------------------------------------------------------
  DHT11
  VCC  -> ESP32 3.3V
  DATA -> ESP32 GPIO 4
  GND  -> ESP32 GND

  I2C LCD
  VCC  -> ESP32 5V
  GND  -> ESP32 GND
  SDA  -> ESP32 GPIO 21
  SCL  -> ESP32 GPIO 22

  Required Libraries:
  - DHT sensor library by Adafruit
  - Adafruit Unified Sensor
  - LiquidCrystal I2C by Frank de Brabander

  LCD Address:
  - 0x27
  ============================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// -------------------- Pin Configuration --------------------
#define DHT_PIN   4
#define DHT_TYPE  DHT22

// -------------------- LCD Configuration --------------------
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS    2

// -------------------- Sensor & LCD -------------------------
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);

  // Start DHT sensor
  dht.begin();

  // Start I2C
  Wire.begin(21, 22);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Startup message
  lcd.setCursor(0, 0);
  lcd.print("ESP32 DHT22");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(2000);

  // Clear LCD
  lcd.clear();
}

void loop() {
  // Read humidity and temperature
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if sensor reading failed
  if (isnan(humidity) || isnan(temperature)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    lcd.setCursor(0, 1);
    lcd.print("Check DHT22");

    Serial.println("Failed to read from DHT22 sensor!");

    delay(10000);
    return;
  }

  // Display temperature
  lcd.setCursor(0, 0);
  lcd.print("Temprature:");
  lcd.print(temperature, 1);
  lcd.print((char)223);
  lcd.print("C   ");

  // Display humidity
  lcd.setCursor(0, 1);
  lcd.print("Humidity  :");
  lcd.print(humidity, 1);
  lcd.print("%   ");

  // Print values to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.print(" C | Humidity: ");
  Serial.print(humidity, 1);
  Serial.println(" C ");

  // DHT22 recommended reading interval
  delay(10000);
}

