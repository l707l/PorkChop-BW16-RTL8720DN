#ifndef PACKET_INJECTION_H
#define PACKET_INJECTION_H

#include <Arduino.h>

// Estructura de paquete Deauth 802.11
typedef struct __attribute__((packed)) {
  uint16_t frame_control;    // 0x00C0 (Deauth)
  uint16_t duration;         // 0xFFFF o 0x013A
  uint8_t destination[6];    // Broadcast FF:FF:FF:FF:FF:FF o MAC cliente objetivo
  uint8_t source[6];         // MAC del AP emisor
  uint8_t bssid[6];          // BSSID del AP
  uint16_t sequence_number;  // 0 (se asigna automáticamente)
  uint16_t reason;           // Código de razón (0x0006, 0x0002, 0x0007, etc.)
} DeauthFrame;

// Estructura de paquete Beacon 802.11
typedef struct __attribute__((packed)) {
  uint16_t frame_control;    // 0x0080 (Beacon)
  uint16_t duration;
  uint8_t destination[6];    // FF:FF:FF:FF:FF:FF
  uint8_t source[6];
  uint8_t bssid[6];
  uint16_t sequence_number;
  uint64_t timestamp;
  uint16_t beacon_interval;  // 0x0064 (100ms)
  uint16_t capabilities;     // 0x1411 / 0x0411 (ESS, Privacy)
  uint8_t ssid_tag;          // 0
  uint8_t ssid_len;
  uint8_t ssid[32];
  uint8_t rates_tag;         // 1
  uint8_t rates_len;
  uint8_t rates[8];
  uint8_t channel_tag;       // 3
  uint8_t channel_len;
  uint8_t channel;
} BeaconFrame;

// Funciones del SDK cerrado de Realtek AmebaD enlazadas en tiempo de compilación
#ifdef __cplusplus
extern "C" {
#endif
  extern uint8_t* rltk_wlan_info;
  void* alloc_mgtxmitframe(void* ptr);
  void update_mgntframe_attrib(void* ptr, void* frame_control);
  int dump_mgntframe(void* ptr, void* frame_control);
#ifdef __cplusplus
}
#endif

// API de inyección de paquetes para RTL8720DN (Dual Band 2.4GHz & 5GHz)
void wifi_tx_raw_frame(void* frame, size_t length);
void wifi_tx_deauth_frame(const uint8_t* ap_mac, const uint8_t* target_mac, uint16_t reason = 0x0006);
void wifi_tx_beacon_frame(const uint8_t* bssid, const char* ssid, uint8_t channel);

#endif // PACKET_INJECTION_H
