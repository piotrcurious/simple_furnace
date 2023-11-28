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
#define COOLING_PIN 9 // Cooling PWM pin
#define THRESHOLD_PIN A2 // Threshold temperature analog input pin

// Define the constants
#define RPM_THRESHOLD 100 // Minimum RPM of input fan
#define RPM_BOOST 200 // RPM of input fan to boost output fan speed
#define RESTART_TIME 200 // Restart time for input fan in ms
#define SAFETY_INTERVAL 500 // Safety task interval in ms
#define TEMPERATURE_THRESHOLD_MIN 40 // Minimum temperature threshold to boost output fan speed in Celsius
#define TEMPERATURE_THRESHOLD_MAX 85 // Maximum temperature threshold to boost output fan speed in Celsius
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
#define COOLING_X 5 // Cooling x position
#define COOLING_Y 20 // Cooling y position
#define COOLING_W 10 // Cooling width
#define COOLING_H 3 // Cooling height
#define THRESHOLD_X 5 // Threshold temperature x position
#define THRESHOLD_Y 18 // Threshold temperature y position
#define THRESHOLD_W 10 // Threshold temperature width
#define THRESHOLD_H 3 // Threshold temperature height

// Declare the global variables
volatile int inputFanCount = 0; // Input fan pulse count
float inputFanRPM = 0; // Input fan RPM
int outputFanSpeed = 0; // Output fan speed
int combustionLevel = 0; // Combustion level
bool beepState = false; // Beep state
int temperature = 0; // Temperature in Celsius
bool overloadState = false; // Overload state
int coolingSpeed = 0; // Cooling speed
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

// Function to read the threshold temperature
void readThresholdTemperature() {
  thresholdTemperature = analogRead(THRESHOLD_PIN); // Read the threshold temperature
  thresholdTemperature = map(thresholdTemperature, 0, 1023, TEMPERATURE_THRESHOLD_MIN, TEMPERATURE_THRESHOLD_MAX); // Map the threshold temperature to Celsius
}

// Function to control the output fan speed
void controlOutputFanSpeed() {
  outputFanSpeed = map(combustionLevel, 0, 1023, 0, 255); // Map the combustion level to output fan speed
  if (inputFanRPM < RPM_BOOST || temperature < thresholdTemperature) { // If the input fan RPM is below the boost threshold or the temperature is below the threshold
    int boostRate = BOOST_VALUE; // Set the boost rate to the default value
    if (temperature < thresholdTemperature) { // If the temperature is below the threshold
      boostRate = thresholdTemperature - temperature; // Set the boost rate to the difference of current temperature and threshold
    }
    if (outputFanSpeed + boostRate > 255) { // If the output fan speed plus the boost rate exceeds the maximum PWM value
      overloadState = true; // Set the overload state to true
      digitalWrite(OVERLOAD_PIN, overloadState); // Write the overload state
    }
    else { // If the output fan speed plus the boost rate does not exceed the maximum PWM value
      outputFanSpeed = outputFanSpeed + boostRate; // Increase the output fan speed by the boost rate
      overloadState = false; // Set the overload state to false
      digitalWrite(OVERLOAD_PIN, overloadState); // Write the overload state
    }
  }
  outputFanSpeed = constrain(outputFanSpeed, 0, 255); // Constrain the output fan speed to valid range
  analogWrite(OUTPUT_FAN_PIN, outputFanSpeed); // Write the output fan speed
}

// Function to control the cooling speed
void controlCoolingSpeed() {
  if (temperature > thresholdTemperature) { // If the temperature is above the threshold
    coolingSpeed = temperature - thresholdTemperature; // Set the cooling speed to the difference of current temperature and threshold
    coolingSpeed = map(coolingSpeed, 0, 50, 0, 255); // Map the cooling speed to PWM value
  }
  else { // If the temperature is below or equal to the threshold
    coolingSpeed = 0; // Set the cooling speed to zero
  }
  coolingSpeed = constrain(coolingSpeed, 0, 255); // Constrain the cooling speed to valid range
  analogWrite(COOLING_PIN, coolingSpeed); // Write the cooling speed
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
  readThresholdTemperature(); // Read the threshold temperature
  controlOutputFanSpeed(); // Control the output fan speed
  controlCoolingSpeed(); // Control the cooling speed
  checkAirflowRate(); // Check the airflow rate
}

