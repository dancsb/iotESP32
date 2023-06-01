#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

static BLEUUID     serviceUUID("57abe72e-fffc-11ed-be56-0242ac120002");
static BLEUUID commandCharUUID("82bebf9a-fffc-11ed-be56-0242ac120002");
static BLEUUID     RGBCharUUID("1623ab5a-fffe-11ed-be56-0242ac120002");

BLECharacteristic *pCommandCharacteristic;
BLECharacteristic *pRGBCharacteristic;

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
  BLEService *pService = pServer->createService(serviceUUID);
  pCommandCharacteristic = pService->createCharacteristic(
                                         commandCharUUID,
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

  pRGBCharacteristic = pService->createCharacteristic(
                                         RGBCharUUID,
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );

  pCommandCharacteristic->setValue("off");
  pRGBCharacteristic->setValue("0 0 0");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readString();
    String words[2];
    words[0] = input.substring(0, input.indexOf(' '));
    words[1] = input.substring(input.indexOf(' ') + 1, input.length() - 1);
    if(words[0] == "led") {
      std::string mode = words[1].c_str();
      pCommandCharacteristic->setValue(mode);
      pCommandCharacteristic->notify(); 
    } else if(words[0] == "rgb") {
      std::string rgb = words[1].c_str();
      pRGBCharacteristic->setValue(rgb);
      pRGBCharacteristic->notify(); 
    }
  }
}