#pragma once
#include <cmath>
#include <cstdlib>
#include <ctime>

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_AGENTS = 10000;

// Sensor parameters
const float SENSOR_ANGLE = 0.523599f;  // 30 degrees in radians
const float SENSOR_DISTANCE = 9.0f;    // how far ahead agents sense
const float ROTATION_ANGLE = 0.523599f; // how much agent turns
const float AGENT_SPEED = 1.0f;

struct Agent {
    float x;
    float y;
    float angle;
};

Agent agents[NUM_AGENTS];

// forward declaration - this function lives in renderer.h
float senseTrail(int x, int y);

void initAgents() {
    srand(time(0));
    for (int i = 0; i < NUM_AGENTS; i++) {
        // spawn in a circle in the center
        float r = 100.0f * sqrt((float)rand() / RAND_MAX);
        float a = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
        agents[i].x = WIDTH / 2 + r * cos(a);
        agents[i].y = HEIGHT / 2 + r * sin(a);
        agents[i].angle = a + 3.14159f; // face outward
    }
}

float senseAt(float x, float y, float angle, float sensorAngle) {
    float sx = x + SENSOR_DISTANCE * cos(angle + sensorAngle);
    float sy = y + SENSOR_DISTANCE * sin(angle + sensorAngle);
    return senseTrail((int)sx, (int)sy);
}

void updateAgents() {
    for (int i = 0; i < NUM_AGENTS; i++) {
        float x = agents[i].x;
        float y = agents[i].y;
        float angle = agents[i].angle;

        // sense in three directions
        float sL = senseAt(x, y, angle, -SENSOR_ANGLE);
        float sF = senseAt(x, y, angle,  0.0f);
        float sR = senseAt(x, y, angle,  SENSOR_ANGLE);

        // turn based on strongest signal
        if (sF >= sL && sF >= sR) {
            // keep going forward
        } else if (sL > sR) {
            agents[i].angle -= ROTATION_ANGLE;
        } else if (sR > sL) {
            agents[i].angle += ROTATION_ANGLE;
        } else {
            // equal - random turn
            if (rand() % 2 == 0)
                agents[i].angle += ROTATION_ANGLE;
            else
                agents[i].angle -= ROTATION_ANGLE;
        }

        // move forward
        agents[i].x += AGENT_SPEED * cos(agents[i].angle);
        agents[i].y += AGENT_SPEED * sin(agents[i].angle);

        // wrap around edges
        if (agents[i].x < 0) agents[i].x = WIDTH - 1;
        if (agents[i].x >= WIDTH) agents[i].x = 0;
        if (agents[i].y < 0) agents[i].y = HEIGHT - 1;
        if (agents[i].y >= HEIGHT) agents[i].y = 0;
    }
}