# 🐷 Porkchop BW16 (RTL8720DN) - Dual-Band 2.4GHz / 5GHz Wi-Fi Deauther & Companion

Implementación del firmware **Porkchop** adaptada para el módulo **BW16 (Realtek RTL8720DN)**, con soporte para pantalla a color **TFT ST7735 de 1.8" (128x160)** y navegación mediante **4 botones físicos**.

Inspirado en [Porkchop-CYD](https://github.com/rgomez31UAQ/Porkchop-CYD) y con inyección de paquetes de [R4TKN BW16](https://github.com/rusyln/R4TKN-FIRMWARE-BW16/) / [RTL8720dn-Deauther](https://github.com/tesa-klebeband/RTL8720dn-Deauther).

---

## ⚡ Características Principales

* **Capacidad Dual-Band Nativa:** Soporte real tanto para redes de **2.4 GHz (Canales 1 al 14)** como de **5.0 GHz (Canales 36 al 165)** gracias al transceptor de radio del RTL8720DN.
* **Mascota Virtual Porkchop:** Cerdito interactivo estilo Pwnagotchi con expresiones visuales dinámicas (Idle, Buscando/Hacker, Atacando/Furioso, Durmiendo, Feliz).
* **Navegación Táctil Reemplazada por 4 Botones Físicos:** Diseñado desde cero para operar sin panel táctil con 4 pulsadores (Arriba, Abajo, OK, Atrás).
* **Kit de Herramientas Wi-Fi:**
  * **ESCANEO DUAL:** Detección de redes y clientes en 2.4 GHz y 5.0 GHz con discriminación de color en pantalla.
  * **DEAUTH SELECTIVO (OINK):** Desautenticación dirigida al punto de acceso seleccionado.
  * **TORMENTA DUAL (WARHOG):** Salto ultra-rápido de canales e inyección continua en ambas frecuencias.
  * **BEACON SPAM (BACON):** Generación masiva de redes Wi-Fi falsas en el aire.
  * **SWINE STATS:** Monitor de paquetes enviados, redes descubiertas y tiempo de actividad.

---

## 🔌 Diagrama de Conexiones (Pinout para BW16)

```
                              BW16 (RTL8720DN)
                            +-------------------+
             (Display CS)   | PA27         3.3V | -> Alimentación Display (VCC)
             (Display BLK)  | PA30         PA28 | -> BOTÓN 4: ATRÁS / SALIR
             BOTÓN 1: UP    | PB1          PA26 | -> Display RES (Reset)
             BOTÓN 2: DOWN  | PB2          PA25 | -> Display DC (Data/Command)
             BOTÓN 3: OK    | PB3          PA15 |
                            | GND          PA14 | -> Display SCL (SPI Clock)
                            | ...          PA13 |
                            | ...          PA12 | -> Display SDA (SPI MOSI)
                            +-------------------+
```

### 1. Pantalla TFT 1.8" ST7735 (128x160 SPI)

| Pin Pantalla ST7735 | Pin BW16 | Función |
| :---: | :---: | :--- |
| **VCC** | **3.3V** | Alimentación lógica (3.3V) |
| **GND** | **GND** | Tierra común |
| **SCL** (SCK) | **PA14** | Reloj SPI (SPI1_CLK) |
| **SDA** (MOSI) | **PA12** | Datos SPI (SPI1_MOSI) |
| **RES** (RST) | **PA26** | Reset de pantalla |
| **DC** (A0) | **PA25** | Selección Datos / Comando |
| **CS** | **PA27** | Chip Select SPI |
| **BLK** (LED) | **PA30** *(o 3.3V)* | Retroiluminación |

---

### 2. Los 4 Botones Físicos (Configuración Active LOW)

Todos los pulsadores se conectan directamente entre el pin indicado y **GND** (utilizan las resistencias *pull-up* internas del chip):

| Botón | Pin en BW16 | Acción en la Interfaz |
| :---: | :---: | :--- |
| **BOTÓN 1 (ARRIBA)** | **PB1** | Subir en el menú / Cambiar estado de ánimo |
| **BOTÓN 2 (ABAJO)** | **PB2** | Bajar en el menú / Modo dormir |
| **BOTÓN 3 (OK / ENTER)** | **PB3** | Seleccionar opción / Entrar al menú / Confirmar |
| **BOTÓN 4 (ATRÁS / EXIT)** | **PA28** | Volver al menú anterior / Detener ataques inmediatamente |

---

## 🕹️ Guía de Uso y Navegación

1. **Pantalla Principal (IDLE):**
   * El cerdito Porkchop parpadea y muestra el nivel actual, redes detectadas y paquetes enviados.
   * Pulsa **`OK`** para abrir el menú de herramientas.
   * Pulsa **`ARRIBA`** o **`ABAJO`** para cambiar manualmente su expresión.

2. **Menú de Opciones:**
   * Usa **`ARRIBA`** y **`ABAJO`** para desplazarte por la lista.
   * Pulsa **`OK`** para seleccionar.
   * Pulsa **`ATRÁS`** para regresar a la pantalla del cerdito.

3. **Durante un Ataque o Escaneo:**
   * Verás al cerdito en modo de ataque (`MOOD_ATTACKING`) con el contador de tramas transmitidas en tiempo real.
   * Pulsa **`ATRÁS`** en cualquier momento para detener la transmisión y regresar a salvo al menú.

---

## 🛠️ Instrucciones de Instalación en Arduino IDE

1. **Añadir el gestor de placas AmebaD:**
   * En Arduino IDE ve a: `Archivo -> Preferencias -> Gestor de URLs adicionales de tarjetas`.
   * Añade la siguiente URL:
     ```text
     https://github.com/ambiot/ambd_arduino/raw/master/Arduino_package/package_realtek.com_amebad_index.json
     ```
   * En `Herramientas -> Placa -> Gestor de tarjetas`, busca **Realtek AmebaD Boards** e instálalo.
   * Selecciona la placa: **Ameba RTL8720DN (BW16)**.

2. **Instalar librerías requeridas:**
   * `Adafruit GFX Library`
   * `Adafruit ST7735 and ST7789 Library`

3. **Compilar y Cargar:**
   * Pon el BW16 en modo de carga (manteniendo pulsado el botón **`BURN`** o conectando `PB1` / `PA0` a tierra al resetear).
   * Selecciona el puerto COM y haz clic en **Subir**.

---

## ⚖️ Aviso Legal
Este software ha sido desarrollado con fines exclusivamente educativos y de auditoría de seguridad inalámbrica en entornos autorizados. El uso indebido para interferir redes sin consentimiento es responsabilidad única del usuario.
