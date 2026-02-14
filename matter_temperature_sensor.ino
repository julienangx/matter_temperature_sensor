/*
 * CAPTEUR MATTER TEMPÉRATURE/HUMIDITÉ
 * 
 * VERSION ALIMENTATION PERMANENTE (sans deep sleep)
 * ESP32-C3 + DHT22 + E-Paper 2.13"
 * 
 * FICHIERS REQUIS dans le même dossier :
 * - qr_gen.h
 * - qr_gen.c
 */

#include <Matter.h>
#include <WiFi.h>
#include <GxEPD2_BW.h>
#include "DHT.h"
#include "qr_gen.h"

// Polices
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// === PINS ===
#define DHT_PIN   4
#define DHT_TYPE  DHT22
#define EPD_BUSY  5
#define EPD_RST   6
#define EPD_DC    7
#define EPD_CS    10
#define EPD_SCK   2
#define EPD_MOSI  3

const uint8_t buttonPin = BOOT_PIN;  // Bouton pour decommission

// === OBJETS ===
GxEPD2_BW<GxEPD2_213_GDEY0213B74, GxEPD2_213_GDEY0213B74::HEIGHT> display(
  GxEPD2_213_GDEY0213B74(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

DHT dht(DHT_PIN, DHT_TYPE);

// Matter endpoints
MatterTemperatureSensor matterTemp;
MatterHumiditySensor matterHumidity;

// === HISTORIQUE ===
#define HISTORY_SIZE 24
float tempHistory[HISTORY_SIZE];
int historyIndex = 0;
bool historyFull = false;

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2 * 60 * 1000;  // 2 minutes
const int SCREEN_WIDTH = 122;
const int SCREEN_HEIGHT = 250;

// Tendances
float lastTemp = NAN;
float lastHumidity = NAN;

// Decommissioning button
uint32_t button_time_stamp = 0;
bool button_state = false;
const uint32_t decommissioningTimeout = 5000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(buttonPin, INPUT_PULLUP);
  
  Serial.println("\n=== CAPTEUR MATTER ===");
  Serial.println("Mode: Alimentation permanente\n");
  
  // Initialiser l'historique
  for (int i = 0; i < HISTORY_SIZE; i++) tempHistory[i] = NAN;
  
  // Init DHT22
  dht.begin();
  delay(2000);
  Serial.println("[OK] DHT22");
  
  // Init écran
  SPI.begin(EPD_SCK, -1, EPD_MOSI, EPD_CS);
  display.init(115200);
  display.setRotation(0);
  Serial.println("[OK] Ecran e-paper");
  
  // Lire température initiale
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp)) temp = 20.0;
  if (isnan(hum)) hum = 50.0;
  
  // IMPORTANT: Initialiser les sensors AVANT Matter.begin()
  matterTemp.begin(temp);
  matterHumidity.begin(hum);
  Serial.println("[OK] Sensors initialises");
  
  // Matter.begin() en DERNIER
  Matter.begin();
  Serial.println("[OK] Matter demarre");
  
  // Vérifier si déjà commissionné
  if (!Matter.isDeviceCommissioned()) {
    Serial.println("\n========================================");
    Serial.println("  APPAREIL NON COMMISSIONNE");
    Serial.println("========================================");
    
    String qrUrl = Matter.getOnboardingQRCodeUrl();
    String manualCode = Matter.getManualPairingCode();
    
    String matterPayload = qrUrl;
    int dataIndex = qrUrl.indexOf("data=");
    if (dataIndex != -1) {
      matterPayload = qrUrl.substring(dataIndex + 5);
    }
    
    Serial.println("Code manuel: " + manualCode);
    Serial.println("QR Payload: " + matterPayload);
    Serial.println("QR URL: " + qrUrl);
    Serial.println("========================================\n");
    
    // Afficher QR sur écran
    displayQRCode(matterPayload, manualCode);
    
    Serial.println(">>> SCANNE LE QR CODE <<<");
    Serial.println("Attente commissioning...\n");
    
    uint32_t timeCount = 0;
    while (!Matter.isDeviceCommissioned()) {
      delay(100);
      if ((timeCount++ % 50) == 0) {
        Serial.println("En attente... (scanne le QR avec SmartThings)");
      }
    }
    
    Serial.println("\n*** COMMISSIONNE! ***\n");
  } else {
    Serial.println("Deja commissionne");
  }
  
  Serial.println("Attente connexion Matter...");
  while (!Matter.isDeviceConnected()) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[OK] Connecte!\n");
  
  Serial.println("Capteur operationnel!");
  Serial.println("Mise a jour toutes les 2 minutes\n");
  
  // Première mise à jour immédiate
  updateAndDisplay();
}

