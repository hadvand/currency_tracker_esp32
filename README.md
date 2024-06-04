# Gesture-controlled Display with MQTT Communication

Welcome to the README for the Gesture-Controlled Currency Display project. This project aims
to create an interactive currency display system controlled by gestures. The display showcases real-time
currency information obtained from a server, and user interaction is facilitated through an
external gesture sensor connected to an ESP32 board.

## Goal

The primary goal of this project is to design and implement a currency display system where users can
navigate through currency information using gestures. The project utilizes an ESP32 board, connects
to a currency station, and incorporates a gesture sensor for intuitive interaction.

## Installation

This section will walk you through the necessary steps to set up and deploy the project on your ESP32
microcontroller. Before you begin, ensure that you have all the required hardware components and a
development environment with the ESP-IDF framework installed.

### Hardware Setup

Before proceeding with the installation, make sure you have the following:

- **ESP32 Board:** The core of the system, responsible for processing data, controlling the display,
and managing user interactions.
- **Gesture Sensor (APDS9960):** An external component for detecting user gestures, enabling a
hands-free and interactive experience.
- **Display (SSD1306):** An external component for displaying UI.

#### OLED Display Setup (SSD1306)

To properly interface the SSD1306 OLED display with the ESP32 microcontroller, you need to connect
the display to specific GPIO pins. Follow the pin configuration below for a successful setup:
- Chip Select GPIO Pin Number: 5
- Data/Command GPIO Pin Number: 27
- Reset GPIO Pin Number: 17
- Master Out Slave In GPIO Pin Number: 23
- Serial Clock GPIO Pin Number: 18
Ensure that these connections are made securely, and double-check the pin mapping on your ESP32
development board to confirm the accurate placement

#### Gesture Sensor Setup (APDS9960)

For the proper integration of the APDS9960 Gesture Sensor with the ESP32, make the following
GPIO pin connections:

- Serial Data Line GPIO Pin Number: 25
- Serial Clock Line GPIO Pin Number: 26

Refer to the APDS9960 sensor and ESP32 hardware documentation for additional details on pin
configuration and any specific considerations for your setup.

Also do not forget to setup VCC and GND on both devices. Ensure that the hardware components
are connected properly to the ESP32.

#### Custom Pins Setup

If you are interested in more custom pins setup, you can change it. All the settings related to pins are
located in main.c file in root folder of a project:

```c
// Pin configurations for SSD1306 OLED display
#define CONFIG_CS_GPIO 5
#define CONFIG_DC_GPIO 27
#define CONFIG_RESET_GPIO 17
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
// Pin configurations for I2C communication
#define CONFIG_SDA_GPIO 25
#define CONFIG_SCL_GPIO 26
// APDS9960 sensor configurations
#define APDS9960_ADDR 0x39
```

### Software Installation

Follow these steps to set up the software environment for your project:

- If not already installed, download and install [Visual Studio Code](https://code.visualstudio.com/) and open it.
  
- Install PlatformIO IDE:

  - Open Visual Studio Code.
  
  - Navigate to the extensions tab.
  
  - Install and configure [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) to run ESP-IDF project.
  
- Copy files to your project folder.

- Update Wi-Fi credentials:
  
  - Open the source file `main.c`.
  
  - Locate and update the **SSID** and **PASSWORD** constants in the code with your Wi-Fi credentials.
  
- PlatformIO build:
  
  - Click on the **Build** button to compile the project.
  
  - After successful compilation, click on **Upload and Run** in the PlatformIO extension.

Ensure a stable internet connection and verify that your ESP32 is properly connected to your development machine during the software installation process.

## Documentation

For any other information look at `dokumentace.pdf`