// Define the pins for the sensors and actuators
#define EXHAUST_IN_PIN A0 // Input exhaust temperature sensor
#define EXHAUST_OUT_PIN A1 // Output exhaust temperature sensor
#define FLUID_IN_PIN A2 // Scrubber fluid input temperature sensor
#define FLUID_OUT_PIN A3 // Scrubber fluid output temperature sensor
#define FAN_PIN 9 // PWM controlled fan
#define PUMP_PIN 10 // PWM controlled pump

// Define the constants for the scrubber system
#define MAX_FLUID_TEMP 80 // Maximum scrubber fluid temperature in Celsius
#define FAN_MIN_SPEED 50 // Minimum fan speed in percentage
#define FAN_MAX_SPEED 100 // Maximum fan speed in percentage
#define PUMP_MIN_SPEED 50 // Minimum pump speed in percentage
#define PUMP_MAX_SPEED 100 // Maximum pump speed in percentage

// Define the variables for the scrubber system
float exhaust_in_temp; // Input exhaust temperature in Celsius
float exhaust_out_temp; // Output exhaust temperature in Celsius
float fluid_in_temp; // Scrubber fluid input temperature in Celsius
float fluid_out_temp; // Scrubber fluid output temperature in Celsius
int fan_speed; // Fan speed in percentage
int pump_speed; // Pump speed in percentage
float heat_extraction; // Heat extraction rate in Watts
float lyapunov; // Lyapunov function value
float epsilon; // Lyapunov stability region threshold
int permute[4]; // Permutation array for greedy algorithm

// Initialize the scrubber system
void setup() {
  // Set the pin modes
  pinMode(EXHAUST_IN_PIN, INPUT);
  pinMode(EXHAUST_OUT_PIN, INPUT);
  pinMode(FLUID_IN_PIN, INPUT);
  pinMode(FLUID_OUT_PIN, INPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  // Set the initial fan and pump speeds
  fan_speed = FAN_MIN_SPEED;
  pump_speed = PUMP_MIN_SPEED;

  // Set the initial epsilon value
  epsilon = 0.01;

  // Set the initial permutation array
  permute[0] = 0;
  permute[1] = 1;
  permute[2] = 2;
  permute[3] = 3;
}

// Update the scrubber system
void loop() {
  // Read the sensor values
  exhaust_in_temp = analogRead(EXHAUST_IN_PIN) * 0.48828125; // Convert from 0-1023 to 0-500
  exhaust_out_temp = analogRead(EXHAUST_OUT_PIN) * 0.48828125; // Convert from 0-1023 to 0-500
  fluid_in_temp = analogRead(FLUID_IN_PIN) * 0.48828125; // Convert from 0-1023 to 0-500
  fluid_out_temp = analogRead(FLUID_OUT_PIN) * 0.48828125; // Convert from 0-1023 to 0-500

  // Calculate the heat extraction rate
  heat_extraction = (exhaust_in_temp - exhaust_out_temp) * 1.2 * 0.001; // Q = m * c * delta T, assume mass flow rate of 1.2 kg/s and specific heat capacity of 1 kJ/kgK

  // Calculate the Lyapunov function value
  lyapunov = heat_extraction - fluid_out_temp; // L = Q - T_out

  // Apply the permuted greedy algorithm to adjust the fan and pump speeds
  for (int i = 0; i < 4; i++) {
    switch (permute[i]) {
      case 0: // Increase fan speed
        if (lyapunov < -epsilon && fan_speed < FAN_MAX_SPEED) {
          fan_speed++;
        }
        break;
      case 1: // Decrease fan speed
        if (lyapunov > epsilon && fan_speed > FAN_MIN_SPEED) {
          fan_speed--;
        }
        break;
      case 2: // Increase pump speed
        if (lyapunov < -epsilon && pump_speed < PUMP_MAX_SPEED && fluid_out_temp < MAX_FLUID_TEMP) {
          pump_speed++;
        }
        break;
      case 3: // Decrease pump speed
        if (lyapunov > epsilon && pump_speed > PUMP_MIN_SPEED) {
          pump_speed--;
        }
        break;
    }
  }

  // Randomize the permutation array
  for (int i = 0; i < 4; i++) {
    int j = random(0, 4); // Pick a random index
    int temp = permute[i]; // Swap the elements
    permute[i] = permute[j];
    permute[j] = temp;
  }

  // Write the fan and pump speeds to the PWM pins
  analogWrite(FAN_PIN, map(fan_speed, 0, 100, 0, 255)); // Convert from 0-100 to 0-255
  analogWrite(PUMP_PIN, map(pump_speed, 0, 100, 0, 255)); // Convert from 0-100 to 0-255

  // Print the scrubber system status to the serial monitor
  Serial.print("Exhaust in temp: ");
  Serial.print(exhaust_in_temp);
  Serial.println(" C");
  Serial.print("Exhaust out temp: ");
  Serial.print(exhaust_out_temp);
  Serial.println(" C");
  Serial.print("Fluid in temp: ");
  Serial.print(fluid_in_temp);
  Serial.println(" C");
  Serial.print("Fluid out temp: ");
  Serial.print(fluid_out_temp);
  Serial.println(" C");
  Serial.print("Fan speed: ");
  Serial.print(fan_speed);
  Serial.println(" %");
  Serial.print("Pump speed: ");
  Serial.print(pump_speed);
  Serial.println(" %");
  Serial.print("Heat extraction: ");
  Serial.print(heat_extraction);
  Serial.println(" W");
  Serial.print("Lyapunov: ");
  Serial.println(lyapunov);
  Serial.println();

  // Wait for 1 second before the next iteration
  delay(1000);
}
