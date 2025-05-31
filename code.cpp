#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>

#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

char ssid[] = ""; 
char pass[] = "c";

#define LED_PIN D5
#define LED_COUNT 98
#define BRIGHTNESS 255

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
WiFiManager wifiManager;

unsigned long LostWiFiMillis; 
unsigned long waveTimer = 0;
unsigned long color2Timer = 0;

bool LostWiFi = false;                  
bool waveMode = false;         // V0 - Бегущая волна
bool color2 = false;           // V1 - Цветовое дыхание

// Переменные для эффектов
int wavePosition = 0;
int waveLength = 10;
int color2Brightness = 0;
bool color2Direction = true;
int color2Hue = 0;

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  strip.begin();           
  strip.show();            
  strip.setBrightness(BRIGHTNESS);

  wifiManager.autoConnect("Connect-WIFI", "PASSWORD");
  
  while (WiFi.status() != WL_CONNECTED){}
  randomSeed(millis());
}

BLYNK_WRITE(V0) // Управление бегущей волной
{
  waveMode = param.asInt();
  if (waveMode) {
    color2 = false;
    Blynk.virtualWrite(V1, 0);
  } else {
    colorOFF();
  }
}

BLYNK_WRITE(V1) // Управление цветовым дыханием
{
  color2 = param.asInt();
  if (color2) {
    waveMode = false;
    Blynk.virtualWrite(V0, 0);
  } else {
    colorOFF();
  }
}

void loop() {
  // Проверка WiFi соединения
  if (WiFi.status() != WL_CONNECTED) {
    if (LostWiFi == 0){
      LostWiFi = 1;
      LostWiFiMillis = millis();
    } else if(millis() - LostWiFiMillis > 180000) {
      ESP.reset();
    }
  } else {
    LostWiFi = 0;
  }

  Blynk.run();

  // Управление эффектами
  if (waveMode) {
    waveEffect();
  } else if (color2) {
    color2Effect();
  }
}

void waveEffect() {
  if (millis() - waveTimer > 80) {
    waveTimer = millis();
    
    // Очищаем ленту
    for(int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    
    // Рисуем бегущую волну циановым цветом
    for(int i = 0; i < waveLength; i++) {
      int pixelPos = (wavePosition + i) % strip.numPixels();
      int brightness = 255 - (i * 25);
      if (brightness < 50) brightness = 50;
      
      strip.setPixelColor(pixelPos, strip.Color(0, brightness, brightness));
    }
    
    strip.show();
    
    wavePosition++;
    if (wavePosition >= strip.numPixels()) {
      wavePosition = 0;
    }
  }
}

void color2Effect() {
  if (millis() - color2Timer > 25) {
    color2Timer = millis();
    
    // Изменяем яркость
    if (color2Direction) {
      color2Brightness += 2;
      if (color2Brightness >= 255) {
        color2Brightness = 255;
        color2Direction = false;
      }
    } else {
      color2Brightness -= 2;
      if (color2Brightness <= 30) {
        color2Brightness = 30;
        color2Direction = true;
      }
    }
    
    // Медленное изменение оттенка
    color2Hue += 50;
    if (color2Hue >= 65536) color2Hue = 0;
    
    int hue = 43690 + (color2Hue / 3);
    if (hue >= 65536) hue -= 65536;
    
    uint32_t color = strip.ColorHSV(hue, 255, color2Brightness);
    
    for(int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
  }
}

void colorOFF(){
  for(int i=0; i<strip.numPixels(); i++) { 
    strip.setPixelColor(i, strip.Color(0, 0, 0));         
  }
  strip.show();
}
