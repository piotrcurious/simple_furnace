// Include the libraries
#include <Ticker.h>
#include <avr/wdt.h>
#include <SmoothThermistor.h> // Include the SmoothThermistor library
#include <VT100.h> // Include the VT100 library

// Define the pins
#define INPUT_FAN_PIN 2 // Input fan hall sensor pin
#define OUTPUT_FAN_PIN 3 // Output fan PWM pin
#define COMBUSTION_PIN A0 // Combustion control analog input pin
#define RESTART_PIN 4 // Input fan restart pin
#define BEEP_PIN 5 // Beep pin
#define TEMPERATURE_PIN A1 // Temperature sensor pin
#define OVERLOAD_PIN 6 // Overload warning light pin

// Define the constants
#define RPM_THRESHOLD 100 // Minimum RPM of input fan
#define RPM_BOOST 200 // RPM of input fan to boost output fan speed
#define RESTART_TIME 500 // Restart time for input fan in ms
#define SAFETY_INTERVAL 100 // Safety task interval in ms
#define TEMPERATURE_THRESHOLD 50 // Temperature threshold to boost output fan speed in Celsius
#define BOOST_VALUE 50 // Output fan speed boost value
#define VISUALIZATION_INTERVAL 1000 // Visualization task interval in ms

// Define the positions and sizes of the visualization objects
#define INPUT_FAN_RPM_X 5 // Input fan RPM x position
#define INPUT_FAN_RPM_Y 5 // Input fan RPM y position
#define INPUT_FAN_RPM_W 10 // Input fan RPM width
#define INPUT_FAN_RPM_H 3 // Input fan RPM height
#define OUTPUT_FAN_SPEED_X 20 // Output fan speed x position
#define OUTPUT_FAN_SPEED_Y 5 // Output fan speed y position
#define OUTPUT_FAN_SPEED_W 10 // Output fan speed width
#define OUTPUT_FAN_SPEED_H 3 // Output fan speed height
#define COMBUSTION_LEVEL_X 5 // Combustion level x position
#define COMBUSTION_LEVEL_Y 10 // Combustion level y position
#define COMBUSTION_LEVEL_W 10 // Combustion level width
#define COMBUSTION_LEVEL_H 3 // Combustion level height
#define TEMPERATURE_X 20 // Temperature x position
#define TEMPERATURE_Y 10 // Temperature y position
#define TEMPERATURE_W 10 // Temperature width
#define TEMPERATURE_H 3 // Temperature height
#define RATIO_X 5 // Ratio x position
#define RATIO_Y 15 // Ratio y position
#define RATIO_W 10 // Ratio width
#define RATIO_H 3 // Ratio height
#define BEEP_X 20 // Beep x position
#define BEEP_Y 15 // Beep y position
#define BEEP_W 10 // Beep width
#define BEEP_H 3 // Beep height
#define OVERLOAD_X 20 // Overload x position
#define OVERLOAD_Y 20 // Overload y position
#define OVERLOAD_W 10 // Overload width
#define OVERLOAD_H 3 // Overload height

// Declare the global variables
volatile int inputFanCount = 0; // Input fan pulse count
float inputFanRPM = 0; // Input fan RPM
int outputFanSpeed = 0; // Output fan speed
int combustionLevel = 0; // Combustion level
bool beepState = false; // Beep state
int temperature = 0; // Temperature in Celsius
bool overloadState = false; // Overload state
Ticker safetyTicker; // Ticker object for safety tasks
Ticker visualizationTicker; // Ticker object for visualization tasks
SmoothThermistor smoothThermistor(TEMPERATURE_PIN, ADC_SIZE_10_BIT, 10000, 10000, 3950, 25, 10); // Create a SmoothThermistor object with the given parameters
VT100 vt100; // Create a VT100 object

// Interrupt service routine for input fan hall sensor
void inputFanISR() {
  inputFanCount++; // Increment the pulse count
}

// Function to measure the input fan RPM
void measureInputFanRPM() {
  inputFanRPM = inputFanCount * 60.0 / 2.0; // Calculate the RPM
  inputFanCount = 0; // Reset the pulse count
}

// Function to read the combustion level
void readCombustionLevel() {
  combustionLevel = analogRead(COMBUSTION_PIN); // Read the combustion level
}

// Function to read the temperature
void readTemperature() {
  temperature = smoothThermistor.temperature(); // Read the temperature using the SmoothThermistor library
}

// Function to control the output fan speed
void controlOutputFanSpeed() {
  outputFanSpeed = map(combustionLevel, 0, 1023, 0, 255); // Map the combustion level to output fan speed
  if (inputFanRPM < RPM_BOOST || temperature > TEMPERATURE_THRESHOLD) { // If the input fan RPM is below the boost threshold or the temperature is above the threshold
    if (outputFanSpeed + BOOST_VALUE > 255) { // If the output fan speed plus the boost value exceeds the maximum PWM value
      overloadState = true; // Set the overload state to true
      digitalWrite(OVERLOAD_PIN, overloadState); // Write the overload state
    }
    else { // If the output fan speed plus the boost value does not exceed the maximum PWM value
      outputFanSpeed = outputFanSpeed + BOOST_VALUE; // Increase the output fan speed by the boost value
      overloadState = false; // Set the overload state to false
      digitalWrite(OVERLOAD_PIN, overloadState); // Write the overload state
    }
  }
  analogWrite(OUTPUT_FAN_PIN, outputFanSpeed); // Write the output fan speed
}

