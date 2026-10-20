//Libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Pins
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI  27  
#define OLED_CLK   14  
#define OLED_DC    25  
#define OLED_CS    26  
#define OLED_RESET 33  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
// ------------------------------------------User Config -----------------------------------------------------------------------------------------
// Wifi
const char* ssid     = "Wifi name";
const char* password = "Wifi password";
// Coordinates
const float LATITUDE  = 12.34;
const float LONGITUDE = 12.34;
//------------------------------------------End of config ----------------------------------------------------------------------------------------- 
unsigned long lastUpdate = 0;
// Update per 10 mins
const unsigned long UPDATE_INTERVAL = 600000; 

// 24x24 Pixel Art Icons for each weather
// Sunny
const unsigned char PROGMEM icon_sun[] = {
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x0c, 0x00, 0xc0, 0x0c, 0x00, 0xc0, 0x00, 0xfc, 0x00, 
  0x00, 0xfc, 0x00, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0xf3, 0x03, 0x3c, 
  0xf3, 0x03, 0x3c, 0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0xfc, 0x00, 
  0x00, 0xfc, 0x00, 0x0c, 0x00, 0xc0, 0x0c, 0x00, 0xc0, 0x30, 0x30, 0x30, 
  0x30, 0x30, 0x30, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00
};
// Partly Cloudy
const unsigned char PROGMEM icon_partly_cloudy[] = {
  0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 
  0x0c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 
  0x03, 0x03, 0xc0, 0xf3, 0x03, 0xc0, 0xf3, 0x0c, 0x3c, 0x03, 0x0c, 0x3c, 
  0x00, 0xf0, 0x07, 0x00, 0xf0, 0x07, 0x03, 0x00, 0x03, 0x03, 0x00, 0x03, 
  0x0c, 0x00, 0x03, 0x0c, 0x00, 0x03, 0x30, 0x00, 0x03, 0x30, 0x00, 0x03, 
  0x30, 0x00, 0x03, 0x30, 0x00, 0x03, 0x0f, 0xff, 0xfc, 0x0f, 0xff, 0xfc
};
// Cloudy
const unsigned char PROGMEM icon_cloud[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0xff, 0x00, 
  0x01, 0xff, 0x80, 0x03, 0xff, 0xc0, 0x07, 0xff, 0xf0, 0x0f, 0xff, 0xf8, 
  0x1f, 0xff, 0xfc, 0x3f, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xfe, 
  0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// Rain
const unsigned char PROGMEM icon_rain[] = {
  0x00, 0x3c, 0x00, 0x00, 0xff, 0x00, 0x01, 0xff, 0x80, 0x03, 0xff, 0xc0, 
  0x07, 0xff, 0xf0, 0x1f, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0x7f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x08, 0x42, 0x10, 
  0x10, 0x84, 0x20, 0x21, 0x08, 0x40, 0x02, 0x10, 0x80, 0x00, 0x00, 0x00, 
  0x08, 0x42, 0x10, 0x10, 0x84, 0x20, 0x21, 0x08, 0x40, 0x02, 0x10, 0x80, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void showStatus(const char* title, const char* subtitle) {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);
  display.drawLine(4, 15, 124, 15, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(6, 5);
  display.print("LIVE WEATHER");

  display.setCursor(12, 26);
  display.println(title);

  display.setCursor(12, 42);
  display.println(subtitle);

  display.display();
}

void renderDashboard(float temp, int humidity, float wind, int code) {
  display.clearDisplay();

  // Header
  display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);
  display.drawLine(4, 15, 124, 15, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(6, 5);
  display.print("LIVE WEATHER");
  display.fillCircle(118, 9, 2, SSD1306_WHITE);

  // Start with Clear
  const unsigned char* selectedIcon = icon_sun;
  const char* conditionLabel = "CLEAR";
  // Clear condition 
  if (code == 0) {
    selectedIcon = icon_sun;
    conditionLabel = "CLEAR";
  } 
  // Partly cloudy
  else if (code == 1 || code == 2) {
    selectedIcon = icon_partly_cloudy;
    conditionLabel = "P.CLD";
  } 
  // Cloudy
  else if (code == 3 || code == 45 || code == 48) {
    selectedIcon = icon_cloud;
    conditionLabel = "CLOUD";
  } 
  // Rain
  else if ((code >= 51 && code <= 67) || (code >= 80 && code <= 99)) {
    selectedIcon = icon_rain;
    conditionLabel = "RAIN";
  }

  // Draw conditions
  display.drawBitmap(8, 20, selectedIcon, 24, 24, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(6, 48);
  display.print(conditionLabel);
  // Temperatures
  display.setTextSize(2);
  display.setCursor(44, 20);
  display.printf("%.1f", temp);
  display.setTextSize(1);
  display.print(" C");
  // Humidity and wind
  display.drawLine(44, 40, 122, 40, SSD1306_WHITE);
  display.setCursor(44, 47);
  display.printf("H:%d%%", humidity);
  display.setCursor(84, 47);
  display.printf("W:%.0fkm", wind);
  display.display();
}

void fetchLiveWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    showStatus("WiFi Dropped", "Reconnecting...");
    return;
  }
 String endpoint = "https://api.open-meteo.com/v1/forecast?latitude=" + String(LATITUDE, 4) + "&longitude=" + String(LONGITUDE, 4) + "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m";
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  if (https.begin(client, endpoint)) {
    int httpCode = https.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();

      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float temp   = doc["current"]["temperature_2m"];
        int humidity = doc["current"]["relative_humidity_2m"];
        float wind   = doc["current"]["wind_speed_10m"];
        int code     = doc["current"]["weather_code"];

        renderDashboard(temp, humidity, wind, code);
      } else {
        showStatus("JSON Error", "Failed to parse");
      }
    } else {
      String errStr = "Code: " + String(httpCode);
      showStatus("HTTP Error", errStr.c_str());
    }
    https.end();
  } else {
    showStatus("HTTPS Error", "Unable to reach API");
  }
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  showStatus("Connecting WiFi", ssid);

  WiFi.begin(ssid, password);
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 40) {
    delay(500);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());
    showStatus("Fetching Live", "Weather Data...");
    fetchLiveWeather();
    lastUpdate = millis();
  } else {
    showStatus("WiFi Timeout", "Check Credentials");
  }
}

void loop() {
  // Wifi reconnection
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000);
    return;
  }

  // Wifi update
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    fetchLiveWeather();
  }
}