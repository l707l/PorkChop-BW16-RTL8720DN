#ifndef PORKCHOP_UI_H
#define PORKCHOP_UI_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Paleta de colores RGB565 retro hacker
#define COLOR_PIG_BG     0x0000  // Negro puro
#define COLOR_PIG_PINK   0xFD5B  // Rosa cerdito (Porkchop)
#define COLOR_PIG_SNOUT  0xFC78  // Rosa oscuro / hocico
#define COLOR_ACCENT     0x07E0  // Verde terminal hacker
#define COLOR_5GHZ       0x07FF  // Cyan eléctrico (para redes 5GHz)
#define COLOR_24GHZ      0xFD20  // Naranja fuego (para redes 2.4GHz)
#define COLOR_DANGER     0xF800  // Rojo alarma / ataque
#define COLOR_WHITE      0xFFFF  // Blanco puro
#define COLOR_GRAY       0x7BEF  // Gris interfaz
#define COLOR_DARK_CARD  0x10A2  // Fondo tarjetas

enum PorkMood {
  MOOD_IDLE,      // Tranquilo, parpadeando
  MOOD_HUNTING,   // Buscando APs / escaneando
  MOOD_ATTACKING, // Desautenticando (enfadado / fuego)
  MOOD_EATING,    // Capturando PMKID / Handshake
  MOOD_HAPPY,     // Nivel subido / éxito
  MOOD_SLEEPING   // Reposo / batería baja
};

// Expresiones de cerdo en ASCII / Gráficos vectoriales compactos (128x160)
namespace PigGraphics {
  // Dibuja el rostro icónico de Porkchop centrado en (cx, cy)
  inline void drawPorkchopFace(Adafruit_ST7735 &tft, int cx, int cy, PorkMood mood) {
    // Cabeza del cerdo (círculo / óvalo rosa)
    tft.fillRoundRect(cx - 28, cy - 22, 56, 44, 14, COLOR_PIG_PINK);
    tft.drawRoundRect(cx - 28, cy - 22, 56, 44, 14, COLOR_WHITE);

    // Orejas triangulares
    tft.fillTriangle(cx - 26, cy - 18, cx - 34, cy - 32, cx - 14, cy - 22, COLOR_PIG_PINK);
    tft.drawTriangle(cx - 26, cy - 18, cx - 34, cy - 32, cx - 14, cy - 22, COLOR_WHITE);
    tft.fillTriangle(cx + 26, cy - 18, cx + 34, cy - 32, cx + 14, cy - 22, COLOR_PIG_PINK);
    tft.drawTriangle(cx + 26, cy - 18, cx + 34, cy - 32, cx + 14, cy - 22, COLOR_WHITE);

    // Hocico (óvalo central)
    tft.fillRoundRect(cx - 14, cy - 2, 28, 16, 7, COLOR_PIG_SNOUT);
    tft.drawRoundRect(cx - 14, cy - 2, 28, 16, 7, COLOR_WHITE);

    // Fosas nasales
    tft.fillCircle(cx - 6, cy + 6, 2, COLOR_PIG_BG);
    tft.fillCircle(cx + 6, cy + 6, 2, COLOR_PIG_BG);

    // Ojos según el estado de ánimo (Mood)
    switch (mood) {
      case MOOD_ATTACKING:
        // Ojos de ataque furioso (> <)
        tft.drawLine(cx - 18, cy - 10, cx - 10, cy - 6, COLOR_DANGER);
        tft.drawLine(cx - 18, cy - 2, cx - 10, cy - 6, COLOR_DANGER);
        tft.drawLine(cx + 18, cy - 10, cx + 10, cy - 6, COLOR_DANGER);
        tft.drawLine(cx + 18, cy - 2, cx + 10, cy - 6, COLOR_DANGER);
        break;

      case MOOD_HUNTING:
        // Ojos con gafas hacker / visor
        tft.fillRoundRect(cx - 20, cy - 11, 16, 8, 2, COLOR_5GHZ);
        tft.fillRoundRect(cx + 4, cy - 11, 16, 8, 2, COLOR_5GHZ);
        tft.drawFastHLine(cx - 4, cy - 7, 8, COLOR_WHITE);
        break;

      case MOOD_SLEEPING:
        // Ojos durmiendo (- -)
        tft.drawFastHLine(cx - 18, cy - 8, 10, COLOR_PIG_BG);
        tft.drawFastHLine(cx + 8, cy - 8, 10, COLOR_PIG_BG);
        tft.setCursor(cx + 24, cy - 24);
        tft.setTextColor(COLOR_5GHZ);
        tft.setTextSize(1);
        tft.print("zZ");
        break;

      case MOOD_HAPPY:
        // Ojos felices (^ ^)
        tft.drawLine(cx - 18, cy - 6, cx - 13, cy - 11, COLOR_PIG_BG);
        tft.drawLine(cx - 13, cy - 11, cx - 8, cy - 6, COLOR_PIG_BG);
        tft.drawLine(cx + 8, cy - 6, cx + 13, cy - 11, COLOR_PIG_BG);
        tft.drawLine(cx + 13, cy - 11, cx + 18, cy - 6, COLOR_PIG_BG);
        break;

      case MOOD_IDLE:
      default:
        // Ojos normales abiertos
        tft.fillCircle(cx - 13, cy - 8, 4, COLOR_WHITE);
        tft.fillCircle(cx + 13, cy - 8, 4, COLOR_WHITE);
        tft.fillCircle(cx - 12, cy - 8, 2, COLOR_PIG_BG);
        tft.fillCircle(cx + 14, cy - 8, 2, COLOR_PIG_BG);
        break;
    }
  }
}

#endif // PORKCHOP_UI_H