void loop() {
  // Mise à jour périodique toutes les 15 minutes
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    updateAndDisplay();
    lastUpdate = millis();
  }
  
  // Gestion bouton decommissioning (BOOT maintenu 5s)
  if (digitalRead(buttonPin) == LOW && !button_state) {
    button_time_stamp = millis();
    button_state = true;
  }
  
  if (digitalRead(buttonPin) == HIGH && button_state) {
    button_state = false;
  }
  
  uint32_t time_diff = millis() - button_time_stamp;
  if (button_state && time_diff > decommissioningTimeout) {
    Serial.println("Decommissioning...");
    Matter.decommission();
    button_time_stamp = millis();
  }
  
  delay(100);
}

void updateAndDisplay() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Erreur lecture DHT22");
    return;
  }
  
  Serial.printf("T: %.1fC, H: %.1f%%\n", temp, hum);
  
  // Calculer tendances
  int tempTrend = 0;  // -1 = baisse, 0 = stable, 1 = hausse
  int humTrend = 0;
  
  if (!isnan(lastTemp)) {
    float tempDiff = temp - lastTemp;
    if (tempDiff > 0.5) tempTrend = 1;
    else if (tempDiff < -0.5) tempTrend = -1;
    
    Serial.printf("Tendance temp: %s (%.1f°C)\n", 
                  tempTrend == 1 ? "↑" : (tempTrend == -1 ? "↓" : "→"), tempDiff);
  }
  
  if (!isnan(lastHumidity)) {
    float humDiff = hum - lastHumidity;
    if (humDiff > 2.0) humTrend = 1;
    else if (humDiff < -2.0) humTrend = -1;
  }
  
  // Sauvegarder pour prochaine comparaison
  lastTemp = temp;
  lastHumidity = hum;
  
  // Update Matter
  matterTemp.setTemperature(temp);
  matterHumidity.setHumidity(hum);
  
  // Ajouter à l'historique
  tempHistory[historyIndex] = temp;
  historyIndex = (historyIndex + 1) % HISTORY_SIZE;
  if (historyIndex == 0) historyFull = true;
  
  // Vérifier connexion
  bool connected = Matter.isDeviceConnected();
  
  // Afficher
  displayData(temp, hum, tempTrend, humTrend, connected);
  
  Serial.println("Prochaine mise a jour dans 2 min\n");
}

