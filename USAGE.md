# Guía de Uso - Sensor de Humedad para Pañal

## Inicio Rápido

### 1. Calibración Inicial
El sensor se calibra automáticamente al encenderse:
- Asegurar que los electrodos estén **secos**
- Encender el dispositivo
- Esperar mensaje "Calibración completa"

### 2. Monitoreo Normal
- El sensor toma lecturas cada 1-2 segundos
- Compara contra el valor base (baseline)
- Requiere lecturas consecutivas para confirmar cambios

### 3. Detección de Humedad
Cuando se detecta humedad:
- **Sonido**: 3 beeps del buzzer
- **Bluetooth**: Mensaje de alerta
- **LED**: Indicador visual (si está conectado)

## Interfaces de Usuario

### Arduino + HC-05: Monitor Serie
```
=== Sensor de Humedad para Pañal ===
Inicializando...
Calibrando...
Baseline establecido: 234
Calibración completa. Iniciando monitoreo...

Valor: 235 | Baseline: 234 | Estado: SECO | Wet/Dry: 0/5
Valor: 238 | Baseline: 234 | Estado: SECO | Wet/Dry: 0/5
Valor: 442 | Baseline: 234 | Estado: SECO | Wet/Dry: 1/0
Valor: 445 | Baseline: 234 | Estado: SECO | Wet/Dry: 2/0
Valor: 448 | Baseline: 234 | Estado: SECO | Wet/Dry: 3/0
*** ALERTA: PAÑAL HÚMEDO ***
```

### ESP32 + BLE: Notificaciones
```
Conectando a cliente BLE...
✓ Cliente BLE conectado
BLE Enviado: CONECTADO:SENSOR_PANAL_OK
BLE Enviado: STATUS:SECO:VAL=512:BASE=489
BLE Enviado: ALERTA:HUMEDO:VAL=756
BLE Enviado: ESTADO:SECO:VAL=498
```

## Mensajes del Sistema

### Tipos de Mensajes BLE

| Mensaje | Significado | Cuándo se envía |
|---------|-------------|-----------------|
| `INICIO:CALIBRACION_OK` | Calibración exitosa | Al iniciar |
| `CONECTADO:SENSOR_PANAL_OK` | Cliente conectado | Nueva conexión BLE |
| `STATUS:SECO:VAL=X:BASE=Y` | Estado periódico | Cada 5 segundos |
| `ALERTA:HUMEDO:VAL=X` | Humedad detectada | Cambio a húmedo |
| `ESTADO:SECO:VAL=X` | Vuelve a seco | Cambio a seco |
| `BASELINE_UPDATE:X` | Baseline ajustado | Ajuste automático |

### Mensajes Bluetooth Serie (HC-05)

| Mensaje | Significado |
|---------|-------------|
| `ALERTA:PAÑAL_HÚMEDO:VAL=X` | Humedad detectada |
| `ESTADO:SECO:VAL=X` | Estado seco |

## Apps Móviles Recomendadas

### Para BLE (ESP32)
1. **nRF Connect** (Android/iOS) - Recomendado
   - Buscar "DiaperSensorESP32"
   - Conectar → Expandir servicio
   - Activar notificaciones (↓)

2. **LightBlue** (iOS/Android)
   - Similar a nRF Connect
   - Interfaz más simple

3. **BLE Scanner** (Android)
   - Búsqueda automática
   - Log de mensajes

### Para Bluetooth Serie (Arduino)
1. **Serial Bluetooth Terminal** (Android)
   - Emparejar con HC-05 (PIN: 1234)
   - Conectar y ver mensajes

2. **Bluetooth Terminal** (iOS)
   - Similar funcionalidad

## Cliente Python (PC)

### Uso Básico
```bash
cd tools
python ble_test_client.py
```

### Salida Esperada
```
=== Cliente de Prueba BLE - Sensor de Pañal ===

Buscando dispositivo 'DiaperSensorESP32'...
✓ Dispositivo encontrado: DiaperSensorESP32 (AA:BB:CC:DD:EE:FF)
Conectando a AA:BB:CC:DD:EE:FF...
✓ Conectado exitosamente
✓ Servicio encontrado: 12345678-1234-1234-1234-123456789abc
✓ Característica encontrada: 87654321-4321-4321-4321-cba987654321

=== Escuchando notificaciones (Ctrl+C para salir) ===

[14:30:25] 🔗 Conexión establecida: SENSOR_PANAL_OK
[14:30:30] 📊 Estado: SECO | Valor: 512 | Baseline: 489
[14:30:45] 🚨 ALERTA: Pañal húmedo detectado (Valor: 756)
🚨 ¡ALERTA DE HUMEDAD DETECTADA! 🚨
[14:31:15] ✓ Estado: Pañal seco (Valor: 498)
```

