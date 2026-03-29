#include "Arduino.h"
#include "Ticker.h"
#include "SmoothThermistor.h"
#include "VT100.h"
#include "avr/wdt.h"
#include "EEPROM.h"
#include <VT100.h>
#include "VT100Visualizer.h"
#include <Ticker.h>

// Define the pins for the sensors, fan and pump
#define TEMP_IN_PIN A0 // Input exhaust temperature sensor
#define TEMP_OUT_PIN A1 // Output exhaust temperature sensor
#define FLUID_IN_PIN A2 // Scrubber fluid input temperature sensor
#define FLUID_OUT_PIN A3 // Scrubber fluid output temperature sensor
#define FAN_PIN 9 // PWM controlled fan
#define PUMP_PIN 10 // PWM controlled pump

// Define the constants for the scrubber system
#define MAX_FLUID_TEMP 80 // Maximum scrubber fluid temperature in Celsius
#define POP_SIZE 16 // Population size
#define NUM_GENES 2 // fan_k, pump_k
#define STEP_SIZE 0.1 // Step size for the greedy algorithm

// Define the variables for the scrubber system
float mutation_rate = 0.02; // Adaptive mutation rate
float temp_in; // Input exhaust temperature in Celsius
float temp_out; // Output exhaust temperature in Celsius
float fluid_in; // Scrubber fluid input temperature in Celsius
float fluid_out; // Scrubber fluid output temperature in Celsius
float fan_duty; // Fan duty cycle in percentage
float pump_duty; // Pump duty cycle in percentage
float power;

struct Candidate {
    float genes[NUM_GENES];
    float score;
};

Candidate population[POP_SIZE];
int best_idx = 0;
float best_score;
float score; // Current score
float delta; // Change in score

VT100 vt100;
VT100Visualizer visualizer(vt100);

void updateVisualization() {
    vt100.clearScreen();
    visualizer.drawBorder(1, 1, 39, 23, VT100::BLUE);
    visualizer.drawHeader("GAMING SCRUBBER v4.0");

    // Scrubber Art
    visualizer.drawScrubberArt(5, 4, (int)(fan_duty * 30), (int)pump_duty);

    // Population Visualization
    vt100.setCursorPosition(21, 3);
    vt100.setForeground(VT100::WHITE);
    vt100.print("[ POPULATION ]");
    for(int i=0; i<POP_SIZE; i++) {
        vt100.setCursorPosition(21, 5 + i);
        if (i == best_idx) {
            vt100.setForeground(VT100::GREEN);
            vt100.setBold(true);
            vt100.print("*");
        } else {
            vt100.setForeground(VT100::WHITE);
            vt100.print(" ");
        }
        // Draw individual
        vt100.print("C"); vt100.print(i);
        vt100.print(": ");
        vt100.print(population[i].genes[0]);
        vt100.print("/");
        vt100.print(population[i].genes[1]);
        vt100.setBold(false);
    }

    vt100.setCursorPosition(3, 15);
    vt100.setForeground(VT100::WHITE);
    vt100.print("TEMP IN:  "); vt100.print(temp_in);
    vt100.setCursorPosition(3, 16);
    vt100.print("TEMP OUT: "); vt100.print(temp_out);

    vt100.setCursorPosition(3, 18);
    vt100.print("M-RATE: "); vt100.print(mutation_rate);
}

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
  vt100.begin(&Serial);

  // Initialize the candidate function parameters
  for (int i = 0; i < POP_SIZE; i++) {
    for (int j = 0; j < NUM_GENES; j++) {
        population[i].genes[j] = random(0, 101) / 100.0;
    }
    population[i].score = -1000;
  }

  // Initialize the best function parameter and score
  best_idx = 0;
  best_score = -1000;
}

float calculate_score(float fk, float pk);

// Run the scrubber system
void loop() {
  power = (temp_in - temp_out) * 1.2;
  Serial.print("Power: "); Serial.println(power);
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

  // Calculate the fan and pump duty cycles using the best genes
  fan_duty = constrain(population[best_idx].genes[0] * 100.0, 0, 100);
  pump_duty = constrain(population[best_idx].genes[1] * 100.0, 0, 100);

  // Write the fan and pump duty cycles
  analogWrite(FAN_PIN, fan_duty * 2.55);
  analogWrite(PUMP_PIN, pump_duty * 2.55);

  // Print the fan and pump duty cycles
  Serial.print("Fan duty cycle: ");
  Serial.print(fan_duty);
  Serial.println(" %");
  Serial.print("Pump duty cycle: ");
  Serial.print(pump_duty);
  Serial.println(" %");

  // Iterate over the array of candidate function parameters
  for (int i = 0; i < POP_SIZE; i++) {
    population[i].score = calculate_score(population[i].genes[0], population[i].genes[1]);

    if (population[i].score > best_score) {
      best_idx = i;
      best_score = population[i].score;
      Serial.print("New Global Best: ");
      Serial.println(best_score);
    }
  }

  // GA: Evolution step
  for (int i = 0; i < POP_SIZE; i++) {
    if (i == best_idx) continue; // Elitism

        power = (temp_in - temp_out) * 1.2; // Update for metrics

    float original_score = population[i].score;
    float old_genes[NUM_GENES];
    for(int j=0; j<NUM_GENES; j++) old_genes[j] = population[i].genes[j];

    // Mutation
    for (int j = 0; j < NUM_GENES; j++) {
        population[i].genes[j] = constrain(population[i].genes[j] + (random(-100, 101)/100.0)*mutation_rate, 0, 1);
    }

    float new_score = calculate_score(population[i].genes[0], population[i].genes[1]);
    delta = new_score - original_score;

    if (delta < 0 && random(0, 100) / 100.0 > exp(delta / STEP_SIZE)) {
        // Reject
        for(int j=0; j<NUM_GENES; j++) population[i].genes[j] = old_genes[j];
        Serial.println("Mutation rejected.");
    } else {
        // Accept
        if (delta > 0) mutation_rate *= 1.01;
    }

    // Occasional Crossover with Best
    if (random(0, 100) < 5) {
        int gene_to_swap = random(NUM_GENES);
        population[i].genes[gene_to_swap] = population[best_idx].genes[gene_to_swap];
    }
  }
  mutation_rate = constrain(mutation_rate, 0.001, 0.2);

  updateVisualization();

  // Wait for a second
  delay(1000);
}

// Define a function to calculate the score for a given function parameter
float calculate_score(float fk, float pk) {
  // Power extracted = deltaT * 1.2
  float p = (temp_in - temp_out) * 1.2;
  float score = 0;
  score += 10.0 * p; // Weight power extraction

  if (fluid_out > MAX_FLUID_TEMP) {
    score -= 1000 * (fluid_out - MAX_FLUID_TEMP);
  }

  // Efficiency penalty based on genes (hypothetical cost)
  score -= 1.0 * (fk * 100.0 + pk * 100.0);

  return score;
}
