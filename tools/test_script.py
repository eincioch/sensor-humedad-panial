#!/usr/bin/env python3
"""
Script de prueba simple para verificar dependencias del cliente BLE
"""

def test_dependencies():
    """Verifica que todas las dependencias estén instaladas"""
    print("Verificando dependencias...")
    
    try:
        import asyncio
        print("✓ asyncio disponible")
    except ImportError:
        print("✗ asyncio no disponible")
        return False
    
    try:
        import bleak
        print(f"✓ bleak disponible (versión: {bleak.__version__})")
    except ImportError:
        print("✗ bleak no disponible")
        print("  Instala con: pip install bleak")
        return False
    
    try:
        import logging
        print("✓ logging disponible")
    except ImportError:
        print("✗ logging no disponible")
        return False
    
    print("\n✓ Todas las dependencias están disponibles")
    return True

def test_bluetooth():
    """Verifica que Bluetooth esté disponible en el sistema"""
    print("\nVerificando Bluetooth del sistema...")
    
    try:
        import asyncio
        from bleak import BleakScanner
        
        async def scan_test():
            try:
                devices = await BleakScanner.discover(timeout=2.0)
                print(f"✓ Escáner BLE funcional. {len(devices)} dispositivos encontrados")
                return True
            except Exception as e:
                print(f"✗ Error en escáner BLE: {e}")
                return False
        
        return asyncio.run(scan_test())
        
    except Exception as e:
        print(f"✗ Error verificando Bluetooth: {e}")
        return False

if __name__ == "__main__":
    print("=== Test de Sistema para Cliente BLE ===\n")
    
    deps_ok = test_dependencies()
    bt_ok = test_bluetooth()
    
    print(f"\n=== Resumen ===")
    print(f"Dependencias: {'✓ OK' if deps_ok else '✗ ERROR'}")
    print(f"Bluetooth: {'✓ OK' if bt_ok else '✗ ERROR'}")
    
    if deps_ok and bt_ok:
        print("\n🎉 Sistema listo para usar el cliente BLE!")
        print("Ejecuta: python ble_test_client.py")
    else:
        print("\n⚠️ Sistema no está listo. Revisa los errores arriba.")