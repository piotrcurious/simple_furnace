// Define the pins for the sensors and actuators
#define SCRUBBER_ACTIVE_PIN 2 // digital input for scrubber active signal
#define INPUT_TEMP_PIN A0 // analog input for input exhaust temperature
#define OUTPUT_TEMP_PIN A1 // analog input for output exhaust temperature
#define FLUID_TEMP_PIN A2 // analog input for scrubber fluid output temperature
#define FAN_PIN 3 // digital output for fan PWM control
#define PUMP_PIN 5 // digital output for pump PWM control

// Define the constants for the scrubber system
#define MAX_FLUID_TEMP 80 // maximum scrubber fluid temperature in degrees Celsius
#define MAX_POWER_POINT 0.8 // maximum power point for extraction of heat from the exhaust gases
#define LEARNING_RATE 0.01 // learning rate for the heuristic gradient descent algorithm

// Define the variables for the scrubber system
float inputTemp; // input exhaust temperature in degrees Celsius
float outputTemp; // output exhaust temperature in degrees Celsius
float fluidTemp; // scrubber fluid output temperature in degrees Celsius
float fanDuty; // fan duty cycle in percentage
float pumpDuty; // pump duty cycle in percentage
float power; // power extracted from the exhaust gases in watts
float powerPrev; // previous power extracted from the exhaust gases in watts
float powerDiff; // difference between the current and previous power
float fanStep; // fan duty cycle step size for the gradient descent algorithm
float pumpStep; // pump duty cycle step size for the gradient descent algorithm

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the pin modes
  pinMode(SCRUBBER_ACTIVE_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Initialize the fan and pump duty cycles to 50%
  fanDuty = 50;
  pumpDuty = 50;

  // Initialize the fan and pump step sizes to 1%
  fanStep = 1;
  pumpStep = 1;

  // Initialize the power extracted from the exhaust gases to zero
  power = 0;
  powerPrev = 0;
  powerDiff = 0;
}

void loop() {
  // Read the scrubber active signal
  bool scrubberActive = digitalRead(SCRUBBER_ACTIVE_PIN);

  // If the scrubber is active, run the controller
  if (scrubberActive) {
    // Read the input exhaust temperature
    inputTemp = analogRead(INPUT_TEMP_PIN) * 0.48828125; // convert from 0-1023 to 0-500 degrees Celsius

    // Read the output exhaust temperature
    outputTemp = analogRead(OUTPUT_TEMP_PIN) * 0.48828125; // convert from 0-1023 to 0-500 degrees Celsius

    // Read the scrubber fluid output temperature
    fluidTemp = analogRead(FLUID_TEMP_PIN) * 0.48828125; // convert from 0-1023 to 0-500 degrees Celsius

    // Calculate the power extracted from the exhaust gases
    power = (inputTemp - outputTemp) * 1.2; // assume a mass flow rate of 1.2 kg/s and a specific heat capacity of 1 kJ/kgK

    // Calculate the difference between the current and previous power
    powerDiff = power - powerPrev;

    // Update the previous power
    powerPrev = power;

    // Print the sensor readings and the power to the serial monitor
    Serial.print("Input Temp: ");
    Serial.print(inputTemp);
    Serial.print(" C, Output Temp: ");
    Serial.print(outputTemp);
    Serial.print(" C, Fluid Temp: ");
    Serial.print(fluidTemp);
    Serial.print(" C, Power: ");
    Serial.print(power);
    Serial.println(" W");

    // Check if the scrubber fluid output temperature is below the maximum limit
    if (fluidTemp < MAX_FLUID_TEMP) {
      // Run the heuristic gradient descent algorithm to find the optimal fan and pump duty cycles
      // The algorithm tries to maximize the power extracted from the exhaust gases
      // The algorithm adjusts the fan and pump duty cycles based on the sign of the power difference
      // The algorithm uses a learning rate to control the step size of the duty cycle changes
      // The algorithm stops when the power difference is zero or the duty cycles reach the boundaries of 0% or 100%

      // Adjust the fan duty cycle
      if (powerDiff > 0) {
        // Increase the fan duty cycle by the fan step size multiplied by the learning rate
        fanDuty += fanStep * LEARNING_RATE;
      } else if (powerDiff < 0) {
        // Decrease the fan duty cycle by the fan step size multiplied by the learning rate
        fanDuty -= fanStep * LEARNING_RATE;
      }

      // Constrain the fan duty cycle between 0% and 100%
      fanDuty = constrain(fanDuty, 0, 100);

      // Adjust the pump duty cycle
      if (powerDiff > 0) {
        // Increase the pump duty cycle by the pump step size multiplied by the learning rate
        pumpDuty += pumpStep * LEARNING_RATE;
      } else if (powerDiff < 0) {
        // Decrease the pump duty cycle by the pump step size multiplied by the learning rate
        pumpDuty -= pumpStep * LEARNING_RATE;
      }

      // Constrain the pump duty cycle between 0% and 100%
      pumpDuty = constrain(pumpDuty, 0, 100);

      // Print the fan and pump duty cycles to the serial monitor
      Serial.print("Fan Duty: ");
      Serial.print(fanDuty);
      Serial.print(" %, Pump Duty: ");
      Serial.print(pumpDuty);
      Serial.println(" %");
    } else {
      // If the scrubber fluid output temperature is above the maximum limit, stop the fan and pump
      fanDuty = 0;
      pumpDuty = 0;

      // Print a warning message to the serial monitor
      Serial.println("Warning: Scrubber fluid temperature exceeded the maximum limit. Fan and pump stopped.");
    }

    // Write the fan and pump duty cycles to the PWM outputs
    // Constrain the values to be within 0 to 255
    analogWrite(FAN_PIN, constrain(map(fanDuty, 0, 100, 0, 255), 0, 255)); // map from 0-100% to 0-255 and constrain
    analogWrite(PUMP_PIN, constrain(map(pumpDuty, 0, 100, 0, 255), 0, 255)); // map from 0-100% to 0-255 and constrain
  } else {
    // If the scrubber is not active, stop the fan and pump
    fanDuty = 0;
    pumpDuty = 0;

    // Write the fan and pump duty cycles to the PWM outputs
    // Constrain the values to be within 0 to 255
    analogWrite(FAN_PIN, constrain(map(fanDuty, 0, 100, 0, 255), 0, 255)); // map from 0-100% to 0-255 and constrain
    analogWrite(PUMP_PIN, constrain(map(pumpDuty, 0, 100, 0, 255), 0, 255)); // map from 0-100% to 0-255 and constrain
  }

  // Wait for 1 second before the next iteration
  delay(1000);
}
