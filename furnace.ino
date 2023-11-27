// Include the libraries
#include <Ticker.h>
#include <avr/wdt.h>
#include <SmoothThermistor.h> // Include the SmoothThermistor library

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

// Declare the global variables
volatile int inputFanCount = 0; // Input fan pulse count
float inputFanRPM = 0; // Input fan RPM
int outputFanSpeed = 0; // Output fan speed
int combustionLevel = 0; // Combustion level
bool beepState = false; // Beep state
int temperature = 0; // Temperature in Celsius
bool overloadState = false; // Overload state
Ticker safetyTicker; // Ticker object for safety tasks
SmoothThermistor smoothThermistor(TEMPERATURE_PIN, ADC_SIZE_10_BIT, 10000, 10000, 3950, 25, 10); // Create a SmoothThermistor object with the given parameters

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

// Setup function
void setup() {
  // Initialize the pins
  pinMode(INPUT_FAN_PIN, INPUT_PULLUP); // Set the input fan pin as input with pullup
  pinMode(OUTPUT_FAN_PIN, OUTPUT); // Set the output fan pin as output
  pinMode(RESTART_PIN, OUTPUT); // Set the restart pin as output
  pinMode(BEEP_PIN, OUTPUT); // Set the beep pin as output
  pinMode(TEMPERATURE_PIN, INPUT); // Set the temperature pin as input
  pinMode(OVERLOAD_PIN, OUTPUT); // Set the overload pin as output
  
  // Attach the interrupt for input fan hall sensor
  attachInterrupt(digitalPinToInterrupt(INPUT_FAN_PIN), inputFanISR, FALLING);
  
  // Start the safety ticker
  safetyTicker.attach_ms(SAFETY_INTERVAL, safetyTask);
  
  // Enable the watchdog timer
  wdt_enable(WDTO_2S);
}

// Loop function
void loop() {
  // Do nothing
  // Reset the watchdog timer
  wdt_reset();
}
