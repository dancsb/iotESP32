#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <BLEDevice.h>
#include <BLEServer.h>

// BLE sevice we advertise
const BLEUUID serviceUUID("57abe72e-fffc-11ed-be56-0242ac120002");
const BLEUUID commandCharUUID("82bebf9a-fffc-11ed-be56-0242ac120002");
const BLEUUID RGBCharUUID("1623ab5a-fffe-11ed-be56-0242ac120002");
BLECharacteristic* pCommandCharacteristic;
BLECharacteristic* pRGBCharacteristic;

// WIFI we connect to
const char* WiFissid = "iotESP32";
const char* WiFipassword = "blewifimqtt";
WiFiClientSecure espClient;

// MQTT we subscribe to
const char* mqttServer = "iot.dancs.org";
const char* mqttUserName = "ESP32_GateWay";
const char* mqttPwd = "a8a2F2tDEJL6fPd";
const char* clientID = "ESP32_GateWay";
PubSubClient client(espClient);

// load lets-encrypt-r3 CA
const char* ca_cert =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n"
  "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
  "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n"
  "WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
  "RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
  "AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n"
  "R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n"
  "sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n"
  "NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n"
  "Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n"
  "/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n"
  "AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n"
  "Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n"
  "FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n"
  "AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n"
  "Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n"
  "gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n"
  "PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n"
  "ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n"
  "CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n"
  "lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n"
  "avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n"
  "yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n"
  "yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n"
  "hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n"
  "HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n"
  "MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n"
  "nLRbwHOoq7hHwg==\n"
  "-----END CERTIFICATE-----\n";


void setup_wifi() {
  delay(10);

  WiFi.begin(WiFissid, WiFipassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//continously try to reconnect to wifi
void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, mqttUserName, mqttPwd)) {
      Serial.println("MQTT connected");
      client.subscribe("command");
      client.subscribe("rgb");
      Serial.println("Topic Subscribed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//subscribe call back
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print("Message size :");
  Serial.println(length);
  Serial.println();
  Serial.println("-----------------------");
  if (String(topic) == "command") {
    pCommandCharacteristic->setValue(String(payload, length).c_str());
    pCommandCharacteristic->notify();
  } else if (String(topic) == "rgb") {
    pRGBCharacteristic->setValue(String(payload, length).c_str());
    pRGBCharacteristic->notify();
  }
}

void setup_mqtt(){
  espClient.setCACert(ca_cert);
  client.setServer(mqttServer, 8883);
  client.setCallback(callback);
}
//class that handles the BLE connect and disconnect actions
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

void setup_ble(){
  BLEDevice::init("ESP32 - GateWay");
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallback());
  BLEService* pService = pServer->createService(serviceUUID);
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
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined!");
}

void setup() {
  //LED states if the 2 devices are connected through BLE or not
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);

  Serial.println("Starting WiFi work!");
  setup_wifi();

  Serial.println("Starting MQTT work!");
  setup_mqtt();
 
  Serial.println("Starting BLE work!");
  setup_ble();
}

void loop() {
  if (!client.connected()) {  //if client is not connected
    reconnect();              //try to reconnect
  }
  client.loop();

  if (Serial.available()) {
    String input = Serial.readString();
    String words[2];
    words[0] = input.substring(0, input.indexOf(' '));
    words[1] = input.substring(input.indexOf(' ') + 1, input.length() - 1);
    if (words[0] == "led") {
      pCommandCharacteristic->setValue(words[1].c_str());
      pCommandCharacteristic->notify();
    } else if (words[0] == "rgb") {
      pRGBCharacteristic->setValue(words[1].c_str());
      pRGBCharacteristic->notify();
    }
  }
}