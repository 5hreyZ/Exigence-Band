               IoT-Based Emergency Health Monitoring & Alert System
This project is an IoT-based health monitoring and emergency alert system that integrates multiple sensors (pulse oximeter, accelerometer, GPS, temperature, heart rate) with OLED display, GSM module, and GPS module to monitor vital signs, detect falls, and send emergency alerts to predefined contacts.
The system continuously monitors health parameters such as Heart Rate, SpO₂, Body Temperature, and Fall Detection. If abnormal values or emergencies are detected, it automatically sends an SMS alert with location details and places emergency calls to saved contacts.

# Features
- **Vital Signs Monitoring**  
  - Heart Rate (BPM)  
  - Blood Oxygen Level (SpO₂ %)  
  - Body Temperature  
- **Fall Detection** using MPU6050 Accelerometer & Gyroscope  
- **Emergency Alerts**  
  - Automatic SMS with live GPS location  
  - Emergency calls to saved contacts  
- **Manual Emergency Trigger** via push button  
- **OLED Display Output** for real-time stats & alerts  
- **Auto-Restart & Retry Mechanism** after emergency events  

---
# Hardware Requirements  
- **NodeMCU ESP8266** (Main controller)  
- **MAX30100** Pulse Oximeter & Heart Rate sensor  
- **MPU6050** Accelerometer + Gyroscope sensor  
- **GPS Module** (NEO-6M or similar)  
- **SIM800L / GSM Module**  
- **SSD1306 OLED Display (I2C)**  
- **Push Button** (Emergency trigger)  
- Jumper wires, Breadboard/PCB  

---
# Software Requirements  
- **Arduino IDE**  
- Install the following libraries:  
  - `Wire.h`  
  - `Adafruit_GFX.h`  
  - `Adafruit_SSD1306.h`  
  - `SoftwareSerial.h`  
  - `TinyGPS++.h`  
  - `MAX30100_PulseOximeter.h`  


# Pin Mapping
| NodeMCU Pin  | Connected To       | Device                  |
| ------------ | ------------------ | ----------------------- |
| **D1 (SCL)** | SCL                | OLED, MPU6050, MAX30100 |
| **D2 (SDA)** | SDA                | OLED, MPU6050, MAX30100 |
| **D0**       | INT                | MAX30100                |
| **D7 (TX)**  | RX                 | GSM Module              |
| **D8 (RX)**  | TX                 | GSM Module              |
| **D4 (TX)**  | RX                 | GPS Module              |
| **D3 (RX)**  | TX                 | GPS Module              |
| **D6**       | Push Button        | Emergency Switch        |
| **3V3**      | VCC (3.3V Devices) | OLED, MPU6050, MAX30100 |
| **Vin**      | VCC (5V Devices)   | GSM, GPS                |
| **GND**      | GND (All modules)  | Common Ground           |

● Contribution: Feel free to fork this repository, submit issues, or create pull requests for improvements.
