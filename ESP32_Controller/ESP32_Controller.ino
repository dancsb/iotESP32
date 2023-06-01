#include "BLEDevice.h"

#define PIN_RED 32    // GIOP32
#define PIN_GREEN 26  // GIOP26
#define PIN_BLUE 25   // GIOP25

static int rgb[3] = { 128, 128, 128 };
static int hue = 0;

// BLE service we connect to
static BLEUUID serviceUUID("57abe72e-fffc-11ed-be56-0242ac120002");
static BLEUUID commandCharUUID("82bebf9a-fffc-11ed-be56-0242ac120002");
static BLEUUID RGBCharUUID("1623ab5a-fffe-11ed-be56-0242ac120002");
//2 caractereistics, one sets the modes, one the color of the "on" mode
static BLERemoteCharacteristic* pCommandCharacteristic;
static BLERemoteCharacteristic* pRGBCharacteristic;
//our BLE device
static BLEAdvertisedDevice* myDevice;

static boolean doConnect = true;
static boolean connected = false;
static boolean doScan = false;

//0 - off, 1 - on, 2 - rainbow
static int ledMode = 0;

//in the command caracteristics we get the mode
static void commandNotifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.write(pData, length);
  String mode((char*)pData, length);
  if (mode == "off") {
    ledMode = 0;
  } else if (mode == "on") {
    ledMode = 1;
  } else if (mode == "rainbow") {
    ledMode = 2;
  }
  Serial.println();
}

//in the rgb caracteristic we get the color for the on mode
static void RGBNotifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.write(pData, length);
  String color((char*)pData, length);
  //the colors are separated by spaces
  rgb[0] = color.substring(0, color.indexOf(' ')).toInt();
  rgb[1] = color.substring(color.indexOf(' ') + 1, color.indexOf(' ', color.indexOf(' ') + 1)).toInt();
  rgb[2] = color.substring(color.indexOf(' ', color.indexOf(' ') + 1) + 1, color.length() - 1).toInt();
  Serial.println();
}
//class that handles the BLE connect and disconnect actions
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
    digitalWrite(LED_BUILTIN, LOW);
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  BLEClient* pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517);  //set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");


  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pCommandCharacteristic = pRemoteService->getCharacteristic(commandCharUUID);
  if (pCommandCharacteristic == nullptr) {
    Serial.print("Failed to find command characteristic UUID: ");
    Serial.println(commandCharUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found command characteristic");

  pRGBCharacteristic = pRemoteService->getCharacteristic(RGBCharUUID);
  if (pRGBCharacteristic == nullptr) {
    Serial.print("Failed to find RGB characteristic UUID: ");
    Serial.println(RGBCharUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found RGB characteristic");

  if (pCommandCharacteristic->canNotify())
    pCommandCharacteristic->registerForNotify(commandNotifyCallback);

  if (pRGBCharacteristic->canNotify())
    pRGBCharacteristic->registerForNotify(RGBNotifyCallback);

  connected = true;
  return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    }  // Found our server
  }    // onResult
};     // MyAdvertisedDeviceCallbacks

void HSVtoRGB(double hsv[], byte _rgb[]) {
  double h = hsv[0];
  double s = hsv[1] / 100.0;
  double v = hsv[2] / 100.0;
  double c = v * s;
  double tmp = h / 60.0;
  double tmp2 = tmp - 2 * floor(tmp / 2);
  double x = c * (1 - abs(tmp2 - 1));
  double m = v - c;
  double r, g, b;
  int i = floor(tmp);

  switch (i) {
    case 0:
      r = c;
      g = x;
      b = 0;
      break;
    case 1:
      r = x;
      g = c;
      b = 0;
      break;
    case 2:
      r = 0;
      g = c;
      b = x;
      break;
    case 3:
      r = 0;
      g = x;
      b = c;
      break;
    case 4:
      r = x;
      g = 0;
      b = c;
      break;
    case 5:
      r = c;
      g = 0;
      b = x;
      break;
  }
  _rgb[0] = constrain((int)255 * (r + m), 0, 255);
  _rgb[1] = constrain((int)255 * (g + m), 0, 255);
  _rgb[2] = constrain((int)255 * (b + m), 0, 255);
}

void setColor(int R, int G, int B) {
  analogWrite(PIN_RED, R);
  analogWrite(PIN_GREEN, G);
  analogWrite(PIN_BLUE, B);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_RED, LOW);
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_BLUE, LOW);
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}  // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (!connected && doScan) {
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
  }
  if (ledMode == 0)
    setColor(0, 0, 0);
  else if (ledMode == 1)
    setColor(rgb[0], rgb[1], rgb[2]);
  else if (ledMode == 2) {
    byte _rgb[3];
    double hsv[3] = { hue, 100, 100 };
    HSVtoRGB(hsv, _rgb);
    setColor(_rgb[0], _rgb[1], _rgb[2]);
    hue++;
    if (hue == 360)
      hue = 0;
  }
  delay(10);
}  // End of loop