// Function to visualize the state of the furnace
void visualizationTask() {
  // Clear the screen
  vt100.clearScreen();
  
  // Draw the input fan RPM
  vt100.setCursorPosition(INPUT_FAN_RPM_X, INPUT_FAN_RPM_Y); // Set the cursor position
  vt100.print("Input fan RPM: "); // Print the label
  vt100.print(inputFanRPM); // Print the value
  
  // Draw the output fan speed
  vt100.setCursorPosition(OUTPUT_FAN_SPEED_X, OUTPUT_FAN_SPEED_Y); // Set the cursor position
  vt100.print("Output fan speed: "); // Print the label
  vt100.print(outputFanSpeed); // Print the value
  
  // Draw the combustion level
  vt100.setCursorPosition(COMBUSTION_LEVEL_X, COMBUSTION_LEVEL_Y); // Set the cursor position
  vt100.print("Combustion level: "); // Print the label
  vt100.print(combustionLevel); // Print the value
  
  // Draw the temperature
  vt100.setCursorPosition(TEMPERATURE_X, TEMPERATURE_Y); // Set the cursor position
  vt100.print("Temperature: "); // Print the label
  vt100.print(temperature); // Print the value
  
  // Draw the ratio of input RPM to output PWM
  vt100.setCursorPosition(RATIO_X, RATIO_Y); // Set the cursor position
  vt100.print("Ratio: "); // Print the label
  if (outputFanSpeed == 0) { // If the output fan speed is zero
    vt100.print("N/A"); // Print N/A
  }
  else { // If the output fan speed is not zero
    vt100.print(inputFanRPM / outputFanSpeed); // Print the ratio
  }
  
  // Draw the beep state
  vt100.setCursorPosition(BEEP_X, BEEP_Y); // Set the cursor position
  vt100.print("Beep: "); // Print the label
  vt100.print(beepState ? "ON" : "OFF"); // Print the state
  
  // Draw the overload state
  vt100.setCursorPosition(OVERLOAD_X, OVERLOAD_Y); // Set the cursor position
  vt100.print("Overload: "); // Print the label
  vt100.print(overloadState ? "ON" : "OFF"); // Print the state
  
  // Draw the cooling speed
  vt100.setCursorPosition(COOLING_X, COOLING_Y); // Set the cursor position
  vt100.print("Cooling speed: "); // Print the label
  vt100.print(coolingSpeed); // Print the value
  
  // Draw the threshold temperature
  vt100.setCursorPosition(THRESHOLD_X, THRESHOLD_Y); // Set the cursor position
  vt100.print("Threshold temperature: "); // Print the label
  vt100.print(thresholdTemperature); // Print the value
}

// Setup function
void setup() {
  // Initialize the pins
  pinMode(INPUT_FAN_PIN, INPUT_PULLUP); // Set the input fan pin as input with pullup
  pinMode(OUTPUT_FAN_PIN, OUTPUT); // Set the output fan pin as output
  pinMode(RESTART_PIN, OUTPUT); // Set the restart pin as output
  pinMode(BEEP_PIN, OUTPUT); // Set the beep pin as output
  pinMode(TEMPERATURE_PIN, INPUT); // Set the temperature pin as input
  pinMode(OVERLOAD_PIN, OUTPUT); // Set the overload pin as output
  pinMode(COOLING_PIN, OUTPUT); // Set the cooling pin as output
  pinMode(THRESHOLD_PIN, INPUT); // Set the threshold pin as input
  
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
