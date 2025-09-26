#!/usr/bin/env python3
"""
Cliente de prueba BLE para Sensor de Humedad de Pañal
=====================================================

Este script se conecta al ESP32 del sensor de pañal y recibe las notificaciones
de estado por Bluetooth Low Energy (BLE).

Requisitos:
- Python 3.7+
- bleak library: pip install bleak

Uso:
    python ble_test_client.py

El script buscará automáticamente el dispositivo "DiaperSensorESP32" y se
conectará para recibir notificaciones.

Creado por @eincioch con asistencia de IA
"""

import asyncio
import logging
import signal
import sys
from datetime import datetime
from typing import Optional

try:
    from bleak import BleakClient, BleakScanner
    from bleak.backends.characteristic import BleakGATTCharacteristic
except ImportError:
    print("Error: La biblioteca 'bleak' no está instalada.")
    print("Instálala con: pip install bleak")
    sys.exit(1)

# Configuración del sensor
DEVICE_NAME = "DiaperSensorESP32"
SERVICE_UUID = "12345678-1234-1234-1234-123456789abc"
CHARACTERISTIC_UUID = "87654321-4321-4321-4321-cba987654321"

# Configuración de logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('ble_test_log.txt', mode='a')
    ]
)
logger = logging.getLogger(__name__)

