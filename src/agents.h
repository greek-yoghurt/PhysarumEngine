#pragma once
#include <cmath>
#include <cstdlib>
#include <ctime>

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;
const int NUM_AGENTS = 10000;

struct Agent {
    float x;
    float y;
    float angle;
};

Agent agents[NUM_AGENTS];

void initAgents() {
    srand(time(0));
    for (int i = 0; i < NUM_AGENTS; i++) {
        agents[i].x = (float)(rand() % WIDTH);
        agents[i].y = (float)(rand() % HEIGHT);
        agents[i].angle = (float)(rand() % 360) * (3.14159f / 180.0f);
    }
}

void updateAgents() {
    float speed = 1.0f;
    for (int i = 0; i < NUM_AGENTS; i++) {
        // move forward in current direction
        agents[i].x += speed * cos(agents[i].angle);
        agents[i].y += speed * sin(agents[i].angle);

        // wrap around edges
        if (agents[i].x < 0) agents[i].x = WIDTH - 1;
        if (agents[i].x >= WIDTH) agents[i].x = 0;
        if (agents[i].y < 0) agents[i].y = HEIGHT - 1;
        if (agents[i].y >= HEIGHT) agents[i].y = 0;
    }
}