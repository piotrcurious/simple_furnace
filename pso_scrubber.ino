#include <Arduino.h>
#include <EEPROM.h>

#define NUM_PARTICLES 5
#define NUM_PARAMS 2 // fan_k and pump_k

struct Particle {
    float pos[NUM_PARAMS];
    float vel[NUM_PARAMS];
    float best_pos[NUM_PARAMS];
    float best_score;
};

Particle swarm[NUM_PARTICLES];
float global_best_pos[NUM_PARAMS];
float global_best_score = -1000000;

float current_score = 0;
float fan_duty = 0, pump_duty = 0, power = 0;

float calculate_fitness(float fk, float pk) {
    float in = analogRead(A0) * 0.48828125;
    float out = analogRead(A1) * 0.48828125;
    float p = (in - out) * 1.2;
    float f_out = analogRead(A3) * 0.48828125;

    float score = p * 10.0;
    if (f_out > 80) score -= (f_out - 80) * 100.0;
    score -= (fk + pk) * 0.5; // Efficiency
    return score;
}

void setup() {
    Serial.begin(9600);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        for (int j = 0; j < NUM_PARAMS; j++) {
            swarm[i].pos[j] = random(0, 100) / 100.0;
            swarm[i].vel[j] = 0;
            swarm[i].best_pos[j] = swarm[i].pos[j];
        }
        swarm[i].best_score = -1000000;
    }
}

void loop() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        float score = calculate_fitness(swarm[i].pos[0], swarm[i].pos[1]);

        if (score > swarm[i].best_score) {
            swarm[i].best_score = score;
            for (int j = 0; j < NUM_PARAMS; j++) swarm[i].best_pos[j] = swarm[i].pos[j];
        }

        if (score > global_best_score) {
            global_best_score = score;
            for (int j = 0; j < NUM_PARAMS; j++) global_best_pos[j] = swarm[i].pos[j];
        }

        // Update velocity and position
        for (int j = 0; j < NUM_PARAMS; j++) {
            swarm[i].vel[j] = 0.5 * swarm[i].vel[j] +
                             2.0 * (random(0, 100)/100.0) * (swarm[i].best_pos[j] - swarm[i].pos[j]) +
                             2.0 * (random(0, 100)/100.0) * (global_best_pos[j] - swarm[i].pos[j]);
            swarm[i].pos[j] = constrain(swarm[i].pos[j] + swarm[i].vel[j], 0, 1);
        }
    }

    // Apply best
    fan_duty = global_best_pos[0] * 100.0;
    pump_duty = global_best_pos[1] * 100.0;
    analogWrite(9, fan_duty * 2.55);
    analogWrite(10, pump_duty * 2.55);

    float in_t = analogRead(A0) * 0.48828125;
    float out_t = analogRead(A1) * 0.48828125;
    power = (in_t - out_t) * 1.2;
    Serial.print("PSO Power: "); Serial.println(power);
    delay(500);
}
