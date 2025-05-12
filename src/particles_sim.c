#include <libdragon.h>
#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
#include <t3d/tpx.h>

// from example 19_particles_tex in Tiny3D

int currentPart  = 0;

color_t blend_colors(color_t colorA, color_t colorB, float t) {
  color_t color;
  color.r = (uint8_t)(colorA.r * (1.0f - t) + colorB.r * t);
  color.g = (uint8_t)(colorA.g * (1.0f - t) + colorB.g * t);
  color.b = (uint8_t)(colorA.b * (1.0f - t) + colorB.b * t);
  color.a = (uint8_t)(colorA.a * (1.0f - t) + colorB.a * t);
  return color;
}



color_t get_rand_color(int bright) {
  return (color_t){
    (uint8_t)(bright + (rand() % (256 - bright))),
    (uint8_t)(bright + (rand() % (256 - bright))),
    (uint8_t)(bright + (rand() % (256 - bright))),
    0xFF
  };
}

// Fire color: white -> yellow/orange -> red -> black
void gradient_fire(uint8_t *color, float t) {
    t = fminf(1.0f, fmaxf(0.0f, t));
    t = 0.8f - t;
    t *= t;

    if (t < 0.25f) { // Dark red to bright red
      color[0] = (uint8_t)(200 * (t / 0.25f)) + 55;
      color[1] = 0;
      color[2] = 0;
    } else if (t < 0.5f) { // Bright red to yellow
      color[0] = 255;
      color[1] = (uint8_t)(255 * ((t - 0.25f) / 0.25f));
      color[2] = 0;
    } else if (t < 0.75f) { // Yellow to white (optional, if you want a bright white center)
      color[0] = 255;
      color[1] = 255;
      color[2] = (uint8_t)(255 * ((t - 0.5f) / 0.25f));
    } else { // White to black
      color[0] = (uint8_t)(255 * (1.0f - (t - 0.75f) / 0.25f));
      color[1] = (uint8_t)(255 * (1.0f - (t - 0.75f) / 0.25f));
      color[2] = (uint8_t)(255 * (1.0f - (t - 0.75f) / 0.25f));
    }
}

float fract(float a) {
  return a - floorf(a);
}

float randNoise3d_rand(float coX, float coY){
  coX = fabsf(coX) * 15.1335f;
  coY = fabsf(coY) * 61.15654f;
  coX += coY;
  return fract(fm_sinf(coX) * 65979.1347f);
}

T3DVec3 randNoise3d(float uvX, float uvY) {
  return (T3DVec3){{
    randNoise3d_rand(uvX + 0.23f * 382.567f, uvX + 0.23f * 382.567f),
    randNoise3d_rand(uvY + 0.65f * 330.356f, uvX + 0.65f * 330.356f),
    randNoise3d_rand(uvX + 0.33f * 356.346f, uvY + 0.33f * 356.346f)
  }};
}

int noise_2d(int x, int y) {
  int n = x + y * 57;
  n = (n << 13) ^ n;
  return (n * (n * n * 60493 + 19990303) + 89);
}

void generate_particles_random(TPXParticle *particles, uint32_t count) {
  for (int i = 0; i < count; i++) {
    int p = i / 2;
    int8_t *ptPos = i % 2 == 0 ? particles[p].posA : particles[p].posB;
    uint8_t *ptColor = i % 2 == 0 ? particles[p].colorA : particles[p].colorB;

    particles[p].sizeA = 20 + (rand() % 10);
    particles[p].sizeB = 20 + (rand() % 10);

    T3DVec3 pos = {{
       (i * 1 + rand()) % 128 - 64,
       (i * 3 + rand()) % 128 - 64,
       (i * 4 + rand()) % 128 - 64
     }};

    t3d_vec3_norm(&pos);
    float len = rand() % 40;
    pos.v[0] *= len;
    pos.v[1] *= len;
    pos.v[2] *= len;

    ptPos[0] = (rand() % 256) - 128;
    ptPos[1] = (rand() % 256) - 128;
    ptPos[2] = (rand() % 256) - 128;

    ptColor[0] = 25 + (rand() % 230);
    ptColor[1] = 25 + (rand() % 230);
    ptColor[2] = 25 + (rand() % 230);
    ptColor[3] = 0; // alpha is the texture offset, as with the global one in 1/4h of a pixel steps
  }
}

void simulate_particles_rain(TPXParticle *particles, uint32_t partCount) {
  // move all up by one unit
  for (int i = 0; i < partCount/2; i++) {
    particles[i].posA[1] -= 14;
    particles[i].posB[1] -= 14;
  }
}

void simulate_particles_embers(TPXParticle *particles, uint32_t partCount) {
  // move all up by one unit
  for (int i = 0; i < partCount/2; i++) {
    int rnd = rand() % 7; particles[i].posA[0] += (rnd - 3) / 3;
    rnd = rand() % 7; particles[i].posA[2] += (rnd - 3) / 3;
    particles[i].posA[1] += 1;
    particles[i].posB[1] += 1;
  }
}

void simulate_particles_snow(TPXParticle *particles, uint32_t partCount) {
  // move all up by one unit
  for (int i = 0; i < partCount/2; i++) {
    int rnd = rand() % 7; particles[i].posA[0] += (rnd - 3) / 3;
    rnd = rand() % 7; particles[i].posA[2] += (rnd - 3) / 3;
    particles[i].posA[1] -= 1;
    particles[i].posB[1] -= 1;
  }
}

/**
 * Particle system for a fire effect.
 * This will simulate particles over time by moving them up and changing their color.
 * The current position is used to spawn new particles, so it can move over time leaving a trail behind.
 */
void simulate_particles_fire(TPXParticle *particles, uint32_t partCount, float posX, float posZ) {
  uint32_t p = currentPart / 2;
  if(currentPart % (1+(rand() % 3)) == 0) {
    int8_t *ptPos = currentPart % 2 == 0 ? particles[p].posA : particles[p].posB;
    int8_t *size = currentPart % 2 == 0 ? &particles[p].sizeA : &particles[p].sizeB;
    uint8_t *color = currentPart % 2 == 0 ? particles[p].colorA : particles[p].colorB;

    ptPos[0] = posX + (rand() % 16) - 8;
    ptPos[1] = -126;
    gradient_fire(color, 0);
    color[3] = (PhysicalAddr(ptPos) % 8) * 32;

    ptPos[2] = posZ + (rand() % 16) - 8;
    *size = 60 + (rand() % 10);
  }
  currentPart = (currentPart + 1) % partCount;

  // move all up by one unit
  for (int i = 0; i < partCount/2; i++) {
    gradient_fire(particles[i].colorA, (particles[i].posA[1] + 127) / 150.0f);
    gradient_fire(particles[i].colorB, (particles[i].posB[1] + 127) / 150.0f);

    particles[i].posA[1] += 1;
    particles[i].posB[1] += 1;
    if(currentPart % 4 == 0) {
      particles[i].sizeA -= 2;
      particles[i].sizeB -= 2;
      if(particles[i].sizeA < 0)particles[i].sizeA = 0;
      if(particles[i].sizeB < 0)particles[i].sizeB = 0;
    }
  }
}
