// Define the pins for the fan and pump
#define FAN_PIN 9
#define PUMP_PIN 10

// Define the analog inputs for the temperatures
#define EXHAUST_IN A0
#define EXHAUST_OUT A1
#define FLUID_IN A2
#define FLUID_OUT A3

// Define the maximum fluid temperature
#define MAX_FLUID_TEMP 80

// Define the array of candidate function parameters
// Each element is a pair of fan and pump duty cycles
// The array is indexed by the input fluid and exhaust temperatures
// The values are chosen arbitrarily for demonstration purposes
int candidates[16][16][2] = {
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 100}, {90, 90}, {80, 80}, {70, 70}, {60, 60}, {50, 50}, {40, 40}, {30, 30}, {20, 20}, {10, 10}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{100, 90}, {90, 80}, {80, 70}, {70, 60}, {60, 50}, {50, 40}, {40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{90, 80}, {80, 70}, {70, 60}, {60, 50}, {50, 40}, {40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{80, 70}, {70, 60}, {60, 50}, {50, 40}, {40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{70, 60}, {60, 50}, {50, 40}, {40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{60, 50}, {50, 40}, {40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{50, 40}, {40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{40, 30}, {30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{30, 20}, {20, 10}, {10, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}}
};

// Define the variables for the current and best parameters
int fan_duty = 0; // The current fan duty cycle
int pump_duty = 0; // The current pump duty cycle
int best_fan_duty = 0; // The best fan duty cycle
int best_pump_duty = 0; // The best pump duty cycle

// Define the variables for the current and best temperatures
float exhaust_in_temp = 0; // The current input exhaust temperature
float exhaust_out_temp = 0; // The current output exhaust temperature
float fluid_in_temp = 0; // The current input fluid temperature
float fluid_out_temp = 0; // The current output fluid temperature
float best_exhaust_in_temp = 0; // The best input exhaust temperature
float best_exhaust_out_temp = 0; // The best output exhaust temperature
float best_fluid_in_temp = 0; // The best input fluid temperature
float best_fluid_out_temp = 0; // The best output fluid temperature

// Define the variables for the current and best indexes
int exhaust_in_index = 0; // The current index for the input exhaust temperature
int fluid_in_index = 0; // The current index for the input fluid temperature
int best_exhaust_in_index = 0; // The best index for the input exhaust temperature
int best_fluid_in_index = 0; // The best index for the input fluid temperature

// Define the variables for the current and best scores
float score = 0; // The current score
float best_score = 0; // The best score

// Define the function to calculate the score
// The score is based on the heat extraction and the fluid temperature limit
float calculate_score(float exhaust_in, float exhaust_out, float fluid_in, float fluid_out) {
  // Calculate the heat extraction
  float heat_extraction = (exhaust_in - exhaust_out) * (fluid_out - fluid_in);
  // Calculate the penalty for exceeding the fluid temperature limit
  float penalty = 0;
  if (fluid_out > MAX_FLUID_TEMP) {
    penalty = (fluid_out - MAX_FLUID_TEMP) * 100;
  }
  // Return the score as the difference between the heat extraction and the penalty
  return heat_extraction - penalty;
}

// Define the function to map the temperature to the array index
// The index is between 0 and 15, corresponding to the 16x16 array
int map_temp_to_index(float temp) {
  // Scale the temperature to the range of 0 to 255
  int scaled_temp = map(temp, 0, 100, 0, 255);
  // Divide the scaled temperature by 16 to get the index
  int index = scaled_temp / 16;
  // Return the index
  return index;
}

// Define the function to update the fan and pump speeds
void update_speeds(int fan, int pump) {
  // Set the fan and pump duty cycles
  analogWrite(FAN_PIN, fan);
  analogWrite(PUMP_PIN, pump);
  // Update the current fan and pump duty cycles
  fan_duty = fan;
  pump_duty = pump;
}

// Define the function to read the temperatures
void read_temps() {
  // Read the analog inputs and convert them to degrees Celsius
  exhaust_in_temp = analogRead(EXHAUST_IN) * 0.48828125;
  exhaust_out_temp = analogRead(EXHAUST_OUT) * 0.48828125;
  fluid_in_temp = analogRead(FLUID_IN) * 0.48828125;
  fluid_out_temp = analogRead(FLUID_OUT) * 0.48828125;
  // Map the input temperatures to the array indexes
  exhaust_in_index = map_temp_to_index(exhaust_in_temp);
  fluid_in_index = map_temp_to_index(fluid_in_temp);
}

// Define the function to optimize the scrubber performance
void optimize() {
  // Read the current temperatures
  read_temps();
  // Calculate the current score
  score = calculate_score(exhaust_in_temp, exhaust_out_temp, fluid_in_temp, fluid_out_temp);
  // Check if the current score is better than the best score
  if (score > best_score) {
    // Update the best score and parameters
    best_score = score;
    best_fan_duty = fan_duty;
    best_pump_duty = pump_duty;
    best_exhaust_in_temp = exhaust_in_temp;
    best_exhaust_out_temp = exhaust_out_temp;
    best_fluid_in_temp = fluid_in_temp;
    best_fluid_out_temp = fluid_out_temp;
    best_exhaust_in_index = exhaust_in_index;
    best_fluid_in_index = fluid_in_index;
  }
  // Iterate over the array of candidate function parameters
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++) {
      // Get the fan and pump duty cycles from the array
      int fan = candidates[i][j][0];
      int pump = candidates[i][j][1];
      // Update the fan and pump speeds
      update_speeds(fan, pump);
      // Read the new temperatures
      read_temps();
      // Calculate the new score
      score = calculate_score(exhaust_in_temp, exhaust_out_temp, fluid_in_temp, fluid_out_temp);
      // Check if the new score is better than the best score
      if (score > best_score) {
        // Update the best score and parameters
        best_score = score;
        best_fan_duty = fan_duty;
        best_pump_duty = pump_duty;
        best_exhaust_in_temp = exhaust_in_temp;
        best_exhaust_out_temp = exhaust_out_temp;
        best_fluid_in_temp = fluid_in_temp;
        best_fluid_out_temp = fluid_out_temp;
        best_exhaust_in_index = exhaust_in_index;
        best_fluid_in_index = fluid_in_index;
      }
    }
  }
  // Set the fan and pump speeds to the best values
  update_speeds(best_fan_duty, best_pump_duty);
}

// Define the setup function
void setup() {
  // Set the fan and pump pins as outputs
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  // Set the initial fan and pump speeds to zero
  update_speeds(0, 0);
  // Set the initial best score to zero
  best_score = 0;
}

// Define the loop function
void loop() {
  // Optimize the scrubber performance
  optimize();
  // Wait for one second
  delay(1000);
}
