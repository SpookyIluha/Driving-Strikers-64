#pragma once
#ifndef PARTICLES_SIM
#define PARTICLES_SIM

// from example 19_particles_tex in Tiny3D

extern int currentPart;

color_t blend_colors(color_t colorA, color_t colorB, float t);



color_t get_rand_color(int bright);

// Fire color: white -> yellow/orange -> red -> black
void gradient_fire(uint8_t *color, float t);

float fract(float a);

float randNoise3d_rand(float coX, float coY);

T3DVec3 randNoise3d(float uvX, float uvY);

int noise_2d(int x, int y);

void generate_particles_random(TPXParticle *particles, uint32_t count);

void simulate_particles_embers(TPXParticle *particles, uint32_t partCount);

void simulate_particles_rain(TPXParticle *particles, uint32_t partCount);

void simulate_particles_snow(TPXParticle *particles, uint32_t partCount);

/**
 * Particle system for a fire effect.
 * This will simulate particles over time by moving them up and changing their color.
 * The current position is used to spawn new particles, so it can move over time leaving a trail behind.
 */
void simulate_particles_fire(TPXParticle *particles, uint32_t partCount, float posX, float posZ);

#endif