void displayQRCode(String qrData, String manualCode) {
  Serial.printf("Generating QR for: %s (%d chars)\n", qrData.c_str(), qrData.length());
  
  QRCode qr;
  uint8_t qrBytes[qrcode_getBufferSize(3)];
  
  int8_t err = qrcode_initText(&qr, qrBytes, 2, ECC_LOW, qrData.c_str());
  if (err != 0) {
    Serial.println("Essai version 3...");
    err = qrcode_initText(&qr, qrBytes, 3, ECC_LOW, qrData.c_str());
    if (err != 0) {
      Serial.println("Erreur QR!");
      return;
    }
  }
  
  Serial.printf("QR: %dx%d modules\n", qr.size, qr.size);
  
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    
    int availableWidth = SCREEN_WIDTH - 8;
    int moduleSize = availableWidth / qr.size;
    if (moduleSize < 2) moduleSize = 2;
    
    int qrPixels = qr.size * moduleSize;
    int qrX = (SCREEN_WIDTH - qrPixels) / 2;
    int qrY = 8;
    
    Serial.printf("Module: %dpx, Total: %dpx\n", moduleSize, qrPixels);
    
    display.fillRect(qrX - 4, qrY - 4, qrPixels + 8, qrPixels + 8, GxEPD_WHITE);
    
    for (uint8_t y = 0; y < qr.size; y++) {
      for (uint8_t x = 0; x < qr.size; x++) {
        if (qrcode_getModule(&qr, x, y)) {
          display.fillRect(qrX + x * moduleSize, qrY + y * moduleSize, 
                          moduleSize, moduleSize, GxEPD_BLACK);
        }
      }
    }
    
    int textY = qrY + qrPixels + 20;
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    
    String fmt = "";
    for (unsigned int i = 0; i < manualCode.length(); i++) {
      fmt += manualCode[i];
      if ((i + 1) % 4 == 0 && i + 1 < manualCode.length()) fmt += "-";
    }
    
    int16_t x1, y1; uint16_t w, h;
    
    if (fmt.length() > 12) {
      int mid = fmt.length() / 2;
      while (mid < fmt.length() && fmt[mid] != '-') mid++;
      
      String l1 = fmt.substring(0, mid);
      String l2 = fmt.substring(mid + 1);
      
      display.getTextBounds(l1, 0, 0, &x1, &y1, &w, &h);
      display.setCursor((SCREEN_WIDTH - w) / 2, textY);
      display.print(l1);
      
      display.getTextBounds(l2, 0, 0, &x1, &y1, &w, &h);
      display.setCursor((SCREEN_WIDTH - w) / 2, textY + 16);
      display.print(l2);
    } else {
      display.getTextBounds(fmt, 0, 0, &x1, &y1, &w, &h);
      display.setCursor((SCREEN_WIDTH - w) / 2, textY);
      display.print(fmt);
    }
    
  } while (display.nextPage());
}

void displayData(float temp, float hum, int tempTrend, int humTrend, bool connected) {
  display.setFullWindow();
  display.firstPage();
  
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    
    // === INDICATEUR CONNEXION (tout en haut) ===
    int wifiX = SCREEN_WIDTH - 15;
    int wifiY = 8;
    
    if (connected) {
      // WiFi connecté : 3 arcs
      display.drawCircle(wifiX, wifiY + 6, 2, GxEPD_BLACK);
      display.drawCircle(wifiX, wifiY + 6, 4, GxEPD_BLACK);
      display.drawCircle(wifiX, wifiY + 6, 6, GxEPD_BLACK);
    } else {
      // Déconnecté : croix
      display.drawLine(wifiX - 4, wifiY, wifiX + 4, wifiY + 8, GxEPD_BLACK);
      display.drawLine(wifiX + 4, wifiY, wifiX - 4, wifiY + 8, GxEPD_BLACK);
    }
    
    // === TEMPÉRATURE (grande, centrée) ===
    display.setFont(&FreeSansBold18pt7b);
    String tempStr = String(temp, 1) + "°";
    int16_t x1, y1; 
    uint16_t w, h;
    display.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);
    int tempX = (SCREEN_WIDTH - w) / 2;
    display.setCursor(tempX, 50);
    display.print(tempStr);
    
    // === HUMIDITÉ (moyenne, en dessous) ===
    display.setFont(&FreeSansBold12pt7b);
    String humStr = String(hum, 1) + "%";
    display.getTextBounds(humStr, 0, 0, &x1, &y1, &w, &h);
    int humX = (SCREEN_WIDTH - w) / 2;
    display.setCursor(humX, 78);
    display.print(humStr);
    
    // === SECTION ICÔNES + TENDANCES (en ligne horizontale) ===
    int iconsY = 95;
    int centerX = SCREEN_WIDTH / 2;
    
    // Zone température (gauche)
    int tempIconX = centerX - 25;
    drawTempIcon(tempIconX, iconsY, temp);
    
    if (tempTrend != 0) {
      drawTrendArrow(tempIconX + 15, iconsY + 4, tempTrend);
    }
    
    // Zone humidité (droite)
    int humIconX = centerX + 8;
    drawHumidityIcon(humIconX, iconsY, hum);
    
    if (humTrend != 0) {
      drawTrendArrow(humIconX + 15, humTrend == 1 ? iconsY + 2 : iconsY + 4, humTrend);
    }
    
    // === LIGNE DE SÉPARATION ===
    display.drawLine(20, 115, SCREEN_WIDTH - 20, 115, GxEPD_BLACK);
    
    // === GRAPHIQUE ===
    drawGraph(15, 125, SCREEN_WIDTH - 30, 95);
    
  } while (display.nextPage());
}

