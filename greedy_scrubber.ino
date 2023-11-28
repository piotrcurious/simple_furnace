// Scrubber controller code
// Input: exhaust temperature, output: exhaust temperature, scrubber fluid input temperature, scrubber fluid output temperature
// Output: PWM controlled fan, PWM controlled scrubber fluid pump
// Objective: maintain maximum power point for extraction of heat from the exhaust gases and not exceed the maximum scrubber fluid temperature
// Method: iterate to detect Lyapunov stability regions by using permuted greedy algorithms as candidate functions and store best candidate parameters into EEPROM

// Define pins and constants
#define FAN_PIN 9 // PWM pin for fan
#define PUMP_PIN 10 // PWM pin for pump
#define IN_TEMP_PIN A0 // Analog pin for input exhaust temperature
#define OUT_TEMP_PIN A1 // Analog pin for output exhaust temperature
#define FLUID_IN_TEMP_PIN A2 // Analog pin for scrubber fluid input temperature
#define FLUID_OUT_TEMP_PIN A3 // Analog pin for scrubber fluid output temperature
#define MAX_FLUID_TEMP 80 // Maximum scrubber fluid temperature in Celsius
#define EEPROM_SIZE 512 // EEPROM size in bytes
#define CANDIDATE_SIZE 4 // Candidate function size in bytes
#define NUM_CANDIDATES 10 // Number of candidate functions to generate
#define MAX_ITERATIONS 100 // Maximum number of iterations to run
#define TOLERANCE 0.01 // Tolerance for convergence

// Define global variables
float in_temp; // Input exhaust temperature in Celsius
float out_temp; // Output exhaust temperature in Celsius
float fluid_in_temp; // Scrubber fluid input temperature in Celsius
float fluid_out_temp; // Scrubber fluid output temperature in Celsius
float fan_duty; // Fan duty cycle in percentage
float pump_duty; // Pump duty cycle in percentage
float power; // Power extracted from the exhaust gases in Watts
float best_power; // Best power achieved so far in Watts
byte best_candidate[CANDIDATE_SIZE]; // Best candidate function parameters so far
byte current_candidate[CANDIDATE_SIZE]; // Current candidate function parameters
int iteration; // Current iteration number
bool converged; // Flag for convergence

// Define helper functions
float read_temp(int pin) {
  // Read the temperature from the analog pin and convert to Celsius
  int raw = analogRead(pin); // Raw analog value from 0 to 1023
  float voltage = raw * 5.0 / 1024.0; // Voltage in Volts
  float temp = (voltage - 0.5) * 100.0; // Temperature in Celsius
  return temp;
}

void write_duty(int pin, float duty) {
  // Write the duty cycle to the PWM pin
  int value = duty * 255.0 / 100.0; // Value from 0 to 255
  analogWrite(pin, value); // Write to PWM pin
}

float calculate_power() {
  // Calculate the power extracted from the exhaust gases
  // Assume a constant mass flow rate of 1 kg/s and a specific heat capacity of 1 kJ/kgK
  float power = (in_temp - out_temp) * 1000.0; // Power in Watts
  return power;
}

void generate_candidate() {
  // Generate a random candidate function parameters
  for (int i = 0; i < CANDIDATE_SIZE; i++) {
    current_candidate[i] = random(256); // Random byte from 0 to 255
  }
}

