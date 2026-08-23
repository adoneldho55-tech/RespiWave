
# Acknowledgement

RespiWave is built using open-source hardware and software components. The project makes use of the ESP32 platform along with libraries from the Adafruit ecosystem for interfacing with the SSD1306 OLED display, BMP180 pressure sensor, MPU6050 motion sensor, and APDS9960 gesture sensor.

Special thanks to the open-source hardware and software communities whose libraries, documentation, and development tools make projects such as RespiWave possible.

# Overview

RespiWave is an ESP32-based sensor monitoring system designed to provide a compact, real-time view of environmental, motion, and interaction data.

The system combines multiple I²C sensors with a local OLED display and a browser-based web dashboard. Sensor readings are processed by the ESP32 and made available both locally on the device and remotely through a web interface over Wi-Fi.

The project is designed as a foundation for a patient-monitoring and safety-oriented sensing platform, where sensor data can be observed continuously without requiring a dedicated computer or mobile application.

**Problem Solved: ** Traditional sensor-monitoring prototypes often require a computer, serial monitor, or separate application to observe live measurements. RespiWave provides a self-contained monitoring interface directly from the ESP32.

**Who is it for:** RespiWave is designed for **students, makers, developers, and researchers** exploring ESP32, IoT, and multi-sensor monitoring systems. It can also serve as a foundation for healthcare-oriented prototypes and sensor-based applications. It is currently a **prototype and development platform**, not a certified medical device.

**Key Features:** RespiWave combines **BMP180 pressure monitoring, MPU6050 motion and temperature sensing, APDS9960 touchless gestures, and an SSD1306 OLED display** with Wi-Fi connectivity. It provides a **responsive real-time web dashboard**, live JSON API, visual system and buzzer status, gesture-based alarm control, and requires **no external server or database**.

# Demo/Examples

# Images

![[respiwave_cover_image.jpeg]]

*The core hardware build for RespiWave, featuring a modular ESP32 setup streaming real-time sensor telemetry to an onboard OLED display.*


![[respiwave_demo_image.jpeg]]

*Real-time telemetry streaming straight from a modular ESP32 sensor array to a live web dashboard.*

![[respiwave_web_interface.jpeg]]

*RespiWave's live web dashboard streaming real-time barometric pressure, motion vectors, and device telemetry over the local network.*
# Video

![[demo_video.mp4]]


---

# Features (Detailed)

RespiWave combines multiple sensors with the ESP32 to provide real-time monitoring through both a local OLED display and a web dashboard.

 **Multi-Sensor Monitoring**

The system integrates:

- **BMP180** for pressure and altitude.
- **MPU6050** for acceleration and temperature.
- **APDS9960** for touchless gesture detection.
- **SSD1306 OLED** for local status information.

 **Real-Time Monitoring**

Sensor readings are continuously processed and displayed through the ESP32. The web dashboard provides live pressure, temperature, acceleration, system status, and buzzer information.

 **Web Dashboard**

The ESP32 hosts a responsive web interface directly, requiring no external server or database. It includes live sensor cards, acceleration visualization, pressure history, connection status, and alarm indicators.

**Gesture & Alarm Control**

The APDS9960 enables touchless interaction using **UP** and **DOWN** gestures. These gestures can be used to snooze an active buzzer while the latest detected gesture is shown on the dashboard.

 **Live API**

A `/data` JSON endpoint provides the latest sensor readings and system information. The dashboard automatically polls the endpoint to keep the displayed data updated in real time.

 **Wi-Fi Connectivity**

The ESP32 connects directly to a configured Wi-Fi network and serves the complete monitoring dashboard over HTTP, allowing the system to be accessed from any device on the same network.

# Usage Instructions

1. **Connect the Hardware**  
    Connect the sensors and OLED to the ESP32 using I²C. The firmware uses **SDA: GPIO 21**, **SCL: GPIO 22**, and **Buzzer: GPIO 25**.
    
2. **Configure Wi-Fi**  
    Enter your Wi-Fi **SSID** and **password** in the firmware.
    
3. **Install & Upload**  
    Install the required Adafruit libraries, select your ESP32 board and port in Arduino IDE, then compile and upload the firmware.
    
4. **Connect & Access**  
    Open the Serial Monitor at **115200 baud**. Once connected, the ESP32 will display its IP address.
    
5. **Open the Dashboard**  
    Open the displayed IP address in a browser on the same Wi-Fi network to access the RespiWave dashboard.
    
6. **Monitor**  
    View live **pressure, temperature, acceleration, system status, buzzer state, and gestures** through the dashboard and OLED display.

# Tech Stack

**Firmware**

- C++
- Arduino Framework
- ESP32 Wi-Fi
- I²C communication
- Embedded HTTP server

**Libraries**

- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit BMP085 Library
- Adafruit MPU6050
- Adafruit APDS9960

**Web Interface**

- HTML5
- CSS3
- Vanilla JavaScript
- Fetch API
- JSON

No external frontend framework or backend server is required
# Requirements / Installations

## Hardware Requirements

- 1× ESP32 development board
- 1× BMP180 pressure sensor
- 1× MPU6050 sensor
- 1× APDS9960 gesture sensor
- 1× SSD1306 128×64 OLED display
- 1× Buzzer
- Jumper wires
- USB cable
- Wi-Fi network

## Software Requirements

- Arduino IDE
- ESP32 board support package
- Required Adafruit libraries
- A modern web browser
- A Wi-Fi network accessible by both the ESP32 and monitoring device

## Installation

1. Install Arduino IDE.
2. Add ESP32 board support.
3. Install the required Adafruit libraries.
4. Open the `respiwave.ino` project.
5. Configure the Wi-Fi SSID and password.
6. Verify the I²C wiring and sensor addresses.
7. Select the correct ESP32 board and serial port.
8. Upload the firmware.
9. Open the Serial Monitor at 115200 baud.
10. Obtain the ESP32 IP address.
11. Open that IP address in a browser on the same network.

# File structure

```
Respiwave/
├── code/
│   └── respiwave.ino
├── media/
│   ├── demo_video.mp4
│   ├── respiwave_cover_image.jpeg
│   ├── respiwave_demo_image.jpeg
│   └── respiwave_web_interface.jpeg
└── README.md
```
# License 

RespiWave is released under the MIT License.
# Contribution

