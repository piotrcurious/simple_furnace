// Define the pins for the sensors and actuators
#define EXHAUST_IN_PIN A0 // Input exhaust temperature sensor
#define EXHAUST_OUT_PIN A1 // Output exhaust temperature sensor
#define FLUID_IN_PIN A2 // Scrubber fluid input temperature sensor
#define FLUID_OUT_PIN A3 // Scrubber fluid output temperature sensor
#define FAN_PIN 9 // PWM controlled fan
#define PUMP_PIN 10 // PWM controlled pump

// Define the constants for the scrubber system
#define MAX_FLUID_TEMP 80 // Maximum scrubber fluid temperature in Celsius
#define NUM_PARAMS 16 // Number of candidate function parameters
#define HISTORY_SIZE 10 // Size of the history buffer

// Define the global variables for the scrubber system
float exhaust_in_temp; // Input exhaust temperature in Celsius
float exhaust_out_temp; // Output exhaust temperature in Celsius
float fluid_in_temp; // Scrubber fluid input temperature in Celsius
float fluid_out_temp; // Scrubber fluid output temperature in Celsius
int fan_duty; // Fan duty cycle in percentage
int pump_duty; // Pump duty cycle in percentage
float params[NUM_PARAMS]; // Array of candidate function parameters
float history[HISTORY_SIZE]; // Array of history values
int index; // Index of the current parameter
int count; // Count of the history values

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the pins
  pinMode(EXHAUST_IN_PIN, INPUT);
  pinMode(EXHAUST_OUT_PIN, INPUT);
  pinMode(FLUID_IN_PIN, INPUT);
  pinMode(FLUID_OUT_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Initialize the parameters with random values between 0 and 1
  randomSeed(analogRead(0));
  for (int i = 0; i < NUM_PARAMS; i++) {
    params[i] = random(0, 100) / 100.0;
  }

  // Initialize the index and count
  index = 0;
  count = 0;
}

void loop() {
  // Read the sensor values
  exhaust_in_temp = analogRead(EXHAUST_IN_PIN) * 0.48828125;
  exhaust_out_temp = analogRead(EXHAUST_OUT_PIN) * 0.48828125;
  fluid_in_temp = analogRead(FLUID_IN_PIN) * 0.48828125;
  fluid_out_temp = analogRead(FLUID_OUT_PIN) * 0.48828125;

  // Print the sensor values
  Serial.print("Exhaust in: ");
  Serial.print(exhaust_in_temp);
  Serial.print(" C, Exhaust out: ");
  Serial.print(exhaust_out_temp);
  Serial.print(" C, Fluid in: ");
  Serial.print(fluid_in_temp);
  Serial.print(" C, Fluid out: ");
  Serial.print(fluid_out_temp);
  Serial.println(" C");

  // Calculate the fan and pump duty cycles using the current parameter
  fan_duty = constrain(params[index] * 100, 0, 100);
  pump_duty = constrain((1 - params[index]) * 100, 0, 100);

  // Set the fan and pump duty cycles
  analogWrite(FAN_PIN, fan_duty * 255 / 100);
  analogWrite(PUMP_PIN, pump_duty * 255 / 100);

  // Print the fan and pump duty cycles
  Serial.print("Fan duty: ");
  Serial.print(fan_duty);
  Serial.print(" %, Pump duty: ");
  Serial.print(pump_duty);
  Serial.println(" %");

  // Calculate the objective function value
  float objective = exhaust_in_temp - exhaust_out_temp; // Maximize the heat extraction
  if (fluid_out_temp > MAX_FLUID_TEMP) { // Penalize the excess fluid temperature
    objective -= (fluid_out_temp - MAX_FLUID_TEMP) * 10;
  }

  // Print the objective function value
  Serial.print("Objective: ");
  Serial.println(objective);

  // Update the history buffer
  history[count] = objective;
  count++;

  // Check if the history buffer is full
  if (count == HISTORY_SIZE) {
    // Calculate the average of the history values
    float average = 0;
    for (int i = 0; i < HISTORY_SIZE; i++) {
      average += history[i];
    }
    average /= HISTORY_SIZE;

    // Print the average of the history values
    Serial.print("Average: ");
    Serial.println(average);

    // Reset the count
    count = 0;

    // Find the best parameter in the array
    int best_index = 0;
    float best_average = 0;
    for (int i = 0; i < NUM_PARAMS; i++) {
      // Calculate the average of the history values for each parameter
      float param_average = 0;
      for (int j = 0; j < HISTORY_SIZE; j++) {
        param_average += history[(i * HISTORY_SIZE + j) % (NUM_PARAMS * HISTORY_SIZE)];
      }
      param_average /= HISTORY_SIZE;

      // Print the average of the history values for each parameter
      Serial.print("Parameter ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(param_average);

      // Compare the average with the best average
      if (param_average > best_average) {
        // Update the best index and best average
        best_index = i;
        best_average = param_average;
      }
    }

    // Print the best index and best average
    Serial.print("Best parameter: ");
    Serial.print(best_index);
    Serial.print(", Best average: ");
    Serial.println(best_average);

    // Check if the current parameter is the best parameter
    if (index == best_index) {
      // Perturb the current parameter with a small random value
      params[index] += random(-10, 10) / 1000.0;
      params[index] = constrain(params[index], 0, 1);
    } else {
      // Move to the next parameter in the array
      index = (index + 1) % NUM_PARAMS;
    }

    // Print the current parameter
    Serial.print("Current parameter: ");
    Serial.println(params[index]);
  }

  // Wait for one second
  delay(1000);
}