void evaluate_candidate() {
  // Evaluate the current candidate function and update the fan and pump duty cycles
  // The candidate function is a linear combination of the input and output temperatures
  // fan_duty = a * in_temp + b * out_temp + c
  // pump_duty = d * in_temp + e * out_temp + f
  // The parameters a, b, c, d, e, f are encoded as bytes and scaled to [-1, 1]
  float a = (current_candidate[0] - 128) / 128.0; // Scale a to [-1, 1]
  float b = (current_candidate[1] - 128) / 128.0; // Scale b to [-1, 1]
  float c = (current_candidate[2] - 128) / 128.0; // Scale c to [-1, 1]
  float d = (current_candidate[3] - 128) / 128.0; // Scale d to [-1, 1]
  float e = (current_candidate[4] - 128) / 128.0; // Scale e to [-1, 1]
  float f = (current_candidate[5] - 128) / 128.0; // Scale f to [-1, 1]
  fan_duty = a * in_temp + b * out_temp + c; // Calculate fan duty
  pump_duty = d * in_temp + e * out_temp + f; // Calculate pump duty
  fan_duty = constrain(fan_duty, 0, 100); // Constrain fan duty to [0, 100]
  pump_duty = constrain(pump_duty, 0, 100); // Constrain pump duty to [0, 100]
}

void permute_candidate() {
  // Permute the current candidate function parameters by swapping two random bytes
  int i = random(CANDIDATE_SIZE); // Random index from 0 to CANDIDATE_SIZE - 1
  int j = random(CANDIDATE_SIZE); // Random index from 0 to CANDIDATE_SIZE - 1
  byte temp = current_candidate[i]; // Temporary variable
  current_candidate[i] = current_candidate[j]; // Swap i and j
  current_candidate[j] = temp; // Swap j and i
}

void update_best() {
  // Update the best candidate function parameters and power if the current power is higher
  if (power > best_power) {
    best_power = power; // Update best power
    for (int i = 0; i < CANDIDATE_SIZE; i++) {
      best_candidate[i] = current_candidate[i]; // Update best candidate
    }
  }
}

void write_eeprom() {
  // Write the best candidate function parameters to the EEPROM
  for (int i = 0; i < CANDIDATE_SIZE; i++) {
    EEPROM.write(i, best_candidate[i]); // Write to EEPROM
  }
}

void read_eeprom() {
  // Read the best candidate function parameters from the EEPROM
  for (int i = 0; i < CANDIDATE_SIZE; i++) {
    best_candidate[i] = EEPROM.read(i); // Read from EEPROM
  }
}

