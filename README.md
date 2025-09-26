# Sensor de Humedad para Pañal (Proyecto: sensor-humedad-panial)

Prototipo educativo para detectar humedad en pañales (bebé o adulto) y enviar una alerta al celular usando Bluetooth Low Energy (ESP32) o un módulo Bluetooth serie (HC-05 con Arduino). Incluye zumbador local opcional.

## Objetivos
- Detección temprana de humedad para mejorar confort y reducir riesgo de dermatitis.
- Diseño simple y replicable.
- Base para futuras mejoras (MQTT, bajo consumo, app móvil, etc.).

## Implementaciones Incluidas
| Plataforma | Conectividad | Archivo principal | Notas |
|------------|--------------|-------------------|-------|
| Arduino UNO + HC-05 | Bluetooth serie (clásico) | arduino/diaper_sensor_basic/diaper_sensor_basic.ino | Envío de texto por Serial. |
| ESP32 | BLE (notify) | esp32/diaper_sensor_esp32_ble/diaper_sensor_esp32_ble.ino | Fácil de ampliar a WiFi/MQTT. |

## Hardware (Versión ESP32)
- ESP32 DevKitC (o equivalente).
- Buzzer activo (o pasivo adaptando código).
- Resistencia 220 kΩ (limitación de corriente al sensor).
- Dos tiras/hilos conductores formando un patrón de peine (separación 3–8 mm).
- Batería LiPo + cargador (TP4056) (opcional).
- Encapsulado / clip fuera del área húmeda.

## Conexión Básica (ESP32)
- Tira A → 3.3 V a través de 220 kΩ.
- Tira B → GPIO 34 (ADC).
- Buzzer → GPIO 18.
- GND común.

## Principio de Funcionamiento
La humedad reduce la resistencia entre las tiras → aumenta la lectura ADC → si supera (baseline + delta), se considera “húmedo”. Se aplican:
- Promedio de muestras.
- Lecturas consecutivas requeridas.
- Histeresis para volver a “seco”.
- Cooldown para no generar spam de alertas.

## Parámetros Clave
| Parámetro | Descripción | Dónde |
|-----------|-------------|-------|
| DELTA_THRESHOLD | Diferencia mínima sobre baseline para declarar humedad. | Ambos códigos |
| HYSTERESIS | Margen para volver a estado seco. (ESP32) | ESP32 |
| ALERT_COOLDOWN_MS | Tiempo mínimo entre alertas repetidas. | Ambos |
| CONSEC_WET / CONSEC_DRY | Lecturas consecutivas para cambio de estado. | ESP32 |

## Flujo Simplificado
1. Calibración inicial (sensor seco).
2. Lecturas periódicas (promedio).
3. Comparación contra threshold.
4. Confirmación por conteo.
5. Notificación (BLE o Serial) + beep.
6. Ajuste lento de baseline (si prolongado en seco).

## Uso (ESP32 + BLE)
1. Cargar `esp32/diaper_sensor_esp32_ble/diaper_sensor_esp32_ble.ino`.
2. Abrir app nRF Connect (Android/iOS).
3. Conectarse a “DiaperSensorESP32”.
4. Activar notificaciones de la characteristic.
5. Recibirás mensajes:  
   - `ALERTA:HUMEDO:VAL=XXXX`  
   - `ESTADO:SECO:VAL=XXXX`

## Script de Prueba (PC)
Ubicado en `tools/ble_test_client.py` (usa `bleak`):
```
pip install bleak
python tools/ble_test_client.py
```

## Versión Arduino (HC-05)
- Conecta HC-05 a RX/TX (cruzados) del Arduino (ajusta BAUD si necesario).
- Observa en monitor serie o app terminal Bluetooth el texto `ALERTA:PAÑAL_HÚMEDO`.

## Buenas Prácticas de Seguridad
- No colocar la electrónica directamente sobre zonas húmedas.
- Aislar pistas y usar tensión baja (3.3 V).
- Medición intermitente reduce riesgo de electrólisis.
- No uso clínico sin certificación.

## Mejoras Futuras
- MQTT (Home Assistant).
- Deep Sleep (ESP32) + temporizador.
- Sensor capacitivo (sin corriente directa entre electrodos).
- Monitor de batería.
- App móvil Flutter / React Native.
- Registro histórico de eventos.

## Estructura del Repositorio
```
.
├── README.md
├── LICENSE
├── .gitignore
├── arduino
│   └── diaper_sensor_basic
│       └── diaper_sensor_basic.ino
├── esp32
│   ├── diaper_sensor_esp32_ble
│   │   └── diaper_sensor_esp32_ble.ino
│   ├── platformio.ini
│   └── notes.md
└── tools
    └── ble_test_client.py
```

## Licencia
MIT (ver LICENSE).

## Aviso
Prototipo educativo. No sustituye supervisión humana ni pretende diagnóstico médico.

## Créditos
Creado por @eincioch con asistencia de IA.
