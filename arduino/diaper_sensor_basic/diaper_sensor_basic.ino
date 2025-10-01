/*
 * Sensor de Humedad para Pañal - Versión Arduino + HC-05
 * 
 * Detecta humedad mediante dos electrodos conductivos y envía alertas
 * por Bluetooth serie (HC-05) cuando se detecta humedad.
 * 
 * Hardware requerido:
 * - Arduino UNO (o compatible)
 * - Módulo Bluetooth HC-05
 * - Buzzer activo
 * - Resistencia 220 kΩ
 * - Dos electrodos conductivos
 * 
 * Conexiones:
 * - Electrodo A → 3.3V a través de resistencia 220 kΩ
 * - Electrodo B → Pin analógico A0
 * - Buzzer → Pin digital 8
 * - HC-05 VCC → 3.3V (o 5V según modelo)
 * - HC-05 GND → GND
 * - HC-05 RX → Pin digital 2 (TX software)
 * - HC-05 TX → Pin digital 3 (RX software)
 * - GND común para todos los componentes
 * 
 * Creado por @eincioch con asistencia de IA
 */

#include <SoftwareSerial.h>

// Configuración de pines
const int SENSOR_PIN = A0;        // Pin analógico para lectura del sensor
const int BUZZER_PIN = 8;         // Pin digital para el buzzer
const int BT_RX_PIN = 2;          // Pin RX del módulo Bluetooth (conectar a TX del HC-05)
const int BT_TX_PIN = 3;          // Pin TX del módulo Bluetooth (conectar a RX del HC-05)

// Parámetros del sensor
const int DELTA_THRESHOLD = 50;   // Diferencia mínima sobre baseline para detectar humedad
const int HYSTERESIS = 20;        // Margen para volver a estado seco
const int SAMPLES_COUNT = 5;      // Número de muestras para promediar
const int CONSEC_WET = 3;         // Lecturas consecutivas requeridas para declarar "húmedo"
const int CONSEC_DRY = 5;         // Lecturas consecutivas requeridas para declarar "seco"

// Tiempos (en milisegundos)
const unsigned long SAMPLE_INTERVAL = 500;     // Intervalo entre muestras
const unsigned long ALERT_COOLDOWN_MS = 10000; // Tiempo mínimo entre alertas
const unsigned long BASELINE_UPDATE_INTERVAL = 30000; // Actualización de baseline

// Variables globales
SoftwareSerial bluetooth(BT_RX_PIN, BT_TX_PIN);
int baseline = 0;                 // Valor de referencia (sensor seco)
bool isWet = false;              // Estado actual del sensor
bool lastWetState = false;       // Estado anterior para detectar cambios
int wetCount = 0;                // Contador de lecturas consecutivas húmedas
int dryCount = 0;                // Contador de lecturas consecutivas secas
unsigned long lastAlertTime = 0; // Tiempo de la última alerta
unsigned long lastSampleTime = 0; // Tiempo de la última muestra
unsigned long lastBaselineUpdate = 0; // Tiempo de la última actualización de baseline
bool initialized = false;        // Flag de inicialización

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);  // Velocidad estándar para HC-05
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("=== Sensor de Humedad para Pañal ===");
  Serial.println("Inicializando...");
  bluetooth.println("Sensor de pañal iniciado");
  
  // Calibración inicial
  delay(2000); // Esperar estabilización
  calibrateBaseline();
  
  Serial.println("Calibración completa. Iniciando monitoreo...");
  bluetooth.println("Calibracion completa");
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
    
    // Debug por Serial
    Serial.print("Valor: ");
    Serial.print(sensorValue);
    Serial.print(" | Baseline: ");
    Serial.print(baseline);
    Serial.print(" | Estado: ");
    Serial.print(isWet ? "HÚMEDO" : "SECO");
    Serial.print(" | Wet/Dry: ");
    Serial.print(wetCount);
    Serial.print("/");
    Serial.println(dryCount);
  }
  
  delay(50); // Pequeña pausa para no saturar el loop
}

void calibrateBaseline() {
  Serial.println("Calibrando baseline...");
  bluetooth.println("Calibrando...");
  
  long sum = 0;
  const int calibrationSamples = 20;
  
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
  
  // Enviar por Bluetooth
  String message = "ALERTA:PAÑAL_HÚMEDO:VAL=" + String(sensorValue);
  bluetooth.println(message);
  
  // Verificar cooldown para sonido y alertas repetidas
  if (currentTime - lastAlertTime >= ALERT_COOLDOWN_MS) {
    soundAlert();
    lastAlertTime = currentTime;
  }
}

void onDryDetected(int sensorValue) {
  Serial.println("Estado: PAÑAL SECO");
  
  // Enviar por Bluetooth
  String message = "ESTADO:SECO:VAL=" + String(sensorValue);
  bluetooth.println(message);
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
    baseline = (baseline * 9 + currentValue) / 10; // Promedio ponderado lento
    Serial.print("Baseline actualizado: ");
    Serial.println(baseline);
  }
}