/*
  ESP32 + DHT22 + LCD 16x2 I2C + Web Server

  Wiring:
  DHT22 : VCC -> 3.3V | DATA -> GPIO 4 | GND -> GND
  LCD   : VCC -> 5V   | GND  -> GND    | SDA -> GPIO 21 | SCL -> GPIO 22

  Library:
  - DHT sensor library (Adafruit)
  - Adafruit Unified Sensor
  - LiquidCrystal I2C (Frank de Brabander)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ---------- Konfigurasi ----------
const char* ssid     = "NAMA_WIFI";
const char* password = "PASSWORD";

#define DHT_PIN     4
#define DHT_TYPE    DHT22

#define LCD_ADDRESS 0x27   // kalau layar kosong, coba 0x3F
#define LCD_COLUMNS 16
#define LCD_ROWS    2

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
WebServer server(80);

float suhu = NAN;
float lembab = NAN;
unsigned long lastRead = 0;
const unsigned long INTERVAL = 2000;

// ---------- Halaman web ----------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 DHT22 Monitor</title>
<style>
  body{margin:0;min-height:100vh;display:flex;flex-direction:column;align-items:center;
       justify-content:center;background:#0f172a;color:#e2e8f0;font-family:Arial,sans-serif}
  h1{font-size:1.3rem;margin-bottom:24px;color:#94a3b8}
  .wrap{display:flex;gap:16px;flex-wrap:wrap;justify-content:center}
  .card{background:#1e293b;border-radius:16px;padding:24px 32px;min-width:160px;
        box-shadow:0 4px 16px rgba(0,0,0,.4)}
  .label{font-size:.9rem;color:#94a3b8}
  .val{font-size:2.6rem;font-weight:bold;margin-top:8px}
  #t{color:#fb923c} #h{color:#38bdf8}
  #status{margin-top:20px;font-size:.8rem;color:#64748b}
</style>
</head>
<body>
  <h1>ESP32 Monitoring Suhu &amp; Kelembaban</h1>
  <div class="wrap">
    <div class="card"><div class="label">Suhu</div><div class="val" id="t">--</div></div>
    <div class="card"><div class="label">Kelembaban</div><div class="val" id="h">--</div></div>
  </div>
  <div id="status">Menunggu data...</div>
<script>
async function update(){
  try{
    const r = await fetch('/data');
    const d = await r.json();
    if(d.ok){
      document.getElementById('t').textContent = d.suhu.toFixed(1) + ' °C';
      document.getElementById('h').textContent = d.lembab.toFixed(1) + ' %';
      document.getElementById('status').textContent = 'Terhubung • update ' + new Date().toLocaleTimeString();
    }else{
      document.getElementById('status').textContent = 'Sensor gagal dibaca';
    }
  }catch(e){
    document.getElementById('status').textContent = 'Koneksi ke ESP32 terputus';
  }
}
update();
setInterval(update, 2000);
</script>
</body>
</html>
)rawliteral";

// ---------- Handler web ----------
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  String json;
  if (isnan(suhu) || isnan(lembab)) {
    json = "{\"ok\":false}";
  } else {
    json = "{\"ok\":true,\"suhu\":" + String(suhu, 1) +
           ",\"lembab\":" + String(lembab, 1) + "}";
  }
  server.send(200, "application/json", json);
}

// ---------- Tampilan LCD ----------
void tampilLCD() {
  if (isnan(suhu) || isnan(lembab)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!   ");
    lcd.setCursor(0, 1);
    lcd.print("Cek wiring DHT22");
    return;
  }
  lcd.setCursor(0, 0);
  lcd.print("Suhu  : ");
  lcd.print(suhu, 1);
  lcd.print((char)223);
  lcd.print("C   ");

  lcd.setCursor(0, 1);
  lcd.print("Lembab: ");
  lcd.print(lembab, 1);
  lcd.print("%   ");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 DHT22");
  lcd.setCursor(0, 1);
  lcd.print("Konek WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 20000) {
      lcd.clear();
      lcd.print("WiFi gagal!");
      Serial.println("\nGagal konek WiFi. Cek SSID/password. Restart...");
      delay(2000);
      ESP.restart();
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Tampilkan IP di LCD supaya mudah dibuka di browser
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP Address:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
  lcd.clear();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web server berjalan...");
}

// ---------- Loop ----------
void loop() {
  server.handleClient();

  if (millis() - lastRead >= INTERVAL) {
    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      lembab = h;
      suhu = t;
      Serial.printf("Suhu: %.1f C | Kelembaban: %.1f %%\n", suhu, lembab);
    } else {
      suhu = NAN;
      lembab = NAN;
      Serial.println("Gagal membaca DHT22!");
    }
    tampilLCD();
  }
}