class DiaperSensorClient:
    """Cliente BLE para el sensor de pañal"""
    
    def __init__(self):
        self.client: Optional[BleakClient] = None
        self.device_address: Optional[str] = None
        self.connected = False
        self.message_count = 0
        
    async def find_device(self, timeout: float = 10.0) -> Optional[str]:
        """Busca el dispositivo del sensor de pañal"""
        print(f"Buscando dispositivo '{DEVICE_NAME}'...")
        logger.info(f"Iniciando búsqueda de dispositivo: {DEVICE_NAME}")
        
        try:
            devices = await BleakScanner.discover(timeout=timeout)
            
            for device in devices:
                if device.name == DEVICE_NAME:
                    print(f"✓ Dispositivo encontrado: {device.name} ({device.address})")
                    logger.info(f"Dispositivo encontrado: {device.name} - {device.address}")
                    return device.address
                    
            print(f"✗ Dispositivo '{DEVICE_NAME}' no encontrado")
            print("Dispositivos BLE disponibles:")
            for device in devices:
                if device.name:
                    print(f"  - {device.name} ({device.address})")
                    
            return None
            
        except Exception as e:
            logger.error(f"Error durante la búsqueda: {e}")
            print(f"Error durante la búsqueda: {e}")
            return None
    
    async def notification_handler(self, sender: BleakGATTCharacteristic, data: bytearray):
        """Maneja las notificaciones recibidas del sensor"""
        try:
            message = data.decode('utf-8')
            timestamp = datetime.now().strftime("%H:%M:%S")
            self.message_count += 1
            
            # Parsear y formatear el mensaje
            formatted_message = self.format_message(message)
            
            print(f"[{timestamp}] {formatted_message}")
            logger.info(f"Mensaje #{self.message_count}: {message}")
            
            # Acciones especiales según el tipo de mensaje
            if "ALERTA:HUMEDO" in message:
                print("🚨 ¡ALERTA DE HUMEDAD DETECTADA! 🚨")
                self.play_alert_sound()
                
        except Exception as e:
            logger.error(f"Error procesando notificación: {e}")
            print(f"Error procesando mensaje: {e}")
    
    def format_message(self, message: str) -> str:
        """Formatea los mensajes para mejor legibilidad"""
        if message.startswith("ALERTA:HUMEDO"):
            parts = message.split(":")
            if len(parts) >= 3:
                value = parts[2].replace("VAL=", "")
                return f"🚨 ALERTA: Pañal húmedo detectado (Valor: {value})"
                
        elif message.startswith("ESTADO:SECO"):
            parts = message.split(":")
            if len(parts) >= 3:
                value = parts[2].replace("VAL=", "")
                return f"✓ Estado: Pañal seco (Valor: {value})"
                
        elif message.startswith("STATUS:"):
            parts = message.split(":")
            if len(parts) >= 4:
                estado = parts[1]
                valor = parts[2].replace("VAL=", "")
                baseline = parts[3].replace("BASE=", "") if "BASE=" in parts[3] else "N/A"
                return f"📊 Estado: {estado} | Valor: {valor} | Baseline: {baseline}"
                
        elif message.startswith("BASELINE_UPDATE"):
            parts = message.split(":")
            if len(parts) >= 2:
                baseline = parts[1]
                return f"🔧 Baseline actualizado: {baseline}"
                
        elif message.startswith("INICIO:"):
            return f"🚀 Sensor iniciado: {message.split(':', 1)[1]}"
            
        elif message.startswith("CONECTADO:"):
            return f"🔗 Conexión establecida: {message.split(':', 1)[1]}"
        
        # Mensaje sin formato especial
        return f"📱 {message}"
    
    def play_alert_sound(self):
        """Reproduce un sonido de alerta en sistemas compatibles"""
        try:
            # En sistemas Unix/Linux
            import os
            os.system('echo -e "\a"')  # Bell sound
        except:
            pass  # No hacer nada si no es posible reproducir sonido
    
    async def connect_and_listen(self):
        """Conecta al dispositivo y escucha notificaciones"""
        if not self.device_address:
            print("Error: Dirección del dispositivo no encontrada")
            return False
        
        try:
            print(f"Conectando a {self.device_address}...")
            self.client = BleakClient(self.device_address)
            
            await self.client.connect()
            self.connected = True
            
            print("✓ Conectado exitosamente")
            print(f"✓ Dispositivo: {await self.client.get_device_info()}")
            logger.info(f"Conectado a dispositivo: {self.device_address}")
            
            # Verificar que el servicio existe
            services = await self.client.get_services()
            service_found = False
            
            for service in services:
                if service.uuid.lower() == SERVICE_UUID.lower():
                    service_found = True
                    print(f"✓ Servicio encontrado: {service.uuid}")
                    
                    for char in service.characteristics:
                        if char.uuid.lower() == CHARACTERISTIC_UUID.lower():
                            print(f"✓ Característica encontrada: {char.uuid}")
                            print(f"  Propiedades: {char.properties}")
                            break
                    break
            
            if not service_found:
                print(f"✗ Servicio {SERVICE_UUID} no encontrado")
                return False
            
            # Suscribirse a notificaciones
            await self.client.start_notify(CHARACTERISTIC_UUID, self.notification_handler)
            print("✓ Suscrito a notificaciones")
            print("\n=== Escuchando notificaciones (Ctrl+C para salir) ===\n")
            
            # Enviar un mensaje de confirmación (si la característica lo soporta)
            try:
                test_message = "CLIENT_CONNECTED"
                await self.client.write_gatt_char(CHARACTERISTIC_UUID, test_message.encode('utf-8'))
                print("✓ Mensaje de prueba enviado al sensor")
            except Exception as e:
                print(f"Nota: No se pudo enviar mensaje de prueba: {e}")
            
            return True
            
        except Exception as e:
            logger.error(f"Error de conexión: {e}")
            print(f"✗ Error de conexión: {e}")
            return False
    
    async def disconnect(self):
        """Desconecta del dispositivo"""
        if self.client and self.connected:
            try:
                await self.client.stop_notify(CHARACTERISTIC_UUID)
                await self.client.disconnect()
                self.connected = False
                print("\n✓ Desconectado del dispositivo")
                logger.info("Desconectado del dispositivo")
            except Exception as e:
                logger.error(f"Error durante desconexión: {e}")
                print(f"Error durante desconexión: {e}")
    
    async def run(self):
        """Ejecuta el cliente BLE"""
        print("=== Cliente de Prueba BLE - Sensor de Pañal ===\n")
        
        # Buscar dispositivo
        self.device_address = await self.find_device()
        if not self.device_address:
            print("\nConsejo: Asegúrate de que:")
            print("1. El ESP32 esté encendido y ejecutando el código del sensor")
            print("2. El Bluetooth esté habilitado en este dispositivo")
            print("3. El sensor no esté conectado a otro dispositivo")
            return
        
        # Conectar y escuchar
        if await self.connect_and_listen():
            try:
                # Mantener la conexión activa
                while self.connected:
                    await asyncio.sleep(1)
                    
                    # Verificar estado de conexión
                    if not await self.client.is_connected():
                        print("✗ Conexión perdida")
                        self.connected = False
                        break
                        
            except KeyboardInterrupt:
                print("\n⏹ Interrupción del usuario detectada")
            except Exception as e:
                logger.error(f"Error durante la ejecución: {e}")
                print(f"Error durante la ejecución: {e}")
        
        await self.disconnect()

def signal_handler(signum, frame):
    """Maneja la señal de interrupción (Ctrl+C)"""
    print("\n⏹ Señal de interrupción recibida. Saliendo...")
    sys.exit(0)

async def main():
    """Función principal"""
    # Configurar el manejador de señales
    signal.signal(signal.SIGINT, signal_handler)
    
    client = DiaperSensorClient()
    await client.run()
    
    print(f"📊 Estadísticas de la sesión:")
    print(f"   Mensajes recibidos: {client.message_count}")
    print("\n¡Gracias por usar el cliente de prueba BLE!")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n⏹ Programa interrumpido por el usuario")
    except Exception as e:
        logger.error(f"Error crítico: {e}")
        print(f"Error crítico: {e}")
        sys.exit(1)