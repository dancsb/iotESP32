#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <BLEDevice.h>
#include <BLEServer.h>

static BLEUUID     serviceUUID("57abe72e-fffc-11ed-be56-0242ac120002");
static BLEUUID commandCharUUID("82bebf9a-fffc-11ed-be56-0242ac120002");
static BLEUUID     RGBCharUUID("1623ab5a-fffe-11ed-be56-0242ac120002");

const char* WiFissid     = "iotESP32";
const char* WiFipassword = "blewifimqtt";
const char* mqttServer   = "ob6e5f6f.ala.us-east-1.emqxsl.com";
const char* mqttUserName = "ESP32_GateWay";
const char* mqttPwd      = "a8a2F2tDEJL6fPd";
const char* clientID     = "ESP32_GateWay";

// load DigiCert Global Root CA ca_cert
const char* ca_cert= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
"QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
"CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
"nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
"43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
"T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
"gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
"BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
"TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
"DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
"hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
"06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
"PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
"YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
"CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure espClient;
PubSubClient client(espClient);

BLECharacteristic *pCommandCharacteristic;
BLECharacteristic *pRGBCharacteristic;

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

void reconnect() {
  while (!client.connected()) {
    if (client.connect(clientID, mqttUserName, mqttPwd)) {
      Serial.println("MQTT connected");
      client.subscribe("command");
      client.subscribe("rgb");
      Serial.println("Topic Subscribed");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);  // wait 5sec and retry
    }
  }
}

//subscribe call back
void callback(char*topic, byte* payload, unsigned int length) {
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
  if(String(topic) == "command") {
    pCommandCharacteristic->setValue(String(payload, length).c_str());
    pCommandCharacteristic->notify();
  } else if(String(topic) == "rgb") {
    pRGBCharacteristic->setValue(String(payload, length).c_str());
    pRGBCharacteristic->notify();
  }
}

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
  Serial.println("Starting WiFi work!");
  setup_wifi();
  Serial.println("Starting BLE work!");
  espClient.setCACert(ca_cert);
  client.setServer(mqttServer, 8883); //setting MQTT server
  client.setCallback(callback); //defining function which will be called when message is recieved.
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
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  if (!client.connected()) { //if client is not connected
    reconnect(); //try to reconnect
  }
  client.loop();

  if (Serial.available()) {
    String input = Serial.readString();
    String words[2];
    words[0] = input.substring(0, input.indexOf(' '));
    words[1] = input.substring(input.indexOf(' ') + 1, input.length() - 1);
    if(words[0] == "led") {
      pCommandCharacteristic->setValue(words[1].c_str());
      pCommandCharacteristic->notify();
    } else if(words[0] == "rgb") {
      pRGBCharacteristic->setValue(words[1].c_str());
      pRGBCharacteristic->notify();
    }
  }
}