// Dessiner icône température (thermomètre)
void drawTempIcon(int x, int y, float temp) {
  display.drawRect(x + 3, y, 4, 12, GxEPD_BLACK);
  display.fillCircle(x + 5, y + 14, 3, GxEPD_BLACK);
  
  int fillLevel = 0;
  if (temp < 18) fillLevel = 3;
  else if (temp < 22) fillLevel = 6;
  else fillLevel = 9;
  
  display.fillRect(x + 4, y + 12 - fillLevel, 2, fillLevel, GxEPD_BLACK);
}

// Dessiner icône humidité (goutte)
void drawHumidityIcon(int x, int y, float hum) {
  display.fillCircle(x + 5, y + 10, 4, GxEPD_BLACK);
  display.fillTriangle(x + 5, y, x + 2, y + 8, x + 8, y + 8, GxEPD_BLACK);
  
  if (hum > 70 || hum < 30) {
    display.fillRect(x + 4, y + 5, 2, 5, GxEPD_WHITE);
    display.fillRect(x + 4, y + 11, 2, 2, GxEPD_WHITE);
  }
}

// Dessiner flèche de tendance
void drawTrendArrow(int x, int y, int trend) {
  if (trend == 1) {
    display.fillTriangle(x, y, x - 4, y + 6, x + 4, y + 6, GxEPD_BLACK);
    display.fillRect(x - 1, y + 5, 2, 6, GxEPD_BLACK);
  } else if (trend == -1) {
    display.fillTriangle(x, y + 11, x - 4, y + 5, x + 4, y + 5, GxEPD_BLACK);
    display.fillRect(x - 1, y, 2, 6, GxEPD_BLACK);
  }
}

void drawGraph(int x, int y, int w, int h) {
  int count = historyFull ? HISTORY_SIZE : historyIndex;
  if (count < 2) {
    display.setFont(NULL);
    display.setCursor(x + w/2 - 10, y + h/2 - 4);
    display.print("...");
    return;
  }
  
  float minT = 100, maxT = -100;
  for (int i = 0; i < count; i++) {
    if (!isnan(tempHistory[i])) {
      if (tempHistory[i] < minT) minT = tempHistory[i];
      if (tempHistory[i] > maxT) maxT = tempHistory[i];
    }
  }
  
  float range = maxT - minT;
  if (range < 2) { 
    float mid = (maxT + minT) / 2; 
    minT = mid - 1; 
    maxT = mid + 1; 
    range = 2; 
  }
  
  display.setFont(NULL);
  display.setCursor(x, y);
  display.print(maxT, 0);
  display.print("'");
  
  display.setCursor(x, y + h - 8);
  display.print(minT, 0);
  display.print("'");
  
  int gX = x + 18;
  int gW = w - 20;
  int gY = y + 5;
  int gH = h - 10;
  
  display.drawLine(gX, gY + gH, gX + gW, gY + gH, GxEPD_BLACK);
  
  int pX = -1, pY = -1;
  for (int i = 0; i < count; i++) {
    int idx = historyFull ? (historyIndex + i) % HISTORY_SIZE : i;
    if (!isnan(tempHistory[idx])) {
      int px = gX + (i * gW) / (count - 1);
      int py = gY + gH - (int)(((tempHistory[idx] - minT) / range) * gH);
      
      if (i == count - 1) {
        display.fillCircle(px, py, 3, GxEPD_BLACK);
      } else {
        display.fillCircle(px, py, 1, GxEPD_BLACK);
      }
      
      if (pX >= 0) {
        display.drawLine(pX, pY, px, py, GxEPD_BLACK);
      }
      
      pX = px;
      pY = py;
    }
  }
}
