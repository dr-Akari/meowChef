# MeowChef: ESP32 Based Pet Feeder with Scheduling

## Overview
MeowChef is an innovative, ESP32-based pet feeder that combines automated feeding with a user-friendly scheduling system. Designed for simplicity and reliability, it allows you to set feeding times and monitor your pet's meals with minimal hassle. 

## Features
- **Scheduled Feeding**: Dispense food automatically based on a customizable schedule.
- **Local Configuration**:
  - **Access Point Mode**: On first-time setup, the device creates a Wi-Fi access point named `MeowChef-Captive`.
  - **Web Interface**: Configure your feeder via [http://meowchef.local](http://meowchef.local).
- **Key Hardware Components**:
  - **Servo Motor**: Continuous rotating MG995 for precise food dispensing.
  - **Controller**: ESP WROOM 32 development module.
  - **Mechanical Assembly**: Utilizes 7 heat inserts with appropriate screws for a robust build.
  - **Feeding Container**: 9.5-10cm diameter vacuum cylindrical container to store pet food.
  - **Camera Integration**: Supports C200/C210/C220 IP cameras (mounting similar to provided attachment). Note: The cameras operate independently from the ESP32 via their native client apps.

## Hardware Setup
- **ESP32 Module**: Acts as the central controller, handling both the scheduling and local configuration.
- **Servo Motor (MG995)**: Provides continuous rotation for controlled food dispensing.
- **3D Printed Parts**: All custom parts are designed in Fusion 360. The project files and STL files are available in the `3DPrintModels` folder.
- **Mounting and Assembly**: Secure all components with the heat inserts and screws, ensuring stable operation.
- **Camera Mounting**: Designed for an integration with IP cameras (C200/C210/C220), though these are configured separately using their native apps.

## Firmware
- The Arduino-based firmware for the ESP32 is located in the `MeowChefLocal` folder.
- The firmware supports initial configuration through the web interface and manages the feeding schedule.

## Setup and Installation
1. **Power Up**: Connect the ESP32 module to a power source.
2. **Connect to Access Point**: On first boot, connect your computer or mobile device to the `MeowChef-Captive` Wi-Fi network.
3. **Configure the Feeder**:
   - Open a web browser and navigate to [http://meowchef.local](http://meowchef.local).
   - Set up your feeding schedule and adjust other settings via the configuration UI.
4. **Assemble the Hardware**:
   - Mount the ESP32 module, servo motor, and feeding container as per the provided assembly instructions.
   - Secure all 3D printed parts using the heat inserts and screws.
5. **Integrate the Camera**: Mount and set up your IP camera (C200/C210/C220) using its native client application.

## To Dos / Future Enhancements
- **Fix Model's Top Shield Mounting Holes**: Refine the design to ensure perfect alignment.
- **Fix Config UI**: Improve the configuration interface for a more intuitive user experience.
- **Add Watchdog Reset**: Implement a 24-hour watchdog reset to enhance system reliability.
- **Add Dispense Failure Tracking**: Introduce a mechanism to monitor and report dispensing failures.

## Acknowledgements
- **Inspiration**: This project draws inspiration from PEDRO, an Arduino-based pet feeder design by Manuel Maeder. For more details, check out the [YouTube video](https://www.youtube.com/watch?v=Uv0lsih8JRA&t=371s).

## License
Apache 2.0

---