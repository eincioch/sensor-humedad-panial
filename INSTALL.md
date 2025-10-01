# Guía de Instalación - Sensor de Humedad para Pañal

## Requisitos del Sistema

### Hardware
- **Para versión Arduino**: Arduino UNO + módulo HC-05
- **Para versión ESP32**: ESP32 DevKitC (recomendado)
- Buzzer activo
- Resistencia 220 kΩ
- Dos electrodos conductivos (alambre, lámina de cobre, etc.)
- Cables de conexión
- Protoboard o PCB

### Software
- **Arduino IDE** 1.8.0+ o **PlatformIO** (recomendado para ESP32)
- **Python 3.7+** (para el cliente de prueba BLE)

## Instalación Paso a Paso

### 1. Configuración del Hardware

#### Versión Arduino + HC-05
```
Arduino UNO    HC-05 Bluetooth    Otros Componentes
============   ===============    =================
3.3V       →   VCC                Electrodo A (via resistencia 220kΩ)
GND        →   GND                GND común
Pin 2      →   TX                 
Pin 3      →   RX                 
Pin A0     ←                      Electrodo B
Pin 8      →                      Buzzer (+)
GND        →                      Buzzer (-)
```

#### Versión ESP32 + BLE
```
ESP32          Componentes
=============  ===========
3.3V       →   Electrodo A (via resistencia 220kΩ)
GPIO 34    ←   Electrodo B
GPIO 18    →   Buzzer (+)
GND        →   GND común y Buzzer (-)
```

### 2. Configuración del Software

#### Opción A: Arduino IDE

1. **Instalar Arduino IDE**
   - Descargar desde https://www.arduino.cc/en/software

2. **Para ESP32, agregar soporte:**
   ```
   File → Preferences → Additional Board Manager URLs:
   https://dl.espressif.com/dl/package_esp32_index.json
   
   Tools → Board → Boards Manager → Buscar "ESP32" → Install
   ```

3. **Configurar la placa:**
   - **Arduino**: Tools → Board → Arduino UNO
   - **ESP32**: Tools → Board → ESP32 Arduino → ESP32 Dev Module

#### Opción B: PlatformIO (Recomendado para ESP32)

1. **Instalar VS Code**
   - Descargar desde https://code.visualstudio.com/

2. **Instalar extensión PlatformIO**
   - Extensions → Buscar "PlatformIO IDE" → Install

3. **Abrir el proyecto**
   ```bash
   cd esp32
   code .
   ```

### 3. Cargar el Código

#### Arduino + HC-05
1. Abrir `arduino/diaper_sensor_basic/diaper_sensor_basic.ino`
2. Verificar conexiones del HC-05 (RX/TX pueden necesitar cruzarse)
3. Seleccionar puerto COM correcto
4. Compilar y subir (Ctrl+U)

#### ESP32 + BLE
1. **Con Arduino IDE:**
   - Abrir `esp32/diaper_sensor_esp32_ble/diaper_sensor_esp32_ble.ino`
   - Tools → Board → ESP32 Dev Module
   - Compilar y subir

2. **Con PlatformIO:**
   ```bash
   cd esp32
   pio run --target upload
   pio device monitor  # Para ver output serial
   ```

### 4. Configuración del Cliente Python (Opcional)

1. **Instalar Python 3.7+**
   - Windows: https://www.python.org/downloads/
   - Linux: `sudo apt install python3 python3-pip`
   - macOS: `brew install python3`

2. **Instalar dependencias:**
   ```bash
   cd tools
   pip install -r requirements.txt
   ```

3. **Probar instalación:**
   ```bash
   python test_script.py
   ```

4. **Ejecutar cliente BLE:**
   ```bash
   python ble_test_client.py
   ```

## Verificación de la Instalación

### 1. Verificar Hardware
- [ ] Todos los cables conectados según el diagrama
- [ ] Alimentación correcta (3.3V o 5V según el módulo)
- [ ] LEDs de los módulos encendidos

### 2. Verificar Software
- [ ] Código se compila sin errores
- [ ] Se puede subir al microcontrolador
- [ ] Output en monitor serie es correcto

### 3. Pruebas Funcionales

#### Arduino + HC-05
1. Abrir monitor serie (9600 baud)
2. Verificar calibración inicial
3. Tocar los electrodos con dedo húmedo
4. Verificar alerta en monitor serie
5. Conectar por Bluetooth con app móvil

#### ESP32 + BLE
1. Abrir monitor serie (115200 baud)
2. Verificar mensajes de BLE e inicio
3. Usar app como nRF Connect para conectarse
4. Verificar notificaciones BLE

## Solución de Problemas Comunes

### Hardware
| Problema | Posible Causa | Solución |
|----------|---------------|----------|
| No se detecta humedad | Electrodos desconectados | Verificar conexiones |
| Lecturas erráticas | Interferencia electromagnética | Alejar de fuentes RF |
| No funciona el buzzer | Polaridad incorrecta | Verificar +/- del buzzer |

### Software
| Problema | Posible Causa | Solución |
|----------|---------------|----------|
| Error de compilación | Librería faltante | Instalar dependencias |
| No se puede subir | Puerto incorrecto | Seleccionar puerto COM correcto |
| BLE no funciona | ESP32 no compatible | Verificar modelo de ESP32 |

### Bluetooth
| Problema | Posible Causa | Solución |
|----------|---------------|----------|
| HC-05 no responde | Baud rate incorrecto | Cambiar a 38400 o 9600 |
| No se puede conectar | Pairing necesario | Emparejar con PIN: 1234 o 0000 |
| BLE no aparece | ESP32 con problemas | Reset del ESP32 |

## Configuración Avanzada

### Ajuste de Sensibilidad
Editar estos valores en el código:
```cpp
const int DELTA_THRESHOLD = 100;  // ↑ Menos sensible, ↓ Más sensible
const int HYSTERESIS = 30;        // Evita oscilaciones
const int CONSEC_WET = 3;         // Confirmaciones necesarias
```

### Optimización de Energía (ESP32)
```cpp
// Agregar al setup():
setCpuFrequencyMhz(80);  // Reducir frecuencia
WiFi.mode(WIFI_OFF);     // Apagar WiFi si no se usa
```

### Configuración de Deep Sleep
```cpp
#include "esp_sleep.h"

void enterDeepSleep() {
  esp_sleep_enable_timer_wakeup(30 * 1000000); // 30 segundos
  esp_deep_sleep_start();
}
```

## Siguientes Pasos

Una vez instalado y funcionando:

1. **Calibrar** para tu tipo de pañal específico
2. **Ajustar sensibilidad** según necesidades
3. **Crear encapsulado** resistente al agua
4. **Desarrollar app móvil** personalizada (opcional)
5. **Integrar con sistema domótico** (MQTT, Home Assistant)

## Soporte

Si tienes problemas:
1. Revisar esta guía completamente
2. Verificar conexiones físicas
3. Comprobar versiones de software
4. Revisar los logs/output serial
5. Crear un issue en GitHub con detalles específicos