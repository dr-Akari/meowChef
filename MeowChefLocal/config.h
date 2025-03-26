#ifndef CONFIG_H
#define CONFIG_H

// ----- Pin & Timing Definitions ----- //
#define RESET_BUTTON_PIN    0    // Pin to clear WiFi credentials
#define PHYS_BUTTON_PIN    15    // Physical pushbutton for dispensing
#define SERVO_PIN          13    // Servo is connected to pin 13

// For a continuous rotation servo:
const double NEUTRAL_POS  = 90;   // Servo stop (neutral)
const double DISPENSE_POS = 0;    // Clockwise rotation (dispense food)

// Default dose when triggered via HomeKit (in seconds)
const double DEFAULT_DOSE = 3;

// Timing constants for physical pushbutton dispensing (in milliseconds)
const unsigned long BASE_DISPENSE_MS = 3000; // Start with 3 seconds
const unsigned long ADD_SINGLE_MS    = 1000; // SINGLE press adds 1 sec
const unsigned long ADD_DOUBLE_MS    = 2000; // DOUBLE press adds 2 sec
const unsigned long ADD_LONG_MS      = 4000; // LONG press adds 4 sec

#endif // CONFIG_H
