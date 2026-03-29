float power;
#include "Arduino.h"
#include "Ticker.h"
#include "SmoothThermistor.h"
#include "VT100.h"
#include "avr/wdt.h"
#include "EEPROM.h"
// Define the pins for the sensors, fan and pump
#define TEMP_IN_PIN A0 // Input exhaust temperature sensor
#define TEMP_OUT_PIN A1 // Output exhaust temperature sensor
#define FLUID_IN_PIN A2 // Scrubber fluid input temperature sensor
#define FLUID_OUT_PIN A3 // Scrubber fluid output temperature sensor
#define FAN_PIN 9 // PWM controlled fan
#define PUMP_PIN 10 // PWM controlled pump

// Define the constants for the scrubber system
#define MAX_FLUID_TEMP 80 // Maximum scrubber fluid temperature in Celsius
#define NUM_PARAMS 16 // Number of candidate function parameters
#define STEP_SIZE 0.1 // Step size for the greedy algorithm
#define SORT_INTERVAL 100 // Interval for sorting the array in milliseconds

// Define the variables for the scrubber system
float temp_in; // Input exhaust temperature in Celsius
float temp_out; // Output exhaust temperature in Celsius
float fluid_in; // Scrubber fluid input temperature in Celsius
float fluid_out; // Scrubber fluid output temperature in Celsius
float fan_duty; // Fan duty cycle in percentage
float pump_duty; // Pump duty cycle in percentage
float params[NUM_PARAMS]; // Array of candidate function parameters
float scores[NUM_PARAMS]; // Array of scores for the candidate function parameters
float best_param; // Best function parameter
float best_score; // Best score
float score; // Current score
float delta; // Change in score
unsigned long last_sort; // Last time the array was sorted

