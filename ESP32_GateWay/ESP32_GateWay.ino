/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-aaaaaaaaaaaa"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;

class MyServerCallback : public BLEServerCallbacks {
void onConnect(BLEServer* pServer) {
    Serial.println("Client connected!");
    digitalWrite(LED_BUILTIN, HIGH);
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected!");
    BLEDevice::startAdvertising();
    digitalWrite(LED_BUILTIN, LOW);
  }
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32 - GateWay");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallback());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

  pCharacteristic->setValue("led: off");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readString();
    Serial.print(msg);
    std::string xd = msg.c_str();
    pCharacteristic->setValue(xd);
    pCharacteristic->notify(); 
  }
}