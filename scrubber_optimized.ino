// Define the pins for the sensors and actuators
//It is best to use several scrubbers running in parallel 
//each having own greedy control algorithm, so they exchange hidden layers by lyapunov controllers. 



#define TEMP_IN A0 // Input exhaust temperature sensor
#define TEMP_OUT A1 // Output exhaust temperature sensor
#define TEMP_FLUID A2 // Scrubber fluid output temperature sensor
#define FAN 9 // PWM controlled fan
#define PUMP 10 // PWM controlled pump

// Define the constants for the system
#define MAX_TEMP 100 // Maximum scrubber fluid temperature in degrees Celsius
#define MAX_POWER 200 // Maximum power point for heat extraction in watts
#define Kp 0.1 // Proportional gain for the fan controller
#define Ki 0.01 // Integral gain for the fan controller
#define Kd 0.05 // Derivative gain for the fan controller
#define EEPROM_ADDR 0 // EEPROM address to store the best candidate function parameters

// Define the variables for the system
float temp_in; // Input exhaust temperature in degrees Celsius
float temp_out; // Output exhaust temperature in degrees Celsius
float temp_fluid; // Scrubber fluid output temperature in degrees Celsius
float power; // Power extracted from the exhaust gases in watts
float fan_duty; // Fan duty cycle in percentage
float pump_duty; // Pump duty cycle in percentage
float error; // Error between the desired and actual power
float integral; // Integral of the error
float derivative; // Derivative of the error
float prev_error; // Previous error
float best_error; // Best error achieved so far
float a, b, c; // Parameters of the candidate function for the pump controller
float best_a, best_b, best_c; // Best parameters of the candidate function for the pump controller
float lyapunov; // Lyapunov function value
float best_lyapunov; // Best Lyapunov function value achieved so far

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // Initialize the fan and pump pins as outputs
  pinMode(FAN, OUTPUT);
  pinMode(PUMP, OUTPUT);

  // Initialize the fan and pump duty cycles to zero
  fan_duty = 0;
  pump_duty = 0;

  // Initialize the error, integral and derivative to zero
  error = 0;
  integral = 0;
  derivative = 0;
  prev_error = 0;

  // Initialize the best error and best Lyapunov function value to a large number
  best_error = 1000;
  best_lyapunov = 1000;

  // Initialize the candidate function parameters to random values
  a = random(-10, 10);
  b = random(-10, 10);
  c = random(-10, 10);

  // Read the best candidate function parameters from the EEPROM
  EEPROM.get(EEPROM_ADDR, best_a);
  EEPROM.get(EEPROM_ADDR + sizeof(float), best_b);
  EEPROM.get(EEPROM_ADDR + 2 * sizeof(float), best_c);

  // Print the initial values of the variables
  Serial.print("temp_in: ");
  Serial.println(temp_in);
  Serial.print("temp_out: ");
  Serial.println(temp_out);
  Serial.print("temp_fluid: ");
  Serial.println(temp_fluid);
  Serial.print("power: ");
  Serial.println(power);
  Serial.print("fan_duty: ");
  Serial.println(fan_duty);
  Serial.print("pump_duty: ");
  Serial.println(pump_duty);
  Serial.print("error: ");
  Serial.println(error);
  Serial.print("integral: ");
  Serial.println(integral);
  Serial.print("derivative: ");
  Serial.println(derivative);
  Serial.print("a: ");
  Serial.println(a);
  Serial.print("b: ");
  Serial.println(b);
  Serial.print("c: ");
  Serial.println(c);
  Serial.print("best_a: ");
  Serial.println(best_a);
  Serial.print("best_b: ");
  Serial.println(best_b);
  Serial.print("best_c: ");
  Serial.println(best_c);
  Serial.print("lyapunov: ");
  Serial.println(lyapunov);
  Serial.print("best_lyapunov: ");
  Serial.println(best_lyapunov);
}

void loop() {
  // Read the input exhaust temperature from the sensor
  temp_in = analogRead(TEMP_IN) * 0.48828125; // Convert the analog value to degrees Celsius

  // Read the output exhaust temperature from the sensor
  temp_out = analogRead(TEMP_OUT) * 0.48828125; // Convert the analog value to degrees Celsius

  // Read the scrubber fluid output temperature from the sensor
  temp_fluid = analogRead(TEMP_FLUID) * 0.48828125; // Convert the analog value to degrees Celsius

  // Calculate the power extracted from the exhaust gases
  power = (temp_in - temp_out) * fan_duty / 100; // Assume a linear relationship between power and temperature difference

  // Calculate the error between the desired and actual power
  error = MAX_POWER - power;

  // Calculate the integral of the error
  integral = integral + error;

  // Calculate the derivative of the error
  derivative = error - prev_error;

  // Update the previous error
  prev_error = error;

  // Calculate the fan duty cycle using a PID controller
  fan_duty = Kp * error + Ki * integral + Kd * derivative;

  // Constrain the fan duty cycle between 0 and 100
  fan_duty = constrain(fan_duty, 0, 100);

  // Write the fan duty cycle to the fan pin
  analogWrite(FAN, fan_duty * 255 / 100); // Convert the percentage to a value between 0 and 255

  // Calculate the pump duty cycle using a candidate function
  pump_duty = a * power + b * temp_fluid + c;

  // Constrain the pump duty cycle between 0 and 100
  pump_duty = constrain(pump_duty, 0, 100);

  // Write the pump duty cycle to the pump pin
  analogWrite(PUMP, pump_duty * 255 / 100); // Convert the percentage to a value between 0 and 255

  // Calculate the Lyapunov function value
  lyapunov = pow(error, 2) + pow(temp_fluid - MAX_TEMP, 2);

  // Check if the Lyapunov function value is lower than the best one so far
  if (lyapunov < best_lyapunov) {
    // Update the best Lyapunov function value
    best_lyapunov = lyapunov;

    // Update the best candidate function parameters
    best_a = a;
    best_b = b;
    best_c = c;

    // Write the best candidate function parameters to the EEPROM
    EEPROM.put(EEPROM_ADDR, best_a);
    EEPROM.put(EEPROM_ADDR + sizeof(float), best_b);
    EEPROM.put(EEPROM_ADDR + 2 * sizeof(float), best_c);
  }

  // Print the current values of the variables
  Serial.print("temp_in: ");
  Serial.println(temp_in);
  Serial.print("temp_out: ");
  Serial.println(temp_out);
  Serial.print("temp_fluid: ");
  Serial.println(temp_fluid);
  Serial.print("power: ");
  Serial.println(power);
  Serial.print("fan_duty: ");
  Serial.println(fan_duty);
  Serial.print("pump_duty: ");
  Serial.println(pump_duty);
  Serial.print("error: ");
  Serial.println(error);
  Serial.print("integral: ");
  Serial.println(integral);
  Serial.print("derivative: ");
  Serial.println(derivative);
  Serial.print("a: ");
  Serial.println(a);
  Serial.print("b: ");
  Serial.println(b);
  Serial.print("c: ");
  Serial.println(c);
  Serial.print("best_a: ");
  Serial.println(best_a);
  Serial.print("best_b: ");
  Serial.println(best_b);
  Serial.print("best_c: ");
  Serial.println(best_c);
  Serial.print("lyapunov: ");
  Serial.println(lyapunov);
  Serial.print("best_lyapunov: ");
  Serial.println(best_lyapunov);
}