// Initialize the scrubber system
void setup() {
  // Set the pin modes
  pinMode(TEMP_IN_PIN, INPUT);
  pinMode(TEMP_OUT_PIN, INPUT);
  pinMode(FLUID_IN_PIN, INPUT);
  pinMode(FLUID_OUT_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Initialize the serial monitor
  Serial.begin(9600);

  // Initialize the candidate function parameters
  for (int i = 0; i < NUM_PARAMS; i++) {
    params[i] = random(0, 100) / 100.0; // Random value between 0 and 1
  }

  // Initialize the best function parameter and score
  best_param = params[0];
  best_score = -1;

  // Initialize the last sort time
  last_sort = millis();
}

// Forward declarations
float calculate_score(float param);
void bubble_sort();
void swap_elements(int i, int j);

// Run the scrubber system
void loop() {
  // Read the sensor values
  temp_in = analogRead(TEMP_IN_PIN) * 0.48828125; // Convert to Celsius
  temp_out = analogRead(TEMP_OUT_PIN) * 0.48828125; // Convert to Celsius
  fluid_in = analogRead(FLUID_IN_PIN) * 0.48828125; // Convert to Celsius
  fluid_out = analogRead(FLUID_OUT_PIN) * 0.48828125; // Convert to Celsius

  // Print the sensor values
  Serial.print("Input exhaust temperature: ");
  Serial.print(temp_in);
  Serial.println(" C");
  Serial.print("Output exhaust temperature: ");
  Serial.print(temp_out);
  Serial.println(" C");
  Serial.print("Scrubber fluid input temperature: ");
  Serial.print(fluid_in);
  Serial.println(" C");
  Serial.print("Scrubber fluid output temperature: ");
  Serial.print(fluid_out);
  Serial.println(" C");

  // Calculate the fan and pump duty cycles using the best function parameter
  fan_duty = constrain(best_param * (temp_in - temp_out), 0, 100); // Linear function with slope best_param
  pump_duty = constrain(best_param * (fluid_out - fluid_in), 0, 100); // Linear function with slope best_param

  // Write the fan and pump duty cycles
  analogWrite(FAN_PIN, fan_duty * 2.55); // Convert to 8-bit value
  analogWrite(PUMP_PIN, pump_duty * 2.55); // Convert to 8-bit value

  // Print the fan and pump duty cycles
  Serial.print("Fan duty cycle: ");
  Serial.print(fan_duty);
  Serial.println(" %");
  Serial.print("Pump duty cycle: ");
  Serial.print(pump_duty);
  Serial.println(" %");

  // Iterate over the array of candidate function parameters
  for (int i = 0; i < NUM_PARAMS; i++) {
    // Calculate the score for the current function parameter
    score = calculate_score(params[i]);

    // Store the score in the array
    scores[i] = score;

    // Print the current function parameter and score
    Serial.print("Function parameter: ");
    Serial.print(params[i]);
    Serial.print(" Score: ");
    Serial.println(score);

    // Compare the score with the best score
    if (score > best_score) {
      // Update the best function parameter and score
      best_param = params[i];
      best_score = score;

      // Print the best function parameter and score
      Serial.print("Best function parameter: ");
      Serial.print(best_param);
      Serial.print(" Best score: ");
      Serial.println(best_score);
    }
  }

  // Apply the greedy algorithm to permute the candidate function parameters
  for (int i = 0; i < NUM_PARAMS; i++) {
    // Calculate the score for the current function parameter
    float original_param = params[i];
    score = scores[i];

    // Add a small random perturbation to the current function parameter
    float mutation = random(-10, 11) / 1000.0;
    params[i] = constrain(original_param + mutation, 0, 1);

    // Calculate the new score for the perturbed function parameter
    delta = calculate_score(params[i]) - score;

    // Print the perturbed function parameter and the change in score
    Serial.print("Perturbed function parameter: ");
    Serial.print(params[i]);
    Serial.print(" Delta: ");
    Serial.println(delta);

    // Accept the perturbation with a probability proportional to the change in score
    if (delta < 0 && random(0, 100) / 100.0 > exp(delta / STEP_SIZE)) {
      // Reject the perturbation and restore the original function parameter accurately
      params[i] = original_param;
      Serial.println("Mutation rejected.");
    } else {
      Serial.println("Mutation accepted.");
    }
  }

  // Check if it is time to sort the array
  if (millis() - last_sort > SORT_INTERVAL) {
    // Sort the array using bubble sort
    bubble_sort();

    // Update the last sort time
    last_sort = millis();
  }

  // Wait for a second
  delay(1000);
}

// Define a function to calculate the score for a given function parameter
float calculate_score(float param) {
  // Balanced score: Maximize heat extraction, Penalize high temperature and energy waste
  float heat_extracted = (temp_in - temp_out);
  float score = 0;
  score += 25.0 * heat_extracted; // Weight heat extraction higher

  if (fluid_out > MAX_FLUID_TEMP) {
    score -= 600.0 * (fluid_out - MAX_FLUID_TEMP); // Severe penalty for safety limit
  } else if (fluid_out > MAX_FLUID_TEMP * 0.85) {
    score -= 60.0 * (fluid_out - MAX_FLUID_TEMP * 0.85); // Progressive safety penalty
  }

  // Penalize high duty cycles (energy cost)
  score -= 0.15 * (fan_duty + pump_duty);

  return score;
}

// Define a function to sort the array using bubble sort
void bubble_sort() {
  // Loop through the array
  for (int i = 0; i < NUM_PARAMS - 1; i++) {
    // Loop through the array again
    for (int j = 0; j < NUM_PARAMS - i - 1; j++) {
      // Compare the scores of the adjacent elements
      if (scores[j] < scores[j + 1]) {
        // Swap the elements if the score is lower
        swap_elements(j, j + 1);
      }
    }
  }
}

// Define a function to swap two elements in the array
void swap_elements(int i, int j) {
  // Store the values in temporary variables
  float temp_param = params[i];
  float temp_score = scores[i];

  // Swap the values
  params[i] = params[j];
  scores[i] = scores[j];
  params[j] = temp_param;
  scores[j] = temp_score;
}
