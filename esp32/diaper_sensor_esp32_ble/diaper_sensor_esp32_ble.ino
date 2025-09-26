/*
 * Sensor de Humedad para Pañal - Versión ESP32 + BLE
 * 
 * Detecta humedad mediante dos electrodos conductivos y envía alertas
 * por Bluetooth Low Energy (BLE) cuando se detecta humedad.
 * 
 * Hardware requerido:
 * - ESP32 DevKitC (o compatible)
 * - Buzzer activo (o pasivo adaptando código)
 * - Resistencia 220 kΩ
 * - Dos electrodos conductivos
 * - Batería LiPo + cargador TP4056 (opcional)
 * 
 * Conexiones:
 * - Electrodo A → 3.3V a través de resistencia 220 kΩ
 * - Electrodo B → GPIO 34 (ADC)
 * - Buzzer → GPIO 18
 * - GND común para todos los componentes
 * 
 * BLE Service: Servicio personalizado para sensor de pañal
 * BLE Characteristic: Notificaciones de estado del sensor
 * 
 * Creado por @eincioch con asistencia de IA
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Configuración de pines
const int SENSOR_PIN = 34;        // Pin analógico GPIO34 (ADC1_CH6)
const int BUZZER_PIN = 18;        // Pin digital GPIO18

// UUIDs para el servicio BLE (generados para este proyecto)
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

// Parámetros del sensor
const int DELTA_THRESHOLD = 100;  // Diferencia mínima sobre baseline para detectar humedad
const int HYSTERESIS = 30;        // Margen para volver a estado seco
const int SAMPLES_COUNT = 8;      // Número de muestras para promediar
const int CONSEC_WET = 3;         // Lecturas consecutivas requeridas para declarar "húmedo"
const int CONSEC_DRY = 5;         // Lecturas consecutivas requeridas para declarar "seco"

// Tiempos (en milisegundos)
const unsigned long SAMPLE_INTERVAL = 1000;    // Intervalo entre muestras
const unsigned long ALERT_COOLDOWN_MS = 15000; // Tiempo mínimo entre alertas
const unsigned long BASELINE_UPDATE_INTERVAL = 60000; // Actualización de baseline
const unsigned long BLE_UPDATE_INTERVAL = 5000; // Intervalo de actualización BLE

// Variables globales
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;

int baseline = 0;                 // Valor de referencia (sensor seco)
bool isWet = false;              // Estado actual del sensor
bool lastWetState = false;       // Estado anterior para detectar cambios
int wetCount = 0;                // Contador de lecturas consecutivas húmedas
int dryCount = 0;                // Contador de lecturas consecutivas secas
unsigned long lastAlertTime = 0; // Tiempo de la última alerta
unsigned long lastSampleTime = 0; // Tiempo de la última muestra
unsigned long lastBaselineUpdate = 0; // Tiempo de la última actualización de baseline
unsigned long lastBLEUpdate = 0; // Tiempo de la última actualización BLE
bool initialized = false;        // Flag de inicialización

// Callbacks para conexión BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("=== Sensor de Humedad para Pañal ESP32 ===");
  Serial.println("Inicializando BLE...");
  
  // Configurar BLE
  setupBLE();
  
  // Calibración inicial
  delay(2000); // Esperar estabilización
  calibrateBaseline();
  
  Serial.println("Calibración completa. Iniciando monitoreo...");
  sendBLENotification("INICIO:CALIBRACION_OK");
  initialized = true;
}

void loop() {
  unsigned long currentTime = millis();
  
  // Verificar si es tiempo de tomar una muestra
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = currentTime;
    
    int sensorValue = readSensorAverage();
    processSensorReading(sensorValue, currentTime);
    
    // Actualización periódica de baseline (solo si está seco por mucho tiempo)
    if (currentTime - lastBaselineUpdate >= BASELINE_UPDATE_INTERVAL && !isWet) {
      updateBaseline(sensorValue);
      lastBaselineUpdate = currentTime;
    }
    
    // Enviar actualización BLE periódica
    if (currentTime - lastBLEUpdate >= BLE_UPDATE_INTERVAL && deviceConnected) {
      sendStatusUpdate(sensorValue);
      lastBLEUpdate = currentTime;
    }
    
    // Debug por Serial
    Serial.print("Valor: ");
    Serial.print(sensorValue);
    Serial.print(" | Baseline: ");
    Serial.print(baseline);
    Serial.print(" | Estado: ");
    Serial.print(isWet ? "HÚMEDO" : "SECO");
    Serial.print(" | BLE: ");
    Serial.print(deviceConnected ? "Conectado" : "Desconectado");
    Serial.print(" | Wet/Dry: ");
    Serial.print(wetCount);
    Serial.print("/");
    Serial.println(dryCount);
  }
  
  // Manejar reconexión BLE
  handleBLEReconnection();
  
  delay(100); // Pequeña pausa para no saturar el loop
}

void setupBLE() {
  // Crear dispositivo BLE
  BLEDevice::init("DiaperSensorESP32");
  
  // Crear servidor BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Crear servicio BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Crear característica BLE
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Agregar descriptor para notificaciones
  pCharacteristic->addDescriptor(new BLE2902());
  
  // Iniciar servicio
  pService->start();
  
  // Iniciar advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  
  Serial.println("Esperando conexión de cliente BLE...");
}

void calibrateBaseline() {
  Serial.println("Calibrando baseline...");
  
  long sum = 0;
  const int calibrationSamples = 30;
  
  for (int i = 0; i < calibrationSamples; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(100);
  }
  
  baseline = sum / calibrationSamples;
  Serial.print("Baseline establecido: ");
  Serial.println(baseline);
}

int readSensorAverage() {
  long sum = 0;
  for (int i = 0; i < SAMPLES_COUNT; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(10);
  }
  return sum / SAMPLES_COUNT;
}

void processSensorReading(int sensorValue, unsigned long currentTime) {
  bool currentReading = sensorValue > (baseline + DELTA_THRESHOLD);
  
  if (currentReading) {
    // Lectura indica humedad
    wetCount++;
    dryCount = 0;
    
    if (!isWet && wetCount >= CONSEC_WET) {
      // Cambio a estado húmedo
      isWet = true;
      onWetDetected(sensorValue, currentTime);
    }
  } else {
    // Lectura indica sequedad
    dryCount++;
    wetCount = 0;
    
    // Usar histéresis para volver a seco
    if (isWet && dryCount >= CONSEC_DRY && sensorValue < (baseline + DELTA_THRESHOLD - HYSTERESIS)) {
      // Cambio a estado seco
      isWet = false;
      onDryDetected(sensorValue);
    }
  }
}

void onWetDetected(int sensorValue, unsigned long currentTime) {
  Serial.println("*** ALERTA: PAÑAL HÚMEDO ***");
  
  // Enviar por BLE
  String message = "ALERTA:HUMEDO:VAL=" + String(sensorValue);
  sendBLENotification(message);
  
  // Verificar cooldown para sonido y alertas repetidas
  if (currentTime - lastAlertTime >= ALERT_COOLDOWN_MS) {
    soundAlert();
    lastAlertTime = currentTime;
  }
}

void onDryDetected(int sensorValue) {
  Serial.println("Estado: PAÑAL SECO");
  
  // Enviar por BLE
  String message = "ESTADO:SECO:VAL=" + String(sensorValue);
  sendBLENotification(message);
}

void sendStatusUpdate(int sensorValue) {
  String status = isWet ? "HUMEDO" : "SECO";
  String message = "STATUS:" + status + ":VAL=" + String(sensorValue) + ":BASE=" + String(baseline);
  sendBLENotification(message);
}

void sendBLENotification(String message) {
  if (deviceConnected && pCharacteristic != nullptr) {
    pCharacteristic->setValue(message.c_str());
    pCharacteristic->notify();
    Serial.print("BLE Enviado: ");
    Serial.println(message);
  }
}

void soundAlert() {
  // Patrón de beeps para alerta
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

void updateBaseline(int currentValue) {
  // Actualización gradual del baseline si ha estado seco por mucho tiempo
  // Esto ayuda a compensar deriva del sensor
  if (abs(currentValue - baseline) < DELTA_THRESHOLD / 2) {
    int oldBaseline = baseline;
    baseline = (baseline * 19 + currentValue) / 20; // Promedio ponderado muy lento
    
    if (abs(baseline - oldBaseline) > 2) { // Solo reportar si hay cambio significativo
      Serial.print("Baseline actualizado: ");
      Serial.print(oldBaseline);
      Serial.print(" -> ");
      Serial.println(baseline);
      
      if (deviceConnected) {
        String message = "BASELINE_UPDATE:" + String(baseline);
        sendBLENotification(message);
      }
    }
  }
}

void handleBLEReconnection() {
  // Manejar reconexión después de desconexión
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Dar tiempo al stack BLE para prepararse
    pServer->startAdvertising(); // Reiniciar advertising
    Serial.println("Iniciando advertising nuevamente");
    oldDeviceConnected = deviceConnected;
  }
  
  // Conectándose
  if (deviceConnected && !oldDeviceConnected) {
    // Hacer algo cuando se conecta un dispositivo
    Serial.println("Cliente BLE conectado");
    sendBLENotification("CONECTADO:SENSOR_PANAL_OK");
    oldDeviceConnected = deviceConnected;
  }
}