## Configuración y Personalización

### Ajustar Sensibilidad

**En el código Arduino/ESP32:**
```cpp
// Más sensible (detecta humedad leve)
const int DELTA_THRESHOLD = 50;   // Valor más bajo

// Menos sensible (solo humedad intensa)  
const int DELTA_THRESHOLD = 150;  // Valor más alto
```

### Cambiar Tiempos de Respuesta
```cpp
// Respuesta más rápida
const int CONSEC_WET = 2;         // 2 confirmaciones
const unsigned long SAMPLE_INTERVAL = 500;  // 0.5 segundos

// Respuesta más lenta (menos falsas alarmas)
const int CONSEC_WET = 5;         // 5 confirmaciones  
const unsigned long SAMPLE_INTERVAL = 2000; // 2 segundos
```

### Modificar Alertas
```cpp
// Sin sonido (solo notificación)
void soundAlert() {
  // Comentar o eliminar código del buzzer
}

// Patrón diferente de beeps
void soundAlert() {
  for (int i = 0; i < 5; i++) {  // 5 beeps en lugar de 3
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);                   // Beeps más cortos
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
}
```

## Casos de Uso Específicos

### 1. Bebé (Pañal Desechable)
- **Sensibilidad**: Media (DELTA_THRESHOLD = 100)
- **Confirmaciones**: 3 consecutivas
- **Posición**: Electrodos en parte externa del pañal

### 2. Adulto Mayor (Pañal Nocturno)
- **Sensibilidad**: Alta (DELTA_THRESHOLD = 70)
- **Confirmaciones**: 2 consecutivas  
- **Alertas**: Sin sonido nocturno, solo BLE

### 3. Monitoreo Médico
- **Sensibilidad**: Muy alta (DELTA_THRESHOLD = 50)
- **Registro**: Log completo de eventos
- **Integración**: MQTT para sistema hospitalario

## Mantenimiento

### Limpieza de Electrodos
- **Frecuencia**: Después de cada uso
- **Método**: Alcohol isopropílico 70%
- **Secado**: Completamente antes de usar

### Calibración Periódica
- **Cuándo**: Si hay falsas alarmas frecuentes
- **Cómo**: Reiniciar el dispositivo con electrodos secos
- **Frecuencia**: Semanal en uso intensivo

### Verificación de Batería (ESP32)
```cpp
// Agregar al código para monitoreo
float batteryVoltage = analogRead(A13) * (3.3 / 4095.0) * 2;
if (batteryVoltage < 3.4) {
  Serial.println("BATERÍA BAJA");
  sendBLENotification("BATTERY_LOW:" + String(batteryVoltage));
}
```

## Integración con Sistemas Domóticos

### Home Assistant (MQTT)
```yaml
# configuration.yaml
sensor:
  - platform: mqtt
    name: "Diaper Sensor"
    state_topic: "diaper/status"
    json_attributes_topic: "diaper/attributes"

automation:
  - alias: "Diaper Alert"
    trigger:
      platform: mqtt
      topic: "diaper/alert"
    action:
      service: notify.mobile_app
      data:
        message: "Pañal húmedo detectado"
```

### Node-RED
```json
[
  {
    "id": "mqtt-in",
    "type": "mqtt in",
    "topic": "diaper/+",
    "broker": "mqtt-broker"
  },
  {
    "id": "notification",
    "type": "pushbullet",
    "title": "Alerta de Pañal"
  }
]
```

## Troubleshooting

### Problemas Comunes

1. **Falsas alarmas frecuentes**
   - Aumentar DELTA_THRESHOLD
   - Aumentar CONSEC_WET
   - Limpiar electrodos

2. **No detecta humedad**
   - Verificar conexiones
   - Disminuir DELTA_THRESHOLD
   - Recalibrar sensor

3. **BLE se desconecta**
   - Verificar distancia (<10m)
   - Restart ESP32
   - Verificar alimentación estable

4. **Consumo alto de batería**
   - Implementar deep sleep
   - Reducir frecuencia de muestreo
   - Usar modo de bajo consumo

### Logs de Debug

**Habilitar debug en ESP32:**
```cpp
#define DEBUG_SENSOR 1

#if DEBUG_SENSOR
  Serial.print("Debug: ");
  Serial.println(mensaje);
#endif
```

**Ver logs detallados:**
```bash
# PlatformIO
pio device monitor --baud 115200

# Arduino IDE
Tools → Serial Monitor → 115200 baud
```

## Próximos Pasos

1. **Prueba en condiciones reales** durante 24-48 horas
2. **Ajusta parámetros** según resultados
3. **Implementa mejoras** (app móvil, MQTT, etc.)
4. **Documenta configuración** específica para tu uso
5. **Comparte feedback** para mejorar el proyecto