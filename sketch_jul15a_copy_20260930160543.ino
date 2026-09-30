#include <WiFi.h>
#include <DHT.h>

// ========================================
// Wi-Fi
// ========================================
const char* ssid = "Kizuto Kung";
const char* password = "Kizuto086!";

// ========================================
// DHT11
// ========================================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ========================================
// GPIO
// ========================================
const int output26 = 26;
const int output27 = 27;

bool state26 = false;
bool state27 = false;

// ========================================
// Sensor Data
// ========================================
float temperature = 0;
float humidity = 0;

unsigned long lastDHTRead = 0;

// ========================================
// Web Server
// ========================================
WiFiServer server(80);

// ========================================
// Setup
// ========================================
void setup() {

  Serial.begin(115200);

  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);

  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  dht.begin();

  // ========================================
  // Wi-Fi
  // ========================================

  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {

    delay(500);
    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WiFi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.begin();

    Serial.println("Web Server Started!");

  } else {

    Serial.println("WiFi Connection Failed!");

    Serial.print("WiFi Status: ");
    Serial.println(WiFi.status());
  }
}

// ========================================
// Loop
// ========================================
void loop() {

  // ========================================
  // ตรวจ Wi-Fi
  // ========================================

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi disconnected. Reconnecting...");

    WiFi.disconnect();
    WiFi.begin(ssid, password);

    delay(1000);

    return;
  }

  // ========================================
  // อ่าน DHT11 ทุก 2 วินาที
  // ========================================

  if (millis() - lastDHTRead >= 2000) {

    lastDHTRead = millis();

    float newHumidity = dht.readHumidity();
    float newTemperature = dht.readTemperature();

    if (!isnan(newHumidity) && !isnan(newTemperature)) {

      humidity = newHumidity;
      temperature = newTemperature;

      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" C | Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

    } else {

      Serial.println("อ่านค่า DHT11 ไม่ได้");
    }
  }

  // ========================================
  // Web Client
  // ========================================

  WiFiClient client = server.available();

  if (!client) {
    return;
  }

  String request = "";

  unsigned long startTime = millis();

  while (client.connected() && millis() - startTime < 300) {

    if (client.available()) {

      char c = client.read();

      request += c;

      if (c == '\n') {
        break;
      }
    }
  }

  // ========================================
  // GPIO 26
  // ========================================

  if (request.indexOf("GET /26/on") >= 0) {

    state26 = true;
    digitalWrite(output26, HIGH);

    Serial.println("GPIO 26 ON");
  }

  if (request.indexOf("GET /26/off") >= 0) {

    state26 = false;
    digitalWrite(output26, LOW);

    Serial.println("GPIO 26 OFF");
  }

  // ========================================
  // GPIO 27
  // ========================================

  if (request.indexOf("GET /27/on") >= 0) {

    state27 = true;
    digitalWrite(output27, HIGH);

    Serial.println("GPIO 27 ON");
  }

  if (request.indexOf("GET /27/off") >= 0) {

    state27 = false;
    digitalWrite(output27, LOW);

    Serial.println("GPIO 27 OFF");
  }

  // ========================================
  // คำนวณ Gauge
  // ========================================

  float tempPercent = temperature / 50.0;

  if (tempPercent < 0) tempPercent = 0;
  if (tempPercent > 1) tempPercent = 1;

  float humidityPercent = humidity / 100.0;

  if (humidityPercent < 0) humidityPercent = 0;
  if (humidityPercent > 1) humidityPercent = 1;

  int tempAngle = tempPercent * 360;
  int humidityAngle = humidityPercent * 360;

  // ========================================
  // HTTP
  // ========================================

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();

  // ========================================
  // HTML
  // ========================================

  client.println("<!DOCTYPE html>");
  client.println("<html lang='th'>");

  client.println("<head>");

  client.println("<meta charset='UTF-8'>");

  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");

  client.println("<meta http-equiv='refresh' content='5'>");

  client.println("<title>ESP32 Smart Control</title>");

  // ========================================
  // CSS
  // ========================================

  client.println("<style>");

  client.println("*{box-sizing:border-box;}");

  client.println("body{");
  client.println("margin:0;");
  client.println("font-family:Arial,sans-serif;");
  client.println("background:#f3f6fa;");
  client.println("color:#1f2937;");
  client.println("}");

  client.println(".header{");
  client.println("background:#111827;");
  client.println("color:white;");
  client.println("padding:25px 15px;");
  client.println("text-align:center;");
  client.println("}");

  client.println(".header h1{");
  client.println("margin:0;");
  client.println("font-size:28px;");
  client.println("}");

  client.println(".header p{");
  client.println("margin:8px 0 0;");
  client.println("color:#9ca3af;");
  client.println("}");

  client.println(".container{");
  client.println("max-width:1000px;");
  client.println("margin:auto;");
  client.println("padding:20px;");
  client.println("}");

  client.println(".section-title{");
  client.println("font-size:22px;");
  client.println("font-weight:bold;");
  client.println("margin:15px 0;");
  client.println("}");

  client.println(".sensor-grid{");
  client.println("display:grid;");
  client.println("grid-template-columns:repeat(2,1fr);");
  client.println("gap:20px;");
  client.println("}");

  client.println(".control-grid{");
  client.println("display:grid;");
  client.println("grid-template-columns:repeat(2,1fr);");
  client.println("gap:20px;");
  client.println("}");

  client.println(".card{");
  client.println("background:white;");
  client.println("border-radius:20px;");
  client.println("padding:25px;");
  client.println("text-align:center;");
  client.println("box-shadow:0 5px 20px rgba(0,0,0,0.08);");
  client.println("}");

  client.println(".card h2{");
  client.println("margin-top:0;");
  client.println("}");

  // ========================================
  // Gauge
  // ========================================

  client.println(".gauge{");
  client.println("width:180px;");
  client.println("height:180px;");
  client.println("border-radius:50%;");
  client.println("margin:20px auto;");
  client.println("display:flex;");
  client.println("align-items:center;");
  client.println("justify-content:center;");
  client.println("}");

  client.println(".temp-gauge{");
  client.print("background:conic-gradient(#2563eb ");
  client.print(tempAngle);
  client.println("deg,#e5e7eb 0deg);");
  client.println("}");

  client.println(".hum-gauge{");
  client.print("background:conic-gradient(#16a34a ");
  client.print(humidityAngle);
  client.println("deg,#e5e7eb 0deg);");
  client.println("}");

  client.println(".gauge-inner{");
  client.println("width:140px;");
  client.println("height:140px;");
  client.println("border-radius:50%;");
  client.println("background:white;");
  client.println("display:flex;");
  client.println("flex-direction:column;");
  client.println("align-items:center;");
  client.println("justify-content:center;");
  client.println("}");

  client.println(".value{");
  client.println("font-size:30px;");
  client.println("font-weight:bold;");
  client.println("}");

  client.println(".unit{");
  client.println("font-size:15px;");
  client.println("color:#777;");
  client.println("}");

  // ========================================
  // Buttons
  // ========================================

  client.println("button{");
  client.println("width:180px;");
  client.println("padding:14px;");
  client.println("font-size:18px;");
  client.println("font-weight:bold;");
  client.println("border:0;");
  client.println("border-radius:12px;");
  client.println("color:white;");
  client.println("cursor:pointer;");
  client.println("}");

  client.println(".on{");
  client.println("background:#16a34a;");
  client.println("}");

  client.println(".off{");
  client.println("background:#dc2626;");
  client.println("}");

  client.println(".status{");
  client.println("font-weight:bold;");
  client.println("margin:15px;");
  client.println("}");

  // ========================================
  // Mobile
  // ========================================

  client.println("@media(max-width:600px){");

  client.println(".sensor-grid,.control-grid{");
  client.println("grid-template-columns:1fr;");
  client.println("}");

  client.println(".header h1{");
  client.println("font-size:23px;");
  client.println("}");

  client.println(".gauge{");
  client.println("width:160px;");
  client.println("height:160px;");
  client.println("}");

  client.println(".gauge-inner{");
  client.println("width:125px;");
  client.println("height:125px;");
  client.println("}");

  client.println("}");

  client.println("</style>");

  client.println("</head>");

  // ========================================
  // BODY
  // ========================================

  client.println("<body>");

  client.println("<div class='header'>");

  client.println("<h1>ESP32 Smart Control</h1>");

  client.println("<p>DHT11 Monitoring & Light Control</p>");

  client.println("</div>");

  client.println("<div class='container'>");

  // ========================================
  // Sensor
  // ========================================

  client.println("<div class='section-title'>Sensor Monitoring</div>");

  client.println("<div class='sensor-grid'>");

  // Temperature

  client.println("<div class='card'>");

  client.println("<h2>Temperature</h2>");

  client.println("<div class='gauge temp-gauge'>");

  client.println("<div class='gauge-inner'>");

  client.println("<div class='value'>");
  client.println(temperature, 1);
  client.println("</div>");

  client.println("<div class='unit'>°C</div>");

  client.println("</div>");
  client.println("</div>");

  client.println("<p>อุณหภูมิจาก DHT11</p>");

  client.println("</div>");

  // Humidity

  client.println("<div class='card'>");

  client.println("<h2>Humidity</h2>");

  client.println("<div class='gauge hum-gauge'>");

  client.println("<div class='gauge-inner'>");

  client.println("<div class='value'>");
  client.println(humidity, 1);
  client.println("</div>");

  client.println("<div class='unit'>%</div>");

  client.println("</div>");
  client.println("</div>");

  client.println("<p>ความชื้นจาก DHT11</p>");

  client.println("</div>");

  client.println("</div>");

  // ========================================
  // Light Control
  // ========================================

  client.println("<div class='section-title'>Light Control</div>");

  client.println("<div class='control-grid'>");

  // LED 1

  client.println("<div class='card'>");

  client.println("<h2>Light 1</h2>");

  if (state26) {

    client.println("<div class='status'>สถานะ: ON</div>");

    client.println("<a href='/26/off'>");
    client.println("<button class='off'>ปิดไฟ</button>");
    client.println("</a>");

  } else {

    client.println("<div class='status'>สถานะ: OFF</div>");

    client.println("<a href='/26/on'>");
    client.println("<button class='on'>เปิดไฟ</button>");
    client.println("</a>");
  }

  client.println("<p>GPIO 26</p>");

  client.println("</div>");

  // LED 2

  client.println("<div class='card'>");

  client.println("<h2>Light 2</h2>");

  if (state27) {

    client.println("<div class='status'>สถานะ: ON</div>");

    client.println("<a href='/27/off'>");
    client.println("<button class='off'>ปิดไฟ</button>");
    client.println("</a>");

  } else {

    client.println("<div class='status'>สถานะ: OFF</div>");

    client.println("<a href='/27/on'>");
    client.println("<button class='on'>เปิดไฟ</button>");
    client.println("</a>");
  }

  client.println("<p>GPIO 27</p>");

  client.println("</div>");

  client.println("</div>");

  // ========================================
  // Footer
  // ========================================

  client.println("<p style='text-align:center;color:#888;margin-top:30px;'>");
  client.println("ESP32 Smart Control System");
  client.println("</p>");

  client.println("</div>");

  client.println("</body>");
  client.println("</html>");

  client.stop();
}