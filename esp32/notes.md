# Notas ESP32 - Sensor de Humedad para Pañal

## Configuración del Entorno de Desarrollo

### Arduino IDE
1. Instalar el soporte para ESP32:
   - Ir a Archivo → Preferencias
   - Agregar URL: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Herramientas → Placa → Gestor de tarjetas → Buscar "ESP32" → Instalar

2. Seleccionar la placa correcta:
   - Herramientas → Placa → ESP32 Arduino → "ESP32 Dev Module"
   - Puerto: Seleccionar el puerto COM correspondiente

### PlatformIO (Recomendado)
1. Instalar VS Code y la extensión PlatformIO
2. Usar el archivo `platformio.ini` incluido
3. Comando de compilación: `pio run`
4. Comando de subida: `pio run --target upload`
5. Monitor serie: `pio device monitor`

## Características del Hardware ESP32

### Ventajas sobre Arduino
- **WiFi y Bluetooth integrados**: BLE nativo sin módulos externos
- **Más memoria**: 520 KB SRAM vs 2 KB en Arduino UNO
- **Procesador más potente**: Dual-core 240 MHz vs 16 MHz
- **Más pines ADC**: 18 canales vs 6 en Arduino UNO
- **Deep Sleep**: Consumo ultra bajo para aplicaciones con batería

### Pines ADC Recomendados
- **GPIO 34**: Pin ADC1_CH6 (usado en el código)
- **GPIO 35**: Pin ADC1_CH7 (alternativa)
- **GPIO 32**: Pin ADC1_CH4 (alternativa)
- **GPIO 33**: Pin ADC1_CH5 (alternativa)

**Nota**: Evitar GPIO 0, 2, 15 (pines de boot) y GPIO 6-11 (flash SPI).

## Configuración BLE

### UUIDs del Servicio
- **Service UUID**: `12345678-1234-1234-1234-123456789abc`
- **Characteristic UUID**: `87654321-4321-4321-4321-cba987654321`

### Mensajes BLE
El sensor envía los siguientes tipos de mensajes:

| Mensaje | Descripción | Ejemplo |
|---------|-------------|---------|
| `INICIO:CALIBRACION_OK` | Confirmación de calibración inicial | - |
| `CONECTADO:SENSOR_PANAL_OK` | Confirmación de conexión BLE | - |
| `ALERTA:HUMEDO:VAL=XXXX` | Alerta de humedad detectada | `ALERTA:HUMEDO:VAL=1250` |
| `ESTADO:SECO:VAL=XXXX` | Cambio a estado seco | `ESTADO:SECO:VAL=980` |
| `STATUS:SECO:VAL=XXXX:BASE=YYYY` | Estado periódico | `STATUS:SECO:VAL=985:BASE=950` |
| `BASELINE_UPDATE:XXXX` | Actualización de baseline | `BASELINE_UPDATE:955` |

## Parámetros de Configuración

### Umbrales del Sensor
```cpp
const int DELTA_THRESHOLD = 100;  // Ajustar según sensibilidad deseada
const int HYSTERESIS = 30;        // Evita oscilaciones en el borde
const int SAMPLES_COUNT = 8;      // Más muestras = menos ruido
```

### Tiempos de Operación
```cpp
const unsigned long SAMPLE_INTERVAL = 1000;        // 1 segundo entre muestras
const unsigned long ALERT_COOLDOWN_MS = 15000;     // 15 segundos entre alertas
const unsigned long BASELINE_UPDATE_INTERVAL = 60000; // 1 minuto para baseline
```

### Contadores de Confirmación
```cpp
const int CONSEC_WET = 3;  // 3 lecturas consecutivas para confirmar húmedo
const int CONSEC_DRY = 5;  // 5 lecturas consecutivas para confirmar seco
```

## Aplicaciones Cliente BLE

### nRF Connect (Móvil)
1. Descargar "nRF Connect for Mobile" (Android/iOS)
2. Escanear dispositivos → Buscar "DiaperSensorESP32"
3. Conectar → Expandir "Unknown Service"
4. Tocar el ícono de flecha hacia abajo en la característica
5. Activar "Notify" para recibir alertas automáticas

### Apps de Terminal Bluetooth
- **Serial Bluetooth Terminal** (Android)
- **Bluetooth Terminal** (iOS)
- **LightBlue** (iOS/Android)

## Optimizaciones de Energía

### Deep Sleep (Futuro)
```cpp
#include "esp_sleep.h"

void enterDeepSleep() {
  esp_sleep_enable_timer_wakeup(30 * 1000000); // 30 segundos
  esp_deep_sleep_start();
}
```

### Configuración de CPU
```cpp
void setup() {
  setCpuFrequencyMhz(80); // Reducir frecuencia para ahorrar energía
}
```

## Solución de Problemas

### BLE No Se Conecta
1. Verificar que el ESP32 esté transmitiendo:
   - Buscar "DiaperSensorESP32" en escáneres BLE
2. Reiniciar el ESP32 si no aparece
3. Verificar alimentación (mínimo 3.3V estable)

### Lecturas Erróneas del Sensor
1. **Valores muy altos constantemente**:
   - Verificar conexión del electrodo B a GPIO 34
   - Verificar resistencia de 220kΩ en electrodo A

2. **Valores muy bajos constantemente**:
   - Verificar conexión de 3.3V al electrodo A
   - Verificar que los electrodos no estén en cortocircuito

3. **Valores inestables**:
   - Aumentar `SAMPLES_COUNT`
   - Verificar conexiones sueltas
   - Alejar de fuentes de interferencia electromagnética

### Compilación
- **Error de memoria**: Usar `Tools → Partition Scheme → "Huge APP"`
- **Error de bibliotecas**: Verificar versión de ESP32 Arduino Core (>= 2.0.0)

## Mejoras Futuras Implementables

### WiFi + MQTT
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "192.168.1.100";
const char* topic_alert = "diaper/alert";
const char* topic_status = "diaper/status";
```

### Servidor Web Local
```cpp
#include <WebServer.h>
WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", generateHTML());
}
```

### OTA (Over-The-Air Updates)
```cpp
#include <ArduinoOTA.h>

void setupOTA() {
  ArduinoOTA.setHostname("diaper-sensor");
  ArduinoOTA.begin();
}
```

## Consideraciones de Seguridad

1. **Aislamiento eléctrico**: Mantener la electrónica alejada del área húmeda
2. **Tensión baja**: Usar solo 3.3V para minimizar riesgos
3. **Encapsulado**: Proteger circuitos con caja resistente al agua
4. **Supervisión humana**: El dispositivo no sustituye el cuidado humano
5. **Uso no médico**: Solo para propósitos educativos y de comodidad

## Registro de Cambios

### v1.0.0 (Inicial)
- Implementación básica de BLE
- Detección de humedad con histéresis
- Calibración automática de baseline
- Buzzer para alertas locales
- Mensajes estructurados por BLE