// Function to check the airflow rate
void checkAirflowRate() {
  if (inputFanRPM < RPM_THRESHOLD) { // If the input fan RPM is below the threshold
    beepState = true; // Set the beep state to true
    digitalWrite(BEEP_PIN, beepState); // Write the beep state
    if (inputFanRPM == 0) { // If the input fan is stalled
      digitalWrite(RESTART_PIN, HIGH); // Write high to the restart pin
      delay(RESTART_TIME); // Wait for the restart time
      digitalWrite(RESTART_PIN, LOW); // Write low to the restart pin
    }
  }
  else { // If the input fan RPM is above the threshold
    beepState = false; // Set the beep state to false
    digitalWrite(BEEP_PIN, beepState); // Write the beep state
  }
}

// Function to perform the safety tasks
void safetyTask() {
  measureInputFanRPM(); // Measure the input fan RPM
  readCombustionLevel(); // Read the combustion level
  readTemperature(); // Read the temperature
  controlOutputFanSpeed(); // Control the output fan speed
  checkAirflowRate(); // Check the airflow rate
}

// Function to draw a box
void drawBox(int x1, int y1, int x2, int y2) {
    vt100.setBold(true);
    for (int x = x1; x <= x2; x++) {
        vt100.setCursorPosition(x, y1); vt100.print("-");
        vt100.setCursorPosition(x, y2); vt100.print("-");
    }
    for (int y = y1; y <= y2; y++) {
        vt100.setCursorPosition(x1, y); vt100.print("|");
        vt100.setCursorPosition(x2, y); vt100.print("|");
    }
    vt100.setBold(false);
}

// Function to visualize the state of the furnace
void visualizationTask() {
  vt100.clearScreen();
  
  // Header
  vt100.setCursorPosition(5, 1);
  vt100.setBold(true);
  vt100.setForeground(VT100::BLUE);
  vt100.print("   OIL FURNACE CONTROL SYSTEM v2.0   ");
  vt100.setBold(false);

  drawBox(1, 2, 38, 8); // Data Box
  drawBox(1, 9, 38, 15); // Status Box

  // Data Section
  vt100.setForeground(VT100::WHITE);
  vt100.setCursorPosition(3, 4);
  vt100.print("FAN RPM:  ");
  vt100.setForeground(inputFanRPM < RPM_THRESHOLD ? VT100::RED : VT100::GREEN);
  vt100.print(inputFanRPM);

  vt100.setForeground(VT100::WHITE);
  vt100.setCursorPosition(3, 6);
  vt100.print("COMB LVL: ");
  vt100.print(combustionLevel);

  vt100.setCursorPosition(20, 4);
  vt100.print("PWM OUT: ");
  vt100.print(outputFanSpeed);

  vt100.setCursorPosition(20, 6);
  vt100.print("TEMP C:  ");
  vt100.setForeground(temperature > TEMPERATURE_THRESHOLD ? VT100::YELLOW : VT100::GREEN);
  vt100.print(temperature);

  // Status Section
  vt100.setForeground(VT100::BLUE);
  vt100.setCursorPosition(15, 10);
  vt100.print("[ SYSTEM STATUS ]");
  
  vt100.setForeground(VT100::WHITE);
  vt100.setCursorPosition(4, 12);
  vt100.print("ALARM BEEP: ");
  if (beepState) { vt100.setForeground(VT100::RED); vt100.setBold(true); vt100.print("!! ACTIVE !!"); }
  else { vt100.setForeground(VT100::GREEN); vt100.print("  STABLE  "); }
  vt100.setBold(false);

  vt100.setForeground(VT100::WHITE);
  vt100.setCursorPosition(4, 14);
  vt100.print("OVERLOAD:   ");
  if (overloadState) {
      vt100.setForeground(VT100::RED);
      vt100.setBackground(1); // Mapped to RED background in renderer if possible or just use FG
      vt100.print("  OVERLOAD  ");
      vt100.setBackground(0);
  }
  else { vt100.setForeground(VT100::GREEN); vt100.print("  OPTIMAL   "); }

  vt100.setForeground(VT100::WHITE);
  vt100.setCursorPosition(2, 17);
  vt100.print("Runtime: ");
  vt100.print(millis() / 1000);
  vt100.print("s");
}

// Setup function
void setup() {
  // Initialize the pins
  pinMode(INPUT_FAN_PIN, INPUT_PULLUP); // Set the input fan pin as input with pullup
  pinMode(OUTPUT_FAN_PIN, OUTPUT); // Setthe output fan pin as output
  pinMode(RESTART_PIN, OUTPUT); // Set the restart pin as output
  pinMode(BEEP_PIN, OUTPUT); // Set the beep pin as output
  pinMode(TEMPERATURE_PIN, INPUT); // Set the temperature pin as input
  pinMode(OVERLOAD_PIN, OUTPUT); // Set the overload pin as output
  
  // Attach the interrupt for input fan hall sensor
  attachInterrupt(digitalPinToInterrupt(INPUT_FAN_PIN), inputFanISR, FALLING);
  
  // Start the safety ticker
  safetyTicker.attach_ms(SAFETY_INTERVAL, safetyTask);
  
  // Start the visualization ticker
  visualizationTicker.attach_ms(VISUALIZATION_INTERVAL, visualizationTask);
  
  // Enable the watchdog timer
  wdt_enable(WDTO_2S);
  
  // Initialize the serial communication
  Serial.begin(9600);
  
  // Initialize the VT100 terminal
  vt100.begin(&Serial);
}

// Loop function
void loop() {
  // Do nothing
  // Reset the watchdog timer
  wdt_reset();
}
