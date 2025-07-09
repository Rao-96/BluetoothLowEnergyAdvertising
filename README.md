# BluetoothLowEnergyAdvertising
Filtering Bluetooth Low energy device using advertising data from Web bluetooth API 

# 🔵 ESP32 BLE + Web Bluetooth Demo

This project demonstrates a **Bluetooth Low Energy (BLE)** setup using an **ESP32** device that advertises a unique identifier, which is then scanned and filtered using a **Web Bluetooth API**-based browser application.


Use case:
Lets say Bluetooth device need to uniquely identify and pair to a host machine.
---

## 🚀 Project Overview

- ✅ **ESP32 BLE Peripheral**
  - Continuously advertises a unique number on manufacturar data under 0xFF header packet.
  - this number is given by user using serial interface.

- 🌐 **Web Bluetooth App**
  - Scans for nearby BLE devices.
  - Filters devices based on specific manufacturar id and unique number.
  - Displays relevant device details in real time.




## 📸 Demo (Screenshots or Video)
see BluetoothLowEnergyAdvertising/demo directory


## 🧩 Tech Stack

| Component          | Tech                     		|
|--------------------|------------------------------------------|
| Microcontroller    | ESP32 (Arduino)				|
| Bluetooth Protocol | BLE (Bluetooth Low Energy) 		|
| Frontend Web App   | HTML + JavaScript (Web Bluetooth API) 	|
| Toolchain          | Arduino IDE 				|

---

## 🔧 Setup Instructions

### 🛠 ESP32 Firmware Setup

1. **Requirements:**
   - ESP32 Dev Board
   - Arduino IDE
   - USB cable

2. **Steps:**
   ```bash
   git clone https://github.com/Rao-96/BluetoothLowEnergyAdvertising.git
   cd BluetoothLowEnergyAdvertising/esp32-firmware

##Future work
1.disconnect from current device if not same previous number not advertised.
2.toggle connection between 2 host devices using 2 unique numbers.