void check_convergence() {
  // Check if the iteration has converged to a Lyapunov stability region
  // A stability region is defined as a set of candidate functions that have the same power output within a tolerance
  // Convergence is achieved if the current power is within the tolerance of the best power
  if (abs(power - best_power) < TOLERANCE) {
    converged = true; // Set convergence flag to true
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600); // Set baud rate to 9600 bps
  // Initialize pins
  pinMode(FAN_PIN, OUTPUT); // Set fan pin as output
  pinMode(PUMP_PIN, OUTPUT); // Set pump pin as output
  pinMode(IN_TEMP_PIN, INPUT); // Set input exhaust temperature pin as input
  pinMode(OUT_TEMP_PIN, INPUT); // Set output exhaust temperature pin as input
  pinMode(FLUID_IN_TEMP_PIN, INPUT); // Set scrubber fluid input temperature pin as input
  pinMode(FLUID_OUT_TEMP_PIN, INPUT); // Set scrubber fluid output temperature pin as input
  // Initialize variables
  in_temp = 0; // Set input exhaust temperature to 0
  out_temp = 0; // Set output exhaust temperature to 0
  fluid_in_temp = 0; // Set scrubber fluid input temperature to 0
  fluid_out_temp = 0; // Set scrubber fluid output temperature to 0
  fan_duty = 0; // Set fan duty cycle to 0
  pump_duty = 0; // Set pump duty cycle to 0
  power = 0; // Set power to 0
  best_power = 0; // Set best power to 0
  iteration = 0; // Set iteration number to 0
  converged = false; // Set convergence flag to false
  // Read the best candidate function parameters from the EEPROM
  read_eeprom();
  // Evaluate the best candidate function and update the fan and pump duty cycles
  evaluate_candidate();
  // Write the fan and pump duty cycles to the PWM pins
  write_duty(FAN_PIN, fan_duty);
  write_duty(PUMP_PIN, pump_duty);
  // Print the initial values
  Serial.print("Iteration: ");
  Serial.println(iteration);
  Serial.print("Input exhaust temperature: ");
  Serial.println(in_temp);
  Serial.print("Output exhaust temperature: ");
  Serial.println(out_temp);
  Serial.print("Scrubber fluid input temperature: ");
  Serial.println(fluid_in_temp);
  Serial.print("Scrubber fluid output temperature: ");
  Serial.println(fluid_out_temp);
  Serial.print("Fan duty cycle: ");
  Serial.println(fan_duty);
  Serial.print("Pump duty cycle: ");
  Serial.println(pump_duty);
  Serial.print("Power: ");
  Serial.println(power);
  Serial.print("Best power: ");
  Serial.println(best_power);
  Serial.print("Best candidate function parameters: ");
  for (int i = 0; i < CANDIDATE_SIZE; i++) {
    Serial.print(best_candidate[i]);
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Converged: ");
  Serial.println(converged);
}

void loop() {
  // Run the main loop until convergence or maximum iterations
  if (!converged && iteration < MAX_ITERATIONS) {
    // Increment the iteration number
    iteration++;
    // Read the input and output temperatures
    in_temp = read_temp(IN_TEMP_PIN);
    out_temp = read_temp(OUT_TEMP_PIN);
    fluid_in_temp = read_temp(FLUID_IN_TEMP_PIN);
    fluid_out_temp = read_temp(FLUID_OUT_TEMP_PIN);
    // Check if the scrubber fluid output temperature is below the maximum
    if (fluid_out_temp < MAX_FLUID_TEMP) {
      // Generate a random candidate function parameters
      generate_candidate();
      // Evaluate the current candidate function and update the fan and pump duty cycles
      evaluate_candidate();
      // Write the fan and pump duty cycles to the PWM pins
      write_duty(FAN_PIN, fan_duty);
      write_duty(PUMP_PIN, pump_duty);
      // Calculate the power extracted from the exhaust gases
      power = calculate_power();
      // Update the best candidate function parameters and power if the current power is higher
      update_best();
      // Check if the iteration has converged to a Lyapunov stability region
      check_convergence();
    }
    else {
      // The scrubber fluid output temperature is too high, reduce the pump duty cycle
      pump_duty = pump_duty * 0.9; // Reduce by 10%
      pump_duty = constrain(pump_duty, 0, 100); // Constrain to [0, 100]
      write_duty(PUMP_PIN, pump_duty); // Write to PWM pin
    }
    // Print the current values
    Serial.print("Iteration: ");
    Serial.println(iteration);
    Serial.print("Input exhaust temperature: ");
    Serial.println(in_temp);
    Serial.print("Output exhaust temperature: ");
    Serial.println(out_temp);
    Serial.print("Scrubber fluid input temperature: ");
    Serial.println(fluid_in_temp);
    Serial.print("Scrubber fluid output temperature: ");
    Serial.println(fluid_out_temp);
    Serial.print("Fan duty cycle: ");
    Serial.println(fan_duty);
    Serial.print("Pump duty cycle: ");
    Serial.println(pump_duty);
    Serial.print("Power: ");
    Serial.println(power);
    Serial.print("Best power: ");
    Serial.println(best_power);
    Serial.print("Best candidate function parameters: ");
    for (int i = 0; i < CANDIDATE_SIZE; i++) {
      Serial.print(best_candidate[i]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Converged: ");
    Serial.println(converged);
  }
  else {
    // The iteration has converged or reached the maximum, stop the loop
    Serial.println("The loop has stopped.");
    // Write the best candidate function parameters to the EEPROM
    write_eeprom();
    // Stop the fan and pump
    write_duty(FAN_PIN, 0);
    write_duty(PUMP_PIN, 0);
    // Exit the loop
    noLoop();
  }
}
