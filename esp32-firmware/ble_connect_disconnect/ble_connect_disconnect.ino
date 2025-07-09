#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include <HIDTypes.h>
#include <HIDKeyboardTypes.h>

#define LED_PIN 2
#define USER_DATA_SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define USER_DATA_CHAR_UUID    "abcd1234-ab12-cd34-ef56-abcdef123456"

BLEHIDDevice* hid;
BLEServer* pServer;
BLECharacteristic* userChar;
BLECharacteristic* inputReport;

bool deviceConnected = false;
unsigned long lastBlink = 0;
bool ledState = false;
std::string currentUniqueID = "";
std::string currentConnectedAddress = "";

bool isValidHexString(const std::string& hex) {
  if (hex.length() % 2 != 0) return false;
  for (char c : hex) {
    if (!isxdigit(c)) return false;
  }
  return true;
}
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    deviceConnected = true;
    Serial.println("Device connected.");
  }

  void onDisconnect(BLEServer* pServer) override {
    deviceConnected = false;
    currentConnectedAddress = "";
    Serial.println("Device disconnected.");
  }
};

void setupHID() {
  hid = new BLEHIDDevice(pServer);

  const uint8_t reportMap[] = {
    0x05, 0x01,
    0x09, 0x06,
    0xA1, 0x01,
    0x85, 0x01,
    0x05, 0x07,
    0x19, 0xE0,
    0x29, 0xE7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,
    0xC0
  };

  hid->manufacturer()->setValue("ESP32 HID Dev");
  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00, 0x01);
  hid->reportMap((uint8_t*)reportMap, sizeof(reportMap));
  hid->startServices();

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);
  hid->setBatteryLevel(100);
}

void sendKey(uint8_t keyCode) {
  if (!deviceConnected || hid == nullptr) return;

  BLECharacteristic* inputReport = hid->inputReport(1);
  uint8_t report[] = {0x02, 0x00, keyCode, 0, 0, 0, 0, 0};
  inputReport->setValue(report, sizeof(report));
  inputReport->notify();
  delay(10);
  uint8_t releaseReport[] = {0x02, 0x00, 0, 0, 0, 0, 0, 0};
  inputReport->setValue(releaseReport, sizeof(releaseReport));
  inputReport->notify();
}

void setupDeviceInfoService() {
  BLEService *deviceInfo = pServer->createService(BLEUUID((uint16_t)0x180A));
  BLECharacteristic* manufacturerChar = deviceInfo->createCharacteristic(BLEUUID((uint16_t)0x2A29), BLECharacteristic::PROPERTY_READ);
  manufacturerChar->setValue("ESP32 Inc.");
  BLECharacteristic* modelChar = deviceInfo->createCharacteristic(BLEUUID((uint16_t)0x2A24), BLECharacteristic::PROPERTY_READ);
  modelChar->setValue("Model X");
  deviceInfo->start();
}

void setupUserDataService() {
  BLEService *userService = pServer->createService(USER_DATA_SERVICE_UUID);
  userChar = userService->createCharacteristic(
    USER_DATA_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  userChar->setValue("Initial");
  userService->start();
}

void advertiseUserDataWithUniqueNumber(const std::string& uniqueNum) {
  currentUniqueID = uniqueNum;

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->stop();
  delay(100);
  pAdvertising->addServiceUUID(BLEUUID((uint16_t)0x180A));
  pAdvertising->setScanResponse(true);

  BLEAdvertisementData advertisementData;
  BLEAdvertisementData scanResponseData;

  std::string mfgData = "\x01\x01";  // Company ID LSB + MSB
  // Validate uniqueNum string
  if (!isValidHexString(uniqueNum)) {
    Serial.println("Error: uniqueNum must be a valid hex string with even length.");
    return;
  }

  // Convert hex string to raw byte array
  std::vector<uint8_t> bytes;
  for (size_t i = 0; i < uniqueNum.length(); i += 2) {
    std::string byteStr = uniqueNum.substr(i, 2);
    uint8_t byte = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    bytes.push_back(byte);
  }

  // Reverse the byte array
  std::reverse(bytes.begin(), bytes.end());

  // Append reversed bytes to manufacturer data
  for (auto b : bytes) {
    mfgData += (char)b;
  }


  advertisementData.setName("ESP-HID-LE");
  advertisementData.setManufacturerData(mfgData);

  pAdvertising->setAdvertisementData(advertisementData);
  pAdvertising->setScanResponseData(scanResponseData);
  pAdvertising->start();

  Serial.print("Advertising Unique ID (reversed): ");
  for (int i = 11; i >= 0; --i) Serial.print(uniqueNum[i]);
  Serial.println();
}

void disconnectCurrentClient() {
  if (deviceConnected) {
    pServer->disconnect(0); // assumes single connection; use connectionId 0
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE HID + Device Info + Dynamic User Data");

  pinMode(LED_PIN, OUTPUT);

  BLEDevice::init("ESP32-BLE-HID");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

//  setupHID();
  setupDeviceInfoService();
  setupUserDataService();

  advertiseUserDataWithUniqueNumber("000000000000");
  Serial.println("Initial advertising started.");
}

void loop() {
  // LED Control
  if (deviceConnected) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    unsigned long now = millis();
    if (now - lastBlink >= 500) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
      lastBlink = now;
    }
  }

  // Serial input handler
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
      std::string inputStr = input.c_str();

      if (deviceConnected) {
        if (inputStr == currentUniqueID) {
          Serial.println("Same number input while connected: disconnecting...");
          disconnectCurrentClient();
        } else {
          Serial.println("Different number input while connected: disconnecting and updating...");
          disconnectCurrentClient();
          advertiseUserDataWithUniqueNumber(inputStr);
        }
      } else {
        Serial.println("Curently device not connected do same...");
        advertiseUserDataWithUniqueNumber(inputStr);
      }
    
  }
}
